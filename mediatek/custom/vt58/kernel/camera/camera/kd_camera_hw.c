#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    printk(KERN_INFO PFX "%s: " fmt, __FUNCTION__ ,##arg)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)         printk(KERN_ERR PFX "%s: " fmt, __FUNCTION__ ,##arg)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#endif
/*[VT58] S Vince Tseng, OPTICAL team usage*/
#define OPTICAL
/*[VT58] E Vince Tseng, OPTICAL team usage*/
 


int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{
u32 pinSetIdx = 0;//default main sensor

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4

#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3
u32 pinSet[3][8] =
					{
                    //for main sensor
/*[VT58] S Vince Tseng, OPTICAL team usage*/
#if defined(OPTICAL)//for MT9p017
					{GPIO_CAMERA_CMRST_PIN,
							GPIO_CAMERA_CMRST_PIN_M_GPIO,   /* mode */
							GPIO_OUT_ONE,                   /* ON state */
							GPIO_OUT_ZERO,                  /* OFF state */
							GPIO_CAMERA_CMRST_PIN,
					 GPIO_CAMERA_CMRST_PIN_M_GPIO,
							GPIO_OUT_ONE,
							GPIO_OUT_ZERO,
					},

#else /*ORG*/
                    {GPIO_CAMERA_CMRST_PIN,
                        GPIO_CAMERA_CMRST_PIN_M_GPIO,   /* mode */
                        GPIO_OUT_ONE,                   /* ON state */
                        GPIO_OUT_ZERO,                  /* OFF state */
                     GPIO_CAMERA_CMPDN_PIN,
                        GPIO_CAMERA_CMPDN_PIN_M_GPIO,
                        GPIO_OUT_ZERO,
                        GPIO_OUT_ONE,
                    },
#endif
/*[VT58] E Vince Tseng, OPTICAL team usage*/
					//for sub sensor
                    {GPIO_CAMERA_CMRST1_PIN,
                     GPIO_CAMERA_CMRST1_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                     GPIO_CAMERA_CMPDN1_PIN,
                        GPIO_CAMERA_CMPDN1_PIN_M_GPIO,
                        GPIO_OUT_ZERO,
                        GPIO_OUT_ONE,
                    },
                    //for main_2 sensor 
                    {GPIO_CAMERA_2_CMRST_PIN,
                        GPIO_CAMERA_2_CMRST_PIN_M_GPIO,   /* mode */
                        GPIO_OUT_ONE,                   /* ON state */
                        GPIO_OUT_ZERO,                  /* OFF state */
                     GPIO_CAMERA_2_CMPDN_PIN,
                        GPIO_CAMERA_2_CMPDN_PIN_M_GPIO,
                        GPIO_OUT_ZERO,
                        GPIO_OUT_ONE,
                    }
                   };

    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
    }
    else if (DUAL_CAMERA_MAIN_SECOND_SENSOR == SensorIdx) {
        pinSetIdx = 2;
    }

    //power ON
    if (On) {
        //in case

#if 0 //TODO: depends on HW layout. Should be notified by SA.
        printk("Set CAMERA_POWER_PULL_PIN for power \n"); 
        if (mt_set_gpio_pull_enable(GPIO_CAMERA_LDO_EN_PIN, GPIO_PULL_DISABLE)) {PK_DBG("[[CAMERA SENSOR] Set CAMERA_POWER_PULL_PIN DISABLE ! \n"); }
        if(mt_set_gpio_mode(GPIO_CAMERA_LDO_EN_PIN, GPIO_CAMERA_LDO_EN_PIN_M_GPIO)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN mode failed!! \n");}
        if(mt_set_gpio_dir(GPIO_CAMERA_LDO_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN dir failed!! \n");}
        if(mt_set_gpio_out(GPIO_CAMERA_LDO_EN_PIN,GPIO_OUT_ONE)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN failed!! \n");}
#endif
        /*[VV58] S Vince Tseng, modify power up sequence*/
        #if defined(OPTICAL)
        /**/
                /*Power on Sequence S*/
                /*----------PUS Main MT9P017----------*/
                if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_MT9P017_RAW,currSensorName)))
                {
                	//1. VOVDD 1.8V
                    PK_DBG("[MT9P017]  DOVDD ON 1.8VsensorIdx:%d \n",SensorIdx);
                	if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
                    {
                        PK_DBG("[CAMERA SENSOR] Fail to enable digital IO power 1.8V\n");
                        //return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }
                    mdelay(1);

                	//2. DVDD 1.8V
                	PK_DBG("[MT9P017] DVDD ON 1.8V sensorIdx:%d \n",SensorIdx);
                    if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1800,mode_name))
                    {
                        PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                        //return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }
                    mdelay(1);

                	//3. PDN/RST High
                	PK_DBG("[MT9P017] PDN/RST High sensorIdx:%d \n",SensorIdx);
                	PK_DBG("[Vince Deubg] PDN/RST High: %d, will do PDN/RST High\n",pinSetIdx); //Debug Usage
                	if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                	{
                		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
                	}
                    mdelay(2);

                	//4. AVDD 2.8V
                	PK_DBG("[MT9P017]AVDD ON 2.8V sensorIdx:%d \n",SensorIdx);
                    if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
                    {
                		PK_DBG("[V_CAMERA SENSOR] M Fail to enable analog power AVDD\n");
                		//return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }
                	mdelay(2);

                	//5. AFVDD 2.8V
                	PK_DBG("[MT9P017] AF ON 2.8V sensorIdx:%d \n",SensorIdx);
                    if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name))
                    {
                        PK_DBG("[V_CAMERA SENSOR] M Fail to enable analog power AFVDD\n");
                        //return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }
                	mdelay(5);

                	//6. disable inactive sensor
                    if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMPDN])
                	{
                	//PDN
                		if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                		if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                        if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
                	//RST
                		if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                		if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                        if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
                	}
                	//finished
                }
                /*----------PUS Main MT9P017----------*/
                /*----------PUS Sub OV7692_YUV----------*/
                if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV7692_YUV,currSensorName)))
                {
                //1. VOVDD ON 1.8 V
                    PK_DBG("[OV7692] VOVDD ON 1.8V sensorIdx:%d \n",SensorIdx);
                	if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
                    {
                        PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                        //return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }
                    mdelay(1);

                //2. AVDD ON 2.8V
                    PK_DBG("[OV7692] AVDD ON 2.8V sensorIdx:%d \n",SensorIdx);
                    if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
                    {
                        PK_DBG("[V_CAMERA SENSOR] S Fail to enable analog power AVDD\n");
                        //return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }
                	//AVDD Enable
                	if(pinSetIdx == 1 && mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
                    if(pinSetIdx == 1 && mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
                    if(pinSetIdx == 1 && mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
                    mdelay(2);

                //3. PDN "LOW" //Note: Sub_CMRST used to enable AVDD
                    if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                	{
                        //PDN pin
                        if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                        if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                        if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
                    }
					/*[VT58][BugID:456] Mark modify delay from 5ms to 20ms for vendor suggestion*/
                	//mdelay(5);
                	mdelay(20);

                //4. disable inactive sensor
                	if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMPDN])
                	{
                	//PDN
                		/*if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                		if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                        if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module*/
                	//RST
                		if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                		if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                        if(mt_set_gpio_out(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
                	}
                //finished
                }
                /*----------PUS Sub OV7692_YUV----------*/
                /*Power on Sequence E*/
        /**/
        #else /*ORG*/
        if(pinSetIdx == 0) {

            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
        }
        
        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
        {
            PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
            //return -EIO;
            goto _kdCISModulePowerOn_exit_;
        }                    
        mdelay(1);
        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
        {
            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
            //return -EIO;
            goto _kdCISModulePowerOn_exit_;
        }

        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name))
        {
             PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
             //return -EIO;
             goto _kdCISModulePowerOn_exit_;
        }
        
        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name))
        {
            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
            //return -EIO;
            goto _kdCISModulePowerOn_exit_;
        }        
        mdelay(10);
        if(pinSetIdx == 0) {

            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
        }

        //disable inactive sensor
        if (GPIO_CAMERA_INVALID != pinSet[1-pinSetIdx][IDX_PS_CMRST]) {
            if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
            if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
        }        

        //enable active sensor
        //RST pin
        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
            mdelay(10);
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
            mdelay(1);

            //PDN pin
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
        }
        #endif
        /*[VV58] E Vince Tseng, modify power up sequence*/
    }
    else {//power OFF

#if 0 //TODO: depends on HW layout. Should be notified by SA.
        printk("Set GPIO 94 for power OFF\n"); 
        if (mt_set_gpio_pull_enable(GPIO_CAMERA_LDO_EN_PIN, GPIO_PULL_DISABLE)) {PK_DBG("[CAMERA SENSOR] Set GPIO94 PULL DISABLE ! \n"); }
        if(mt_set_gpio_mode(GPIO_CAMERA_LDO_EN_PIN, GPIO_CAMERA_LDO_EN_PIN_M_GPIO)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
        if(mt_set_gpio_dir(GPIO_CAMERA_LDO_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
        if(mt_set_gpio_out(GPIO_CAMERA_LDO_EN_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}    	    
#endif
        /*[VT58] S Vince Tseng, modify power down sequence*/
#if defined(OPTICAL)
        		/**/
                /*Power off Sequence S*/
                if (pinSetIdx == 0)
                {
                /*----------PDS Main MT9P017_RAW----------*/
                PK_DBG("[Vince Debug][MT9P017] PDS");
                	//1. PDN/RST Low
                	PK_DBG("[MT9P017] PDN/RST Low sensorIdx:%d \n",SensorIdx);
                	if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                	{
                		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
                	}
                	//2. VOVDD 1.8V
                    PK_DBG("[MT9P017]  DOVDD OFF 1.8VsensorIdx:%d \n",SensorIdx);
                	if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2, mode_name))
                    {
                        PK_DBG("[CAMERA SENSOR] Fail to disable digital IO power 1.8V\n");
                        //return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }

                	//3. AVDD 2.8V
                	PK_DBG("[MT9P017]AVDD OFF 2.8V sensorIdx:%d \n",SensorIdx);
                    if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A, mode_name))
                    {
                		PK_DBG("[CAMERA SENSOR] Fail to disable analog power\n");
                		//return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }

                	//4. DVDD 1.8V
                	PK_DBG("[MT9P017] DVDD OFF 1.8V sensorIdx:%d \n",SensorIdx);
                    if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name))
                    {
                        PK_DBG("[CAMERA SENSOR] Fail to disable digital power\n");
                        //return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }

                	//5. AFVDD 2.8V
                	PK_DBG("[MT9P017] AF DVDD 2.8V OFF sensorIdx:%d \n",SensorIdx);
                    if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,mode_name))
                    {
                        PK_DBG("[CAMERA SENSOR] Fail to disable analog power\n");
                        //return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }
                	mdelay(10);
                }
                /*----------PDS Main MT9P017_RAW----------*/
                /*----------PDS Sub OV7692_YUV----------*/
                if (pinSetIdx == 1)
                {
               	PK_DBG("[Vince Debug][OV7692_YUV] PDS");
                //1. PDN "High" //Note: Sub_CMRST used to enable AVDD
                    if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                	{
                        //PDN pin
                        if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                        if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                        if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
                    }

                //2. AVDD ON 2.8V
                    PK_DBG("[OV7692] AVDD OFF 2.8V sensorIdx:%d \n",SensorIdx);
                    if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A, mode_name))
                    {
                        PK_DBG("[CAMERA SENSOR] Fail to disable analog power\n");
                        //return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }
                	//AVDD disable
                	if(pinSetIdx == 1 && mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
                    if(pinSetIdx == 1 && mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
                    if(pinSetIdx == 1 && mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}

                //3. VOVDD ON 1.8 V
                    PK_DBG("[OV7692] VOVDD OFF 1.8V sensorIdx:%d \n",SensorIdx);
                	if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
                    {
                        PK_DBG("[CAMERA SENSOR] Fail to disable digital power\n");
                        //return -EIO;
                        goto _kdCISModulePowerOn_exit_;
                    }
                    mdelay(10);
                }
                /*----------PDS Sub OV7692_YUV----------*/
                /*Power off Sequence E*/
                /**/
#else /*ORG*/

                //PK_DBG("[OFF]sensorIdx:%d \n",SensorIdx);
                if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                    if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
                    if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                    if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
                    if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                    if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
            	    if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
                }


                if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,mode_name))
                {
                    PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
                    //return -EIO;
                    goto _kdCISModulePowerOn_exit_;
                }
                mdelay(1);
                if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
                    PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
                    //return -EIO;
                    goto _kdCISModulePowerOn_exit_;
                }
                mdelay(1);
            	if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
                    PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
                    //return -EIO;
                    goto _kdCISModulePowerOn_exit_;
                }
                mdelay(1);
                if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
                {
                    PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                    //return -EIO;
                    goto _kdCISModulePowerOn_exit_;
                }

#endif
    /*[VT58] E Vince Tseng, modify power down sequence*/
    }//

	return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;
}

EXPORT_SYMBOL(kdCISModulePowerOn);



