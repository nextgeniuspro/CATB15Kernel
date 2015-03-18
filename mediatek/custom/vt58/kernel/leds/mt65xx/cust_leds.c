#include <cust_leds.h>
#include <mach/mt_pwm.h>

#include <linux/kernel.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>

extern int mtkfb_set_backlight_level(unsigned int level);
extern int mtkfb_set_backlight_pwm(int div);

#define ERROR_BL_LEVEL 0xFFFFFFFF
unsigned int brightness_mapping(unsigned int level)
{
    if(level>=30 && level<=255) { // user changable by using Setting->Display->Brightness
		return (level-14)/18; //Jacky update for not entering Case 15 
	}
	else if(level>0 && level<30) { // used to fade out for 7 seconds before shut down backlight
		return 0;
	}
	return ERROR_BL_LEVEL;
}

unsigned int Cust_SetBacklight(int level, int div)
{
    mtkfb_set_backlight_pwm(div);
    mtkfb_set_backlight_level(brightness_mapping(level));
    return 0;
}

static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	//{"red",               MT65XX_LED_MODE_NONE, -1,{0}},
	//{"green",             MT65XX_LED_MODE_NONE, -1,{0}},
	//{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK4,{0}},
	{"green",             MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK4,{0}},
	{"blue",              MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK4,{0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_BUTTON,{0}},
	//{"lcd-backlight",     MT65XX_LED_MODE_CUST, (int)Cust_SetBacklight,{0}},
	{"lcd-backlight",     MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_LCD_BOOST,{0}},
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}

