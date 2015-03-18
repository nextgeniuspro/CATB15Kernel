/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>

#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#include <linux/module.h>
#include <linux/input.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>

//#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>

#include <linux/dma-mapping.h>
//#include <mach/mt6577_boot.h> /*Set your boot */
#include <linux/debugfs.h>



#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/ioctl.h>
#include <linux/miscdevice.h>

#include "cust_gpio_usage.h"
#include "tpd_custom_ektf2132.h"
#include "tpd.h"

#define selftest_Xmax 10000
#define selftest_Xmin 800
#define selftest_Ymax 10000
#define selftest_Ymin 800
#define selftest_Bmax 900
#define selftest_Bmin 350
#define mutualtest_max 1100
#define mutualtest_min 250

#define selftest_raw_file  "/data/selftest_raw_file.txt"
#define mutualtest_raw_file  "/data/mutualtest_raw_file.txt"

#define IAP_PORTION 1

#define CMD_S_PKT			0x52
#define CMD_R_PKT			0x53
#define CMD_W_PKT			0x54

//OY add for FW & SW img compare
#define Latest_FW_version	0x0100
//for debug
int EKTF2132_TP_DEBUG = 0;

extern struct tpd_device *tpd;
 
struct i2c_client *i2c_client = NULL;
struct task_struct *thread = NULL; 

//*******************************************************************************************
// Elan data struct and variable                                                            |
//*******************************************************************************************

static int elan_tp_x_resolution		= 0;
static int elan_tp_y_resolution		= 0;
static int elan_lcm_x_resolution	= 480;
static int elan_lcm_y_resolution	= 800;
static int elan_fw_id				= 0x00;
static int elan_fw_ver				= 0x00;
static int elan_bc_version			= 0x00;
int work_lock=0x00;
int power_lock=0x00;
static uint8_t elan_recovery   		= 0x00;
static uint8_t elan_i2c_data[3] 	= {0x15, 0x20, 0x21};/*I2C devices address*/  
static int elan_circuit_ver			= 0x01;
static int elan_page_size			= 132;
static int elan_page_num			= 249;
static int elan_ack_fail			= 0x00;
static int elan_ack_ok				= 0xAA;
static int elan_ack_rewrite			= 0x55;
static int elan_e_fd				= -1;


#define PAGERETRY  30
#define IAPRESTART 5
#define CMD_54001234	1

#define CUSTOMER_IOCTLID	0xA0
#define IOCTL_CIRCUIT_CHECK  _IOR(CUSTOMER_IOCTLID, 1, int)


static DECLARE_WAIT_QUEUE_HEAD(waiter);
 
static void tpd_eint_interrupt_handler(void);
 
#ifdef MT6577
	extern void mt65xx_eint_unmask(unsigned int line);
	extern void mt65xx_eint_mask(unsigned int line);
	extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
	extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
	extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
#endif

static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);

static int tpd_flag = 0;
static int point_num = 0;
static int max_point_num = 5;

//for checking auto firmware upgrade status
int firmware_check_flag = 0;

#define TPD_OK 0

//*******************************************************************************************
// i2c board info																			|
//*******************************************************************************************
static const struct i2c_device_id ektf2132_tpd_id[] = {{"ektf2132",0},{}};
static struct i2c_board_info __initdata ektf2132_i2c_tpd={ I2C_BOARD_INFO("ektf2132", 0x15)};
 
 
//*******************************************************************************************
// i2c driver object                                                                        |
//*******************************************************************************************
static struct i2c_driver tpd_i2c_driver = 
{
	.driver 	= {
		.name = "ektf2132",//.name = TPD_DEVICE,
		//.owner = THIS_MODULE,
  	},
	.probe 		= tpd_probe,
	.remove 	= __devexit_p(tpd_remove),
	.id_table 	= ektf2132_tpd_id,
	.detect 	= tpd_detect,
	//.address_data = &addr_data,
};

struct elan_ktf2k_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct workqueue_struct *elan_wq;
	struct work_struct work;
	struct early_suspend early_suspend;
	int intr_gpio;
// Firmware Information
	int fw_ver;
	int fw_id;
	int bc_ver;
	int x_resolution;
	int y_resolution;
// For Firmare Update 
	struct miscdevice firmware;
	struct hrtimer timer;
};

static struct elan_ktf2k_ts_data *private_ts;

static ssize_t elan_ktf2k_gpio_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = 0;
	struct elan_ktf2k_ts_data *ts = private_ts;

	//ret = gpio_get_value(ts->intr_gpio);
ret = mt_get_gpio_in(GPIO_CTP_EINT_PIN);
	printk(KERN_DEBUG "[ELAN]GPIO_TP_INT_N=%d\n", ts->intr_gpio);
	sprintf(buf, "GPIO_TP_INT_N=%d\n", ret);
	ret = strlen(buf) + 1;
	return ret;
}

static DEVICE_ATTR(gpio, S_IRUGO, elan_ktf2k_gpio_show, NULL);

static ssize_t elan_ktf2k_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct elan_ktf2k_ts_data *ts = private_ts;

	sprintf(buf, "%s_x%4.4x\n", "ELAN_KTF2132", ts->fw_ver);
	ret = strlen(buf) + 1;
	return ret;
}

static struct dentry *dent;

static void ektf2k_create_debugfs_entries(u8 *enable)
{    
	dent = debugfs_create_dir("ektf2k_ts", NULL);
	if (dent) {
		debugfs_create_u8("tp_debug_enable", 0644, dent, enable);      
	}
}

static void ektf2k_destroy_debugfs_entries(void)
{
	if (dent)
		debugfs_remove_recursive(dent);
}

#if IAP_PORTION
uint8_t ic_status=0x00;	//0:OK 1:master fail 2:slave fail
int update_progree=0;
uint8_t I2C_DATA[3] = {0x15, 0x20, 0x21};/*I2C devices address*/  
int is_OldBootCode = 0; // 0:new 1:old



/*The newest firmware, if update must be changed here*/
static uint8_t file_fw_data[] = {
//#include "fw_data.i"
//#include "eKTF2136_v08_VT58_V1688_fw_data.i"
//#include "eKTF2136_v08_VT58_V9999_fw_data.i"
#include "eKTF2136_v08_VT58_V0100_fw_data.i"
};


enum
{
	PageSize		= 132,
	PageNum		        = 249,
	ACK_Fail		= 0x00,
	ACK_OK			= 0xAA,
	ACK_REWRITE		= 0x55,
};

enum
{
	E_FD			= -1,
};
#endif


#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif

#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif

struct touch_info {
    int y[5];
    int x[5];
    int p[5];
    int id[5];
    int count;
};

//*******************************************************************************************
// Virtual key for GPIO                                                                     |
//*******************************************************************************************

static bool key_is_down 		= false;
static bool touch_is_down 		= false;
static bool key_is_detection	= false;
static int 	key_register 		= 0;
static int 	prev_key_register 	= 0;
//*******************************************************************************************
// Elan function delcation                                                                  |
//*******************************************************************************************

static int 	__hello_packet_handler(struct i2c_client *client);
static int 	__fw_packet_handler(struct i2c_client *client);
static void tpd_resume(struct early_suspend *h );
static void tpd_suspend(struct early_suspend *h );
static int 	elan_ktf2k_ts_rough_calibrate(struct i2c_client *client);
static int 	elan_ktf2k_ts_get_data(struct i2c_client *client, uint8_t *cmd, uint8_t *buf, size_t size);
static int 	IAPReset(struct i2c_client *client);
static int 	EnterISPMode(struct i2c_client *client, uint8_t  *isp_cmd);
static int 	CheckIapMode(void);
static int 	WritePage(uint8_t * szPage, int byte);
static int 	GetAckData(struct i2c_client *client);
//static void print_progress(int page, int ic_num, int j);
//static int Update_FW_One(struct i2c_client *client, int recovery);
#if IAP_PORTION
int Update_FW_One(/*struct file *filp,*/ struct i2c_client *client, int elan_recovery);
#endif
int elan_iap_open(struct inode *inode, struct file *filp);
int elan_iap_release(struct inode *inode, struct file *filp);
static ssize_t elan_iap_write(struct file *filp, const char *buff, size_t count, loff_t *offp);
ssize_t elan_iap_read(struct file *filp, char *buff, size_t count, loff_t *offp);
static long elan_iap_ioctl(/*struct inode *inode,*/ struct file *filp,unsigned int cmd, unsigned long arg);

//*******************************************************************************************
// Elan FW update by apk file                                                               |
//*******************************************************************************************

struct file_operations elan_touch_fops =
{
    .open           = elan_iap_open,
    .write          = elan_iap_write,
    .read           = elan_iap_read,
    .release        = elan_iap_release,
    .unlocked_ioctl = elan_iap_ioctl,
};

#define ELAN_IOCTLID	0xD0
#define IOCTL_I2C_SLAVE	_IOW(ELAN_IOCTLID,  1, int)
#define IOCTL_MAJOR_FW_VER  _IOR(ELAN_IOCTLID, 2, int)
#define IOCTL_MINOR_FW_VER  _IOR(ELAN_IOCTLID, 3, int)
#define IOCTL_RESET  _IOR(ELAN_IOCTLID, 4, int)
#define IOCTL_IAP_MODE_LOCK  _IOR(ELAN_IOCTLID, 5, int)
#define IOCTL_CHECK_RECOVERY_MODE  _IOR(ELAN_IOCTLID, 6, int)
#define IOCTL_FW_VER  _IOR(ELAN_IOCTLID, 7, int)
#define IOCTL_X_RESOLUTION  _IOR(ELAN_IOCTLID, 8, int)
#define IOCTL_Y_RESOLUTION  _IOR(ELAN_IOCTLID, 9, int)
#define IOCTL_FW_ID  _IOR(ELAN_IOCTLID, 10, int)
#define IOCTL_ROUGH_CALIBRATE  _IOR(ELAN_IOCTLID, 11, int)
#define IOCTL_IAP_MODE_UNLOCK  _IOR(ELAN_IOCTLID, 12, int)
#define IOCTL_I2C_INT  _IOR(ELAN_IOCTLID, 13, int)
#define IOCTL_RESUME  _IOR(ELAN_IOCTLID, 14, int)
#define IOCTL_POWER_LOCK  _IOR(ELAN_IOCTLID, 15, int)
#define IOCTL_POWER_UNLOCK  _IOR(ELAN_IOCTLID, 16, int)
#define IOCTL_BC_VER  _IOR(ELAN_IOCTLID, 18, int)

#if IAP_PORTION
#define IOCTL_FW_UPDATE  _IOR(ELAN_IOCTLID, 17, int)
#define IOCTL_GET_UPDATE_PROGREE	_IOR(CUSTOMER_IOCTLID,  2, int)
#endif

static int __fw_packet_handler(struct i2c_client *client)
{
	int rc;
	int major, minor;
	uint8_t cmd[] 		= {0x53, 0x00, 0x00, 0x01};/* Get Firmware Version*/
	uint8_t cmd_x[] 	= {0x53, 0x60, 0x00, 0x00}; /*Get x resolution*/
	uint8_t cmd_y[] 	= {0x53, 0x63, 0x00, 0x00}; /*Get y resolution*/
	uint8_t cmd_id[] 	= {0x53, 0xf0, 0x00, 0x01}; /*Get firmware ID*/
    uint8_t cmd_bc[] 	= {0x53, 0x01, 0x00, 0x01};/* Get BootCode Version*/
	uint8_t buf_recv[4] = {0};

	printk("[ELAN] %s: n",__func__);

	//Firmware Version
	rc = elan_ktf2k_ts_get_data(client, cmd, buf_recv, 4);
	if (rc < 0)
	{
		return rc;
	}

	major = ((buf_recv[1] & 0x0f) << 4) | ((buf_recv[2] & 0xf0) >> 4);
	minor = ((buf_recv[2] & 0x0f) << 4) | ((buf_recv[3] & 0xf0) >> 4);
	elan_fw_ver = major << 8 | minor;
	
	// Firmware ID
	rc = elan_ktf2k_ts_get_data(client, cmd_id, buf_recv, 4);
	if (rc < 0)
	{
		return rc;
	}
	major = ((buf_recv[1] & 0x0f) << 4) | ((buf_recv[2] & 0xf0) >> 4);
	minor = ((buf_recv[2] & 0x0f) << 4) | ((buf_recv[3] & 0xf0) >> 4);
	elan_fw_id = major << 8 | minor;

	// X Resolution
	rc = elan_ktf2k_ts_get_data(client, cmd_x, buf_recv, 4);
	if (rc < 0)
	{
		return rc;
	}
	minor = ((buf_recv[2])) | ((buf_recv[3] & 0xf0) << 4);
	elan_tp_x_resolution = minor;
	
	// Y Resolution	
	rc = elan_ktf2k_ts_get_data(client, cmd_y, buf_recv, 4);
	if (rc < 0)
	{
		return rc;
	}
	minor = ((buf_recv[2])) | ((buf_recv[3] & 0xf0) << 4);
	elan_tp_y_resolution = minor;

	// Bootcode version
	rc = elan_ktf2k_ts_get_data(client, cmd_bc, buf_recv, 4);
	if (rc < 0)
	{
		return rc;
	}
	major = ((buf_recv[1] & 0x0f) << 4) | ((buf_recv[2] & 0xf0) >> 4);
	minor = ((buf_recv[2] & 0x0f) << 4) | ((buf_recv[3] & 0xf0) >> 4);
	elan_bc_version = major << 8 | minor;
	
	printk( "[ELAN] %s: firmware version: 0x%4.4x\n",__func__, elan_fw_ver);
	printk( "[ELAN] %s: firmware ID: 0x%4.4x\n",__func__, elan_fw_id);
	printk( "[ELAN] %s: x resolution: %d, y resolution: %d\n",__func__, elan_tp_x_resolution, elan_tp_y_resolution);
	printk( "[ELAN] %s: bootcode version: 0x%4.4x\n",__func__, elan_bc_version);
	return 0;
}

static int elan_ktf2k_ts_rough_calibrate(struct i2c_client *client)
{
	uint8_t cmd[] = {0x54, 0x29, 0x00, 0x01};

	printk("[ELAN] %s: enter\n", __func__);

	if ((i2c_master_send(client, cmd, sizeof(cmd))) != sizeof(cmd)) 
	{
		dev_err(&client->dev,"[ELAN] %s: i2c_master_send failed\n", __func__);
		return -EINVAL;
	}
	return 0;
}

#if IAP_PORTION

static int EnterISPMode(struct i2c_client *client, uint8_t  *isp_cmd)
{
	char buff[4] = {0};
	int len = 0;
	
	len = i2c_master_send(i2c_client, isp_cmd,  sizeof(isp_cmd));
	if (len != sizeof(buff)) 
	{
		printk("[ELAN] ERROR: EnterISPMode fail! len=%d\r\n", len);
		return -1;
	}
	else
	{
		printk("[ELAN] IAPMode write data successfully! cmd = [%2x, %2x, %2x, %2x]\n", isp_cmd[0], isp_cmd[1], isp_cmd[2], isp_cmd[3]);
	}
	return 0;
}


int ExtractPage(struct file *filp, uint8_t * szPage, int byte)
{
	int len = 0;

	len = filp->f_op->read(filp, szPage,byte, &filp->f_pos);
	if (len != byte) 
	{
		printk("[ELAN] ExtractPage ERROR: read page error, read error. len=%d\r\n", len);
		return -1;
	}

	return 0;
}

static int WritePage(uint8_t * szPage, int byte)
{
	int len = 0;

	len = i2c_master_send(i2c_client, szPage,  byte);
	if (len != byte) 
	{
		printk("[ELAN] ERROR: write page error, write error. len=%d\r\n", len);
		return -1;
	}
	return 0;
}

static int GetAckData(struct i2c_client *client)
{
	int len = 0;
	char buff[2] = {0};
	
	len = i2c_master_recv(i2c_client, buff, sizeof(buff));

	if (len != sizeof(buff)) 
	{
		printk("[ELAN] ERROR: read data error, write 50 times error. len=%d\r\n", len);
		return -1;
	}

	if (buff[0] == 0xaa/* && buff[1] == 0xaa*/)
	{ 
		return elan_ack_ok;
	}
	else if (buff[0] == 0x55 && buff[1] == 0x55)
	{
		return elan_ack_rewrite;
	}
	else
	{
		return elan_ack_fail;
	}
	return 0;
}

void print_progress(int page, int ic_num, int j)
{
	int i, percent,page_tatol,percent_tatol;
	char str[256];
	str[0] = '\0';
	for (i=0; i<((page)/10); i++) {
		str[i] = '#';
		str[i+1] = '\0';
	}
	
	page_tatol=page+249*(ic_num-j);
	percent = ((100*page)/(249));
	percent_tatol = ((100*page_tatol)/(249*ic_num));

	if ((page) == (249))
		percent = 100;

	if ((page_tatol) == (249*ic_num))
		percent_tatol = 100;		

	printk("\rprogress %s| %d%%", str, percent);
	
	if (page == (249))
		printk("\n");
}

static int IAPReset(struct i2c_client *client)
{
	int res;
	
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(20);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(100);

	printk("[ELAN] read Hello packet data!\n"); 	  
	res= __hello_packet_handler(client);
	return res;
}

static int CheckIapMode(void)
{
	char buff[4] = {0},len = 0;
	
	len = i2c_master_recv(i2c_client, buff, sizeof(buff));
	if (len != sizeof(buff)) 
	{
		printk("[ELAN] CheckIapMode ERROR: read data error,len=%d\r\n", len);
		return -1;
	}
	else
	{
		
		if (buff[0] == 0x55 && buff[1] == 0xaa && buff[2] == 0x33 && buff[3] == 0xcc)
		{
			return 0;
		}
		else
		{
			printk("[ELAN] Mode= 0x%x 0x%x 0x%x 0x%x\r\n", buff[0], buff[1], buff[2], buff[3]);
			printk("[ELAN] ERROR:  CheckIapMode error\n");
			return -1;
		}
	}
	printk("\n");	
}

int Update_FW_One(struct i2c_client *client, int elan_recovery)
{
	int res = 0,ic_num = 1;
	int iPage = 0, rewriteCnt = 0; //rewriteCnt for PAGE_REWRITE
	int i = 0;
	uint8_t data;
	//struct timeval tv1, tv2;
	int restartCnt = 0, checkCnt = 0; // For IAP_RESTART
	//uint8_t recovery_buffer[4] = {0};
	int byte_count;
	uint8_t *szBuff = NULL;
	int curIndex = 0;
#ifdef CMD_54001234
	uint8_t isp_cmd[] = {0x54, 0x00, 0x12, 0x34};	 //54 00 12 34
#else
	uint8_t isp_cmd[] = {0x45, 0x49, 0x41, 0x50};	 //45 49 41 50
#endif

	printk("[ELAN] %s:  ic_num=%d\n", __func__, ic_num);
IAP_RESTART:	
	//reset
// modify    


	data=I2C_DATA[0];//Master
	printk("[ELAN] %s: address data=0x%x \r\n", __func__, data);

	if(elan_recovery != 0x80)
	{
        	printk("[ELAN] Firmware upgrade normal mode !\n");
		/*gpio_set_value(SYSTEM_RESET_PIN_SR,0);
		mdelay(20);
		gpio_set_value(SYSTEM_RESET_PIN_SR,1);
		mdelay(500);*/
		res = EnterISPMode(private_ts->client, isp_cmd);	 //enter ISP mode
	

	//res = i2c_master_recv(private_ts->client, recovery_buffer, 4);   //55 aa 33 cc 
	//printk("[ELAN] recovery byte data:%x,%x,%x,%x \n",recovery_buffer[0],recovery_buffer[1],recovery_buffer[2],recovery_buffer[3]);		
	
		mdelay(10);
#if 1
		//Chech IC's status is 55 aa 33 cc
		res = CheckIapMode();	 //Step 1 enter ISP mode
		if (res == -1) //CheckIapMode fail
		{	
			checkCnt ++;
			if (checkCnt >= 5)
			{
				printk("[ELAN] ERROR: CheckIapMode %d times fails!\n", IAPRESTART);
				firmware_check_flag = 1;
				return E_FD;
			}
			else
			{
				printk("[ELAN] CheckIapMode retry %dth times! And restart IAP~~~\n\n", checkCnt);
				goto IAP_RESTART;
			}
		}
		else
			printk("[ELAN]  CheckIapMode ok!\n");
#endif
	} else
		printk("[ELAN] Firmware upgrade recovery mode !\n");
	// Send Dummy Byte	
	printk("[ELAN] send one byte data:%x,%x",private_ts->client->addr,data);
	res = i2c_master_send(private_ts->client, &data,  sizeof(data));
	if(res!=sizeof(data))
	{
		printk("[ELAN] dummy error code = %d\n",res);
	}	
	mdelay(50);


	// Start IAP
	for( iPage = 1; iPage <= PageNum; iPage++ ) 
	{
PAGE_REWRITE:
#if 1 // 8byte mode
		// 8 bytes
		//szBuff = fw_data + ((iPage-1) * PageSize); 
		for(byte_count=1;byte_count<=17;byte_count++)
		{
			if(byte_count!=17)
			{		
				printk(" byte %d\n",byte_count);
				printk("curIndex =%d\n",curIndex);
				szBuff = file_fw_data + curIndex;
				curIndex =  curIndex + 8;

				//ioctl(fd, IOCTL_IAP_MODE_LOCK, data);
				res = WritePage(szBuff, 8);
			}
			else
			{
				printk("byte %d\n",byte_count);
				printk("curIndex =%d\n",curIndex);
				szBuff = file_fw_data + curIndex;
				curIndex =  curIndex + 4;
				//ioctl(fd, IOCTL_IAP_MODE_LOCK, data);
				res = WritePage(szBuff, 4); 
			}
		} // end of for(byte_count=1;byte_count<=17;byte_count++)
#endif 
#if 0 // 132byte mode		
		szBuff = file_fw_data + curIndex;
		curIndex =  curIndex + PageSize;
		res = WritePage(szBuff, PageSize);
#endif
#if 0
		if(iPage==249 || iPage==1)
		{
			mdelay(600); 			 
		}
		else
		{
			mdelay(50); 			 
		}
#endif	
		res = GetAckData(private_ts->client);

		if (ACK_OK != res) 
		{
			mdelay(50); 
			printk("[ELAN] ERROR: GetAckData fail! res=%d\r\n", res);
			if ( res == ACK_REWRITE ) 
			{
				rewriteCnt = rewriteCnt + 1;
				if (rewriteCnt == PAGERETRY)
				{
					printk("[ELAN] ID 0x%02x %dth page ReWrite %d times fails!\n", data, iPage, PAGERETRY);
					firmware_check_flag = 2;
					return E_FD;
				}
				else
				{
					printk("[ELAN] ---%d--- page ReWrite %d times!\n",  iPage, rewriteCnt);
					curIndex = curIndex - PageSize;
					goto PAGE_REWRITE;
				}
			}
			else
			{
				restartCnt = restartCnt + 1;
				if (restartCnt >= 5)
				{
					printk("[ELAN] ID 0x%02x ReStart %d times fails!\n", data, IAPRESTART);
					firmware_check_flag = 3;
					return E_FD;
				}
				else
				{
					printk("[ELAN] ===%d=== page ReStart %d times!\n",  iPage, restartCnt);
					goto IAP_RESTART;
				}
			}
		}
		else
		{   printk("[ELAN] data : 0x%02x ",  data);  
			rewriteCnt=0;
			print_progress(iPage,ic_num,i);
		}

		mdelay(10);
	} // end of for(iPage = 1; iPage <= PageNum; iPage++)
	
	if (IAPReset(client) > 0)
		printk("[ELAN] Update ALL Firmware successfully!\n");
        __fw_packet_handler(client);
		firmware_check_flag = 4;
	return 0;
}

#endif
// End Firmware Update

int elan_iap_open(struct inode *inode, struct file *filp)
{ 
	printk("[ELAN]into elan_iap_open\n");		
	return 0;
}

int elan_iap_release(struct inode *inode, struct file *filp)
{    
	return 0;
}

static ssize_t elan_iap_write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{  
    int 	ret;
    char 	*tmp;
    struct 	i2c_adapter *adap = i2c_client->adapter;    	
    struct 	i2c_msg msg;
    
	printk("[ELAN]into elan_iap_write\n");
    if (count > 8192)
	{
		count = 8192;
	}
	tmp = kmalloc(count, GFP_KERNEL);

	if (tmp == NULL)
	{
		return -ENOMEM;
	}

	if (copy_from_user(tmp, buff, count)) 
	{
		return -EFAULT;
	}

	ret = i2c_master_send(i2c_client, tmp, count);

	kfree(tmp);
	return (ret == 1) ? count : ret;
}

ssize_t elan_iap_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{    
	char *tmp;
	int ret;  
	long rc;
    struct i2c_adapter *adap = i2c_client->adapter;
	struct i2c_msg msg;

	printk("[ELAN]into elan_iap_read\n");
	if (count > 8192)
	{
		count = 8192;
	}

	tmp = kmalloc(count, GFP_KERNEL);

	if (tmp == NULL)
	{
		return -ENOMEM;
	}
	ret = i2c_master_recv(i2c_client, tmp, count);
	if (ret >= 0)
	{
		rc = copy_to_user(buff, tmp, count);
	}
	kfree(tmp);
	return (ret == 1) ? count : ret;
}

static long elan_iap_ioctl(/*struct inode *inode,*/ struct file *filp,    unsigned int cmd, unsigned long arg)
{
	int __user *ip = (int __user *)arg;
	printk("[ELAN]into elan_iap_ioctl\n");
	printk("[ELAN]cmd value %x\n",cmd);
	printk("[ELAN] elan_fw_ver = %d, elan_fw_id = %d, elan_recovery = %d, firmware_check_flag = %d\n",elan_fw_ver,elan_fw_id,elan_recovery,firmware_check_flag);
	switch (cmd) 
	{        
		case IOCTL_I2C_SLAVE: 			
			i2c_client->addr = (int __user)arg;
			i2c_client->addr &= I2C_MASK_FLAG; 
			i2c_client->addr |= I2C_ENEXT_FLAG;
			//file_fops_addr = 0x15;
			break;   
		case IOCTL_MAJOR_FW_VER:
			break;        
		case IOCTL_MINOR_FW_VER:
			break;        
		case IOCTL_RESET:
			mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
			mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
			mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
			msleep(20);
			mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
			mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
			mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
			msleep(100);
			break;
		case IOCTL_IAP_MODE_LOCK:
			//if(work_lock==0)
			{
				printk("[ELAN]%s %x=IOCTL_IAP_MODE_LOCK\n", __func__,IOCTL_IAP_MODE_LOCK);
				//work_lock=1;
				mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
				//cancel_work_sync(&private_ts->work);
			}
			break;
			case IOCTL_IAP_MODE_UNLOCK:
			//if(work_lock==1)
			{			
				//work_lock=0;
				//enable_irq(CUST_EINT_TOUCH_PANEL_NUM);
				mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
			}
			break;
		case IOCTL_CHECK_RECOVERY_MODE:
			return elan_recovery;
			break;
		case IOCTL_FW_VER:
			__fw_packet_handler(i2c_client);//private_ts->client);
			return elan_fw_ver;
			break;
		case IOCTL_X_RESOLUTION:
			__fw_packet_handler(i2c_client);//private_ts->client);
			return elan_tp_x_resolution;
			break;
		case IOCTL_Y_RESOLUTION:
			__fw_packet_handler(i2c_client);//private_ts->client);
			return elan_tp_y_resolution;
			break;
		case IOCTL_FW_ID:
			__fw_packet_handler(i2c_client);//private_ts->client);
			return elan_fw_id;
			break;
		case IOCTL_ROUGH_CALIBRATE:
			return elan_ktf2k_ts_rough_calibrate(i2c_client);//private_ts->client);
		case IOCTL_I2C_INT:
			put_user(mt_get_gpio_in(GPIO_CTP_EINT_PIN),ip);
			printk("[ELAN]GPIO_CTP_EINT_PIN = %d\n", mt_get_gpio_in(GPIO_CTP_EINT_PIN));
			break;	
		case IOCTL_RESUME:
			//tpd_resume(i2c_client);
			break;	
		case IOCTL_CIRCUIT_CHECK:
			return elan_circuit_ver;
			break;
		case IOCTL_POWER_LOCK:
			power_lock=1;
			break;
		case IOCTL_POWER_UNLOCK:
			power_lock=0;
			break;
#if IAP_PORTION		
		case IOCTL_GET_UPDATE_PROGREE:
			update_progree=(int __user)arg;
			break; 

		case IOCTL_FW_UPDATE:
			elan_recovery = IAPReset(private_ts->client);
			Update_FW_One(private_ts->client, elan_recovery);
#endif
		case IOCTL_BC_VER:
			__fw_packet_handler(i2c_client);
			return elan_bc_version;
			break;
		default:
			break;   
	}       
	return 0;
}

//*******************************************************************************************
// input subsystem report : down                                                            |
//*******************************************************************************************
static  void tpd_down(int x, int y, int p) 
{
	// input_report_abs(tpd->dev, ABS_PRESSURE, p);
	input_report_key(tpd->dev, BTN_TOUCH, 1);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, p); 
	input_mt_sync(tpd->dev);
     
	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
	{   
		tpd_button(x, y, 1);  
	}
	
	if(y > TPD_RES_Y) //virtual key debounce to avoid android ANR issue
	{
		msleep(50);
		printk("[ELAN]D virtual key \n");
	}
	TPD_EM_PRINT(x, y, x, y, p-1, 1);
}


//*******************************************************************************************
// input subsystem report : up                                                              |
//******************************************************************************************* 
static  void tpd_up(int x, int y,int *count) 
{
	if(EKTF2132_TP_DEBUG){
	printk("[ELAN][%s] x=%d, y=%d \n",__func__,x,y);
		}

	//if(*count>0) {
	//input_report_abs(tpd->dev, ABS_PRESSURE, 0);
	input_report_key(tpd->dev, BTN_TOUCH, 0);
	//input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
	//input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	//input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	//printk("U[%4d %4d %4d] ", x, y, 0);
	input_mt_sync(tpd->dev);
	TPD_EM_PRINT(x, y, x, y, 0, 0);
	//	 (*count)--;
	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
	{   
		tpd_button(x, y, 0); 
	}   		 
}


//*******************************************************************************************
// read the touch data from i2c                                                             |
//*******************************************************************************************
static int tpd_touchinfo(struct touch_info *cinfo, struct touch_info *pinfo)
{
	char 	data[30] 			= {0};
 	u16 	high_byte,low_byte;
	int 	rc 					= 0;

	rc = i2c_master_recv(i2c_client, data, 18);
	if (rc != 18)
	{
		if(EKTF2132_TP_DEBUG){
		printk("[ELAN][%s] The first package error.\n",__func__);
			}
	}

	if(EKTF2132_TP_DEBUG){
	printk("[ELAN][%s] P1: %x %x %x %x %x %x %x %x\n", __func__,data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
		}


	point_num 	= 0;
	cinfo->p[0] = 0;
	cinfo->p[1] = 0;
	cinfo->p[2] = 0;
	cinfo->p[3] = 0;
	cinfo->p[4] = 0;
	cinfo->id[0]= 1;
	cinfo->id[1]= 2;
	cinfo->id[2]= 3;
	cinfo->id[3]= 4;
	cinfo->id[4]= 5;
	key_register = 0;
	
	key_register = data[17];
	if(key_register == 129) //1000 0001
	{
		point_num++;
		key_register = 1;
	}
	else if(key_register == 65) // 0100 0001
	{
		point_num++;
		key_register = 2;
	}
	else if(key_register == 33) // 0010 0001
	{
		point_num++;
		key_register = 3;
	}
	else 
	{
		key_register = 0;
	}
	if(EKTF2132_TP_DEBUG){
	printk("[ELAN]%s:key register = %d \n",__func__,key_register);
    printk("[ELAN]%s:key_is_down = %d \n",__func__,key_is_down);
    printk("[ELAN]%s:key_is_detection = %d \n",__func__,key_is_detection);
    printk("[ELAN]%s:touch_is_down = %d \n",__func__,touch_is_down);
		}
	
	if(key_register > 0) //key event is report by IC
    {
        if(!key_is_down && !touch_is_down) //first key event , detect it.
        {
            key_is_detection = true;
            prev_key_register = key_register; //backup the key_register , used for release key event.
            return true; //return to report key event.
        }

        if(key_is_down && !touch_is_down) //later key event. report the first key event.
        {
            return true; //return to report key event.
        }
    }

	if(key_register == 0) //key event is clean.
    {
        if(key_is_detection) // key is press now.
        {
            key_is_detection = false; //release the key.
            return true; //
        }
    }
	
    //Point 1
	if(data[1] & 0x08)
	{
		point_num++;
		cinfo->p[0] = 1;          
		//x
		high_byte = data[2];
		high_byte <<= 4;
		high_byte &= 0xF00;

		low_byte = data[3];
		cinfo->x[0] = high_byte | low_byte;
		
		//y
		high_byte = data[2];
		high_byte <<= 8;
		high_byte &= 0xF00;

		low_byte = data[4];
		cinfo->y[0] = high_byte | low_byte;

		cinfo->x[0] = ( cinfo->x[0] * elan_lcm_x_resolution) / elan_tp_x_resolution;
		cinfo->y[0] = ( cinfo->y[0] * elan_lcm_y_resolution) / elan_tp_y_resolution;

		if(EKTF2132_TP_DEBUG){
		printk("[ELAN][%s] x1=%d,y1=%d\n",__func__,cinfo->x[0],cinfo->y[0]);
			}
	}

	//Point 2
	if(data[1] & 0x10)	
	{
		point_num++;
		cinfo->p[1] = 1;
		//x
		high_byte = data[5];
		high_byte <<= 4;
		high_byte &= 0xF00;

		low_byte = data[6];
		cinfo->x[1] = high_byte | low_byte;
	
		//y
		high_byte = data[5];
		high_byte <<= 8;
		high_byte &= 0xF00;

		low_byte = data[7];
		cinfo->y[1] = high_byte | low_byte;

		cinfo->x[1] = ( cinfo->x[1] * elan_lcm_x_resolution) / elan_tp_x_resolution;
        cinfo->y[1] = ( cinfo->y[1] * elan_lcm_y_resolution) / elan_tp_y_resolution;

		if(EKTF2132_TP_DEBUG){
		printk("[ELAN][%s] x2=%d,y2=%d\n",__func__,cinfo->x[1],cinfo->y[1]);
			}
	}

	//Point 3
	if(data[1] & 0x20)
	{
		point_num++;
		cinfo->p[2] = 1;
		//x
		high_byte = data[8];
		high_byte <<= 4;
		high_byte &= 0xF00;

		low_byte = data[9];
		cinfo->x[2] = high_byte | low_byte;

		//y
		high_byte = data[8];
		high_byte <<= 8;
		high_byte &= 0xF00;
	
		low_byte = data[10];
		cinfo->y[2] = high_byte | low_byte;

		cinfo->x[2] = ( cinfo->x[2] * elan_lcm_x_resolution) / elan_tp_x_resolution;
        cinfo->y[2] = ( cinfo->y[2] * elan_lcm_y_resolution) / elan_tp_y_resolution;

	    if(EKTF2132_TP_DEBUG){
		printk("[ELAN][%s] x3=%d,y3=%d\n",__func__,cinfo->x[2],cinfo->y[2]);
		}
	}

	//Point 4
	if(data[1] & 0x40)
	{
		point_num++;
		cinfo->p[3] = 1;
		//x
		high_byte = data[11];
		high_byte <<= 4;
		high_byte &= 0xF00;
        
		low_byte = data[12];
		cinfo->x[3] = high_byte | low_byte;

		//y
		high_byte = data[11];
		high_byte <<= 8;
		high_byte &= 0xF00;

		low_byte = data[13];
		cinfo->y[3] = high_byte | low_byte;
		
		cinfo->x[3] = ( cinfo->x[3] * elan_lcm_x_resolution) / elan_tp_x_resolution;
        cinfo->y[3] = ( cinfo->y[3] * elan_lcm_y_resolution) / elan_tp_y_resolution;

		if(EKTF2132_TP_DEBUG){
		printk("[ELAN][%s] x4=%d,y4=%d\n",__func__,cinfo->x[3],cinfo->y[3]);
			}
	}

	//Point 5
	if(data[1] & 0x80)
	{
		point_num++;
		cinfo->p[4] = 1;
		//x
		high_byte = data[14];
		high_byte <<= 4;
		high_byte &= 0xF00;
        
		low_byte = data[15];
		cinfo->x[4] = high_byte | low_byte;

		//y
		high_byte = data[14];
		high_byte <<= 8;
		high_byte &= 0xF00;

		low_byte = data[16];
		cinfo->y[4] = high_byte | low_byte;

		cinfo->x[4] = ( cinfo->x[4] * elan_lcm_x_resolution) / elan_tp_x_resolution;
        cinfo->y[4] = ( cinfo->y[4] * elan_lcm_y_resolution) / elan_tp_y_resolution;

		if(EKTF2132_TP_DEBUG){
		printk("[ELAN][%s] x5=%d,y5=%d\n",__func__,cinfo->x[4],cinfo->y[4]);
			}
	}

	return true;
};


//*******************************************************************************************
// interrupt handler function                                                               |
//*******************************************************************************************
static void tpd_eint_interrupt_handler(void)
{
	if(EKTF2132_TP_DEBUG){
	printk("[ELAN][%s]:enter.\n",__func__);
		}
	tpd_flag = 1;
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	wake_up_interruptible(&waiter);
}


//*******************************************************************************************
// kernel thread function                                                                   |
//*******************************************************************************************
static int touch_event_handler(void *unused)
{
	struct touch_info cinfo, pinfo;
	int i=0;

	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	sched_setscheduler(current, SCHED_RR, &param);
 
	do
	{
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
		set_current_state(TASK_INTERRUPTIBLE); 
		wait_event_interruptible(waiter,tpd_flag!=0);
	
		tpd_flag = 0;		 
		set_current_state(TASK_RUNNING);
	
		if (tpd_touchinfo(&cinfo, &pinfo)) 
		{
			if(EKTF2132_TP_DEBUG){
			printk("[ELAN][%s] point_num = %d \n",__func__,point_num);
			printk("[ELAN][%s] *********** firmware_check_flag = %d \n",__func__,firmware_check_flag);
				}

			if(point_num >0 || key_is_detection) 
			{
				if(key_is_detection)
				{
					if(prev_key_register == 1)
					{
					if(EKTF2132_TP_DEBUG){
						printk("[ELAN]%s:[DOWN]report KEY_MENU\n",__func__);
						}
						input_report_key(tpd->kpd,KEY_MENU,1);
                                                //input_report_key(tpd->kpd,KEY_BACK,1);
					}
					else if(prev_key_register == 2)
					{
					if(EKTF2132_TP_DEBUG){
						printk("[ELAN]%s:[DOWN]report KEY_HOMEPAGE\n",__func__);
						}
						input_report_key(tpd->kpd,KEY_HOMEPAGE,1);
					}
					else if(prev_key_register == 3)
					{
					if(EKTF2132_TP_DEBUG){
						printk("[ELAN]%s:[DOWN]report KEY_BACK\n",__func__);
						}
						input_report_key(tpd->kpd,KEY_BACK,1);
                                                //input_report_key(tpd->kpd,KEY_MENU,1);
					}
					else
					{
					if(EKTF2132_TP_DEBUG){
						printk("[ELAN]%s:[DOWN]key register error.\n",__func__);
						}
					}
					key_is_down = true;
					input_sync(tpd->kpd);
				}
				else
				{
					for(i=0; i<max_point_num; i++)
					{
						if(cinfo.p[i]) 
						{
							touch_is_down = true;
							tpd_down(cinfo.x[i], cinfo.y[i], cinfo.id[i]);
						}
					}
					input_sync(tpd->dev);
				}
			}
			else  
			{
				if(key_is_down)
				{
					if(prev_key_register == 1)
					{
					if(EKTF2132_TP_DEBUG){
						printk("[ELAN]%s:[UP]report KEY_MENU\n",__func__);
						}
						input_report_key(tpd->kpd,KEY_MENU,0);
                                                //input_report_key(tpd->kpd,KEY_BACK,0);
					}
					else if(prev_key_register == 2)
					{
					if(EKTF2132_TP_DEBUG){
						printk("[ELAN]%s:[UP]report KEY_HOMEPAGE\n",__func__);
						}
						input_report_key(tpd->kpd,KEY_HOMEPAGE,0);
					}
					else if(prev_key_register == 3)
					{
					if(EKTF2132_TP_DEBUG){
						printk("[ELAN]%s:[UP]report KEY_BACK\n",__func__);
						}
						input_report_key(tpd->kpd,KEY_BACK,0);
                                                //input_report_key(tpd->kpd,KEY_MENU,0);
					}
					else
					{
					if(EKTF2132_TP_DEBUG){
						printk("[ELAN]%s:[UP]register error\n",__func__);
						}
					}
					key_is_down = false;
					input_sync(tpd->kpd);
				}
				else
				{
					touch_is_down = false;
					tpd_up(cinfo.x[0], cinfo.y[0], 0);
					input_sync(tpd->dev);
				}
			}
		}
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
 	}while(!kthread_should_stop());
	return 0;
}
 

//*******************************************************************************************
// ELAN polling	interrupt status                                                            |
//*******************************************************************************************
static int __elan_ktf2k_ts_poll(struct i2c_client *client)
{
	struct elan_ktf2k_ts_data *ts = i2c_get_clientdata(client);
	int status = 0, retry = 10;

	do 
	{
		//status = gpio_get_value(ts->intr_gpio);
		status = mt_get_gpio_in(GPIO_CTP_EINT_PIN);
		//printk("%s: status = %d\n", __func__, status);
		retry--;
		mdelay(20);
	} while (status == 1 && retry > 0);

	//printk( "[elan]%s: poll interrupt status %s\n",__func__, status == 1 ? "high" : "low");
	return (status == 0 ? 0 : -ETIMEDOUT);
}

static int elan_ktf2k_ts_poll(struct i2c_client *client)
{
	return __elan_ktf2k_ts_poll(client);
}


//*******************************************************************************************
// Elan get i2c data function                                                               |
//******************************************************************************************* 
static int elan_ktf2k_ts_get_data(struct i2c_client *client, uint8_t *cmd, uint8_t *buf, size_t size)
{
	int rc;
    
	if (buf == NULL)
	{
		if(EKTF2132_TP_DEBUG){
		printk("[ELAN][%s] buf is null,return error.\n",__func__);
			}
		return -EINVAL;
	}
     
	if ((i2c_master_send(client, cmd, 4)) != 4) 
	{
		if(EKTF2132_TP_DEBUG){
		printk("[ELAN][%s] i2c send fail,return error.\n",__func__);
			}
		return -EINVAL;
	}

	rc = elan_ktf2k_ts_poll(client);
	if (rc < 0)
	{
		if(EKTF2132_TP_DEBUG){
		printk("[ELAN][%s] elan poll fail,return error.\n",__func__);
			}
		return -EINVAL;
	}
	else 
	{
		if (i2c_master_recv(client, buf, size) != size || buf[0] != 0x52)
		{
			if(EKTF2132_TP_DEBUG){
			printk("[ELAN][%s] i2c recv fail,return error.\n",__func__);
				}
			return -EINVAL;
		}
	}    
	return 0;
}

//*******************************************************************************************
// Elan hello package handler                                                               |
//*******************************************************************************************
static int __hello_packet_handler(struct i2c_client *client)
{ 
	int rc;
	uint8_t buf[8] = { 0 };
	
	rc = elan_ktf2k_ts_poll(client);
	if (rc < 0) 
	{
		if(EKTF2132_TP_DEBUG){
	    printk( "[ELAN][%s] Int poll failed!\n", __func__);
			}
		elan_recovery=0x80;
		return elan_recovery;
	}

	rc = i2c_master_recv(client, buf, 8);
	if(EKTF2132_TP_DEBUG){
	printk("[ELAN][%s] hello packet %2x:%2X:%2x:%2x:%2x:%2X:%2x:%2x\n", __func__, buf[0], buf[1], buf[2], buf[3] , buf[4], buf[5], buf[6], buf[7]);
		}

	if(buf[0]==0x55 && buf[1]==0x55 && buf[2]==0x80 && buf[3]==0x80)
	{
		elan_recovery=0x80;
		return elan_recovery; 
	}
	return 0;
}


//*******************************************************************************************
// Elan IC setup function                                                                   |
//*******************************************************************************************
static int elan_ktf2k_ts_setup(struct i2c_client *client)
{ 
	int rc;
	//int count = 3
	//retry:
	rc = __hello_packet_handler(client);
	
	if(EKTF2132_TP_DEBUG){
	printk("[ELAN][%s] hellopacket's rc = %d\n",__func__,rc);
		}

	//mdelay(10);
	//if (rc != 0x80)
	//{
	//    rc = __fw_packet_handler(client);
	//    if (rc < 0)
	//	    printk("[elan] %s, fw_packet_handler fail, rc = %d", __func__, rc);
	//    printk("[elan] %s: firmware checking done.\n", __func__);
	    //Check for FW_VERSION, if 0x0000 means FW update fail!
	    //if ( FW_VERSION == 0x00)
	    //{
	//	rc = 0x80;
	//	printk("[elan] FW_VERSION = %d, last FW update fail\n", FW_VERSION);
	//    }
    	//}
	return rc;
}


//*******************************************************************************************
// i2c callback function	                                                                |
//*******************************************************************************************
static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info)
{
	strcpy(info->type, TPD_DEVICE);
	return 0;
}

static ssize_t ektf2k_store_config(struct device_driver *ddri, const char *buf, size_t count)
{
	int retry, als_deb, ps_deb, mask, hthres, lthres;
	if(buf[0]=='1')
	{ 
        uint8_t cmd[] = {CMD_W_PKT, 0x58, 0x00, 0x01};
        printk("[ELAN] TPD wake up\n");
        if ((i2c_master_send(private_ts->client, cmd, 4)) != 4) 
        {
            printk("[ELAN] %s: i2c_master_send failed\n", __func__);
            return -1;
        }
        msleep(100);
        mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	}
	else if (buf[0]=='0')
	{
        int retval = TPD_OK;
        uint8_t cmd[] = {CMD_W_PKT, 0x50, 0x00, 0x01};
        printk("[ELAN] TPD enter sleep\n");
        if ((i2c_master_send(private_ts->client, cmd, 4)) != 4) 
        {
    		printk("[ELAN] %s: i2c_master_send failed\n", __func__);
    		return -1;
        }
        mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

	}
    else
    {
    ;
    }
	return count;    
}

static DRIVER_ATTR(tp_onoff,  S_IWUSR | S_IRUGO,NULL,ektf2k_store_config);

static ssize_t ektf2k_self_test_show(struct device_driver *ddri, const char *buf, size_t count)
{
	struct elan_ktf2k_ts_data *ts = private_ts;

	int len = 0;
	int i =0;
	int test_result = 0;
	int X_fail = 0;
	int Y_fail = 0;
	int B_fail = 0;
	int fail_CH_X = 0;
	int fail_CH_Y = 0;
	int fail_CH_B = 0;
	int status = 0;
	int result_tag = 0;
	int read_count = 0;
	int retry = 200;
	int counter = 0;
	
	uint8_t cmd_selftest_start1[] = {0x55, 0x55, 0x55, 0x55};
	uint8_t cmd_selftest_start2[] = {0x59, 0xE0, 0x00, 0x1A, 0x00, 0x00, 0x28, 0x00, 0x00, 0x08, 0x00};
	
	uint8_t cmd_selftest_end1[] = {0x9F, 0x00, 0x00, 0x01};
	uint8_t cmd_selftest_end2[] = {0xA5, 0xA5, 0xA5, 0xA5};
	
	char buff_X[26] = {0};
	char buff_Y[40] = {0};
	char buff_B[8] = {0};

	ssize_t num_read_chars = 0;

	char selftest_buf[127]="";
	struct file *filp;
	mm_segment_t fs;

	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
//test mode start
	if ((i2c_master_send(private_ts->client, cmd_selftest_start1, 4)) != 4)
	{
	printk("[ELAN] %s: i2c_master_send failed\n", __func__);
		result_tag = 5;
    	goto self_test_fail;
    }	
	msleep(10);
	if ((i2c_master_send(private_ts->client, cmd_selftest_start2, 11)) != 11)
	{
	printk("[ELAN] %s: i2c_master_send failed\n", __func__);
		result_tag = 5;
    	goto self_test_fail;
    }
	msleep(10);


	for(read_count=0;read_count<100;read_count++)  // set read time limit 100 time
	{
		if (mt_get_gpio_in(GPIO_CTP_EINT_PIN)==0)
		{
			len = i2c_master_recv(i2c_client, buff_X, sizeof(buff_X));
					printk("[ELAN] buff_X[0]=0x%x\r\n",buff_X[0]);
		}
		msleep(5);
		if (mt_get_gpio_in(GPIO_CTP_EINT_PIN)==0)
			{
			len = i2c_master_recv(i2c_client, buff_Y, sizeof(buff_Y));
					printk("[ELAN] buff_Y[0]=0x%x\r\n",buff_Y[0]);
			}
		msleep(5);
		if (mt_get_gpio_in(GPIO_CTP_EINT_PIN)==0)
			{
			len = i2c_master_recv(i2c_client, buff_B, sizeof(buff_B));
					printk("[ELAN] buff_B[0]=0x%x\r\n",buff_B[0]);
			}
		msleep(5);
		if ((buff_X[0] == 0x90)&&(buff_Y[0]==0x91)&&(buff_B[0]==0x92))
			{
			goto self_getdata_OK;
			}
		else
		{

			printk("[ELAN]======== X Start=========\n");
			printk("[ELAN][Self-X-Axis]: %x %x %x %x %x %x %x %x\n",buff_X[0], buff_X[1], buff_X[2], buff_X[3], buff_X[4], buff_X[5], buff_X[6], buff_X[7]);
			printk("[ELAN][Self-X-Axis]: %x %x %x %x %x %x %x %x\n",buff_X[8], buff_X[9], buff_X[10], buff_X[11], buff_X[12], buff_X[13], buff_X[14], buff_X[15]);	
			printk("[ELAN][Self-X-Axis]: %x %x %x %x %x %x %x %x\n",buff_X[16], buff_X[17], buff_X[18], buff_X[19], buff_X[20], buff_X[21], buff_X[22], buff_X[23]);	
			printk("[ELAN][Self-X-Axis]: %x %x\n",buff_X[24], buff_X[25]);			
			printk("[ELAN]======== X END=========\n");
			printk("[ELAN]======== Y Start=========\n");
			printk("[ELAN][Self-Y-Axis]: %x %x %x %x %x %x %x %x\n",buff_Y[0], buff_Y[1], buff_Y[2], buff_Y[3], buff_Y[4], buff_Y[5], buff_Y[6], buff_Y[7]);
			printk("[ELAN][Self-Y-Axis]: %x %x %x %x %x %x %x %x\n",buff_Y[8], buff_Y[9], buff_Y[10], buff_Y[11], buff_Y[12], buff_Y[13], buff_Y[14], buff_Y[15]);	
			printk("[ELAN][Self-Y-Axis]: %x %x %x %x %x %x %x %x\n",buff_Y[16], buff_Y[17], buff_Y[18], buff_Y[19], buff_Y[20], buff_Y[21], buff_Y[22], buff_Y[23]);	
			printk("[ELAN][Self-Y-Axis]: %x %x %x %x %x %x %x %x\n",buff_Y[24], buff_Y[25], buff_Y[26], buff_Y[27], buff_Y[28], buff_Y[29], buff_Y[30], buff_Y[31]);	
			printk("[ELAN][Self-Y-Axis]: %x %x %x %x %x %x %x %x\n",buff_Y[32], buff_Y[33], buff_Y[34], buff_Y[35], buff_Y[36], buff_Y[37], buff_Y[38], buff_Y[39]);		
			printk("[ELAN]======== Y END=========\n");	
			printk("[ELAN]======== B Start=========\n");
			printk("[ELAN][Self-B-Axis]: %x %x %x %x %x %x %x %x\n",buff_B[0], buff_B[1], buff_B[2], buff_B[3], buff_B[4], buff_B[5], buff_B[6], buff_B[7]);
			printk("[ELAN]======== B END=========\n");							

			buff_X[0] =0;
			buff_Y[0] =0;
			buff_B[0] =0;
		}

	}
	if(read_count == 100)
		result_tag = 4;


self_getdata_OK:

#if EKTF2132_TP_DEBUG
	printk("[ELAN] finish buff_X 0x%x%x\r\n",buff_X[0] ,buff_X[1]);
	printk("[ELAN] finish buff_Y 0x%x%x\r\n",buff_Y[0] ,buff_Y[1]);
	printk("[ELAN] finish buff_B 0x%x%x\r\n",buff_B[0] ,buff_B[1]);
#endif

for(i=2;  i<sizeof(buff_X)/sizeof(buff_X[0]); i+=2)
{
	test_result = (buff_X[i] << 8) | buff_X[i+1];
//		printk("[ELAN]TP selfest Channel X[%d] = %d\n",i/2, test_result);
	if(test_result>selftest_Xmax|| test_result<selftest_Xmin)
	{
		result_tag = 1;
		X_fail = test_result;
		fail_CH_X = i/2;
		printk("[ELAN]TP selfest FAIL Channel X[%d] = %d\n",i/2, test_result);
		break;
	}
}

for(i=2;  i<sizeof(buff_Y)/sizeof(buff_Y[0]); i+=2)
{
	test_result = (buff_Y[i] << 8) | buff_Y[i+1];
//		printk("[ELAN]TP selfest Channel Y[%d] = %d\n",i/2, test_result);
	if(test_result>selftest_Ymax|| test_result<selftest_Ymin)
	{
		result_tag = 2;
		Y_fail = test_result;
		fail_CH_Y = i/2;
		printk("[ELAN] TP selfest FAIL Channel Y[%d] = %d\n",i/2, test_result);
		break;
	}
}

for(i=2;  i<sizeof(buff_B)/sizeof(buff_B[0]); i+=2)
{
	test_result = (buff_B[i] << 8) | buff_B[i+1];
//		printk("[ELAN]TP selfest Channel B[%d] = %d\n",i/2, test_result);
	if(test_result>selftest_Bmax|| test_result<selftest_Bmin)
	{
		result_tag = 3;
		B_fail = test_result;
		fail_CH_B = i/2;
		printk("[ELAN] TP selfest FAIL Channel B[%d] = %d\n",i/2, test_result);
		break;
	}
}

self_test_fail:

	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	if ((i2c_master_send(private_ts->client, cmd_selftest_end1, 4)) != 4)
	{
		printk("[ELAN] %s: i2c_master_send failed\n", __func__);
	}	
	if ((i2c_master_send(private_ts->client, cmd_selftest_end2, 4)) != 4)
	{
		printk("[ELAN] %s: i2c_master_send failed\n", __func__);
	}

	if(result_tag == 0) {
		num_read_chars += sprintf(&(buf[num_read_chars]), "Pass      \n");
	}else {
		if(result_tag == 1) {
			num_read_chars += sprintf(&(buf[num_read_chars]), "Fail CH_X[%d]=%d\n",fail_CH_X, X_fail);
		}else if(result_tag == 2) {
			num_read_chars += sprintf(&(buf[num_read_chars]), "Fail CH_Y[%d]=%d\n",fail_CH_Y, Y_fail);
		}else if(result_tag == 3) {
			num_read_chars += sprintf(&(buf[num_read_chars]), "Fail CH_B[%d]=%d\n",fail_CH_B, B_fail);
		}else if(result_tag == 4) {
			num_read_chars += sprintf(&(buf[num_read_chars]), "Test Overtime\n");
		}else if(result_tag == 5) {
			num_read_chars += sprintf(&(buf[num_read_chars]), "I2C send Fail\n");
		}
	}

//write raw data to file
	filp = filp_open(selftest_raw_file, O_CREAT | O_RDWR, 0666);
	if(!IS_ERR(filp))
	{
		printk("open return=%ld\n", IS_ERR(filp));
		fs=get_fs();
		set_fs(KERNEL_DS);
		filp->f_op->llseek(filp, 0, SEEK_SET);

		for(i=2;  i<sizeof(buff_X)/sizeof(buff_X[0]); i+=2)
		{
			test_result = (buff_X[i] << 8) | buff_X[i+1];
			sprintf(selftest_buf,"CH_X[%d] = %d\r\n",i/2, test_result);
			filp->f_op->write(filp, (char *)selftest_buf, strlen(selftest_buf),&filp->f_pos);
		}

		for(i=2;  i<sizeof(buff_Y)/sizeof(buff_Y[0]); i+=2)
		{
			test_result = (buff_Y[i] << 8) | buff_Y[i+1];
			sprintf(selftest_buf,"CH_Y[%d] = %d\r\n",i/2, test_result);
			filp->f_op->write(filp, (char *)selftest_buf, strlen(selftest_buf),&filp->f_pos);
		}

		for(i=2;  i<sizeof(buff_B)/sizeof(buff_B[0]); i+=2)
		{
			test_result = (buff_B[i] << 8) | buff_B[i+1];
			sprintf(selftest_buf,"CH_B[%d] = %d\r\n",i/2, test_result);
			filp->f_op->write(filp, (char *)selftest_buf, strlen(selftest_buf),&filp->f_pos);
		}
		set_fs(fs);
	}else {
		printk("open file fail, return=%ld\n", IS_ERR(filp));
	}
	filp_close(filp,NULL);

	return num_read_chars;

}

static DRIVER_ATTR(self_test, 0664, ektf2k_self_test_show, NULL);


static ssize_t ektf2k_mutual_test_show(struct device_driver *ddri, const char *buf, size_t count)
{
	struct elan_ktf2k_ts_data *ts = private_ts;

	int len = 0;
	int i =0;
	int j =0;
	int status = 0;
	int test_result = 0;
	int keep_on = 0;	
	int result_tag = 0;
	int read_count = 0;

	ssize_t num_read_chars = 0;
	
	uint8_t cmd_mutualtest_start1[] = {0x55, 0x55, 0x55, 0x55};
	uint8_t cmd_mutualtest_start2[] = {0x59, 0xC0, 0x13, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
	uint8_t cmd_mutualtest_end1[] = {0x9F, 0x00, 0x00, 0x01};
	uint8_t cmd_mutualtest_end2[] = {0xA5, 0xA5, 0xA5, 0xA5};
	
	char buff_mutual[19][27] = {0};

	char mutualtest_buf[127] = "";
	struct file *filp;
	mm_segment_t fs;

	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

//test mode start
	if ((i2c_master_send(private_ts->client, cmd_mutualtest_start1, 4)) != 4)
	{
		result_tag = 3;
	printk("[ELAN] %s: i2c_master_send failed\n", __func__);
    	goto mutual_test_fail;
    }	
	msleep(10);
	if ((i2c_master_send(private_ts->client, cmd_mutualtest_start2, 11)) != 11)
	{
	    result_tag = 3;
	printk("[ELAN] %s: i2c_master_send failed\n", __func__);
    	goto mutual_test_fail;
    }
	msleep(10);

	for(read_count=0;read_count<100;read_count++)  // set read time limit 100 time
		{
		for(i=0;i<19;i++){
			{
			if (mt_get_gpio_in(GPIO_CTP_EINT_PIN)==0)
				len = i2c_master_recv(i2c_client, buff_mutual[i], sizeof(buff_mutual[i]));
				printk("[ELAN] buff_mutual[%d][2]=0x%x\r\n", i, buff_mutual[i][2]);
			}
			msleep(5);
		}
		
		keep_on = 1;
		for(i=0;i<19;i++)
		{
			if(buff_mutual[i][2] == i)
			{
				keep_on = keep_on && 1;
			}else{
				keep_on = keep_on && 0;
			}
		}

		if (keep_on)
		{
			goto mutual_getdata_OK;
		}
		else
		{
			for(i=0;i<19;i++)
			{
				printk("[ELAN][Mutual-#%d-Channel]: %x %x %x %x %x %x %x %x\n",i ,buff_mutual[i][0], buff_mutual[i][1], buff_mutual[i][2], buff_mutual[i][3], buff_mutual[i][4], buff_mutual[i][5], buff_mutual[i][6], buff_mutual[i][7]);
				printk("[ELAN][Mutual-#%d-Channel]: %x %x %x %x %x %x %x %x\n",i ,buff_mutual[i][8], buff_mutual[i][9], buff_mutual[i][10], buff_mutual[i][11], buff_mutual[i][12], buff_mutual[i][13], buff_mutual[i][14], buff_mutual[i][15]);	
				printk("[ELAN][Mutual-#%d-Channel]: %x %x %x %x %x %x %x %x\n",i ,buff_mutual[i][16], buff_mutual[i][17], buff_mutual[i][18], buff_mutual[i][19], buff_mutual[i][20], buff_mutual[i][21], buff_mutual[i][22], buff_mutual[i][23]);	
				printk("[ELAN][Mutual-#%d-Channel]: %x %x %x\n",i ,buff_mutual[i][24], buff_mutual[i][25], buff_mutual[i][26]);	
			}
			buff_mutual[19][27] = 0;
		}
		
	}
	if(read_count == 100)
		result_tag = 2;

mutual_getdata_OK:	
	for(i=0; i<19; i++)
	{
		for(j=3; j<sizeof(buff_mutual[i])/sizeof(buff_mutual[i][0]); j+=2)
		{
			test_result = (buff_mutual[i][j] << 8) | buff_mutual[i][j+1];
			if(test_result>mutualtest_max || test_result<mutualtest_min)
			{
				result_tag = 1;
				printk("[ELAN] TP mutual-test FAIL node[%d][%d] = %d\n",i ,j/2-1, test_result);
				goto mutual_test_fail;
			}
//			printk("[ELAN] buff_mutual[%d][%d] = %d\r\n",i ,j/2-1, test_result);
		}
	}

mutual_test_fail:

	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
//test mode end
	if ((i2c_master_send(private_ts->client, cmd_mutualtest_end1, 4)) != 4)
	{
		printk("[ELAN] %s: i2c_master_send failed\n", __func__);
	}	
	if ((i2c_master_send(private_ts->client, cmd_mutualtest_end2, 4)) != 4)
	{
		printk("[ELAN] %s: i2c_master_send failed\n", __func__);
	}

	if(result_tag == 0) {
		num_read_chars += sprintf(&(buf[num_read_chars]), "Pass      \n");
	}else {
		if(result_tag == 1) {
			num_read_chars += sprintf(&(buf[num_read_chars]), "FAIL N[%d][%d]=%d\n",i ,j/2-1, test_result);
		}else if(result_tag == 2) {
			num_read_chars += sprintf(&(buf[num_read_chars]), "Test Overtime\n");
		}else if(result_tag == 3) {
			num_read_chars += sprintf(&(buf[num_read_chars]), "I2C send Fail\n");
		}
	}

//write raw data to file
	filp = filp_open(mutualtest_raw_file, O_CREAT | O_RDWR, 0666);
	if(!IS_ERR(filp))
	{
		printk("open return=%ld\n", IS_ERR(filp));
		fs=get_fs();
		set_fs(KERNEL_DS);
		filp->f_op->llseek(filp, 0, SEEK_SET);

		for(i=0; i<19; i++)
		{
			for(j=3; j<sizeof(buff_mutual[i])/sizeof(buff_mutual[i][0]); j+=2)
			{
				test_result = (buff_mutual[i][j] << 8) | buff_mutual[i][j+1];
				sprintf(mutualtest_buf,"Node[%d][%d] = %d\r\n",i ,j/2-1, test_result);
				filp->f_op->write(filp,(char *)mutualtest_buf, strlen(mutualtest_buf),&filp->f_pos);
			}
		}
		set_fs(fs);
	} else {
		printk("open file fail, return=%ld\n", IS_ERR(filp));
	}
	filp_close(filp,NULL);

	return num_read_chars;

}

static DRIVER_ATTR(mutual_test, 0664, ektf2k_mutual_test_show, NULL);


static ssize_t ektf2k_fw_upgrade_show(struct device_driver *ddri, const char *buf, size_t count)
{
	struct elan_ktf2k_ts_data *ts = private_ts;
	int ret = 0;
	work_lock=1;
	disable_irq(CUST_EINT_TOUCH_PANEL_NUM);
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

	ret = Update_FW_One(private_ts->client, 0x00);
	if(ret < 0)//for recovery
	{
	elan_recovery = IAPReset(private_ts->client);
		printk("[ELAN] RECOVERY=%x \n",elan_recovery);
	ret = Update_FW_One(private_ts->client, elan_recovery);
	}
	work_lock=0;
	enable_irq(CUST_EINT_TOUCH_PANEL_NUM);
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

	if(ret<0)
		return sprintf(buf, "Update_FW_One fail\n");
	else
		return sprintf(buf, "Update_FW_One success\n");
}

static DRIVER_ATTR(fw_upgrade, 0664, ektf2k_fw_upgrade_show, NULL);

static ssize_t ektf2k_device_test_show(struct device_driver *ddri, const char *buf, size_t count)
{
	struct elan_ktf2k_ts_data *ts = private_ts;
	uint8_t elan_cmd_fw_ver[]	= {0x53, 0x00, 0x00, 0x01}; //Get FW version
	uint8_t elan_cmd_fw_id[]	= {0x53, 0xf0, 0x00, 0x01}; //Get FW id
    uint8_t elan_buf_recv[4] 	= {0};
    int 	elan_major;
	int 	elan_minor;
	int 	elan_rc				=0;

	   
		//power on, need confirm with SA
		hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
		hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_1800, "TP");	  
	
		printk("[ELAN][%s] Power on.\n",__func__);
	
		msleep(10);
	
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	
		msleep(100);
		
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
		
		msleep(20);
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	
		msleep(100);
	
		mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
		mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
		mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
		mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
	
		mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
		mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
		mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 0);
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	
		msleep(100);
	
		tpd_load_status = 1;
	
		mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	
		printk("[ELAN][%s] check i2c functionality\n",__func__);
	
		if (!i2c_check_functionality(i2c_client->adapter, I2C_FUNC_I2C))
		{
			printk("[ELAN][%s] I2C fumctionality check error.\n",__func__);
	//		return -1;
		}
	
		printk("[ELAN][%s] read hello package.\n",__func__);
	
		if (elan_ktf2k_ts_setup(i2c_client))
		{
			printk(KERN_INFO "[ELAN][%s] No Elan chip inside\n",__func__);
			//fw_err = -ENODEV; 
		}
	
		//Firmware Version
		elan_rc = elan_ktf2k_ts_get_data(private_ts->client, elan_cmd_fw_ver, elan_buf_recv, 4);
		if (elan_rc < 0)
		{
	//		  return elan_rc;
		}
	
		elan_major = ((elan_buf_recv[1] & 0x0f) << 4) | ((elan_buf_recv[2] & 0xf0) >> 4);
		elan_minor = ((elan_buf_recv[2] & 0x0f) << 4) | ((elan_buf_recv[3] & 0xf0) >> 4);
		elan_fw_ver = elan_major << 8 | elan_minor;
	
		printk("[ELAN][%s]:TP FW Version=%d.\n",__func__,elan_fw_ver);

}

static DRIVER_ATTR(dev_test, 0664, ektf2k_device_test_show, NULL);


static ssize_t ektf2k_fw_version_show(struct device_driver *ddri, const char *buf, size_t count)
{
	struct elan_ktf2k_ts_data *ts = private_ts;

		printk("[ELAN][%s]:TP FW Version=%d.\n",__func__,elan_fw_ver);
	return sprintf(buf, "0x%4.4x\n", elan_fw_ver);
}

static DRIVER_ATTR(ektf2k_fw_version, 0664, ektf2k_fw_version_show, NULL);

static ssize_t tp_dbg_tag_onoff(struct device_driver *ddri, const char *buf, size_t count)
{
	if(buf[0] == '1')
	{
		EKTF2132_TP_DEBUG = 1;
		printk("[ELAN]tp_dbg_tag set enable\n");
		return sprintf(buf, "[ELAN]tp_dbg_tag set enable\n");
	}else if(buf[0] == '0'){
		EKTF2132_TP_DEBUG = 0;
		printk("[ELAN]tp_dbg_tag set disable\n");
		return sprintf(buf, "[ELAN]tp_dbg_tag set disable\n");
	}else{
		printk("[ELAN]invalid format\n");
		return sprintf(buf, "[ELAN]invalid format\n");
	}

}

static DRIVER_ATTR(dbg_tag_onoff, 0664, NULL, tp_dbg_tag_onoff);

static ssize_t Shipping_FW_version_show(struct device_driver *ddri, const char *buf, size_t count)
{
	return sprintf(buf, "Shipping_FW_version = 0x%4.4x\n",Latest_FW_version);
}

static DRIVER_ATTR(Shipping_FW_version, 0664, Shipping_FW_version_show, NULL);


static ssize_t FW_version_compare_show(struct device_driver *ddri, const char *buf, size_t count)
{
	if(elan_fw_ver==Latest_FW_version)
		return sprintf(buf, "1\n");
	else
		return sprintf(buf, "0\n");
}

static DRIVER_ATTR(FW_version_compare, 0664, FW_version_compare_show, NULL);

static struct driver_attribute *ektf2k_attr_list[] = {
    &driver_attr_tp_onoff,
	&driver_attr_self_test,
	&driver_attr_mutual_test,
	&driver_attr_fw_upgrade,
	&driver_attr_dev_test,
	&driver_attr_ektf2k_fw_version,
	&driver_attr_dbg_tag_onoff,
	&driver_attr_Shipping_FW_version,
	&driver_attr_FW_version_compare,
};

static int ektf2k_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(ektf2k_attr_list)/sizeof(ektf2k_attr_list[0]));

	for(idx = 0; idx < num; idx++)
	{
		if((err = driver_create_file(driver, ektf2k_attr_list[idx])))
		{            
			printk("[ELAN]driver_create_file (%s) = %d\n", ektf2k_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
/*----------------------------------------------------------------------------*/
static int ektf2k_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(ektf2k_attr_list)/sizeof(ektf2k_attr_list[0]));

	if (!driver)
	return -EINVAL;

	for (idx = 0; idx < num; idx++) 
	{
		driver_remove_file(driver, ektf2k_attr_list[idx]);
	}
	
	return err;
}

static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{	 
	static struct elan_ktf2k_ts_data ts;
	int 	retval 				= TPD_OK;
	//char 	data;
	uint8_t elan_cmd_x[] 		= {0x53, 0x60, 0x00, 0x00}; //Get x resolution
    uint8_t elan_cmd_y[] 		= {0x53, 0x63, 0x00, 0x00}; //Get y resolution
	uint8_t elan_cmd_fw_ver[]	= {0x53, 0x00, 0x00, 0x01}; //Get FW version
	uint8_t elan_cmd_fw_id[]	= {0x53, 0xf0, 0x00, 0x01}; //Get FW id
    uint8_t elan_buf_recv[4] 	= {0};
    int 	elan_major;
	int 	elan_minor;
    int 	elan_rc				=0;

	int New_FW_ID;	
	int New_FW_VER;	

	printk("[ELAN][%s] enter.\n",__func__);

	client->addr |= I2C_ENEXT_FLAG;
	i2c_client = client;
	i2c_client->timing = 400;
	private_ts = &ts;
	private_ts->client = client;
   
	#ifdef MT6577
	//power on, need confirm with SA
//	hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
//	hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_1800, "TP");	  
	#endif	

	printk("[ELAN][%s] Power on.\n",__func__);

	msleep(10);

	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

	msleep(100);
	
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	
	msleep(20);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

	msleep(100);

	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);

	mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 0);
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

	msleep(100);

	tpd_load_status = 1;

	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

	printk("[ELAN][%s] check i2c functionality\n",__func__);

	if (!i2c_check_functionality(i2c_client->adapter, I2C_FUNC_I2C))
	{
		printk("[ELAN][%s] I2C fumctionality check error.\n",__func__);
//		return -1;
	}

	printk("[ELAN][%s] read hello package.\n",__func__);

	if (elan_ktf2k_ts_setup(i2c_client))
	{
		printk(KERN_INFO "[ELAN][%s] No Elan chip inside\n",__func__);
		//fw_err = -ENODEV; 
	}

	//Firmware Version
    elan_rc = elan_ktf2k_ts_get_data(client, elan_cmd_fw_ver, elan_buf_recv, 4);
    if (elan_rc < 0)
    {
//        return elan_rc;
    }

    elan_major = ((elan_buf_recv[1] & 0x0f) << 4) | ((elan_buf_recv[2] & 0xf0) >> 4);
    elan_minor = ((elan_buf_recv[2] & 0x0f) << 4) | ((elan_buf_recv[3] & 0xf0) >> 4);
    elan_fw_ver = elan_major << 8 | elan_minor;

	printk("[ELAN][%s]:TP FW Version=%d.\n",__func__,elan_fw_ver);

	//Firmware ID
	elan_rc = elan_ktf2k_ts_get_data(client, elan_cmd_fw_id, elan_buf_recv, 4);
	if (elan_rc < 0)
	{
//		return elan_rc;
	}
	elan_major = ((elan_buf_recv[1] & 0x0f) << 4) | ((elan_buf_recv[2] & 0xf0) >> 4);
	elan_minor = ((elan_buf_recv[2] & 0x0f) << 4) | ((elan_buf_recv[3] & 0xf0) >> 4);
	elan_fw_id = elan_major << 8 | elan_minor;

	printk("[ELAN][%s]:TP FW ID=%d.\n",__func__,elan_fw_id);

	printk("[ELAN][%s] get RESOLUTION.\n",__func__);

	//get TP resolution
	// X Resolution
	elan_rc = elan_ktf2k_ts_get_data(client, elan_cmd_x, elan_buf_recv, 4);
	if (elan_rc < 0)
	{
//		return elan_rc;
	}
	elan_minor = ((elan_buf_recv[2])) | ((elan_buf_recv[3] & 0xf0) << 4);
	elan_tp_x_resolution = elan_minor;

	// Y Resolution 
	elan_rc = elan_ktf2k_ts_get_data(client, elan_cmd_y, elan_buf_recv, 4);
	if (elan_rc < 0)
	{
//		return elan_rc;
	}
	elan_minor = ((elan_buf_recv[2])) | ((elan_buf_recv[3] & 0xf0) << 4);
	elan_tp_y_resolution = elan_minor;
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

	printk("[ELAN][%s]:TP_X_RESOLUTION=%d,TP_Y_RESOLUTION=%d.\n",__func__,elan_tp_x_resolution,elan_tp_y_resolution);

	// Elan FW apk update
		// MISC
		ts.firmware.minor = MISC_DYNAMIC_MINOR;
		ts.firmware.name = "elan-iap";
		ts.firmware.fops = &elan_touch_fops;
		ts.firmware.mode = S_IRWXUGO;
		if (misc_register(&ts.firmware) < 0)
			printk("[ELAN] misc_register failed!!");
		else
		    printk("[ELAN] misc_register finished!!");
	// Elan FW apk update

	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	if (IS_ERR(thread))
	{
		retval = PTR_ERR(thread);
	}

#if IAP_PORTION
		if(1)
		{
			printk("[ELAN]misc_register finished!!");
			work_lock=1;
			disable_irq(CUST_EINT_TOUCH_PANEL_NUM);
			mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
			power_lock = 1;
	/* FW ID & FW VER*/
			printk("[ELAN]  [7bd0]=0x%02x,  [7bd1]=0x%02x, [7bd2]=0x%02x, [7bd3]=0x%02x\n",	file_fw_data[31696],file_fw_data[31697],file_fw_data[31698],file_fw_data[31699]);
			New_FW_ID = file_fw_data[31699]<<8	| file_fw_data[31698] ;
			New_FW_VER = file_fw_data[31697]<<8  | file_fw_data[31696] ;
			printk("[ELAN]  FW_ID=0x%x,	 New_FW_ID=0x%x \n",  elan_fw_id, New_FW_ID);
			printk("[ELAN] FW_VERSION=0x%x,   New_FW_VER=0x%x \n",  elan_fw_ver, New_FW_VER);
	/* for firmware auto-upgrade  */
			//if (New_FW_ID   ==  FW_ID){
/* disable auto fw upgrade
                           if (New_FW_VER > (elan_fw_ver)){
							Update_FW_One(client, elan_recovery);
							firmware_check_flag = 100+firmware_check_flag;
                           	}
*/
			//} else {						  
							//printk("FW_ID is different!");
			//}

			if (elan_fw_id == 0){
				elan_recovery = IAPReset(private_ts->client);
				printk("[ELAN] RECOVERY=%x \n",elan_recovery);
				Update_FW_One(private_ts->client, elan_recovery);
				firmware_check_flag = 110+firmware_check_flag;
				}

			work_lock=0;
			enable_irq(CUST_EINT_TOUCH_PANEL_NUM);
			mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

			//Firmware Version update again
			elan_rc = elan_ktf2k_ts_get_data(client, elan_cmd_fw_ver, elan_buf_recv, 4);
			if (elan_rc < 0)
			{
		//		  return elan_rc;
			}
			elan_major = ((elan_buf_recv[1] & 0x0f) << 4) | ((elan_buf_recv[2] & 0xf0) >> 4);
			elan_minor = ((elan_buf_recv[2] & 0x0f) << 4) | ((elan_buf_recv[3] & 0xf0) >> 4);
			elan_fw_ver = elan_major << 8 | elan_minor;
			
		}
#endif
	ektf2k_create_attr(&tpd_i2c_driver.driver);

	return 0;
}

static int __devexit tpd_remove(struct i2c_client *client)
{
    ektf2k_delete_attr(&tpd_i2c_driver.driver);
	return 0;
}
 
 
//*******************************************************************************************
// MTK callback function                                                                    |
//*******************************************************************************************
static int tpd_local_init(void)
{
	if(i2c_add_driver(&tpd_i2c_driver)!=0)
	{
		if(EKTF2132_TP_DEBUG){
		printk("[ELAN][%s]: i2c add driver fail.\n",__func__);
			}

      	return -1;
	}

	if(tpd_load_status == 0) 
	{
		i2c_del_driver(&tpd_i2c_driver);
		return -1;
	}
	
	#ifdef TPD_HAVE_BUTTON    
		if(EKTF2132_TP_DEBUG){
        printk("[ELAN][%s]:TPD_HAS_BUTTON.\n",__func__);
			}
		tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
	#endif   
  
	#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
 		TPD_DO_WARP = 1;
		memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
		memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
	#endif 

	tpd_type_cap = 1;
	return 0; 
}

static void tpd_resume(struct early_suspend *h )
{
	uint8_t cmd[] = {0x54, 0x58, 0x00, 0x01};
    printk("[ELAN][%s] Touch Driver Resume.\n",__func__);

	printk("*******[QUALITY:ektf2132:%s:F7.2:Entry]>>>>>>>",__func__);
/*
	#ifdef MT6577
    //power on, need confirm with SA
    hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
    hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_1800, "TP");
    #endif

    msleep(10);

    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

    msleep(100);

    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);

    msleep(20);
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	
	msleep(200);
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
*/
	if(work_lock)
		return ;
/* Work around:use reset sensor instead of low power mode when suspend
	if ((i2c_master_send(i2c_client, cmd, sizeof(cmd))) != sizeof(cmd)) 
	{
		printk("[ELAN] %s: i2c_master_send failed\n", __func__);
		return -EINVAL;
	}
*/
// Work around:use reset sensor instead of low power mode when suspend
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

	msleep(200);
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

	printk("*******[QUALITY:ektf2132:%s:F7.2:Exit]<<<<<<<",__func__);	

	return;
}

static void tpd_suspend(struct early_suspend *h )
{
	uint8_t cmd[] = {0x54, 0x50, 0x00, 0x01};
	
	printk("[ELAN][%s] Touch Driver suspend.\n",__func__);

	printk("*******[QUALITY:ektf2132:%s:F7.2:Entry]>>>>>>>",__func__);
/*
    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
    hwPowerDown(MT65XX_POWER_LDO_VGP, "TP");
	hwPowerDown(MT65XX_POWER_LDO_VGP2,"TP");
*/
	if(work_lock)
			return ;

    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);

// Work around:use reset sensor instead of low power mode when suspend

	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
/* Work around:use reset sensor instead of low power mode when suspend
	if ((i2c_master_send(i2c_client, cmd, sizeof(cmd))) != sizeof(cmd)) 
	{
		printk(&i2c_client->dev,"[ELAN] %s: i2c_master_send failed\n", __func__);
		return -EINVAL;
	}
*/
	printk("*******[QUALITY:ektf2132:%s:F7.2:Exit]<<<<<<<",__func__);

    return ;
} 


//*******************************************************************************************
// MTK driver object	                                                                    |
//*******************************************************************************************
static struct tpd_driver_t tpd_device_driver = 
{
	.tpd_device_name = "EKTF2132",
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
	#ifdef TPD_HAVE_BUTTON
	.tpd_have_button = 1,
	#else
	.tpd_have_button = 0,
	#endif		
};

//*******************************************************************************************
// #02# ioctl interface                                                                     |
//*******************************************************************************************
static  int             fw_version_release(struct inode *, struct file *);
static  int             fw_version_open(struct inode *, struct file *);
static  ssize_t         fw_version_write(struct file *file, const char *buf, size_t count, loff_t *ppos);
static  ssize_t         fw_version_read(struct file *file, char *buf, size_t count, loff_t *ppos);
static  struct          cdev fw_version_cdev;
static  struct          class *fw_version_class;
static  int             fw_version_major = 0;

static int  fw_version_open(struct inode *inode, struct file *filp)
{
        printk("--------------------open-------------------\n");
        return 0;
}
EXPORT_SYMBOL(fw_version_open);

static int fw_version_release(struct inode *inode, struct file *filp)
{
        return 0;
}
EXPORT_SYMBOL(fw_version_release);

static ssize_t  fw_version_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
        int ret;
        char *tmp;

        printk("-------------------write---------------------%s\n",buf);

		/*
        if (count > 8192)
                count = 8192;

        tmp = (char *)kmalloc(count,GFP_KERNEL);
        if (tmp==NULL)
                return -ENOMEM;
        if (copy_from_user(tmp,buf,count)) {
                kfree(tmp);
                return -EFAULT;
        }

        ret = i2c_master_send(i2c_client, tmp, count);
        kfree(tmp);
		*/
        return ret;
}
EXPORT_SYMBOL(fw_version_write);

static ssize_t  fw_version_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
        u8 fw_version =0;
        int ret =1;

		fw_version = elan_fw_ver;

        printk("------------------------read--------------------------\n");

        //ret = i2c_smbus_read_i2c_block_data(i2c_client, 0xa6, 1, &fw_version);

        printk("----CCI----%d \n",elan_fw_ver);

		//ret = i2c_master_recv(i2c_client, tmp, count);
        //if (ret >= 0)
                ret = copy_to_user(buf,&elan_fw_ver,1)?-EFAULT:ret;
        return ret;
}
EXPORT_SYMBOL(fw_version_read);

static struct file_operations nc_fops = {
        .owner =        THIS_MODULE,
        .write          = fw_version_write,
        .read           = fw_version_read,
        .open           = fw_version_open,
       // .unlocked_ioctl = elan_iap_ioctl,
        .release        = fw_version_release,
};


//*******************************************************************************************
// Linux module callback                                                                    |
//*******************************************************************************************
static int __init tpd_driver_init(void) 
{
	printk("[ELAN][%s] EKTF2132 touch panel driver init\n",__func__);
	i2c_register_board_info(0, &ektf2132_i2c_tpd, 1);
	if(tpd_driver_add(&tpd_device_driver) < 0)
	{
		printk("[ELAN][%s] Add EKTF2132 driver failed\n",__func__);
	}
	
	//OKOK #02# for fw version start
	int result;
	int err = 0;

	dev_t devno = MKDEV(fw_version_major, 0);
	result  = alloc_chrdev_region(&devno, 0, 1, "fwVersionDev");
	if(result < 0)
	{
		printk("[ELAN]fail to allocate chrdev (%d) \n", result);
		return 0;
	}
	fw_version_major = MAJOR(devno);
	cdev_init(&fw_version_cdev, &nc_fops);
	fw_version_cdev.owner = THIS_MODULE;
	fw_version_cdev.ops = &nc_fops;
	err =  cdev_add(&fw_version_cdev, devno, 1);
	if(err)
	{
		printk("[ELAN]fail to add cdev (%d) \n", err);
		return 0;
	}

	fw_version_class = class_create(THIS_MODULE, "fwVersionDev");
	if (IS_ERR(fw_version_class)) 
	{
		result = PTR_ERR(fw_version_class);
		unregister_chrdev(fw_version_major, "fwVersionDev");
		printk("[ELAN]fail to create class (%d) \n", result);
		return result;
	}
	device_create(fw_version_class, NULL, MKDEV(fw_version_major, 0), NULL, "fwVersionDev");
	//OKOK #02# for fw version end

	return 0;
}
 
static void __exit tpd_driver_exit(void) 
{
	printk("[ELAN][%s] MediaTek EKTF2132 touch panel driver exit\n",__func__);
	//input_unregister_device(tpd->dev);
	tpd_driver_remove(&tpd_device_driver);
}
 
module_init(tpd_driver_init);
module_exit(tpd_driver_exit);



