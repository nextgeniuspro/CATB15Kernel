

/*< XASP-360 linghai 20120626 begin */
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include "tpd_custom_s2202.h"
#include "cust_gpio_usage.h"
#include "tpd.h"

#ifdef MT6575
#include <mach/mt6575_pm_ldo.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_boot.h>
#endif
#ifdef MT6577
#include <mach/mt6577_pm_ldo.h>
#include <mach/mt6577_typedefs.h>
#include <mach/mt6577_boot.h>
#endif
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/boot.h>


/* < DTS2012042504399  gkf61766 20120425 begin */
//#define TPD_UPDATE_FIRMWARE
/* DTS2012042504399  gkf61766 20120425 end> */
//#define HAVE_TOUCH_KEY
#ifdef TPD_UPDATE_FIRMWARE
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/namei.h>
#include <linux/vmalloc.h>

static struct i2c_client *g_client = NULL;
static ssize_t update_firmware_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf);
static ssize_t update_firmware_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

static int ts_firmware_file(void);
static int i2c_update_firmware(struct i2c_client *client); 

/* we changed the mode of these files and directories 
 * the meet the requirements of Android Gingerbread CTS tests
 */  
static struct kobj_attribute update_firmware_attribute = {
	/* < DTS2012042803609 gkf61766 20120428 begin */
	.attr = {.name = "t8830_update_firmware", .mode = 0664},
	/* DTS2012042803609 gkf61766 20120428 end > */
	.show = update_firmware_show,
	.store = update_firmware_store,
};
#endif /* TPD_UPDATE_FIRMWARE */

/* < DTS2012031404176  linghai 20120314 begin */
#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT]=TPD_KEYS;
static int tpd_keys_dim_local_wvga[TPD_KEY_COUNT][4]=TPD_KEYS_DIM_WVGA;
#endif
/* DTS2012031404176  linghai 20120314 end> */
//add by huxin
#ifdef HAVE_TOUCH_KEY
const u16 touch_key_array[] = { KEY_BACK, KEY_HOMEPAGE, KEY_MENU};
#define MAX_KEY_NUM ( sizeof( touch_key_array )/sizeof( touch_key_array[0] ) )
#endif


#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

static struct point {
	int x;
	int raw_x;
	int y;
	int raw_y;
	int z;
	int status;
};

struct function_descriptor {
	__u8 queryBase;
	__u8 commandBase;
	__u8 controlBase;
	__u8 dataBase;
	__u8 intSrc;
#define FUNCTION_VERSION(x) ((x >> 5) & 3)
#define INTERRUPT_SOURCE_COUNT(x) (x & 7)

	__u8 functionNumber;
};

/* < DTS2012031404176  linghai 20120314 begin */
struct tpd_data {
	struct i2c_client *client;
	struct work_struct  work;
	struct point *cur_points;
	struct point *pre_points;
	u8 points_supported;
	u8 data_base;
	u8 data_length;
	u8 *data;
	int f11_max_x, f11_max_y;
};

static int ts_x_max = 0;
static int ts_y_max = 0;
static int lcd_x = 0;
static int lcd_y = 0;
static int jisuan = 0;
/* DTS2012031404176 linghai 20120314 end > */

extern struct tpd_device *tpd;
static struct tpd_data *ts = NULL;
static struct workqueue_struct *mtk_tpd_wq;
static struct function_descriptor fd_01;
static struct function_descriptor fd_34;
static u8 boot_mode;

/* Register */
#define FD_ADDR_MAX    	0xE9
#define FD_ADDR_MIN    	0x05
#define FD_BYTE_COUNT 	6


/* Function extern */
static void tpd_eint_handler(void);
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En, kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void), kal_bool auto_umask);
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static void tpd_work_func(struct work_struct *work);
static int tpd_i2c_read_data(struct i2c_client *client, u8 command, char *buf, int count);
static int tpd_i2c_write_byte_data(struct i2c_client *client, u8 command, u8 data);
static void tpd_down(int x, int y, int p);
static void tpd_up(int x, int y);
static int tpd_power(struct i2c_client *client, int on);
static void tpd_clear_interrupt(struct i2c_client *client);

/* < DTS2012042504399  gkf61766 20120425 begin */
static u16 get_ic_name(void);
static u8 get_module_id(void);
static u8 get_config_version(void);
static char * get_touch_module_name(u8 module_id);
char * get_synaptics_touch_info(void);
char * get_touch_info(void);

#define BYD     1
#define CMI     2
#define TRULY   3
#define TPK     4
#define LENSONE 5
#define OFILM   6
#define EELY    7
#define SUCCESS 8
#define ALPS    9
/* DTS2012042504399  gkf61766 20120425 end> */

/* < DTS2012042803609 gkf61766 20120428 begin */
static int sync_debug_mask = 1;
#define TP_DBG(x...) do {\
    if (sync_debug_mask) \
        printk( x);\
    } while (0)
module_param_named(tp_debug, sync_debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
/* DTS2012042803609 gkf61766 20120428 end > */
static const struct i2c_device_id tpd_id[] = {{TPD_DEVICE,0},{}};
/* < DTS2012040603460 gkf61766 20120406 begin */
static unsigned short force[] = {0,0xE0,I2C_CLIENT_END,I2C_CLIENT_END}; 
/* < DTS2012040603460 gkf61766 20120406 end */
static const unsigned short * const forces[] = { force, NULL };
//static struct i2c_client_address_data addr_data = { .forces = forces, };
static struct i2c_board_info __initdata i2c_tpd={ I2C_BOARD_INFO("mtk-tpd", (0xE0>>1))};


static struct i2c_driver tpd_i2c_driver = {
	.driver = {
		.name = TPD_DEVICE,
		.owner = THIS_MODULE,
	},
	.probe = tpd_probe,
	.remove = __devexit_p(tpd_remove),
	.id_table = tpd_id,
	.detect = tpd_detect,
	.address_list = (const unsigned short*) forces,  
	//.address_data = &addr_data,
};

static int tpd_i2c_read_data(struct i2c_client *client, u8 command, char *buf, int count)
{
	int ret = 0;
	u16 old_flag = client->addr;
/* < DTS2012042504399  gkf61766 20120425 begin */
	client->addr = client->addr & I2C_MASK_FLAG ;
	client->ext_flag =client->ext_flag | I2C_WR_FLAG | I2C_RS_FLAG | I2C_ENEXT_FLAG;
/* DTS2012042504399  gkf61766 20120425 end > */
	buf[0] = command;
	ret = i2c_master_send(client, buf, (count << 8 | 1));
	
	client->addr = old_flag;
	return ret;
}
static int tpd_i2c_write_byte_data(struct i2c_client *client, u8 command, u8 data)
{
	int ret = 0;
	u8 buf[2] = {command, data};
    client->ext_flag =0;
	ret = i2c_master_send(client, buf, 2);
	return ret;
}

static void tpd_down(int x, int y, int p)
{
	input_report_abs(tpd->dev, ABS_PRESSURE, p);
	input_report_key(tpd->dev, BTN_TOUCH, 1);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(tpd->dev);

	#ifdef TPD_HAVE_BUTTON
	/*BEGIN PN: DTS2012051505359 ,modified by s00179437 , 2012-05-31*/
	if (FACTORY_BOOT == boot_mode || RECOVERY_BOOT == boot_mode)
	/*END PN: DTS2012051505359 ,modified by s00179437 , 2012-05-31*/
	{   
		tpd_button(x, y, 1);  
	}	
	#endif
	/* < DTS2012042803609 gkf61766 20120428 begin */
	TP_DBG("=================>D---[%4d %4d %4d]\n", x, y, p);
	/* DTS2012042803609 gkf61766 20120428 end > */
	TPD_DOWN_DEBUG_TRACK(x,y);
}
 
static void tpd_up(int x, int y)
{
	//input_report_abs(tpd->dev, ABS_PRESSURE, 0);
	input_report_key(tpd->dev, BTN_TOUCH, 0);
	//input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
	//input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	//input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(tpd->dev);

	#ifdef TPD_HAVE_BUTTON
	/*BEGIN PN: DTS2012051505359 ,modified by s00179437 , 2012-05-31*/
	if (FACTORY_BOOT == boot_mode || RECOVERY_BOOT == boot_mode)
	/*END PN: DTS2012051505359 ,modified by s00179437 , 2012-05-31*/
	{   
		tpd_button(x, y, 0); 
	}   
	#endif
	/* < DTS2012042803609 gkf61766 20120428 begin */
	TP_DBG("==================>U---[%4d %4d %4d]\n", x, y, 0);
	/* DTS2012042803609 gkf61766 20120428 end > */
	TPD_UP_DEBUG_TRACK(x,y);
}


static void tpd_work_func(struct work_struct *work)
{
	u8 i = 0 ;
	u8 finger_status = 0;
	u8 finger_status_reg = 0;
	u8 loop = ts->data_length / 8;
	u8 fsr_len = (ts->points_supported + 3) / 4;
	u8 *pdata = ts->data;
	u8 *finger_reg = NULL;
	struct point *ppt = NULL;

	tpd_clear_interrupt(ts->client);

	for (i = 0; i < loop; i++) {
		tpd_i2c_read_data(ts->client, ts->data_base + 8*i, ts->data + 8*i, 8);
	}

	if (ts->data_length % 8) {
		tpd_i2c_read_data(ts->client, ts->data_base + 8*i, ts->data + 8*i, ts->data_length % 8);
	}
	
	for (i = 0; i < ts->points_supported; i++) {
		if (!(i % 4))
			finger_status_reg = pdata[i / 4];

		finger_status = (finger_status_reg >> ((i % 4) * 2)) & 3;

		ppt = &ts->cur_points[i];
		ppt->status = finger_status;

		/* < DTS2012040200621  gaofeng 20120412 begin */
		if (0x01==finger_status) {
		/* DTS2012040200621  gaofeng 20120412 end > */
			finger_reg = &pdata[fsr_len + 5 * i];
			
			ppt->raw_x = ppt->x = (finger_reg[0] << 4) | (finger_reg[2] & 0x0F);
			ppt->raw_y = ppt->y = (finger_reg[1] << 4) | ((finger_reg[2] >> 4) & 0x0F);
			ppt->z = finger_reg[4];

			/* < DTS2012031404176  linghai 20120314 begin */
			#ifdef TPD_HAVE_CALIBRATION
			//tpd_calibrate(&ppt->x, &ppt->y);
			#endif
          //  TP_DBG("========>[%4d %4d]\n",ppt->raw_x,ppt->raw_y);	
			ppt->x = ppt->raw_x * lcd_x /ts_x_max;
			ppt->y = ppt->raw_y* lcd_y / 1970;
			/* DTS2012031404176  linghai 20120314 end > */
			tpd_down(ppt->x, ppt->y, ppt->z);
			TPD_EM_PRINT(ppt->raw_x, ppt->raw_y, ppt->x, ppt->y, ppt->z, 1);
			
		} else {
			ppt = &ts->pre_points[i];
			if (ppt->status) {
				//printk("finger [%d] status [%d]  ", i, ppt->status);
				tpd_up(ppt->x, ppt->y);
				TPD_EM_PRINT(ppt->raw_x, ppt->raw_y, ppt->x, ppt->y, ppt->z, 0);
			}
		}
	}
//add by huxin
#ifdef HAVE_TOUCH_KEY
   u8  Button = 0; 
   tpd_i2c_read_data(ts->client,0x0200,&Button,1);
   TP_DBG("======>hx read button!!!!\n");
   if (Button)
   {
       for (i = 0; i < MAX_KEY_NUM; i++)
       {
           input_report_key(tpd->dev, touch_key_array[i], Button & (0x01 << i));
		   TP_DBG("======>hx report button[%4d]\n",Button & (0x01 << i));
       }
     
   }
#endif
	input_sync(tpd->dev);

	ppt = ts->pre_points;
	ts->pre_points = ts->cur_points;
	ts->cur_points = ppt;
}

static void tpd_eint_handler(void)
{
	TPD_DEBUG("TPD interrupt has been triggered\n");
	queue_work(mtk_tpd_wq, &ts->work);
}

static int tpd_power(struct i2c_client *client, int on)
{
	int ret = 0;
	//int i = 0;
	//u8 temp = 0;

	if (on)
    {       
        ret = tpd_i2c_write_byte_data(client, fd_01.controlBase, 0x80);/*sensor on*/	
		if (ret < 0)
        {
        	TPD_DMESG("Error sensor can not wake up\n");
			return ret;
        }      
		
        ret = tpd_i2c_write_byte_data(client, fd_01.commandBase, 0x01);/*touchscreen reset*/
        if (ret < 0)
        {
            TPD_DMESG("Error chip can not reset\n");
			return ret;
        }

        msleep(200); /* wait for device reset; */
	}
	else 
    {
		ret = tpd_i2c_write_byte_data(client, fd_01.controlBase, 0x81); /* set touchscreen to deep sleep mode*/
		if (ret < 0)
        {
            TPD_DMESG("Error touch can not enter very-low power state\n");
			return ret;
        }      
	}

	return ret;	
}

static int tpd_detect (struct i2c_client *client, int kind, struct i2c_board_info *info) 
{
	strcpy(info->type, TPD_DEVICE);    
	return 0;
}

static int __devexit tpd_remove(struct i2c_client *client)
{
	TPD_DEBUG("TPD removed\n");
	return 0;
}

static void tpd_clear_interrupt(struct i2c_client *client)
{
	int i = 0;
	for( i = 5; i >0 ; i--)
    {
        if(i2c_smbus_read_byte_data(ts->client, 0x14) >= 0)
        {
            break;
        }
    }
}

static int tpd_rmi4_read_pdt(struct tpd_data *ts)
{
	int ret = 0;
	int nFd = 0;
	u8 fd_reg = 0;
	/* < DTS2012031404176  linghai 20120314 begin */
	u8 query[14];
	struct function_descriptor fd;
	int count;
	/* DTS2012031404176  linghai 20120314 end > */
	
	for (fd_reg = FD_ADDR_MAX; fd_reg >= FD_ADDR_MIN; fd_reg -= FD_BYTE_COUNT)     
    {
		ret = tpd_i2c_read_data(ts->client, fd_reg, &fd, FD_BYTE_COUNT);
		if (ret < 0) {
			TPD_DMESG("Error I2C read failed querying RMI4 $%02X capabilities\n", ts->client->addr);
			return ret;
		}
		if (!fd.functionNumber) 
        {
			/* End of PDT */
			ret = nFd;
			TPD_DMESG("Error Read %d functions from PDT\n", fd.functionNumber);
			break;
		}

		++nFd;

		switch (fd.functionNumber) {
            case 0x34:
                fd_34.queryBase = fd.queryBase;
                fd_34.dataBase = fd.dataBase;
				/* < DTS2012042504399  gkf61766 20120425 begin */
                fd_34.controlBase = fd.controlBase;
				/* DTS2012042504399  gkf61766 20120425 end > */
                break;
				
			case 0x01: /* Interrupt */
                fd_01.queryBase = fd.queryBase;
                fd_01.dataBase = fd.dataBase;
                fd_01.commandBase = fd.commandBase;
                fd_01.controlBase = fd.controlBase;
				break;
				
			case 0x11: /* 2D */
				/* < DTS2012031404176  linghai 20120314 begin */
				ts->data_base = fd.dataBase;
			//	TPD_DMESG("hx tpd Begin Read 0x11 queryBase data");
				ret = tpd_i2c_read_data(ts->client, fd.queryBase+1, &query[1], 1);
			//	TPD_DMESG("hx tpd End Read 0x11 queryBase data");
				if (ret < 0) {
					TPD_DMESG("Error reading F11 query registers\n");
				}

				ts->points_supported = (query[1] & 7) + 1;
				if (ts->points_supported == 6)
					ts->points_supported = 10;

				for(count =6;count<=9;count++)
				{
				    ret = tpd_i2c_read_data(ts->client, fd.controlBase+count, &query[count], 1);
			//		TPD_DMESG("hx tpd end read controlBase!");
				    if (ret < 0) {
				    TPD_DMESG("Error reading F11 query registers\n");
				    }
					
				}

				ts->f11_max_x = ((query[7] & 0x0f) * 0x100) | query[6];
				ts->f11_max_y = ((query[9] & 0x0f) * 0x100) | query[8];
				/* DTS2012031404176  linghai 20120314 end > */
				
				ts->pre_points = kzalloc(ts->points_supported * sizeof(struct point), GFP_KERNEL);
				if (ts->pre_points == NULL) {
			        TPD_DMESG("Error zalloc failed!\n");
					ret = -ENOMEM;
					return ret;
				}

				ts->cur_points = kzalloc(ts->points_supported * sizeof(struct point), GFP_KERNEL);
				if (ts->cur_points == NULL) {
			        TPD_DMESG("Error zalloc failed!\n");
					ret = -ENOMEM;
					return ret;
				}

				ts->data_length = ((ts->points_supported + 3) / 4) + 5 * ts->points_supported;
				ts->data = kzalloc(ts->data_length, GFP_KERNEL);
				if (ts->data == NULL) {
			        TPD_DMESG("Error zalloc failed!\n");
					ret = -ENOMEM;
					return ret;
				}

				TPD_DEBUG("%d fingers\n", ts->points_supported);
				break;
				
 			case 0x30: /* GPIO */
				/*ts->hasF30 = true;

				ts->f30.data_offset = fd.dataBase;
				ts->f30.interrupt_offset = interruptCount / 8;
				ts->f30.interrupt_mask = ((1 < INTERRUPT_SOURCE_COUNT(fd.intSrc)) - 1) << (interruptCount % 8);

				ret = i2c_transfer(ts->client->adapter, query_i2c_msg, 2);
				if (ret < 0)
					printk(KERN_ERR "Error reading F30 query registers\n");


				ts->f30.points_supported = query[1] & 0x1F;
				ts->f30.data_length = data_length = (ts->f30.points_supported + 7) / 8;*/

				break;
			default:
				break;
		}
	}

	return ret;
}

/* < DTS2012040603460 gkf61766 20120406 begin */
static int tpd_checkdevice(struct i2c_client *client)
{
	int num = 0;
	int ret = 0;
//	hw_product_type boardType ;
	
//	boardType = get_hardware_product_version();
//	if((boardType & HW_VER_MAIN_MASK) != HW_T8830_VER)
//	{
//		TPD_DMESG("=====>>>>machine is not 8830,ret: %d\n",boardType);
//		return 	 -ENOMEM;
//	}
	ret = tpd_i2c_read_data(ts->client, 0x00, &num, 1);
	if(ret < 0 )
	{
		TPD_DEBUG( "tpd_work_func: i2c failed\n");
		return  -ENOMEM;
	}
	return 0;
}
/* < DTS2012040603460 gkf61766 20120406 end */

static int tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int i;
	int ret = 0;  
	//char product_id[6] = {0};
    //add by huxin
	#ifdef HAVE_TOUCH_KEY
    s32 idx = 0;
    #endif
	//add by huxin
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
	
#ifdef TPD_POWER_SOURCE_CUSTOM
    hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#else
    hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
    hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif  
	msleep(10);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(50);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(50);

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) 
    {
        TPD_DMESG("Error zalloc failed!\n");
        ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	ts->client = client;
	i2c_set_clientdata(client, ts);
	
/* < DTS2012040603460 gkf61766 20120406 begin */
	ret = tpd_checkdevice(ts->client);
	if(ret<0)
	{
		TPD_DMESG("tm1896 check device  error\n");
		goto err_tpd_checkdevice_faild;
	}
/* < DTS2012040603460 gkf61766 20120406 end */

	ret = tpd_rmi4_read_pdt(ts);
	if (ret <= 0) {
		if (ret == 0)
			TPD_DMESG("Empty PDT\n");

		TPD_DMESG("Error identifying device (%d)\n", ret);
		ret = -ENODEV;
		goto err_pdt_read_failed;
	}
	
	/* < DTS2012031404176  linghai 20120314 begin */
	ts_x_max =ts->f11_max_x;
	ts_y_max =ts->f11_max_y;
	
 	//if(machine_is_T8830())
	//{
		lcd_x=LCD_X_WVGA;
		lcd_y=LCD_Y_WVGA;
		//jisuan=LCD_JS_WVGA;
	//}
	/* DTS2012031404176  linghai 20120314 end > */

	ret = tpd_power(client, 1);
    if (ret < 0) 
    {
        TPD_DMESG("Error poweron failed\n");
        goto err_power_on_failed;
    }

	tpd_clear_interrupt(client);
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
    {
		TPD_DMESG("Error need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
	
    mtk_tpd_wq = create_singlethread_workqueue("mtk_tpd_wq");
    if (!mtk_tpd_wq)
    {
        TPD_DMESG("Error Could not create work queue mtk_tpd_wq: no memory");
        ret = -ENOMEM;
        goto error_wq_creat_failed; 
    }

	INIT_WORK(&ts->work, tpd_work_func);
	
#ifdef TPD_UPDATE_FIRMWARE
    /* < DTS2012042803609 gkf61766 20120428 begin */
    g_client = client;
    /* DTS2012042803609 gkf61766 20120428 end > */
    for (i = 0 ; i < 3; i++) 
    {
        ret= ts_firmware_file();   
        if (!ret)
        {
			break;
        }
    }
#endif /* TPD_UPDATE_FIRMWARE */
//add by huxin
#ifdef HAVE_TOUCH_KEY

    for (idx = 0; idx < MAX_KEY_NUM; idx++)
    {
        input_set_capability(tpd->dev, EV_KEY, touch_key_array[idx]);
    }

#endif

	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);

	mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_handler, 1); 
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);


	tpd_load_status = 1;
	TPD_DMESG("%s: Touch Panel Device Probe %s\n", __func__, (ret < 0) ? "FAIL" : "PASS");

	return 0;

err_pdt_read_failed:
err_check_functionality_failed:
err_power_on_failed:
error_wq_creat_failed:	
/* < DTS2012040603460 gkf61766 20120406 begin */
err_tpd_checkdevice_faild:	
err_alloc_data_failed:
    if(NULL != ts)
    {
        kfree(ts);
    }
#ifdef TPD_POWER_SOURCE_CUSTOM
    hwPowerDown(TPD_POWER_SOURCE_CUSTOM,  "TP");
#else
    hwPowerDown(MT65XX_POWER_LDO_VGP2,  "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
    hwPowerDown(TPD_POWER_SOURCE_1800,  "TP");
#endif  
/* < DTS2012040603460 gkf61766 20120406 end */
	return ret;
}

#ifdef TPD_UPDATE_FIRMWARE
struct RMI4_FDT{
    unsigned char m_QueryBase;
    unsigned char m_CommandBase;
    unsigned char m_ControlBase;
    unsigned char m_DataBase;
    unsigned char m_IntSourceCount;
    unsigned char m_ID;
};

static int RMI4_read_PDT(struct i2c_client *client)
{
	// Read config data
	struct RMI4_FDT temp_buf;
	struct RMI4_FDT m_PdtF34Flash;
	struct RMI4_FDT m_PdtF01Common;
	/* < DTS2012042504399  gkf61766 20120425 begin */
	//delete one line
	/* DTS2012042504399  gkf61766 20120425 end > */
	unsigned short start_addr; 
	int ret = 0;

	memset(&m_PdtF34Flash,0,sizeof(struct RMI4_FDT));
	memset(&m_PdtF01Common,0,sizeof(struct RMI4_FDT));

	for(start_addr = 0xe9; start_addr > 10; start_addr -= sizeof(struct RMI4_FDT))
	{
		/* < DTS2012042504399  gkf61766 20120425 begin */
		ret = tpd_i2c_read_data(ts->client, start_addr, &temp_buf, sizeof(struct RMI4_FDT));
		if (ret < 0) {
			TPD_DMESG("Error I2C read failed querying RMI4 $%02X capabilities\n", ts->client->addr);
			return ret;
		}
		/* DTS2012042504399  gkf61766 20120425 end > */

		if(temp_buf.m_ID == 0x34)
		{
			memcpy(&m_PdtF34Flash,&temp_buf,sizeof(struct RMI4_FDT ));
		}
		else if(temp_buf.m_ID == 0x01)
		{
			memcpy(&m_PdtF01Common,&temp_buf,sizeof(struct RMI4_FDT ));
		}
		else if (temp_buf.m_ID == 0)  //end of PDT
		{		
			break;
		}
	  }

	if((m_PdtF01Common.m_CommandBase != fd_01.commandBase) || (m_PdtF34Flash.m_QueryBase != fd_34.queryBase))
	{
		printk("%s:%d: RIM4 PDT has changed!!!\n",__FUNCTION__,__LINE__);
		
		ret = tpd_rmi4_read_pdt(ts);
		if(ret < 0)
		{
			printk("read pdt error:!\n");
			return -1;
		}
		
		return 0;
	}

	return 0;

}

//to be improved .......
int RMI4_wait_attn_hw(struct i2c_client * client,int udleay)
{
	int loop_count=0;
	int ret=0;

	do{
		mdelay(udleay);
		ret = i2c_smbus_read_byte_data(client,fd_34.dataBase+18);//read Flash Control
		// Clear the attention assertion by reading the interrupt status register
		i2c_smbus_read_byte_data(client,fd_01.dataBase+1);//read the irq Interrupt Status
	}while(loop_count++ < 0x10 && (ret != 0x80));

	if(loop_count >= 0x10)
	{
		TPD_DEBUG("RMI4 wait attn timeout:ret=0x%x\n",ret);
		return -1;
	}
	return 0;
}

int RMI4_disable_program_hw(struct i2c_client *client)
{
	unsigned char cdata; 
	unsigned int loop_count=0;
  
	printk("RMI4 disable program...\n");
	// Issue a reset command
	i2c_smbus_write_byte_data(client,fd_01.commandBase,0x01);

	// Wait for ATTN to be asserted to see if device is in idle state
	RMI4_wait_attn_hw(client,20);

	// Read F01 Status flash prog, ensure the 6th bit is '0'
	do{
		cdata = i2c_smbus_read_byte_data(client,fd_01.dataBase);
		udelay(2);
	} while(((cdata & 0x40) != 0) && (loop_count++ < 10));

	//Rescan the Page Description Table
	return RMI4_read_PDT(client);
}

static int RMI4_enable_program(struct i2c_client *client)
{
	unsigned short bootloader_id = 0 ;
	int ret = -1;
	printk("RMI4 enable program...\n");
	 // Read and write bootload ID
	bootloader_id = i2c_smbus_read_word_data(client,fd_34.queryBase);
	i2c_smbus_write_word_data(client,fd_34.dataBase+2,bootloader_id);//write Block Data 0

	  // Issue Enable flash command
	if(i2c_smbus_write_byte_data(client, fd_34.dataBase+18, 0x0F) < 0) //write Flash Control
	{
		TPD_DEBUG("RMI enter flash mode error\n");
		return -1;
	}
	ret = RMI4_wait_attn_hw(client,12);

	//Rescan the Page Description Table
	RMI4_read_PDT(client);
	return ret;
}

static unsigned long ExtractLongFromHeader(const unsigned char* SynaImage) 
{
  	return((unsigned long)SynaImage[0] +
         (unsigned long)SynaImage[1]*0x100 +
         (unsigned long)SynaImage[2]*0x10000 +
         (unsigned long)SynaImage[3]*0x1000000);
}

static int RMI4_check_firmware(struct i2c_client *client,const unsigned char *pgm_data)
{
	unsigned long checkSumCode;
	unsigned long m_firmwareImgSize;
	unsigned long m_configImgSize;
	unsigned short m_bootloadImgID; 
	unsigned short bootloader_id;
	const unsigned char *SynaFirmware;
	unsigned char m_firmwareImgVersion;
	unsigned short UI_block_count;
	unsigned short CONF_block_count;
	unsigned short fw_block_size;

  	SynaFirmware = pgm_data;
	checkSumCode = ExtractLongFromHeader(&(SynaFirmware[0]));
	m_bootloadImgID = (unsigned int)SynaFirmware[4] + (unsigned int)SynaFirmware[5]*0x100;
	m_firmwareImgVersion = SynaFirmware[7];
	m_firmwareImgSize    = ExtractLongFromHeader(&(SynaFirmware[8]));
	m_configImgSize      = ExtractLongFromHeader(&(SynaFirmware[12]));
 
	UI_block_count  = i2c_smbus_read_word_data(client,fd_34.queryBase+5);//read Firmware Block Count 0
	fw_block_size = i2c_smbus_read_word_data(client,fd_34.queryBase+3);//read Block Size 0
	CONF_block_count = i2c_smbus_read_word_data(client,fd_34.queryBase+7);//read Configuration Block Count 0
	bootloader_id = i2c_smbus_read_word_data(client,fd_34.queryBase);

	return (m_firmwareImgVersion != 0 || bootloader_id == m_bootloadImgID) ? 0 : -1;

}


static int RMI4_write_image(struct i2c_client *client,unsigned char type_cmd,const unsigned char *pgm_data)
{
	unsigned short block_size;
	unsigned short img_blocks;
	unsigned short block_index;
	const unsigned char * p_data;
	int i;

	block_size = i2c_smbus_read_word_data(client,fd_34.queryBase+3);//read Block Size 0
	
	switch(type_cmd )
	{
		case 0x02:
			img_blocks = i2c_smbus_read_word_data(client,fd_34.queryBase+5);	//read UI Firmware
			break;
		case 0x06:
			img_blocks = i2c_smbus_read_word_data(client,fd_34.queryBase+7);	//read Configuration Block Count 0	
			break;
		default:
			TPD_DEBUG("image type error\n");
			goto error;
	}

	p_data = pgm_data;
	
	for(block_index = 0; block_index < img_blocks; ++block_index)
	{
		printk("#");
		// Write Block Number
		if(i2c_smbus_write_word_data(client, fd_34.dataBase,block_index) < 0)
		{
			TPD_DEBUG("write block number error\n");
			goto error;
		}

		for(i=0;i<block_size;i++)
		{
			if(i2c_smbus_write_byte_data(client, fd_34.dataBase+2+i, *(p_data+i)) < 0) //write Block Data
			{
				TPD_DEBUG("RMI4_write_image: block %d data 0x%x error\n",block_index,*p_data);
				goto error;
			}
			udelay(15);
		}
		
		p_data += block_size;	

		// Issue Write Firmware or configuration Block command
		if(i2c_smbus_write_word_data(client, fd_34.dataBase+18, type_cmd) < 0) //write Flash Control
		{
			TPD_DEBUG("issue write command error\n");
			goto error;
		}

		// Wait ATTN. Read Flash Command register and check error
		if(RMI4_wait_attn_hw(client,5) != 0)
		{
			goto error;
		}
	}

	return 0;
error:
	return -1;
}


static int RMI4_program_configuration(struct i2c_client *client,const unsigned char *pgm_data )
{
	int ret;
	unsigned short block_size;
	unsigned short ui_blocks;

	printk("\nRMI4 program Config firmware...\n");
	block_size = i2c_smbus_read_word_data(client,fd_34.queryBase+3);//read Block Size 0
	ui_blocks = i2c_smbus_read_word_data(client,fd_34.queryBase+5);	//read Firmware Block Count 0

	if(RMI4_write_image(client, 0x06,pgm_data+ui_blocks*block_size ) < 0)
	{
		TPD_DEBUG("write configure image error\n");
		return -1;
	}
	
	ret = i2c_smbus_read_byte_data(client,fd_34.dataBase+18);	//read Flash Control
	return ((ret & 0xF0) == 0x80 ? 0 : ret);
}

static int RMI4_program_firmware(struct i2c_client *client,const unsigned char *pgm_data)
{
	int ret=0;
	unsigned short bootloader_id;

	printk("RMI4 program UI firmware...\n");

	//read and write back bootloader ID
	bootloader_id = i2c_smbus_read_word_data(client,fd_34.queryBase);
	i2c_smbus_write_word_data(client,fd_34.dataBase+2, bootloader_id ); //write Block Data0

	//issue erase commander
	if(i2c_smbus_write_byte_data(client, fd_34.dataBase+18, 0x03) < 0) //write Flash Control
	{
		TPD_DEBUG("RMI4_program_firmware error, erase firmware error \n");
		return -1;
	}
	RMI4_wait_attn_hw(client,300);

	//check status
	if((ret = i2c_smbus_read_byte_data(client,fd_34.dataBase+18)) != 0x80) //check Flash Control
	{
		return -1;
	}

	//write firmware
	if( RMI4_write_image(client,0x02,pgm_data) <0 )
	{
		TPD_DEBUG("write UI firmware error!\n");
		return -1;
	}

	ret = i2c_smbus_read_byte_data(client,fd_34.dataBase+18); //read Flash Control
	return ((ret & 0xF0) == 0x80 ? 0 : ret);
}

static int synaptics_download(struct i2c_client *client,const unsigned char *pgm_data)
{
	int ret;

	ret = RMI4_read_PDT(client);
	if(ret != 0)
	{
		printk("RMI page func check error\n");
		return -1;
	}

	ret = RMI4_enable_program(client);
	if( ret != 0)
	{
		printk("%s:%d:RMI enable program error,return...\n",__FUNCTION__,__LINE__);
		goto error;
	}

	ret = RMI4_check_firmware(client,pgm_data);
	if( ret != 0)
	{
		printk("%s:%d:RMI check firmware error,return...\n",__FUNCTION__,__LINE__);
		goto error;
	}

	ret = RMI4_program_firmware(client, pgm_data + 0x100);
	if( ret != 0)
	{
		printk("%s:%d:RMI program firmware error,return...",__FUNCTION__,__LINE__);
		goto error;
	}

	RMI4_program_configuration(client, pgm_data +  0x100);
	return RMI4_disable_program_hw(client);

error:
	RMI4_disable_program_hw(client);
	printk("%s:%d:error,return ....",__FUNCTION__,__LINE__);
	return -1;

}

static int i2c_update_firmware(struct i2c_client *client) 
{
	char *buf;
	struct file	*filp;
	struct inode *inode = NULL;
	mm_segment_t oldfs;
	uint16_t	length;
	int ret = 0;
	
	/* < DTS2012042803609 gkf61766 20120428 begin */
	char * module_name = NULL;
	u8 module_id=0;
	const char *filename_TPK="/sdcard/update/synaptics/TPK/synaptics.img";
	const char *filename_OFILM="/sdcard/update/synaptics/OFILM/synaptics.img";
	char *filename=NULL;
	module_id = get_module_id();
	if(TPK==module_id)
	{
		filename = filename_TPK;
	}else if(OFILM==module_id)
	{
		filename = filename_OFILM;
	}else
	{
		printk("==>%s: madule name is error : %s id:%d\n",__FUNCTION__,module_name,module_id);
		return -1;
	}
	TP_DBG("=============>filename :%s\n",filename);
	/* DTS2012042803609 gkf61766 20120428 end > */
	/* open file */
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	filp = filp_open(filename, O_RDONLY, S_IRUSR);
	if (IS_ERR(filp))
	{
            printk("%s: file %s filp_open error\n", __FUNCTION__,filename);
            set_fs(oldfs);
            return -1;
	}

	if (!filp->f_op)
	{
            printk("%s: File Operation Method Error\n", __FUNCTION__);
            filp_close(filp, NULL);
            set_fs(oldfs);
            return -1;
	}

    inode = filp->f_path.dentry->d_inode;
    if (!inode) 
    {
        printk("%s: Get inode from filp failed\n", __FUNCTION__);
        filp_close(filp, NULL);
        set_fs(oldfs);
        return -1;
    }

    /* file's size */
    length = i_size_read(inode->i_mapping->host);
    if (!( length > 0 && length < 62*1024 ))
    {
        printk("file size error\n");
        filp_close(filp, NULL);
        set_fs(oldfs);
        return -1;
    }

	/* allocation buff size */
	buf = vmalloc(length+(length%2));		/* buf size if even */
	if (!buf) 
	{
		printk("alloctation memory failed\n");
		filp_close(filp, NULL);
		set_fs(oldfs);
		return -1;
	}

    /* read data */
    if (filp->f_op->read(filp, buf, length, &filp->f_pos) != length)
    {
        printk("%s: file read error\n", __FUNCTION__);
        filp_close(filp, NULL);
        set_fs(oldfs);
        vfree(buf);
        return -1;
    }

	ret = synaptics_download(client,buf);

 	filp_close(filp, NULL);
	set_fs(oldfs);
	vfree(buf);
	return ret;
}

static int ts_firmware_file(void)
{
	int ret;
	struct kobject *kobject_ts;
	kobject_ts = kobject_create_and_add("touch_screen", NULL);
	if (!kobject_ts)
	{
		printk("create kobjetct error!\n");
		return -1;
	}
	
	ret = sysfs_create_file(kobject_ts, &update_firmware_attribute.attr);
	if (ret) {
		kobject_put(kobject_ts);
		printk("create file error\n");
		return -1;
	}
	return 0;	
}
/* < DTS2012042504399  gkf61766 20120425 begin */
static u16 get_ic_name(void)
{
	char ic_name_buffer[2];
	int ret;
	u8 addr = fd_01.queryBase+17;
	u16 touch_ic_name =0;
		ret = tpd_i2c_read_data(ts->client, addr, &ic_name_buffer,sizeof(ic_name_buffer));
		if (ret < 0) {
			TPD_DMESG("Error I2C read failed querying RMI4 $%02X capabilities\n", ts->client->addr);
			return ret;
		}
	touch_ic_name = ic_name_buffer[1] * 0x100 + ic_name_buffer[0];
	return touch_ic_name;
}


static u8 get_module_id(void)
{
	char productid[11];
	int ret ;
	int i;
	u32 module_id = 0;
	u8 querybase = 0;
	RMI4_enable_program(ts->client);
	/* refresh the queryBase address after entering bootloader mode */
	querybase = fd_01.queryBase + 11;
	ret=i2c_smbus_read_byte_data(ts->client, querybase+9);
	RMI4_disable_program_hw(ts->client);
	module_id = ret-48;
	//	pr_info("===========>module_id :%d\n",module_id);
	return (u8)module_id;
}


static u8 get_config_version(void)
{
	char configver[5];
	int ret ;
	u8 config_ver = 0;
	tpd_i2c_read_data(ts->client, fd_34.controlBase, configver, 4);
	config_ver = (((configver[0]-48)*10+(configver[1]-48)*10)+(configver[2]-48)*10)+(configver[3]-48);
	return config_ver;
}

static char * get_touch_module_name(u8 module_id)
{
	switch(module_id)
	{
		case BYD:
			return "BYD";
		case CMI:
			return "CMI";
		case TRULY:
			return "TRULY";
		case TPK:
			return "TPK";
		case LENSONE:
			return "LENSONE";
		case OFILM:
			return "OFILM";
		case EELY:
			return "EELY";
		case SUCCESS:
			return "SUCCESS";
		case ALPS:
			return "ALPS";
		default:
			return "unknow";
	}

	return NULL;
}
/* named Rule
 * 2202 3200 : syanptics-IC-Module.ver
 * for example: syanptics-3200-tpk.2
 *
 * 2000 2100 3000 :syanptics-Module.ver
 * for example: syanptics-tpk.2
 */
char * get_synaptics_touch_info(void)
{
	u32 config_id = 0;
	char * module_name = NULL;

	u16 touch_ic_name = 0;
	u8 module_id=0;
	u8 config_v = 0;
	static char touch_info[50] = {0};
	
	config_v = get_config_version();
	module_id = get_module_id();
	touch_ic_name=get_ic_name();
	module_name = get_touch_module_name(module_id);
	if (module_name == NULL)
	{
		return NULL;
	}
	if (touch_ic_name == 2202)
	{
		config_id = config_v;
		sprintf(touch_info,"synaptics-2202-%s.%d",module_name,config_id);		
	}
	else if (touch_ic_name == 3200)
	{
		config_id = config_v;
		sprintf(touch_info,"synaptics-3200-%s.%d",module_name,config_id);	
	}
	else
	{
		config_id = config_v;
		sprintf(touch_info,"synaptics-%s.%d",module_name,config_id);	
	}

	return touch_info;
}
/* get touch info */
char * get_touch_info(void)
{
	char * touch_info = NULL;

	touch_info = get_synaptics_touch_info();
	if (touch_info != NULL)
	{
		return touch_info;
	}
	return NULL;
	
}

/* DTS2012042504399  gkf61766 20120425 end > */
/*
 * The "update_firmware" file where a static variable is read from and written to.
 */
static ssize_t update_firmware_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf)
{
	/* < DTS2012042504399  gkf61766 20120425 begin */
	char * touch_info =NULL;
	touch_info=get_touch_info();
	return sprintf(buf,"%s\n",touch_info);
	/* DTS2012042504399  gkf61766 20120425 end > */
}

static ssize_t update_firmware_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	/* < DTS2012042803609 gkf61766 20120428 begin */
	int ret = -1;
	/* DTS2012042803609 gkf61766 20120428 end > */

	printk("#################update_firmware_store######################\n");
	if ( buf[0] == '2')
	{
		/* driver detect its device  */
		ret = i2c_smbus_read_byte_data(g_client, fd_01.queryBase);
		printk("The if of synaptics device is : %d\n",ret);
		/* < DTS2012042803609 gkf61766 20120428 begin */
		mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
		/* DTS2012042803609 gkf61766 20120428 end > */
		/*update firmware*/
		ret = i2c_update_firmware(g_client);
		/* < DTS2012042803609 gkf61766 20120428 begin */
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		/* DTS2012042803609 gkf61766 20120428 end > */
 
		if( 0 != ret )
		{
			printk("Update firmware failed!\n");
			ret = -1;
		} 
		else 
		{
			printk("Update firmware success!\n");
			//arm_pm_restart(0,&ret);
			ret = 1;
		}
	}
	
	return ret;
 }
#endif 

static int tpd_local_init(void)
{
	TPD_DMESG("TM1896 I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);
 
	if(i2c_add_driver(&tpd_i2c_driver)!=0)
	{
		TPD_DMESG("Error unable to add i2c driver.\n");
		return -1;
	}
   
/* < DTS2012031404176  linghai 20120314 begin */   
#ifdef TPD_HAVE_BUTTON     
//	if(machine_is_T8830())
//	{		
		 tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local_wvga);
//	}	
#endif   
  
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif 

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    memcpy(tpd_calmat, tpd_def_calmat_local, 8*4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);	
#endif  

	boot_mode = get_boot_mode();
	if (boot_mode == 3) boot_mode = NORMAL_BOOT;

	TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
	tpd_type_cap = 1;
    return 0; 
 }

static int tpd_resume(struct i2c_client *client)
{
	TPD_DEBUG("TPD wake up\n");

/*
#ifdef TPD_CLOSE_POWER_IN_SLEEP	
	hwPowerOn(TPD_POWER_SOURCE,VOL_3300,"TP"); 
#else
#ifdef MT6573
	mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif	
	msleep(100);

	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(1);  
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
#endif
*/
   	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(50);  
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	
	tpd_power(ts->client, 1);
	tpd_clear_interrupt(ts->client);

	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
	
	return 0;
}
 
static int tpd_suspend(struct i2c_client *client, pm_message_t message)
{
	TPD_DEBUG("TPD enter sleep\n");
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
   
	tpd_power(ts->client, 0);

/*	 
#ifdef TPD_CLOSE_POWER_IN_SLEEP	
	hwPowerDown(TPD_POWER_SOURCE,"TP");
#else
i2c_smbus_write_i2c_block_data(i2c_client, 0xA5, 1, &data);  //TP enter sleep mode
#ifdef MT6573
mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
#endif

#endif
*/
	 return 0;
 } 


static struct tpd_driver_t tpd_device_driver = {
	.tpd_device_name = "tm1896",
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
	.tpd_have_button = 1,
#else
	.tpd_have_button = 0,
#endif		
};

static int __init tpd_driver_init(void)
{
	TPD_DMESG("TM1896 touch panel driver init\n");
	i2c_register_board_info(0, &i2c_tpd, 1);
	if(tpd_driver_add(&tpd_device_driver) < 0)
		TPD_DMESG("Error Add TM1896 driver failed\n");
	return 0;
}

static void __exit tpd_driver_exit(void)
{
	TPD_DMESG("TM1896 touch panel driver exit\n");
	tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

MODULE_DESCRIPTION("Mediatek TM1896 Driver");

/* XASP-360 linghai 20120626 end >*/
