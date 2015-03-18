#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>
//#include <mach/mt6577_pm_ldo.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 0,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x72, 0x48, 0x78, 0x00},
    /*Lenovo-sw chenlj2 add 2011-06-03,modify parameter below two lines*/
    //.als_level  = { 4, 40,  80,   120,   160, 250,  400, 800, 1200,  1600, 2000, 3000, 5000, 10000, 65535},
    //.als_value  = {10, 20,20,  120, 120, 280,  280,  280, 1600,  1600,  1600,  6000,  6000, 9000,  10240, 10240},
//[BugId 230]VT58 modify als_level, als_value table start 20130111
    //.als_level  = { 5, 10,  25,   50,  100, 150,  200, 500, 1000,  1500, 2000, 3000, 5000, 8000, 10000},
    //.als_value  = {0, 50,  100,  150, 200, 250,  300,  300, 1600,  1600,  1600,  6000,  6000, 9000,  10240, 10240},
//[BugId 230]VT58 modify als_level, als_value table end 20130111
//[BugId 312] Colby add for update ALS mapping table from AVAGO start 20130130
	.als_level  = { 5, 15, 65, 130,  180,  240, 600,  2400, 7500, 12000, 20000},
	.als_value  = {0, 10, 50,  100,  150, 200,  280,  1600,  6000, 9000, 10240, 10240},
//[BugId 312] Colby add for update ALS mapping table from AVAGO end 20130130
    .ps_threshold_high = 950,
    .ps_threshold_low = 850,
    .ps_threshold = 900,
};
struct alsps_hw *get_cust_alsps_hw(void) {
	 cust_alsps_hw.power_id = MT65XX_POWER_NONE;
     cust_alsps_hw.power_vol  = VOL_DEFAULT;
    return &cust_alsps_hw;
}
int APDS9930_CMM_PPCOUNT_VALUE = 0x0A;
int APDS9930_CMM_CONTROL_VALUE = 0x20; //0x64;
int APDS9930_CMM_POFFSET_VALUE = 0x0;//0x7F;
int ZOOM_TIME = 4;
