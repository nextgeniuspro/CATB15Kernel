/* drivers/hwmon/mt6516/amit/APDS9930.c - APDS9930 ALS/PS driver
 * 
 * Author: MingHsien Hsieh <minghsien.hsieh@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>

#if 0
#include <mach/mt6577_devs.h>
#include <mach/mt6577_typedefs.h>
#include <mach/mt6577_gpio.h>
#include <mach/mt6577_pm_ldo.h>
#endif

#if 1
//#include <mach/mt_devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#endif

#define POWER_NONE_MACRO MT65XX_POWER_NONE

#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_alsps.h>
#include "APDS9930.h"
/******************************************************************************
 * configuration
*******************************************************************************/
/*----------------------------------------------------------------------------*/

#define APDS9930_DEV_NAME     "APDS9930"
/*----------------------------------------------------------------------------*/
#define APS_TAG                  "[ALS/PS] "
#define APS_FUN(f)               printk(KERN_INFO APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)    printk(KERN_ERR  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    printk(KERN_ERR APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    printk(KERN_INFO APS_TAG fmt, ##args) 

#define I2C_FLAG_WRITE	0
#define I2C_FLAG_READ	1


/******************************************************************************
 * extern functions
*******************************************************************************/
	extern void mt65xx_eint_unmask(unsigned int line);
	extern void mt65xx_eint_mask(unsigned int line);
	extern void mt65xx_eint_set_polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
	extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
	extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
	extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
										 kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
										 kal_bool auto_umask);
	long APDS9930_read_ps(struct i2c_client *client, u16 *data);
	
/*----------------------------------------------------------------------------*/
static struct i2c_client *APDS9930_i2c_client = NULL;
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id APDS9930_i2c_id[] = {{APDS9930_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_APDS9930={ I2C_BOARD_INFO("APDS9930", 0x39)};
/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int APDS9930_i2c_remove(struct i2c_client *client);
static int APDS9930_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_suspend(struct i2c_client *client, pm_message_t msg);
static int APDS9930_i2c_resume(struct i2c_client *client);

static DEFINE_MUTEX(APDS9930_mutex);


static struct APDS9930_priv *g_APDS9930_ptr = NULL;

 struct PS_CALI_DATA_STRUCT
{
    int close;
    int far_away;
    int valid;
} ;

static struct PS_CALI_DATA_STRUCT ps_cali={0,0,0};
static int intr_flag_value = 0;
int g_pcount = 0;//Colby add 20121219
int g_poffset = 0;//Colby add 20121219
int g_ps_close = 0;//Colby add 20121225
int g_ps_far = 0;//Colby add 20121225
int g_result_ct = 0; //Colby add for nvdata 20130114
int g_result_ps_cover = 0; //Colby add for nvdata 20130114
int g_result_als_value = 0; //Colby add for nvdata 20130114
int CCI_debug_for_APDS9930 = 0; //Colby add debug flag for showing log 20130121
int CCI_cal = 0; //Colby add for PL sensor calibration 20130123
/*----------------------------------------------------------------------------*/
typedef enum {
    CMC_BIT_ALS    = 1,
    CMC_BIT_PS     = 2,
} CMC_BIT;
/*----------------------------------------------------------------------------*/
struct APDS9930_i2c_addr {    /*define a series of i2c slave address*/
    u8  write_addr;  
    u8  ps_thd;     /*PS INT threshold*/
};
/*----------------------------------------------------------------------------*/
struct APDS9930_priv {
    struct alsps_hw  *hw;
    struct i2c_client *client;
    struct work_struct  eint_work;

    /*i2c address group*/
    struct APDS9930_i2c_addr  addr;
    
    /*misc*/
    u16		    als_modulus;
    atomic_t    i2c_retry;
    atomic_t    als_suspend;
    atomic_t    als_debounce;   /*debounce time after enabling als*/
    atomic_t    als_deb_on;     /*indicates if the debounce is on*/
    atomic_t    als_deb_end;    /*the jiffies representing the end of debounce*/
    atomic_t    ps_mask;        /*mask ps: always return far away*/
    atomic_t    ps_debounce;    /*debounce time after enabling ps*/
    atomic_t    ps_deb_on;      /*indicates if the debounce is on*/
    atomic_t    ps_deb_end;     /*the jiffies representing the end of debounce*/
    atomic_t    ps_suspend;


    /*data*/
    u16         als;
    u16          ps;
    u8          _align;
    u16         als_level_num;
    u16         als_value_num;
    u32         als_level[C_CUST_ALS_LEVEL-1];
    u32         als_value[C_CUST_ALS_LEVEL];

    atomic_t    als_cmd_val;    /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_cmd_val;     /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_thd_val_high;     /*the cmd value can't be read, stored in ram*/
	atomic_t    ps_thd_val_low;     /*the cmd value can't be read, stored in ram*/
    ulong       enable;         /*enable mask*/
    ulong       pending_intr;   /*pending interrupt*/

    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif     
};
/*----------------------------------------------------------------------------*/
static struct i2c_driver APDS9930_i2c_driver = {	
	.probe      = APDS9930_i2c_probe,
	.remove     = APDS9930_i2c_remove,
	.detect     = APDS9930_i2c_detect,
	.suspend    = APDS9930_i2c_suspend,
	.resume     = APDS9930_i2c_resume,
	.id_table   = APDS9930_i2c_id,
	.driver = {
		.name           = APDS9930_DEV_NAME,
	},
};

static struct APDS9930_priv *APDS9930_obj = NULL;
static struct platform_driver APDS9930_alsps_driver;
struct apds9930_threshold {
	unsigned  int result_ct;//Colby add for nvram data 20130114
	unsigned  int result_ps_cover;//Colby add for nvram data 20130114
	unsigned  int result_als_value;//Colby add for nvram data 20130114
	unsigned  int pules;
	unsigned int  offset; //Colby add 20121218
    unsigned  int ps_high_thd;
    unsigned  int ps_low_thd;
};
/*------------------------i2c function for 89-------------------------------------*/
int APDS9930_i2c_master_operate(struct i2c_client *client, const char *buf, int count, int i2c_flag)
{
	int res = 0;
	mutex_lock(&APDS9930_mutex);
	switch(i2c_flag){	
	case I2C_FLAG_WRITE:
	client->addr &=I2C_MASK_FLAG;
	res = i2c_master_send(client, buf, count);
	client->addr &=I2C_MASK_FLAG;
	break;
	
	case I2C_FLAG_READ:
	client->addr &=I2C_MASK_FLAG;
	client->addr |=I2C_WR_FLAG;
	client->addr |=I2C_RS_FLAG;
	res = i2c_master_send(client, buf, count);
	client->addr &=I2C_MASK_FLAG;
	break;
	default:
	APS_LOG("APDS9930_i2c_master_operate i2c_flag command not support!\n");
	break;
	}
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	mutex_unlock(&APDS9930_mutex);
	return res;
	EXIT_ERR:
	mutex_unlock(&APDS9930_mutex);
	APS_ERR("APDS9930_i2c_transfer fail\n");
	return res;
}

/*----------------------------------------------------------------------------*/
int APDS9930_get_addr(struct alsps_hw *hw, struct APDS9930_i2c_addr *addr)
{
	if(!hw || !addr)
	{
		return -EFAULT;
	}
	addr->write_addr= hw->i2c_addr[0];
	return 0;
}

//Colby add attr start 20121218

static int APDS9930_set_ppoffset(struct i2c_client *client,unsigned int offset)
{
	APS_ERR("[Colby]APDS9930_set_ppoffset enter\n");
	//printk("set poffset function");
	long res =0;
	u8 databuf[2];
	databuf[0]= APDS9930_CMM_POFFSET;
	databuf[1] = offset;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	return 0;
EXIT_ERR:
	APS_ERR("APDS9930_set_Poffset fail\n");
	return res;

}

//Colby add attr end 20121218

static int APDS9930_set_ppcount(struct i2c_client *client,unsigned int pules)
{
	//printk("set pcount function");
	long res =0;
	u8 databuf[2];	
	databuf[0]=APDS9930_CMM_PPCOUNT;    
	databuf[1] = pules;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	return 0;
EXIT_ERR:
	APS_ERR("APDS9930_set_Pcount fail\n");
	return res;
	
}

/*----------------------------------------------------------------------------*/
static void APDS9930_power(struct alsps_hw *hw, unsigned int on) 
{
	static unsigned int power_on = 0;

	//APS_LOG("power %s\n", on ? "on" : "off");

	if(hw->power_id != POWER_NONE_MACRO)
	{
		if(power_on == on)
		{
			APS_LOG("ignore power control: %d\n", on);
		}
		else if(on)
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "APDS9930")) 
			{
				APS_ERR("power on fails!!\n");
			}
		}
		else
		{
			if(!hwPowerDown(hw->power_id, "APDS9930")) 
			{
				APS_ERR("power off fail!!\n");   
			}
		}
	}
	power_on = on;
}
/*----------------------------------------------------------------------------*/
static long APDS9930_enable_als(struct i2c_client *client, int enable)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];	  
	long res = 0;

	databuf[0]= APDS9930_CMM_ENABLE;
	res = APDS9930_i2c_master_operate(client, databuf, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

//Colby add debug flag for showing log start 20130121
	if(CCI_debug_for_APDS9930)
		APS_LOG("[Colby]APDS9930_CMM_ENABLE als value = %x\n",databuf[0]);
	//APS_LOG("APDS9930_CMM_ENABLE als value = %x\n",databuf[0]);
//Colby add debug flag for showing log end 20130121
	
	if(enable)
		{
			databuf[1] = databuf[0]|0x03;
			databuf[0] = APDS9930_CMM_ENABLE;
			//APS_LOG("APDS9930_CMM_ENABLE enable als value = %x\n",databuf[1]);
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			
		}
	else {
		if(test_bit(CMC_BIT_PS, &obj->enable))
			databuf[1] = databuf[0]&0xFD;
		else
			databuf[1] = databuf[0]&0xF8;
		
			databuf[0] = APDS9930_CMM_ENABLE;
			//APS_LOG("APDS9930_CMM_ENABLE disable als value = %x\n",databuf[1]);
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			clear_bit(CMC_BIT_ALS, &obj->enable);
		}
	return 0;
		
EXIT_ERR:
	APS_ERR("APDS9930_enable_als fail\n");
	return res;
}

/*----------------------------------------------------------------------------*/
static long APDS9930_enable_ps(struct i2c_client *client, int enable)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	long res = 0;
	
	databuf[0]= APDS9930_CMM_ENABLE;
	res = APDS9930_i2c_master_operate(client, databuf, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

//Colby add debug flag for showing log start 20130121
	if(CCI_debug_for_APDS9930)
		APS_LOG("[Colby]APDS9930_CMM_ENABLE ps value = %x\n",databuf[0]);
	//APS_LOG("APDS9930_CMM_ENABLE ps value = %x\n",databuf[0]);
//Colby add debug flag for showing log end 20130121
	
	if(enable)
		{
			databuf[1] = databuf[0]|0x05;
			databuf[0] = APDS9930_CMM_ENABLE;
			//APS_LOG("APDS9930_CMM_ENABLE enable ps value = %x\n",databuf[1]);	
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}
	else{
		if(test_bit(CMC_BIT_ALS, &obj->enable))
			databuf[1] = databuf[0]&0xFB;
		else
			databuf[1] = databuf[0]&0xF8;
		
			databuf[0] = APDS9930_CMM_ENABLE;
			//APS_LOG("APDS9930_CMM_ENABLE disable ps value = %x\n",databuf[1]);	
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}
	return 0;
	
EXIT_ERR:
	APS_ERR("APDS9930_enable_ps fail\n");
	return res;
}
/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
static int APDS9930_check_and_clear_intr(struct i2c_client *client) 
{
	int res,intp,intl;
	u8 buffer[2];

	if (mt_get_gpio_in(GPIO_ALS_EINT_PIN) == 1) /*skip if no interrupt*/  
	    return 0;

	buffer[0] = APDS9930_CMM_STATUS;
	res = APDS9930_i2c_master_operate(client, buffer, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	res = 0;
	intp = 0;
	intl = 0;
	if(0 != (buffer[0] & 0x20))
	{
		res = 1;
		intp = 1;
	}
	if(0 != (buffer[0] & 0x10))
	{
		res = 1;
		intl = 1;		
	}

	if(1 == res)
	{
		if((1 == intp) && (0 == intl))
		{
			buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x05);
		}
		else if((0 == intp) && (1 == intl))
		{
			buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x06);
		}
		else
		{
			buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x07);
		}

		res = APDS9930_i2c_master_operate(client, buffer, 0x1, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		else
		{
			res = 0;
		}
	}

	return res;

EXIT_ERR:
	APS_ERR("APDS9930_check_and_clear_intr fail\n");
	return 1;
}
/*----------------------------------------------------------------------------*/

/*yucong add for interrupt mode support MTK inc 2012.3.7*/
static int APDS9930_check_intr(struct i2c_client *client) 
{
	int res,intp,intl;
	u8 buffer[2];

	if (mt_get_gpio_in(GPIO_ALS_EINT_PIN) == 1) /*skip if no interrupt*/  
	return 0;

	buffer[0] = APDS9930_CMM_STATUS;
	res = APDS9930_i2c_master_operate(client, buffer, 0x101, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	res = 0;
	intp = 0;
	intl = 0;
	if(0 != (buffer[0] & 0x20))
	{
		res = 0;
		intp = 1;
	}
	if(0 != (buffer[0] & 0x10))
	{
		res = 0;
		intl = 1;		
	}

	return res;

EXIT_ERR:
	APS_ERR("APDS9930_check_intr fail\n");
	return 1;
}

static int APDS9930_clear_intr(struct i2c_client *client) 
{
	int res;
	u8 buffer[2];
	
	buffer[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x07);
	res = APDS9930_i2c_master_operate(client, buffer, 0x1, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	else
	{
		res = 0;
	}
	return res;

EXIT_ERR:
	APS_ERR("APDS9930_check_and_clear_intr fail\n");
	return 1;
}


/*-----------------------------------------------------------------------------*/
void APDS9930_eint_func(void)
{
	struct APDS9930_priv *obj = g_APDS9930_ptr;
	if(!obj)
	{
		return;
	}
	//APS_LOG(" debug eint function performed!\n");
	schedule_work(&obj->eint_work);
}

/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
int APDS9930_setup_eint(struct i2c_client *client)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);        

	g_APDS9930_ptr = obj;
	
	mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT);
	mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, TRUE);
	mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);

	mt65xx_eint_set_sens(CUST_EINT_ALS_NUM, CUST_EINT_ALS_SENSITIVE);
	mt65xx_eint_set_polarity(CUST_EINT_ALS_NUM, CUST_EINT_ALS_POLARITY);
	mt65xx_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_EN, CUST_EINT_ALS_POLARITY, APDS9930_eint_func, 0);

	mt65xx_eint_unmask(CUST_EINT_ALS_NUM);  
    return 0;
}

/*----------------------------------------------------------------------------*/

static int APDS9930_init_client(struct i2c_client *client)
{
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
	u8 databuf[2];    
	int res = 0;

	databuf[0] = (TAOS_TRITON_CMD_REG|TAOS_TRITON_CMD_SPL_FN|0x00);
	res = APDS9930_i2c_master_operate(client, databuf, 0x1, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	databuf[0] = APDS9930_CMM_ENABLE;
	if(obj->hw->polling_mode_ps == 1)
	databuf[1] = 0x0f;
	if(obj->hw->polling_mode_ps == 0)
//Colby set AIEN to 0 start 20130121
	//databuf[1] = 0x3f;
	databuf[1] = 0x2f;
//Colby set AIEN to 0 end 20130121
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	databuf[0] = APDS9930_CMM_ATIME;    
	databuf[1] = 0xDB;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	databuf[0] = APDS9930_CMM_PTIME;    
	databuf[1] = 0xFF;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

	databuf[0] = APDS9930_CMM_WTIME;    
	databuf[1] = 0xF6;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	if(0 == obj->hw->polling_mode_ps)
	{
		if(1 == ps_cali.valid)
		{
			databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;	
			databuf[1] = (u8)(ps_cali.far_away & 0x00FF);
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			databuf[0] = APDS9930_CMM_INT_LOW_THD_HIGH;	
			databuf[1] = (u8)((ps_cali.far_away & 0xFF00) >> 8);
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;	
			databuf[1] = (u8)(ps_cali.close & 0x00FF);
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			databuf[0] = APDS9930_CMM_INT_HIGH_THD_HIGH;	
			databuf[1] = (u8)((ps_cali.close & 0xFF00) >> 8);;
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
		}
		else
		{
			databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;	
			databuf[1] = (u8)(750 & 0x00FF);
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			databuf[0] = APDS9930_CMM_INT_LOW_THD_HIGH;	
			databuf[1] = (u8)((750 & 0xFF00) >> 8);
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;	
			databuf[1] = (u8)(900 & 0x00FF);
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}
			databuf[0] = APDS9930_CMM_INT_HIGH_THD_HIGH;	
			databuf[1] = (u8)((900 & 0xFF00) >> 8);;
			res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
			if(res <= 0)
			{
				goto EXIT_ERR;
			}

		}

		databuf[0] = APDS9930_CMM_Persistence;
		databuf[1] = 0x22;
		res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}

	}

	databuf[0] = APDS9930_CMM_CONFIG;    
	databuf[1] = 0x00;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

       /*Lenovo-sw chenlj2 add 2011-06-03,modified pulse 2  to 4 */
	databuf[0] = APDS9930_CMM_PPCOUNT;    
	databuf[1] = APDS9930_CMM_PPCOUNT_VALUE;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}

        /*Lenovo-sw chenlj2 add 2011-06-03,modified gain 16  to 1 */
	databuf[0] = APDS9930_CMM_CONTROL;    
	databuf[1] = APDS9930_CMM_CONTROL_VALUE;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	databuf[0] = APDS9930_CMM_POFFSET;
	databuf[1] = APDS9930_CMM_POFFSET_VALUE;
	res = APDS9930_i2c_master_operate(client, databuf, 0x2, I2C_FLAG_WRITE);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	if((res = APDS9930_setup_eint(client))!=0)
	{
		APS_ERR("setup eint: %d\n", res);
		return res;
	}
	if((res = APDS9930_check_and_clear_intr(client)))
	{
		APS_ERR("check/clear intr: %d\n", res);
	    return res;
	}
	
	return APDS9930_SUCCESS;

EXIT_ERR:
	APS_ERR("init dev: %d\n", res);
	return res;
}

//Colby add attr start 20121218

/*----------------------------------------------------------------------------*/

int APDS9930_read_avg_ps(int sSampleNo)
{
	int PSData = 0;
	int DataCount = 0;
	int sAvePsData = 0;

	while(DataCount < sSampleNo)
	{
		msleep(100);
		APDS9930_read_ps(APDS9930_obj->client, &APDS9930_obj->ps);
		PSData = APDS9930_obj->ps;

		sAvePsData +=  PSData;
		DataCount++;
	}
	sAvePsData /= sSampleNo;
	return sAvePsData;
}

/*----------------------------------------------------------------------------*/

//Colby add for PL sensor calibration start 20130123
static bool APDS9930_PS_test_8mm()
{
//Test PS Black card 0.8 cm
	int test_ps_code;
	bool ps_test_cover = false;
	int err=0;

	// APDS9930_set_ppcount: set pcount
	if((err = APDS9930_set_ppcount(APDS9930_obj->client, g_pcount))){
		APS_ERR("[Colby]set p sensor count fail\n");
		}
	APS_ERR("[Colby]set p sensor count Success\n");

	//APDS9930_set_ppoffset: set poffset
	if((err = APDS9930_set_ppoffset(APDS9930_obj->client, g_poffset))){
		APS_ERR("[Colby]set p sensor offset fail\n");
		}
	APS_ERR("[Colby]set p sensor offset Success\n");

	test_ps_code = APDS9930_read_avg_ps(1);
	msleep(100);
	test_ps_code = APDS9930_read_avg_ps(5);
	g_result_ps_cover = test_ps_code;
	if(test_ps_code >= g_ps_close){
		ps_test_cover = true;
		APS_ERR("Test PS Black card 0.8 cm SUCCESS: ps_test_cover = %d, g_result_ps_cover = %d\n", ps_test_cover, g_result_ps_cover);
		}
	else{
		ps_test_cover = false;
		APS_ERR("Test PS Black card 0.8 cm FAIL: ps_test_cover = %d, g_result_ps_cover = %d\n", ps_test_cover, g_result_ps_cover);
		}
	return ps_test_cover;
}
static bool APDS9930_PS_cal_CT()
{
//Test PS Black card CT
	int ps_far= 0;
	int ps_ct = 0;
	int err=0;
	bool ps_calibration_CT = false;

	// APDS9930_set_ppcount: set pcount
	if((err = APDS9930_set_ppcount(APDS9930_obj->client, g_pcount))){
		APS_ERR("[Colby]set p sensor count fail\n");
		}
	APS_ERR("[Colby]set p sensor count Success\n");

	//APDS9930_set_ppoffset: set poffset
	if((err = APDS9930_set_ppoffset(APDS9930_obj->client, g_poffset))){
		APS_ERR("[Colby]set p sensor offset fail\n");
		}
	APS_ERR("[Colby]set p sensor offset Success\n");

	ps_ct = APDS9930_read_avg_ps(1);
	msleep(100);
	ps_ct = APDS9930_read_avg_ps(5);
	APS_ERR("ps_ct = %d\n", ps_ct);

	/* Far = CT + (Near - CT)/2 */
	ps_far = ps_ct + (g_ps_close - ps_ct)/2;
	APS_ERR("ps_far = %d\n", ps_far);

	//(Near - CT) should be bigger than 100
	//CT should be smaller than F2N
	g_ps_far = ps_far;
	g_result_ct = ps_ct;
	if(((g_ps_close - ps_ct) > 100) && (g_ps_close > ps_ct)){
		ps_calibration_CT = true;
		APS_ERR("ITEM_PROXIMITY_TEST_CT SUCCESS: ps_calibration_CT = %d, g_ps_far = %d, g_result_ct = %d\n", ps_calibration_CT, g_ps_far, g_result_ct);
		}
	else{
		ps_calibration_CT = false;
		APS_ERR("ITEM_PROXIMITY_TEST_CT FAIL: ps_calibration_CT = %d, g_ps_far = %d, g_result_ct = %d\n", ps_calibration_CT, g_ps_far, g_result_ct);
		}
	return ps_calibration_CT;
}

static bool APDS9930_PS_cal_B_15mm()
{
//PS cal Black card 1.5 cm
	int AvePSData_ct = 0;
	int Pcount = 10,Mincount = 3;
	int Poffset = 0, Maxoffset = 128;
	int ps_close = 0, ThdL_auto = 0;
	bool ps_calibration = false;
	int err=0;
	int MAXCT = 720;
	int MINCT = 100;
	APS_ERR("MAXCT = %d, MINCT = %d\n", MAXCT, MINCT);

	APS_ERR("Wait for calibrating ...\n");
	APS_ERR("Initial value: Pcount = %d, Poffset = %d\n", Pcount, Poffset);
	APS_ERR("Start calibrating ...\n");

	// APDS9930_set_ppcount: set pcount
	if((err = APDS9930_set_ppcount(APDS9930_obj->client,Pcount))){
		APS_ERR("[Colby]set p sensor count fail\n");
		}
	APS_ERR("[Colby]set p sensor count Success\n");

	//APDS9930_set_ppoffset: set poffset
	if((err = APDS9930_set_ppoffset(APDS9930_obj->client,Poffset))){
		APS_ERR("[Colby]set p sensor offset fail\n");
		}
	APS_ERR("[Colby]set p sensor offset Success\n");

	//Calibration start
	while(Pcount >= Mincount){
		//read ps code for 5 times
		AvePSData_ct = APDS9930_read_avg_ps(1);
		msleep(100);
		AvePSData_ct = APDS9930_read_avg_ps(5);
		APS_ERR("Calibration: AvePSData_ct = %d, Pcount = %d, Poffset = %d\n", AvePSData_ct, Pcount, Poffset);

	if(AvePSData_ct < MAXCT && AvePSData_ct > MINCT){
		//calibration sucess!!
		ps_calibration = true;

		ps_close = AvePSData_ct; //*****************This is F2N threshold***************

		g_pcount = Pcount;
		g_poffset = Poffset;
		g_ps_close = ps_close;

		APS_ERR("Calibration successfully!!!!!!!!!!!!\n");
		APS_ERR("AvePSData_ct = %d, Pcount = %d, Poffset = %d\n", AvePSData_ct, Pcount, Poffset);
		APS_ERR("ps_close = %d\n", ps_close);
		break;
		}
	else{
		Poffset+=16;
		if(Poffset <= Maxoffset){
			if(Poffset == Maxoffset)
				Poffset = 127;

			//APDS9930_set_ppoffset: set poffset
			if((err = APDS9930_set_ppoffset(APDS9930_obj->client,Poffset))){
				APS_ERR("[Colby]set p sensor offset fail\n");
				}
			continue;
			}
		else{
			Pcount --;
			if(Pcount < Mincount){
				ps_calibration = false;

				g_pcount = Pcount;
				g_poffset = Poffset;
				g_ps_close = ps_close;

				APS_ERR("Pcount is too small than Mincount\n");
				APS_ERR("Calibration fail..............\n");
				//printf("AvePSData_ct = %d, Pcount = %d, Poffset = %d\n", AvePSData_ct, lps->PPcount, lps->PPoffset);
				APS_ERR("Pcount < 3, AvePSData_ct = %d\n", AvePSData_ct);
				break;
				}

			// APDS9930_set_ppcount: set pcount
			if((err = APDS9930_set_ppcount(APDS9930_obj->client,Pcount))){
				APS_ERR("[Colby]set p sensor count fail\n");
				}

			Poffset = 0; // poffset start over

			//APDS9930_set_ppoffset: set poffset
			if((err = APDS9930_set_ppoffset(APDS9930_obj->client,Poffset))){
				APS_ERR("[Colby]set p sensor offset fail\n");
				}
			continue;
			}
		}
	}
	return ps_calibration;
}
//Colby add for PL sensor calibration end 20130123

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_show_ps(struct device_driver *ddri, char *buf)
{
	int ps_rawdata;
	int err=0;
	APS_ERR("[Colby]apds9930_show_ps\n");
	//enable PS start
	if((err = APDS9930_enable_ps(APDS9930_obj->client, 1))){
		APS_ERR("enable ps fail: %ld\n", err);
		}
	APS_ERR("enable ps success\n");
	set_bit(CMC_BIT_PS, &APDS9930_obj->enable);
	//enable PS end
	ps_rawdata = APDS9930_read_avg_ps(1);
	//disable PS start
		if((err = APDS9930_enable_ps(APDS9930_obj->client, 0))){
			APS_ERR("disable ps fail: %ld\n", err);
			}
		clear_bit(CMC_BIT_PS, &APDS9930_obj->enable);
		//disable PS end
	APS_ERR("[Colby]apds9930_show_ps ps_rawdata = %d\n", ps_rawdata);

	return scnprintf(buf, PAGE_SIZE, "ps_rawdata = %d\n", ps_rawdata);
}

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_store_ps(struct device_driver *ddri, const char *buf, size_t count)
{

	APS_ERR("[Colby]apds9930_store_ps\n");

	return;
}

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_show_nvdata(struct device_driver *ddri, char *buf)
{
	APS_ERR("[Colby]apds9930_show_nvdata\n");
	APS_ERR("[Colby]g_poffset = %d\n", g_poffset);
	APS_ERR("[Colby]g_pcount =  %d\n", g_pcount);
	APS_ERR("[Colby]g_ps_close = %d\n", g_ps_close);
	APS_ERR("[Colby]g_ps_far =  %d\n", g_ps_far);
//Colby add for nvdata start 20130114
	APS_ERR("[Colby]g_result_ct =  %d\n", g_result_ct);
	APS_ERR("[Colby]g_result_ps_cover =  %d\n", g_result_ps_cover);
	APS_ERR("[Colby]g_result_als_value =  %d\n", g_result_als_value);
//Colby add for nvdata end 20130114
	//scnprintf(buf, PAGE_SIZE, "%d\n", kd);
	return scnprintf(buf, PAGE_SIZE, "g_poffset = %d, g_pcount = %d, g_ps_close = %d, g_ps_far = %d, g_result_ct = %d, g_result_ps_cover = %d, g_result_als_value = %d\n", g_poffset, g_pcount, g_ps_close, g_ps_far, g_result_ct, g_result_ps_cover, g_result_als_value);
}

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_store_nvdata(struct device_driver *ddri, const char *buf, size_t count)
{
	APS_ERR("[Colby]apds9930_store_nvdata\n");
}

/*----------------------------------------------------------------------------*/

//Colby add for PL sensor calibration start 20130123
static ssize_t apds9930_show_calibration(struct device_driver *ddri, char *buf)
{
	APS_ERR("[Colby]apds9930_show_calibration\n");
	bool ps_calibration = false;
	bool ps_calibration_CT = false;
	bool ps_test_cover = false;
	int err=0;
	APS_ERR("[Colby]CCI_cal = %d\n", CCI_cal);

	//enable PS start
	if((err = APDS9930_enable_ps(APDS9930_obj->client, 1))){
		APS_ERR("enable ps fail: %ld\n", err);
		}
	APS_ERR("enable ps success\n");
	set_bit(CMC_BIT_PS, &APDS9930_obj->enable);
	//enable PS end
	switch (CCI_cal)
	{
	case 1:
	//PS cal Black card 1.5 cm
		//do calibration
		ps_calibration = APDS9930_PS_cal_B_15mm();

		//disable PS start
		if((err = APDS9930_enable_ps(APDS9930_obj->client, 0))){
			APS_ERR("disable ps fail: %ld\n", err);
			}
		clear_bit(CMC_BIT_PS, &APDS9930_obj->enable);
		//disable PS end
		APS_ERR("[Colby]APDS9930_PS_cal_B_15mm done\n");
		if(ps_calibration){
			atomic_set(&APDS9930_obj->ps_thd_val_high, g_ps_close);
			APS_ERR("[Colby]APDS9930_PS_cal_B_15mm ps_thd_val_high = %d\n", APDS9930_obj->ps_thd_val_high);
			return scnprintf(buf, PAGE_SIZE, "PS cal Black card 1.5 cm OK!!\nps_calibration = %d\ng_pcount = %d, g_poffset = %d, g_ps_close = %d\n", ps_calibration, g_pcount, g_poffset, g_ps_close);
			}
		else{
			return scnprintf(buf, PAGE_SIZE, "PS cal Black card 1.5 cm not OK..\nps_calibration = %d\ng_pcount = %d, g_poffset = %d, g_ps_close = %d\n", ps_calibration, g_pcount, g_poffset, g_ps_close);
			}
		break;

	case 2:
	//Test PS Black card CT
		//do calibration
		ps_calibration_CT = APDS9930_PS_cal_CT();

		//disable PS start
		if((err = APDS9930_enable_ps(APDS9930_obj->client, 0))){
			APS_ERR("disable ps fail: %ld\n", err);
			}
		APS_ERR("disable ps success\n");
		clear_bit(CMC_BIT_PS, &APDS9930_obj->enable);
		//disable PS end
		APS_ERR("[Colby]APDS9930_PS_cal_CT done\n");
		if(ps_calibration_CT){
			atomic_set(&APDS9930_obj->ps_thd_val_low, g_ps_far);
			APS_ERR("[Colby]APDS9930_PS_cal_CT ps_thd_val_low = %d\n", APDS9930_obj->ps_thd_val_low);
			return scnprintf(buf, PAGE_SIZE, "Test PS Black card CT OK!!\nps_calibration_CT = %d\ng_pcount = %d, g_poffset = %d, g_ps_close = %d, g_ps_far = %d, g_result_ct = %d\n", ps_calibration_CT, g_pcount, g_poffset, g_ps_close, g_ps_far, g_result_ct);
			}
		else{
			return scnprintf(buf, PAGE_SIZE, "Test PS Black card CT not OK..\nps_calibration_CT = %d\ng_pcount = %d, g_poffset = %d, g_ps_close = %d, g_ps_far = %d, g_result_ct = %d\n", ps_calibration_CT, g_pcount, g_poffset, g_ps_close, g_ps_far, g_result_ct);
			}
		break;

	case 3:
	//Test PS Black card 0.8 cm
		ps_test_cover = APDS9930_PS_test_8mm();

		//disable PS start
		if((err = APDS9930_enable_ps(APDS9930_obj->client, 0))){
			APS_ERR("disable ps fail: %ld\n", err);
			}
		APS_ERR("disable ps success\n");
		clear_bit(CMC_BIT_PS, &APDS9930_obj->enable);
		//disable PS end
		APS_ERR("[Colby]APDS9930_PS_test_8mm done\n");
		if(ps_test_cover){
			return scnprintf(buf, PAGE_SIZE, "Test PS Black card 0.8 cm OK!!\nps_test_cover = %d\ng_pcount = %d, g_poffset = %d, g_ps_close = %d, g_ps_far = %d, g_result_ct = %d, g_result_ps_cover = %d\n", ps_test_cover, g_pcount, g_poffset, g_ps_close, g_ps_far, g_result_ct, g_result_ps_cover);
			}
		else{
			return scnprintf(buf, PAGE_SIZE, "Test PS Black card 0.8 cm not OK..\nps_test_cover = %d\ng_pcount = %d, g_poffset = %d, g_ps_close = %d, g_ps_far = %d, g_result_ct = %d, g_result_ps_cover = %d\n", ps_test_cover, g_pcount, g_poffset, g_ps_close, g_ps_far, g_result_ct, g_result_ps_cover);
			}
		break;

	default:
		APS_ERR("%s not supported = %d", __FUNCTION__, CCI_cal);
		err = -1;
		break;
	}
	return scnprintf(buf, PAGE_SIZE, "Please do =>echo X > calibration, then cat calibration againn\nX=1: PS cal Black card 1.5 cm\nX=2: Test PS Black card CT\nX=3: Test PS Black card 0.8 cm\n");
}

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_store_calibration(struct device_driver *ddri, const char *buf, size_t count)
{
	APS_ERR("[Colby]apds9930_store_calibration\n");
	int cal;
	if(1 != sscanf(buf, "%d", &cal))
	{
		APS_ERR("[Colby]invalid format: '%s'\n", buf);
		return 0;
	}
	CCI_cal = cal;
	APS_ERR("[Colby]CCI_cal = %d\n", CCI_cal);
}

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_show_setpoffset(struct device_driver *ddri, char *buf)
{
	APS_ERR("[Colby]apds9930_show_setpoffset\n");
	APS_ERR("[Colby]g_poffset = %d\n", g_poffset);
	return scnprintf(buf, PAGE_SIZE, "g_poffset = %d\n", g_poffset);
}

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_store_setpoffset(struct device_driver *ddri, const char *buf, size_t count)
{
	int setoffset = 0;
	int err=0;
	APS_ERR("[Colby]apds9930_store_setpoffset\n");

	if(1 != sscanf(buf, "%d", &setoffset))
	{
		APS_ERR("[Colby]invalid format: '%s'\n", buf);
		return 0;
	}
	g_poffset = setoffset;
	APS_ERR("[Colby]apds9930_store_setpoffset g_poffset = %d, setoffset = %d", g_poffset, setoffset);
	//APDS9930_set_ppoffset: set poffset
	if((err = APDS9930_set_ppoffset(APDS9930_obj->client, g_poffset))){
		APS_ERR("[Colby]set p sensor offset fail\n");
		}
	APS_ERR("[Colby]set p sensor offset Success\n");
	return;
}

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_show_setpcount(struct device_driver *ddri, char *buf)
{
	APS_ERR("[Colby]apds9930_show_setpcount\n");
	APS_ERR("[Colby]g_pcount = %d\n", g_pcount);
	return scnprintf(buf, PAGE_SIZE, "g_pcount = %d\n", g_pcount);
}

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_store_setpcount(struct device_driver *ddri, const char *buf, size_t count)
{
	int setpcount = 0;
	int err=0;
	APS_ERR("[Colby]apds9930_store_setpcount\n");

	if(1 != sscanf(buf, "%d", &setpcount))
	{
		APS_ERR("[Colby]invalid format: '%s'\n", buf);
		return 0;
	}
	g_pcount = setpcount;
	APS_ERR("[Colby]apds9930_store_ccidebug g_pcount = %d, setpcount = %d", g_pcount, setpcount);
	// APDS9930_set_ppcount: set pcount
	if((err = APDS9930_set_ppcount(APDS9930_obj->client, g_pcount))){
		APS_ERR("[Colby]set p sensor count fail\n");
		}
	APS_ERR("[Colby]set p sensor count Success\n");
	return;
}
//Colby add for PL sensor calibration end 20130123

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_show_ccidebug(struct device_driver *ddri, char *buf)
{

	APS_ERR("[Colby]apds9930_show_ccidebug\n");
	APS_ERR("[Colby]CCI_debug_for_APDS9930 = %d\n", CCI_debug_for_APDS9930);

	return scnprintf(buf, PAGE_SIZE, "CCI_debug_for_APDS9930 = %d\n", CCI_debug_for_APDS9930);

}

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_store_ccidebug(struct device_driver *ddri, const char *buf, size_t count)
{
	int enable = 0;
	APS_ERR("[Colby]apds9930_store_ccidebug\n");
	if(1 != sscanf(buf, "%d", &enable))
	{
		APS_ERR("[Colby]invalid format: '%s'\n", buf);
		return 0;
	}
	APS_ERR("[Colby]apds9930_store_ccidebug enable = %d", enable);

	if(enable){
		CCI_debug_for_APDS9930 = 1;
		APS_ERR("[Colby]apds9930_store_ccidebug set enable");
		}
	else{
		CCI_debug_for_APDS9930 = 0;
		APS_ERR("[Colby]apds9930_store_ccidebug set disable");
		}
	return;
}

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_show_offset(struct device_driver *ddri, char *buf)
{
/*	u8 buffer[2];
	int res = 0;
	u16 data;*/
	APS_ERR("[Colby]apds9930_show_offset\n");
/*
	buffer[0] = APDS9930_CMM_POFFSET;
	res = APDS9930_i2c_master_operate(APDS9930_obj->client, buffer, 0x101, I2C_FLAG_READ);

	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	data = buffer[0];
	APS_ERR("[Colby]data = %d\n", data);
	return; */
}

/*----------------------------------------------------------------------------*/

static ssize_t apds9930_store_offset(struct device_driver *ddri, const char *buf, size_t count)
{
	//APDS9930_i2c_client
	//g_APDS9930_ptr
	unsigned int poffset;
	unsigned int pules = 2;
	int ps_code;

	APS_ERR("[Colby]apds9930_store_config\n");

	while(pules <= 10){
		//for(pules = 2; pules > 10 ; pules++ ){
		poffset = 127;

		APS_ERR("[Colby]pules = %d\n", pules);
		APDS9930_set_ppcount(APDS9930_obj->client, pules);
		msleep(100);

		//for(poffset=127; poffset > 0; poffset--){
		while(poffset > 0){
			APS_ERR("[Colby]poffset = %d\n", poffset);
			APDS9930_set_ppoffset(APDS9930_obj->client, poffset);
			msleep(100);
			//APDS9930_read_ps(APDS9930_obj->client, &APDS9930_obj->ps);
			//ps_code = APDS9930_read_avg_ps(1);
			//msleep(100);
			ps_code = APDS9930_read_avg_ps(20);
			//APS_ERR("[Colby]ps = %d\n", APDS9930_obj->ps);
			APS_ERR("[Colby]ps = %d\n", ps_code);

			poffset = poffset - 1;
			}
		pules = pules + 1;
		}
	//return snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&g_APDS9930_ptr->ps));
	return;
}

/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(setpoffset,  S_IWUSR | S_IRUGO, apds9930_show_setpoffset, apds9930_store_setpoffset);
static DRIVER_ATTR(setpcount,  S_IWUSR | S_IRUGO, apds9930_show_setpcount, apds9930_store_setpcount);
static DRIVER_ATTR(offset,  S_IWUSR | S_IRUGO, apds9930_show_offset, apds9930_store_offset);
static DRIVER_ATTR(calibration,  S_IWUSR | S_IRUGO, apds9930_show_calibration, apds9930_store_calibration);
static DRIVER_ATTR(nvdata,  S_IWUSR | S_IRUGO, apds9930_show_nvdata, apds9930_store_nvdata);
static DRIVER_ATTR(ps,  S_IWUSR | S_IRUGO, apds9930_show_ps, apds9930_store_ps);
static DRIVER_ATTR(ccidebug,  S_IWUSR | S_IRUGO, apds9930_show_ccidebug, apds9930_store_ccidebug);

/*----------------------------------------------------------------------------*/
static struct driver_attribute *apds9930_attr_list[] = {
    &driver_attr_offset,
    &driver_attr_calibration,
    &driver_attr_nvdata,
    &driver_attr_ps,
    &driver_attr_ccidebug,
    &driver_attr_setpoffset,
    &driver_attr_setpcount,
};

/*----------------------------------------------------------------------------*/

static int apds9930_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = (int)(sizeof(apds9930_attr_list)/sizeof(apds9930_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if((err = driver_create_file(driver, apds9930_attr_list[idx])))
		{
			APS_ERR("[Colby]driver_create_file (%s) = %d\n", apds9930_attr_list[idx]->attr.name, err);
			break;
		}
	}
	return err;
}

/*----------------------------------------------------------------------------*/

	static int apds9930_delete_attr(struct device_driver *driver)
	{
	int idx ,err = 0;
	int num = (int)(sizeof(apds9930_attr_list)/sizeof(apds9930_attr_list[0]));

	if (!driver)
	return -EINVAL;

	for (idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, apds9930_attr_list[idx]);
	}

	return err;
}

/*----------------------------------------------------------------------------*/

//Colby add attr end 20121218

/****************************************************************************** 
 * Function Configuration
******************************************************************************/
int APDS9930_read_als(struct i2c_client *client, u16 *data)
{	 
	struct APDS9930_priv *obj = i2c_get_clientdata(client);
	u16 c0_value, c1_value;	 
	u32 c0_nf, c1_nf;
	u8 buffer[2];
	u16 atio;
	int res = 0;

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}
	
	buffer[0]=APDS9930_CMM_C0DATA_L;
	res = APDS9930_i2c_master_operate(client, buffer, 0x201, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	c0_value = buffer[0] | (buffer[1]<<8);
	c0_nf = obj->als_modulus*c0_value;
	//APS_LOG("c0_value=%d, c0_nf=%d, als_modulus=%d\n", c0_value, c0_nf, obj->als_modulus);

	buffer[0]=APDS9930_CMM_C1DATA_L;
	res = APDS9930_i2c_master_operate(client, buffer, 0x201, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	c1_value = buffer[0] | (buffer[1]<<8);
	c1_nf = obj->als_modulus*c1_value;	
	//APS_LOG("c1_value=%d, c1_nf=%d, als_modulus=%d\n", c1_value, c1_nf, obj->als_modulus);
	
	if((c0_value > c1_value) &&(c0_value < 50000))
	{  	/*Lenovo-sw chenlj2 add 2011-06-03,add {*/
		atio = (c1_nf*100)/c0_nf;

	//APS_LOG("atio = %d\n", atio);
	if(atio<30)
	{
		*data = (13*c0_nf - 24*c1_nf)/10000;
	}
	else if(atio>= 30 && atio<38) /*Lenovo-sw chenlj2 add 2011-06-03,modify > to >=*/
	{ 
		*data = (16*c0_nf - 35*c1_nf)/10000;
	}
	else if(atio>= 38 && atio<45)  /*Lenovo-sw chenlj2 add 2011-06-03,modify > to >=*/
	{ 
		*data = (9*c0_nf - 17*c1_nf)/10000;
	}
	else if(atio>= 45 && atio<54) /*Lenovo-sw chenlj2 add 2011-06-03,modify > to >=*/
	{ 
		*data = (6*c0_nf - 10*c1_nf)/10000;
	}
	else
		*data = 0;
	/*Lenovo-sw chenlj2 add 2011-06-03,add }*/
    }
	else if (c0_value > 50000)
	{
		*data = 65535;
	}
	else if(c0_value == 0)
        {
                *data = 0;
        }
        else
	{
		APS_DBG("APDS9930_read_als als_value is invalid!!\n");
		return -1;
	}	

	//APS_LOG("APDS9930_read_als als_value_lux = %d\n", *data);
	return 0;	 

	
	
EXIT_ERR:
	APS_ERR("APDS9930_read_ps fail\n");
	return res;
}
int APDS9930_read_als_ch0(struct i2c_client *client, u16 *data)
{	 
	//struct APDS9930_priv *obj = i2c_get_clientdata(client);
	u16 c0_value;	 
	u8 buffer[2];
	int res = 0;
	
	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

//get adc channel 0 value
	buffer[0]=APDS9930_CMM_C0DATA_L;
	res = APDS9930_i2c_master_operate(client, buffer, 0x201, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	c0_value = buffer[0] | (buffer[1]<<8);
	*data = c0_value;
	//APS_LOG("c0_value=%d\n", c0_value);
	return 0;	 

	
	
EXIT_ERR:
	APS_ERR("APDS9930_read_ps fail\n");
	return res;
}
/*----------------------------------------------------------------------------*/

static int APDS9930_get_als_value(struct APDS9930_priv *obj, u16 als)
{
	int idx;
	int invalid = 0;
	for(idx = 0; idx < obj->als_level_num; idx++)
	{
		if(als < obj->hw->als_level[idx])
		{
			break;
		}
	}
	
	if(idx >= obj->als_value_num)
	{
		APS_ERR("APDS9930_get_als_value exceed range\n"); 
		idx = obj->als_value_num - 1;
	}
	
	if(1 == atomic_read(&obj->als_deb_on))
	{
		unsigned long endt = atomic_read(&obj->als_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->als_deb_on, 0);
		}
		
		if(1 == atomic_read(&obj->als_deb_on))
		{
			invalid = 1;
		}
	}

	if(!invalid)
	{
		//APS_ERR("ALS: %05d => %05d\n", als, obj->hw->als_value[idx]);	
		return obj->hw->als_value[idx];
	}
	else
	{
		//APS_ERR("ALS: %05d => %05d (-1)\n", als, obj->hw->als_value[idx]);    
		return -1;
	}
}
/*----------------------------------------------------------------------------*/
long APDS9930_read_ps(struct i2c_client *client, u16 *data)
{
	//struct APDS9930_priv *obj = i2c_get_clientdata(client);	
	u8 buffer[2];
	long res = 0;

	if(client == NULL)
	{
		APS_DBG("CLIENT CANN'T EQUL NULL\n");
		return -1;
	}

	buffer[0]=APDS9930_CMM_PDATA_L;
	res = APDS9930_i2c_master_operate(client, buffer, 0x201, I2C_FLAG_READ);
	if(res <= 0)
	{
		goto EXIT_ERR;
	}
	
	*data = buffer[0] | (buffer[1]<<8);
	//APS_LOG("yucong APDS9930_read_ps ps_data=%d, low:%d  high:%d", *data, buffer[0], buffer[1]);
	//APS_LOG("[Colby]APDS9930_read_ps ps_data=%d", *data); //Colby add
	return 0;    

EXIT_ERR:
	APS_ERR("APDS9930_read_ps fail\n");
	return res;
}
/*----------------------------------------------------------------------------*/
static int APDS9930_get_ps_value(struct APDS9930_priv *obj, u16 ps)
{
	int val;// mask = atomic_read(&obj->ps_mask);
	int invalid = 0;
	static int val_temp=1;

	if(ps_cali.valid == 1)
		{
			if((ps >ps_cali.close))
			{
				val = 0;  /*close*/
				val_temp = 0;
				intr_flag_value = 1;
			}
			
			else if((ps < ps_cali.far_away))
			{
				val = 1;  /*far away*/
				val_temp = 1;
				intr_flag_value = 0;
			}
			else
				val = val_temp;

			APS_LOG("APDS9930_get_ps_value val  = %d",val);
	}
	else
	{
		APS_ERR("[Colby]APDS9930_get_ps_value enter ps_thd_val_high = %d, ps_thd_val_low = %d\n", obj->ps_thd_val_high, obj->ps_thd_val_low);
			if((ps  > atomic_read(&obj->ps_thd_val_high)))
			{
				val = 0;  /*close*/
				val_temp = 0;
				intr_flag_value = 1;
			}
			else if((ps  < atomic_read(&obj->ps_thd_val_low)))
			{
				val = 1;  /*far away*/
				val_temp = 1;
				intr_flag_value = 0;
			}
			else
			       val = val_temp;	
			
	}
	
	if(atomic_read(&obj->ps_suspend))
	{
		invalid = 1;
	}
	else if(1 == atomic_read(&obj->ps_deb_on))
	{
		unsigned long endt = atomic_read(&obj->ps_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->ps_deb_on, 0);
		}
		
		if (1 == atomic_read(&obj->ps_deb_on))
		{
			invalid = 1;
		}
	}
	else if (obj->als > 45000)
	{
		//invalid = 1;
		APS_DBG("ligh too high will result to failt proximiy\n");
		return 1;  /*far away*/
	}

	if(!invalid)
	{
		//APS_DBG("PS:  %05d => %05d\n", ps, val);
		return val;
	}	
	else
	{
		return -1;
	}	
}


/*----------------------------------------------------------------------------*/
/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
//#define DEBUG_APDS9930
static void APDS9930_eint_work(struct work_struct *work)
{
	struct APDS9930_priv *obj = (struct APDS9930_priv *)container_of(work, struct APDS9930_priv, eint_work);
	int err;
	hwm_sensor_data sensor_data;
	u8 databuf[3];
	int res = 0;

	if((err = APDS9930_check_intr(obj->client)))
	{
		APS_ERR("APDS9930_eint_work check intrs: %d\n", err);
	}
	else
	{
		//get raw data
		APDS9930_read_ps(obj->client, &obj->ps);
		APDS9930_read_als_ch0(obj->client, &obj->als);
//Colby add debug flag for showing log start 20130121
		//APS_LOG("APDS9930_eint_work rawdata ps=%d als_ch0=%d!\n",obj->ps,obj->als);
		if(CCI_debug_for_APDS9930)
			APS_LOG("APDS9930_eint_work rawdata ps=%d als_ch0=%d!\n",obj->ps,obj->als);
//Colby add debug flag for showing log end 20130121
		if(obj->als > 40000)
			{
			APS_LOG("APDS9930_eint_work ALS too large may under lighting als_ch0=%d!\n",obj->als);
			return;
			}
		sensor_data.values[0] = APDS9930_get_ps_value(obj, obj->ps);
		sensor_data.value_divide = 1;
		sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;	

#ifdef DEBUG_APDS9930
		databuf[0]= APDS9930_CMM_ENABLE;
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x101, I2C_FLAG_READ);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		APS_LOG("APDS9930_eint_work APDS9930_CMM_ENABLE ps value = %x\n",databuf[0]);
		
		databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_READ);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		APS_LOG("APDS9930_eint_work APDS9930_CMM_INT_LOW_THD_LOW before databuf[0]=%d databuf[1]=%d!\n",databuf[0],databuf[1]);

		databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_READ);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		APS_LOG("APDS9930_eint_work APDS9930_CMM_INT_HIGH_THD_LOW before databuf[0]=%d databuf[1]=%d!\n",databuf[0],databuf[1]);
#endif
/*singal interrupt function add*/
		if(intr_flag_value){
						databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;	
						databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_low)) & 0x00FF);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
						
						databuf[0] = APDS9930_CMM_INT_LOW_THD_HIGH;	
						databuf[1] = (u8)(((atomic_read(&obj->ps_thd_val_low)) & 0xFF00) >> 8);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
						databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;	
						databuf[1] = (u8)(0x00FF);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
						
						databuf[0] = APDS9930_CMM_INT_HIGH_THD_HIGH; 
						databuf[1] = (u8)((0xFF00) >> 8);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
						
				}
				else{	
						databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;	
						databuf[1] = (u8)(0 & 0x00FF);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
						
						databuf[0] = APDS9930_CMM_INT_LOW_THD_HIGH;	
						databuf[1] = (u8)((0 & 0xFF00) >> 8);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
						
						databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;	
						databuf[1] = (u8)((atomic_read(&obj->ps_thd_val_high)) & 0x00FF);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
					
						databuf[0] = APDS9930_CMM_INT_HIGH_THD_HIGH; 
						databuf[1] = (u8)(((atomic_read(&obj->ps_thd_val_high)) & 0xFF00) >> 8);
						res = APDS9930_i2c_master_operate(obj->client, databuf, 0x2, I2C_FLAG_WRITE);
						res = i2c_master_send(obj->client, databuf, 0x2);
						if(res <= 0)
						{
							goto EXIT_ERR;
						}
				}
				
		//let up layer to know
		#ifdef DEBUG_APDS9930
		databuf[0] = APDS9930_CMM_INT_LOW_THD_LOW;
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_READ);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		APS_LOG("APDS9930_eint_work APDS9930_CMM_INT_LOW_THD_LOW after databuf[0]=%d databuf[1]=%d!\n",databuf[0],databuf[1]);

		databuf[0] = APDS9930_CMM_INT_HIGH_THD_LOW;
		res = APDS9930_i2c_master_operate(obj->client, databuf, 0x201, I2C_FLAG_READ);
		if(res <= 0)
		{
			goto EXIT_ERR;
		}
		APS_LOG("APDS9930_eint_work APDS9930_CMM_INT_HIGH_THD_LOW after databuf[0]=%d databuf[1]=%d!\n",databuf[0],databuf[1]);
		#endif
		if(CCI_debug_for_APDS9930)
			APS_LOG("[Colby]APDS9930_eint_work() hwmsen_get_interrupt_data values[0] = %d\n", (sensor_data.values[0] ));
		if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
		{
		  APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
		}
	}
	
	APDS9930_clear_intr(obj->client);
	mt65xx_eint_unmask(CUST_EINT_ALS_NUM); 
	return;
	EXIT_ERR:
	APDS9930_clear_intr(obj->client);
	mt65xx_eint_unmask(CUST_EINT_ALS_NUM); 
	APS_ERR("i2c_transfer error = %d\n", res);
	return;
}


/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int APDS9930_open(struct inode *inode, struct file *file)
{
	file->private_data = APDS9930_i2c_client;

	if (!file->private_data)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}
	
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int APDS9930_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

/*----------------------------------------------------------------------------*/
static long APDS9930_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct APDS9930_priv *obj = i2c_get_clientdata(client);  
	long err = 0;
	void __user *ptr = (void __user*) arg;
	int dat;
	uint32_t enable;
	int ps_result;
	int Pcount;
	int Poffset; //Colby add 20121218
	struct apds9930_threshold calibration_threshold;
	unsigned int pules;
	unsigned int offset; //Colby add 20121218
	switch (cmd)
	{
		case ALSPS_SET_PS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
				if((err = APDS9930_enable_ps(obj->client, 1)))
				{
					APS_ERR("enable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				set_bit(CMC_BIT_PS, &obj->enable);
			}
			else
			{
				if((err = APDS9930_enable_ps(obj->client, 0)))
				{
					APS_ERR("disable ps fail: %ld\n", err); 
					goto err_out;
				}
				
				clear_bit(CMC_BIT_PS, &obj->enable);
			}
			break;

		case ALSPS_GET_PS_MODE:
			enable = test_bit(CMC_BIT_PS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_DATA:    
			if((err = APDS9930_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			
			dat = APDS9930_get_ps_value(obj, obj->ps);
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;

		case ALSPS_GET_PS_RAW_DATA:    
			if((err = APDS9930_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			
			dat = obj->ps;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}  
			break;              

		case ALSPS_SET_ALS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
				if((err = APDS9930_enable_als(obj->client, 1)))
				{
					APS_ERR("enable als fail: %ld\n", err); 
					goto err_out;
				}
				set_bit(CMC_BIT_ALS, &obj->enable);
			}
			else
			{
				if((err = APDS9930_enable_als(obj->client, 0)))
				{
					APS_ERR("disable als fail: %ld\n", err); 
					goto err_out;
				}
				clear_bit(CMC_BIT_ALS, &obj->enable);
			}
			break;

		case ALSPS_GET_ALS_MODE:
			enable = test_bit(CMC_BIT_ALS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_ALS_DATA: 
			if((err = APDS9930_read_als(obj->client, &obj->als)))
			{
				goto err_out;
			}

			dat = APDS9930_get_als_value(obj, obj->als);
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;

		case ALSPS_GET_ALS_RAW_DATA:    
			if((err = APDS9930_read_als(obj->client, &obj->als)))
			{
				goto err_out;
			}

			dat = obj->als;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}              
			break;
		/*----------------------------------for factory mode test---------------------------------------*/
		case ALSPS_GET_PS_TEST_RESULT:
			if((err = APDS9930_read_ps(obj->client, &obj->ps)))
			{
				goto err_out;
			}
			if(obj->ps > atomic_read(&obj->ps_thd_val_high))
				{
					ps_result = 0;
				}
			else	ps_result = 1;
				
			if(copy_to_user(ptr, &ps_result, sizeof(ps_result)))
			{
				err = -EFAULT;
				goto err_out;
			}			   
			break;
		case ALSPS_IOCTL_SET_CALI:
			if(copy_from_user(&calibration_threshold, ptr, sizeof(calibration_threshold)))
                        {
                                err = -EFAULT;
                                goto err_out;
                        }
			calibration_threshold.ps_high_thd = 950;
			g_ps_close = calibration_threshold.ps_high_thd;//Colby add 20121225
            atomic_set(&obj->ps_thd_val_high, g_ps_close);
				//printk("set hight threshold :0x%x\n",(obj->ps_thd_val_high).counter);
				APS_ERR("[Colby]set hight threshold :0x%x\n", (obj->ps_thd_val_high).counter);
				//g_ps_close = calibration_threshold.ps_high_thd;//Colby add 20121225
			calibration_threshold.ps_low_thd = 850;
			g_ps_far = calibration_threshold.ps_low_thd;//Colby add 20121225
            atomic_set(&obj->ps_thd_val_low, g_ps_far);
				//printk("set low threshold :0x%x\n",(obj->ps_thd_val_low).counter);
				APS_ERR("[Colby]set low threshold :0x%x\n", (obj->ps_thd_val_low).counter);
				//g_ps_far = calibration_threshold.ps_low_thd;//Colby add 20121225
            pules=calibration_threshold.pules;
		g_pcount = pules; //Colby add 20121219
			if(APDS9930_set_ppcount(obj->client,pules))
			{
				 err = -EFAULT;
                 goto err_out;
				//printk("set p sensor pules fault\n");
				APS_ERR("set p sensor pules fault\n");
			}
			else
			{
				//printk("set p sensor pules Success\n");
				APS_ERR("set p sensor pules Success\n");
				//break;//Colby mod

		    }

//Colby add for nvdata start 20130114
	g_result_ct = calibration_threshold.result_ct;
	g_result_ps_cover = calibration_threshold.result_ps_cover;
	g_result_als_value = calibration_threshold.result_als_value;
//Colby add for nvdata end 20130114

	//Colby add start 20121218
            offset = calibration_threshold.offset;
		g_poffset = offset; //Colby add 20121219
			if(APDS9930_set_ppoffset(obj->client, offset))
			{
				 err = -EFAULT;
                 goto err_out;
				//printk("set p sensor offset fault\n");
				APS_ERR("set p sensor offset fault\n");
			}
			else
			{
				//printk("set p sensor offset Success\n");
				APS_ERR("set p sensor offset Success\n");
				break;
				
		    }
	//Colby add end 20121218

		 case ALSPS_SET_PS_PCOUNT:
		   	if(copy_from_user(&Pcount, ptr, sizeof(Pcount)))
		   	{
				//printk("error to set PCOUNT\n");
				APS_ERR("[Colby]error to set PCOUNT\n");
                                err = -EFAULT;
                                goto err_out;
            }
            if((err = APDS9930_set_ppcount(obj->client,Pcount)))
            {
				APS_ERR("[Colby]set p sensor count fail\n");
				goto err_out;
			}
			APS_ERR("[Colby]set p sensor count Success\n");
			break;
	//Colby add start 20121218
	 case ALSPS_SET_PS_POFFSET:
		if(copy_from_user(&Poffset, ptr, sizeof(Poffset))){
			//printk("error to set PCOUNT\n");
			APS_ERR("[Colby]error to set PCOUNT\n");
			err = -EFAULT;
			goto err_out;
			}
		if((err = APDS9930_set_ppoffset(obj->client,Poffset))){
			APS_ERR("[Colby]set p sensor offset fail\n");
			goto err_out;
			}
			//printk("set p sensor offset Success\n");
			APS_ERR("[Colby]set p sensor offset Success\n");
		break;
	//Colby add end 20121218
			/*------------------------------------------------------------------------------------------*/
		default:
			APS_ERR("%s not supported = 0x%04x", __FUNCTION__, cmd);
			err = -ENOIOCTLCMD;
			break;
	}

	err_out:
	return err;    
}
/*----------------------------------------------------------------------------*/
static struct file_operations APDS9930_fops = {
	.owner = THIS_MODULE,
	.open = APDS9930_open,
	.release = APDS9930_release,
	.unlocked_ioctl = APDS9930_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice APDS9930_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &APDS9930_fops,
};
/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_suspend(struct i2c_client *client, pm_message_t msg) 
{
//	struct APDS9930_priv *obj = i2c_get_clientdata(client);    
//	int err;
	APS_FUN();    
#if 0
	if(msg.event == PM_EVENT_SUSPEND)
	{   
		if(!obj)
		{
			APS_ERR("null pointer!!\n");
			return -EINVAL;
		}
		
		atomic_set(&obj->als_suspend, 1);
		if(err = APDS9930_enable_als(client, 0))
		{
			APS_ERR("disable als: %d\n", err);
			return err;
		}

		atomic_set(&obj->ps_suspend, 1);
		if(err = APDS9930_enable_ps(client, 0))
		{
			APS_ERR("disable ps:  %d\n", err);
			return err;
		}
		
		APDS9930_power(obj->hw, 0);
	}
#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_resume(struct i2c_client *client)
{
//	struct APDS9930_priv *obj = i2c_get_clientdata(client);        
//	int err;
	APS_FUN();
#if 0
	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}

	APDS9930_power(obj->hw, 1);
	if(err = APDS9930_init_client(client))
	{
		APS_ERR("initialize client fail!!\n");
		return err;        
	}
	atomic_set(&obj->als_suspend, 0);
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if(err = APDS9930_enable_als(client, 1))
		{
			APS_ERR("enable als fail: %d\n", err);        
		}
	}
	atomic_set(&obj->ps_suspend, 0);
	if(test_bit(CMC_BIT_PS,  &obj->enable))
	{
		if(err = APDS9930_enable_ps(client, 1))
		{
			APS_ERR("enable ps fail: %d\n", err);                
		}
	}
#endif
	return 0;
}
/*----------------------------------------------------------------------------*/
static void APDS9930_early_suspend(struct early_suspend *h) 
{   /*early_suspend is only applied for ALS*/
	struct APDS9930_priv *obj = container_of(h, struct APDS9930_priv, early_drv);   
	int err;
	APS_FUN();    

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}
	
	#if 1
	atomic_set(&obj->als_suspend, 1);
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if((err = APDS9930_enable_als(obj->client, 0)))
		{
			APS_ERR("disable als fail: %d\n", err); 
		}
	}
	#endif
}
/*----------------------------------------------------------------------------*/
static void APDS9930_late_resume(struct early_suspend *h)
{   /*early_suspend is only applied for ALS*/
	struct APDS9930_priv *obj = container_of(h, struct APDS9930_priv, early_drv);         
	int err;
	APS_FUN();

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}

        #if 1
	atomic_set(&obj->als_suspend, 0);
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if((err = APDS9930_enable_als(obj->client, 1)))
		{
			APS_ERR("enable als fail: %d\n", err);        

		}
	}
	#endif
}
/*----------------------------------------------------------------------------*/
static int temp_als = 0;
static int ALS_FLAG = 0;

int APDS9930_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int value;
	int err = 0;
	
	hwm_sensor_data* sensor_data;
	struct APDS9930_priv *obj = (struct APDS9930_priv *)self;
	if(CCI_debug_for_APDS9930)
		APS_ERR("[Colby]APDS9930_ps_operate() enter command = %d\n", command);
	//APS_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{	
				value = *(int *)buff_in;
				if(value)
				{
					if((err = APDS9930_enable_ps(obj->client, 1)))
					{
						APS_ERR("enable ps fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_PS, &obj->enable);
					#if 1
					if(!test_bit(CMC_BIT_ALS, &obj->enable))
					{
						ALS_FLAG = 1;
						if((err = APDS9930_enable_als(obj->client, 1)))
						{
							APS_ERR("enable als fail: %d\n", err); 
							return -1;
						}
					}
					#endif
				}
				else
				{
					if((err = APDS9930_enable_ps(obj->client, 0)))
					{
						APS_ERR("disable ps fail: %d\n", err); 
						return -1;
					}
					clear_bit(CMC_BIT_PS, &obj->enable);
					#if 1
					if(ALS_FLAG == 1)
					{
						if((err = APDS9930_enable_als(obj->client, 0)))
						{
							APS_ERR("disable als fail: %d\n", err); 
							return -1;
						}
						ALS_FLAG = 0;
					}
					#endif
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;	
				APDS9930_read_ps(obj->client, &obj->ps);
				APDS9930_read_als_ch0(obj->client, &obj->als);
				APS_ERR("APDS9930_ps_operate als data=%d!\n",obj->als);
				sensor_data->values[0] = APDS9930_get_ps_value(obj, obj->ps);
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;			
			}
			break;
		default:
			APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}


int APDS9930_als_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct APDS9930_priv *obj = (struct APDS9930_priv *)self;
	if(CCI_debug_for_APDS9930)
		APS_ERR("[Colby]APDS9930_als_operate() enter command = %d\n", command);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;				
				if(value)
				{
					if((err = APDS9930_enable_als(obj->client, 1)))
					{
						APS_ERR("enable als fail: %d\n", err); 
						return -1;
					}
					set_bit(CMC_BIT_ALS, &obj->enable);
				}
				else
				{
					if((err = APDS9930_enable_als(obj->client, 0)))
					{
						APS_ERR("disable als fail: %d\n", err); 
						return -1;
					}
					clear_bit(CMC_BIT_ALS, &obj->enable);
				}
				
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;
				/*yucong MTK add for fixing known issue*/
				#if 1
				APDS9930_read_als(obj->client, &obj->als);
				if(obj->als == 0)
				{
					sensor_data->values[0] = temp_als;				
				}else{
					u16 b[2];
					int i;
					for(i = 0;i < 2;i++){
					APDS9930_read_als(obj->client, &obj->als);
					b[i] = obj->als;
					if(CCI_debug_for_APDS9930)
						APS_ERR("[Colby]APDS9930_als_operate() b[%d]  = %d\n", i, obj->als);
					}
					(b[1] > b[0])?(obj->als = b[0]):(obj->als = b[1]);
					sensor_data->values[0] = APDS9930_get_als_value(obj, obj->als);
					if(CCI_debug_for_APDS9930)
						APS_ERR("[Colby]APDS9930_als_operate() values[0] = %d\n", sensor_data->values[0] );
					temp_als = sensor_data->values[0];
				}
				#endif
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
			}
			break;
		default:
			APS_ERR("light sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}


/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_detect(struct i2c_client *client, struct i2c_board_info *info) 
{    
	strcpy(info->type, APDS9930_DEV_NAME);
	return 0;
}

/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct APDS9930_priv *obj;
	struct hwmsen_object obj_ps, obj_als;
	int err = 0;

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	memset(obj, 0, sizeof(*obj));
	APDS9930_obj = obj;
	obj->hw = get_cust_alsps_hw();
	APDS9930_get_addr(obj->hw, &obj->addr);

	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
	INIT_WORK(&obj->eint_work, APDS9930_eint_work);
	obj->client = client;
	i2c_set_clientdata(client, obj);	
	atomic_set(&obj->als_debounce, 50);
	atomic_set(&obj->als_deb_on, 0);
	atomic_set(&obj->als_deb_end, 0);
	atomic_set(&obj->ps_debounce, 10);
	atomic_set(&obj->ps_deb_on, 0);
	atomic_set(&obj->ps_deb_end, 0);
	atomic_set(&obj->ps_mask, 0);
	atomic_set(&obj->als_suspend, 0);
	atomic_set(&obj->als_cmd_val, 0xDF);
	atomic_set(&obj->ps_cmd_val,  0xC1);
	atomic_set(&obj->ps_thd_val_high,  obj->hw->ps_threshold_high);
	atomic_set(&obj->ps_thd_val_low,  obj->hw->ps_threshold_low);
	obj->enable = 0;
	obj->pending_intr = 0;
	obj->als_level_num = sizeof(obj->hw->als_level)/sizeof(obj->hw->als_level[0]);
	obj->als_value_num = sizeof(obj->hw->als_value)/sizeof(obj->hw->als_value[0]);  
	/*Lenovo-sw chenlj2 add 2011-06-03,modified gain 16 to 1/5 accoring to actual thing */
	obj->als_modulus = (400*100*ZOOM_TIME)/(1*150);//(1/Gain)*(400/Tine), this value is fix after init ATIME and CONTROL register value
										//(400)/16*2.72 here is amplify *100 //16
	BUG_ON(sizeof(obj->als_level) != sizeof(obj->hw->als_level));
	memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
	BUG_ON(sizeof(obj->als_value) != sizeof(obj->hw->als_value));
	memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));
	atomic_set(&obj->i2c_retry, 3);
	set_bit(CMC_BIT_ALS, &obj->enable);
	set_bit(CMC_BIT_PS, &obj->enable);

	
	APDS9930_i2c_client = client;
	
	if(1 == obj->hw->polling_mode_ps)
		//if (1)
		{
			obj_ps.polling = 1;
		}
		else
		{
			obj_ps.polling = 0;
		}
	
	if((err = APDS9930_init_client(client)))
	{
		goto exit_init_failed;
	}
	APS_LOG("APDS9930_init_client() OK!\n");

	if((err = misc_register(&APDS9930_device)))
	{
		APS_ERR("APDS9930_device register failed\n");
		goto exit_misc_device_register_failed;
	}
//Colby add attr start20121218
	if((err = apds9930_create_attr(&APDS9930_alsps_driver.driver)))
		{
			APS_ERR("[Colby]create attribute err = %d\n", err);
			goto exit_create_attr_failed;
		}
//Colby add attr end 20121218
/*
	if(err = APDS9930_create_attr(&APDS9930_alsps_driver.driver))
	{
		APS_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}
*/
	obj_ps.self = APDS9930_obj;
	
	obj_ps.sensor_operate = APDS9930_ps_operate;
	if((err = hwmsen_attach(ID_PROXIMITY, &obj_ps)))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}
	
	obj_als.self = APDS9930_obj;
	obj_als.polling = 1;
	obj_als.sensor_operate = APDS9930_als_operate;
	if((err = hwmsen_attach(ID_LIGHT, &obj_als)))
	{
		APS_ERR("attach fail = %d\n", err);
		goto exit_create_attr_failed;
	}


#if defined(CONFIG_HAS_EARLYSUSPEND)
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = APDS9930_early_suspend,
	obj->early_drv.resume   = APDS9930_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif

	APS_LOG("%s: OK\n", __func__);
	return 0;

	exit_create_attr_failed:
	misc_deregister(&APDS9930_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(client);
	//exit_kfree:
	kfree(obj);
	exit:
	APDS9930_i2c_client = NULL;           
//	MT6516_EINTIRQMask(CUST_EINT_ALS_NUM);  /*mask interrupt if fail*/
	APS_ERR("%s: err = %d\n", __func__, err);
	return err;
}
/*----------------------------------------------------------------------------*/
static int APDS9930_i2c_remove(struct i2c_client *client)
{
	int err;	
//Colby add attr start 20121218
	if((err = apds9930_delete_attr(&APDS9930_i2c_driver.driver)))
		{
		APS_ERR("[Colby]apds9930_delete_attr fail: %d\n", err);
		}
//Colby add attr end 20121218

/*	
	if(err = APDS9930_delete_attr(&APDS9930_i2c_driver.driver))
	{
		APS_ERR("APDS9930_delete_attr fail: %d\n", err);
	} 
*/
	if((err = misc_deregister(&APDS9930_device)))
	{
		APS_ERR("misc_deregister fail: %d\n", err);    
	}
	
	APDS9930_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));

	return 0;
}
/*----------------------------------------------------------------------------*/
static int APDS9930_probe(struct platform_device *pdev) 
{
	struct alsps_hw *hw = get_cust_alsps_hw();

	APDS9930_power(hw, 1);    
	//APDS9930_force[0] = hw->i2c_num;
	//APDS9930_force[1] = hw->i2c_addr[0];
	//APS_DBG("I2C = %d, addr =0x%x\n",APDS9930_force[0],APDS9930_force[1]);
	if(i2c_add_driver(&APDS9930_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	} 
	return 0;
}
/*----------------------------------------------------------------------------*/
static int APDS9930_remove(struct platform_device *pdev)
{
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();    
	APDS9930_power(hw, 0);    
	i2c_del_driver(&APDS9930_i2c_driver);
	return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver APDS9930_alsps_driver = {
	.probe      = APDS9930_probe,
	.remove     = APDS9930_remove,    
	.driver     = {
		.name  = "als_ps",
//		.owner = THIS_MODULE,
	}
};
/*----------------------------------------------------------------------------*/
static int __init APDS9930_init(void)
{
	//APS_FUN();
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_LOG("%s: i2c_number=%d\n", __func__,hw->i2c_num); 
	i2c_register_board_info(hw->i2c_num, &i2c_APDS9930, 1);
	if(platform_driver_register(&APDS9930_alsps_driver))
	{
		APS_ERR("failed to register driver");
		return -ENODEV;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit APDS9930_exit(void)
{
	APS_FUN();
	platform_driver_unregister(&APDS9930_alsps_driver);
}
/*----------------------------------------------------------------------------*/
module_init(APDS9930_init);
module_exit(APDS9930_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("Dexiang Liu");
MODULE_DESCRIPTION("APDS9930 driver");
MODULE_LICENSE("GPL");

