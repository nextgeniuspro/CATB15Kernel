#include <asm/arch/mt6577_gpio.h>
#include <cci_drv_common.h>
#if 1 /* ***** CCI BSP.SYS ***** */
#include <asm/arch/mt65xx_typedefs.h>//weli
#endif /* ***** CCI BSP.SYS ***** */



/*
 * owner: brook start
 */
/************************************************/
/*                                              */
/*    Phase_ID:                                 */
/*              GPIO102  GPIO101  GPIO100       */
/*    DVT        0        0        0            */
/*    PVT        0        0        1            */
/*                                              */
/*                                              */
/*    Project_ID:                               */
/*              GPIO105                         */
/*    VT58       0                              */
/*                                              */
/*                                              */
/*    Band_ID:                                  */
/*              GPIO109                         */
/*    Band 1,8   0                              */
/*    Band 2,5   1                              */
/************************************************/
                                                    
//         GPIO100  GPIO101   GPIO102   GPIO105   GPIO109  
//         (HW_ID1) (HW_ID2)  (HW_ID3)  (HW_ID4)  (HW_ID5) 
//  0x00 |  0        0         0         0           0      VT58_DVT_Band1,8                    
//  0x01 |  1        0         0         0           0      VT58_PVT_Band1,8
//  0x10 |  0        0         0         0           1      VT58_DVT_Band2,5
//  0x11 |  1        0         0         0           1      VT58_PVT_Band2,5

static s32 platform_id = -1;

static void cci_get_platform_id(void)
{
	platform_id = 0;
/*
	platform_id |= (mt_get_gpio_out(100)&0x01)?0x01:0x00;
	platform_id |= (mt_get_gpio_out(101)&0x01)?0x02:0x00;
	platform_id |= (mt_get_gpio_out(102)&0x01)?0x04:0x00;
	platform_id |= (mt_get_gpio_out(105)&0x01)?0x08:0x00;
	platform_id |= (mt_get_gpio_out(109)&0x01)?0x10:0x00;
*/
	platform_id |= (mt_get_gpio_in(100)&0x01)?0x01:0x00;
	platform_id |= (mt_get_gpio_in(101)&0x01)?0x02:0x00;
	platform_id |= (mt_get_gpio_in(102)&0x01)?0x04:0x00;
	platform_id |= (mt_get_gpio_in(105)&0x01)?0x08:0x00;
	platform_id |= (mt_get_gpio_in(109)&0x01)?0x10:0x00;
// CL update for codebase sharing GPIO Issue
}

/*bool cci_platform_is_ha01(void)
{
        u32 value = 0;
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id                    *\n");
        printf("*  cci_platform_is_ha01, platform_id = %d       *\n",platform_id);
        printf("*************************************************\n");
        value = platform_id & 0x08;
        if(value == 0x00)
          return true;
        return false;
}

bool cci_platform_is_ha03(void)
{
        u32 value;
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        value = platform_id & 0x08;
        if(value == 0x08)
          return true;
        return false;
}

bool cci_platform_is_ha01_dvt1(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x00 || platform_id == 0x10)
          return true;
        return false;
}

bool cci_platform_is_ha03_dvt1(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x08 || platform_id == 0x18)
          return true;
        return false;
}

bool cci_platform_is_ha01_dvt2(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x01 || platform_id == 0x11)
          return true;
        return false;
}

bool cci_platform_is_ha03_dvt2(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x09 || platform_id == 0x19)
          return true;
        return false;
}

bool cci_platform_is_ha01_pvt(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x02 || platform_id == 0x12)
          return true;
        return false;
}

bool cci_platform_is_ha03_pvt(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x0A || platform_id == 0x1A)
          return true;
        return false;
}
bool cci_platform_is_ha05_dvt(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x0D || platform_id == 0x1D)
          return true;
        return false;
}
bool cci_platform_is_ha05_pvt(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x0E || platform_id == 0x1E)
          return true;
        return false;
}
bool cci_platform_is_ha05_mp(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x0F || platform_id == 0x1F)
          return true;
        return false;
}*/
bool cci_platform_is_dvt(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x00 || platform_id == 0x10)
          return true;
        return false;
}
bool cci_platform_is_pvt(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x01 || platform_id == 0x11)
          return true;
        return false;
}
bool cci_platform_is_band_1_8(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x00 || platform_id == 0x01)
          return true;
        return false;
}
bool cci_platform_is_band_2_5(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if(platform_id == 0x10 || platform_id == 0x11)
          return true;
        return false;
}
/*
 * owner: brook end
 */

#if 0 /* ***** CCI BSP.SYS ***** */
/*****************************************************************/
/*
 * owner: weli start
 */
int g_R_BAT_SENSE = 0;
int g_R_I_SENSE = 0;
int g_R_CHARGER_1 = 0;
int g_R_CHARGER_2 = 0;

//////////////////////////////////////////////////////////////////////////////////
void bmt_charger_ov_check(void)
{
	if(cci_platform_is_ha01())
	{
		ha01_bmt_charger_ov_check();
	}
	else
	{
		ha03_bmt_charger_ov_check();
	}
}

kal_bool pmic_chrdet_status(void)
{
	if(cci_platform_is_ha01())
	{
		ha01_pmic_chrdet_status();
	}
	else
	{
		ha03_pmic_chrdet_status();
	}
}
/*
 * owner: weli end
 */
/******************************************************************/

#endif /* ***** CCI BSP.SYS ***** */
