#ifndef _CCI_DRV_COMMON_H
#define _CCI_DRV_COMMON_H


extern int setup_cci_project_id(char*);
extern int setup_cci_emcp_id(char*);
extern int setup_cci_project(char*);
extern int setup_cci_boot_reason(char*);
//extern int cci_platform_is_ha01(void);
//extern int cci_platform_is_ha03(void);
//extern int cci_platform_is_dvt1(void);
//extern int cci_platform_is_dvt2(void);

extern int cci_project_id(void);

extern int cci_platform_is_pvt(void);
extern int cci_platform_is_pvt_sb(void);
extern int cci_platform_is_pvt_sc(void);
extern int cci_platform_is_pvt_sl_sb_sc(void);
extern int cci_platform_is_pvt_sb_sc(void);
extern int cci_platform_is_dvt(void);
extern int cci_platform_is_band_1_8(void);
extern int cci_platform_is_band_2_5(void);
extern int cci_platform_is_unknown_phase(void);
extern int cci_platform_is_unknown_band(void);
//extern int cci_platform_is_mp(void);
/*
 * owner: brook end
 */
#if 0 /* ***** CCI BSP.SYS ***** */
/*weli begin***************************************/
#include <linux/types.h>
#include <linux/wakelock.h>
#include <linux/fs.h>
#include <linux/power_supply.h>
#include <mach/mt6575_typedefs.h>
#include <common_cust_fuel_gauge.h>//weli revise share codebase issue @20120216
#include <common_cust_battery.h>//weli revise share codebase issue @20120216
#include <../../drivers/power/mt6575_battery.h>
#define UINT32 unsigned long
#define UINT16 unsigned short
#define UINT8 unsigned char 
////////////////////////////////////////////////////

//////////////////////////////////////////////////
//kal_uint32 g_eco_version = 0;
///////////////////////////////////////////////////////////////////////////////////////////
//// Global Variable
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct 
{
    kal_bool   	bat_exist;
    kal_bool   	bat_full;  
    kal_bool   	bat_low;  
    UINT32  	bat_charging_state;
    UINT32  	bat_vol;            
    kal_bool 	charger_exist;   
    UINT32  	pre_charging_current;
    UINT32  	charging_current;
    UINT32  	charger_vol;        
    UINT32   	charger_protect_status; 
    UINT32  	ISENSE;                
    UINT32  	ICharging;
    INT32   	temperature;
    UINT32  	total_charging_time;
    UINT32  	PRE_charging_time;
	UINT32  	CC_charging_time;
	UINT32  	TOPOFF_charging_time;
	UINT32  	POSTFULL_charging_time;
    UINT32   	charger_type;
    UINT32   	PWR_SRC;
    UINT32   	SOC;
    UINT32   	ADC_BAT_SENSE;
    UINT32   	ADC_I_SENSE;
} PMU_ChargerStruct;

typedef enum 
{
    PMU_STATUS_OK = 0,
    PMU_STATUS_FAIL = 1,
}PMU_STATUS;
//static CHARGER_TYPE CHR_Type_num = CHARGER_UNKNOWN;
//PMU_ChargerStruct BMT_status;


struct mt6575_ac_data {
    struct power_supply psy;
    int AC_ONLINE;    
};

struct mt6575_usb_data {
    struct power_supply psy;
    int USB_ONLINE;    
};

struct mt6575_battery_data {
    struct power_supply psy;
    int BAT_STATUS;
    int BAT_HEALTH;
    int BAT_PRESENT;
    int BAT_TECHNOLOGY;
    int BAT_CAPACITY;
    //Add for Battery Service
    int BAT_batt_vol;
    int BAT_batt_temp;
    //Add for EM 
    int BAT_TemperatureR;
    int BAT_TempBattVoltage;
    int BAT_InstatVolt;
    int BAT_BatteryAverageCurrent;
    int BAT_BatterySenseVoltage;
    int BAT_ISenseVoltage;
    int BAT_ChargerVoltage;
};

static enum power_supply_property mt6575_ac_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property mt6575_usb_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property mt6575_battery_props[] = {
    POWER_SUPPLY_PROP_STATUS,
    POWER_SUPPLY_PROP_HEALTH,
    POWER_SUPPLY_PROP_PRESENT,
    POWER_SUPPLY_PROP_TECHNOLOGY,
    POWER_SUPPLY_PROP_CAPACITY,
    //Add for Battery Service 
    POWER_SUPPLY_PROP_batt_vol,
    POWER_SUPPLY_PROP_batt_temp,    
    //Add for EM 
    POWER_SUPPLY_PROP_TemperatureR,
    POWER_SUPPLY_PROP_TempBattVoltage,
    POWER_SUPPLY_PROP_InstatVolt,
    POWER_SUPPLY_PROP_BatteryAverageCurrent,
    POWER_SUPPLY_PROP_BatterySenseVoltage,
    POWER_SUPPLY_PROP_ISenseVoltage,
    POWER_SUPPLY_PROP_ChargerVoltage,
};
/////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//// Battery Temprature Parameters and functions
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct{
    INT32 BatteryTemp;
    INT32 TemperatureR;
}BATT_TEMPERATURE;


void BatThread_XGPTConfig(void);
extern void ha01_BatThread_XGPTConfig(void);
extern void ha03_BatThread_XGPTConfig(void);

void get_pmic_auxadc_reg(void);
extern void ha01_get_pmic_auxadc_reg(void);
extern void ha03_get_pmic_auxadc_reg(void);

void get_pmic_auxadc_reg_all(void);
extern void ha01_get_pmic_auxadc_reg_all(void);
extern void ha03_get_pmic_auxadc_reg_all(void);

int PMIC_IMM_GetOneChannelValue(int , int );
extern int ha01_PMIC_IMM_GetOneChannelValue(int , int );
extern int ha03_PMIC_IMM_GetOneChannelValue(int , int );

int PMIC_IMM_GetOneChannelValueSleep(int , int );
extern int ha01_PMIC_IMM_GetOneChannelValueSleep(int , int);
extern int ha03_PMIC_IMM_GetOneChannelValueSleep(int , int);

kal_bool upmu_is_chr_det(void);
extern kal_bool ha01_upmu_is_chr_det(void);
extern kal_bool ha03_upmu_is_chr_det(void);

void wake_up_bat (void);
extern void ha01_wake_up_bat (void);
extern void ha03_wake_up_bat (void);

int init_proc_log(void);
extern int ha01_init_proc_log(void);
extern int ha03_init_proc_log(void);

INT16 BattThermistorConverTemp(INT32 Res);
extern INT16 ha01_BattThermistorConverTemp(INT32 Res);
extern INT16 ha03_BattThermistorConverTemp(INT32 Res);

INT16 BattVoltToTemp(UINT32 dwVolt);
extern INT16 ha01_BattVoltToTemp(UINT32 dwVolt);
extern INT16 ha03_BattVoltToTemp(UINT32 dwVolt);

void BATTERY_SetUSBState(int usb_state_value);
extern void ha01_BATTERY_SetUSBState(int usb_state_value);
extern void ha03_BATTERY_SetUSBState(int usb_state_value);

kal_bool pmic_chrdet_status(void);
extern kal_bool ha01_pmic_chrdet_status(void);
extern kal_bool ha03_pmic_chrdet_status(void);

void select_charging_curret(void);
extern void ha01_select_charging_curret(void);
extern void ha03_select_charging_curret(void);

void ChargerHwInit(void);
extern void ha01_ChargerHwInit(void);
extern void ha03_ChargerHwInit(void);

void pchr_turn_off_charging (void);
void ha01_pchr_turn_off_charging (void);
void ha03_pchr_turn_off_charging (void);

void pchr_turn_on_charging (void);
extern void ha01_pchr_turn_on_charging (void);
extern void ha03_pchr_turn_on_charging (void);

int BAT_CheckPMUStatusReg(void);
extern int ha01_BAT_CheckPMUStatusReg(void);
extern int ha03_BAT_CheckPMUStatusReg(void);

unsigned long BAT_Get_Battery_Voltage(int polling_mode);
unsigned long ha01_BAT_Get_Battery_Voltage(int polling_mode);
unsigned long ha03_BAT_Get_Battery_Voltage(int polling_mode);

int g_Get_I_Charging(void);
extern int ha01_g_Get_I_Charging(void);
extern int ha03_g_Get_I_Charging(void);

void BAT_GetVoltage(void);
extern void ha01_BAT_GetVoltage(void);
extern void ha03_BAT_GetVoltage(void);

UINT32 BattVoltToPercent(UINT16 dwVoltage);
extern UINT32 ha01_BattVoltToPercent(UINT16 dwVoltage);
extern UINT32 ha03_BattVoltToPercent(UINT16 dwVoltage);

int BAT_CheckBatteryStatus(void);
extern int ha01_BAT_CheckBatteryStatus(void);
extern int ha03_BAT_CheckBatteryStatus(void);

PMU_STATUS BAT_BatteryStatusFailAction(void);
extern PMU_STATUS ha01_BAT_BatteryStatusFailAction(void);
extern PMU_STATUS ha03_BAT_BatteryStatusFailAction(void);

PMU_STATUS BAT_ChargingOTAction(void);
extern PMU_STATUS ha01_BAT_ChargingOTAction(void);
extern PMU_STATUS ha03_BAT_ChargingOTAction(void);

PMU_STATUS BAT_BatteryFullAction(void);
extern PMU_STATUS ha01_BAT_BatteryFullAction(void);
extern PMU_STATUS ha03_BAT_BatteryFullAction(void);

PMU_STATUS BAT_PreChargeModeAction(void);
extern PMU_STATUS ha01_BAT_PreChargeModeAction(void);
extern PMU_STATUS ha03_BAT_PreChargeModeAction(void);

PMU_STATUS BAT_ConstantCurrentModeAction(void);
extern PMU_STATUS ha01_BAT_ConstantCurrentModeAction(void);
extern PMU_STATUS ha03_BAT_ConstantCurrentModeAction(void);

PMU_STATUS BAT_TopOffModeAction(void);
extern PMU_STATUS ha01_BAT_TopOffModeAction(void);
extern PMU_STATUS ha03_BAT_TopOffModeAction(void);

PMU_STATUS BAT_PostFullModeAction(void);
extern PMU_STATUS ha01_BAT_PostFullModeAction(void);
extern PMU_STATUS ha03_BAT_PostFullModeAction(void);

void mt_battery_notify_check(void);
extern void ha01_mt_battery_notify_check(void);
extern void ha03_mt_battery_notify_check(void);

void BAT_thread(void);
extern void ha01_BAT_thread(void);
extern void ha03_BAT_thread(void);

int bat_thread_kthread(void *x);
extern int ha01_bat_thread_kthread(void *x);
extern int ha03_bat_thread_kthread(void *x);

void bat_thread_wakeup(UINT16 i);
extern void ha01_bat_thread_wakeup(UINT16 i);
extern void ha03_bat_thread_wakeup(UINT16 i);

void BatThread_XGPTConfig(void);
extern void  ha01_BatThread_XGPTConfig(void);
extern void  ha03_BatThread_XGPTConfig(void);

int charger_hv_detect_sw_thread_handler(void *unused);
extern int ha01_charger_hv_detect_sw_thread_handler(void *unused);
extern int ha03_charger_hv_detect_sw_thread_handler(void *unused);

void charger_hv_detect_sw_workaround_init(void);
extern void ha01_charger_hv_detect_sw_workaround_init(void);
extern void ha03_charger_hv_detect_sw_workaround_init(void);

void MTKFG_PLL_Control(kal_bool en);
extern void ha01_MTKFG_PLL_Control(kal_bool en);
extern void ha03_MTKFG_PLL_Control(kal_bool en);

int fgauge_get_saddles(void);
extern int ha01_fgauge_get_saddles(void);
extern int ha03_fgauge_get_saddles(void);

int fgauge_get_saddles_r_table(void);
extern int ha01_fgauge_get_saddles_r_table(void);
extern int ha03_fgauge_get_saddles_r_table(void);

BATTERY_PROFILE_STRUC_P fgauge_get_profile(kal_uint32 temperature);
extern BATTERY_PROFILE_STRUC_P ha01_fgauge_get_profile(kal_uint32 temperature);
extern BATTERY_PROFILE_STRUC_P ha03_fgauge_get_profile(kal_uint32 temperature);

R_PROFILE_STRUC_P fgauge_get_profile_r_table(kal_uint32 temperature);
extern R_PROFILE_STRUC_P ha01_fgauge_get_profile_r_table(kal_uint32 temperature);
extern R_PROFILE_STRUC_P ha03_fgauge_get_profile_r_table(kal_uint32 temperature);

void FGADC_dump_parameter(void);
extern void ha01_FGADC_dump_parameter(void);
extern void ha03_FGADC_dump_parameter(void);

void FGADC_dump_register(void);
extern void ha01_FGADC_dump_register(void);
extern void ha03_FGADC_dump_register(void);

kal_uint32 fg_get_data_ready_status(void);
extern kal_uint32 ha01_fg_get_data_ready_status(void);
extern kal_uint32 ha03_fg_get_data_ready_status(void);

kal_uint32 fg_get_sw_clear_status(void);
extern kal_uint32 ha01_fg_get_sw_clear_status(void);
extern kal_uint32 ha03_fg_get_sw_clear_status(void);

void update_fg_dbg_tool_value(void);
extern void ha01_update_fg_dbg_tool_value(void);
extern void ha03_update_fg_dbg_tool_value(void);

kal_int32 fgauge_read_temperature(void);
extern kal_int32 ha01_fgauge_read_temperature(void);
extern kal_int32 ha03_fgauge_read_temperature(void);

void dump_nter(void);
extern void ha01_dump_nter(void);
extern void ha03_dump_nter(void);

void dump_car(void);
extern void ha01_dump_car(void);
extern void ha03_dump_car(void);

kal_int32 fgauge_read_columb(void);
extern kal_int32 ha01_fgauge_read_columb(void);
extern kal_int32 ha03_fgauge_read_columb(void);

kal_int32 fgauge_read_columb_reset(void);
extern kal_int32 ha01_fgauge_read_columb_reset(void);
extern kal_int32 ha03_fgauge_read_columb_reset(void);

kal_int32 fgauge_read_current(void);
extern kal_int32 ha01_fgauge_read_current(void);
extern kal_int32 ha03_fgauge_read_current(void);

kal_int32 fgauge_read_voltage(void);
extern kal_int32 ha01_fgauge_read_voltage(void);
extern kal_int32 ha03_fgauge_read_voltage(void);

kal_int32 fgauge_compensate_battery_voltage(kal_int32 ori_voltage);
extern kal_int32 ha01_fgauge_compensate_battery_voltage(kal_int32 ori_voltage);
extern kal_int32 ha03_fgauge_compensate_battery_voltage(kal_int32 ori_voltage);

kal_int32 fgauge_compensate_battery_voltage_recursion(kal_int32 ori_voltage, kal_int32 recursion_time);
extern kal_int32 ha01_fgauge_compensate_battery_voltage_recursion(kal_int32 ori_voltage, kal_int32 recursion_time);
extern kal_int32 ha03_fgauge_compensate_battery_voltage_recursion(kal_int32 ori_voltage, kal_int32 recursion_time);

void fgauge_construct_battery_profile(kal_int32 temperature, BATTERY_PROFILE_STRUC_P temp_profile_p);
extern void ha01_fgauge_construct_battery_profile(kal_int32 temperature, BATTERY_PROFILE_STRUC_P temp_profile_p);
extern void ha03_fgauge_construct_battery_profile(kal_int32 temperature, BATTERY_PROFILE_STRUC_P temp_profile_p);

void fgauge_construct_r_table_profile(kal_int32 temperature, R_PROFILE_STRUC_P temp_profile_p);
extern void ha01_fgauge_construct_r_table_profile(kal_int32 temperature, R_PROFILE_STRUC_P temp_profile_p);
extern void ha03_fgauge_construct_r_table_profile(kal_int32 temperature, R_PROFILE_STRUC_P temp_profile_p);

kal_int32 fgauge_get_dod0(kal_int32 voltage, kal_int32 temperature, kal_bool bOcv);
extern kal_int32 ha01_fgauge_get_dod0(kal_int32 voltage, kal_int32 temperature, kal_bool bOcv);
extern kal_int32 ha03_fgauge_get_dod0(kal_int32 voltage, kal_int32 temperature, kal_bool bOcv);

void fg_qmax_update_for_aging(void);
extern void ha01_fg_qmax_update_for_aging(void);
extern void ha03_fg_qmax_update_for_aging(void);

kal_int32 fgauge_update_dod(void);
extern kal_int32 ha01_fgauge_update_dod(void);
extern kal_int32 ha03_fgauge_update_dod(void);

kal_int32 fgauge_read_capacity(kal_int32 type);
extern kal_int32 ha01_fgauge_read_capacity(kal_int32 type);
extern kal_int32 ha03_fgauge_read_capacity(kal_int32 type);

kal_int32 fgauge_read_capacity_by_v(void);
extern kal_int32 ha01_fgauge_read_capacity_by_v(void);
extern kal_int32 ha03_fgauge_read_capacity_by_v(void);

kal_int32 fgauge_read_r_bat_by_v(kal_int32 voltage);
extern kal_int32 ha01_fgauge_read_r_bat_by_v(kal_int32 voltage);
extern kal_int32 ha03_fgauge_read_r_bat_by_v(kal_int32 voltage);

kal_int32 fgauge_read_v_by_capacity(int bat_capacity);
extern kal_int32 ha01_fgauge_read_v_by_capacity(int bat_capacity);
extern kal_int32 ha03_fgauge_read_v_by_capacity(int bat_capacity);

void fgauge_Normal_Mode_Work(void);
extern void ha01_fgauge_Normal_Mode_Work(void);
extern void ha03_fgauge_Normal_Mode_Work(void);

kal_int32 fgauge_get_Q_max(kal_int16 temperature);
extern kal_int32 ha01_fgauge_get_Q_max(kal_int16 temperature);
extern kal_int32 ha03_fgauge_get_Q_max(kal_int16 temperature);

kal_int32 fgauge_get_Q_max_high_current(kal_int16 temperature);
extern kal_int32 ha01_fgauge_get_Q_max_high_current(kal_int16 temperature);
extern kal_int32 ha03_fgauge_get_Q_max_high_current(kal_int16 temperature);

void fgauge_initialization(void);
extern void ha01_fgauge_initialization(void);
extern void ha03_fgauge_initialization(void);

kal_int32 FGADC_Get_BatteryCapacity_CoulombMothod(void);
extern kal_int32 ha01_FGADC_Get_BatteryCapacity_CoulombMothod(void);
extern kal_int32 ha03_FGADC_Get_BatteryCapacity_CoulombMothod(void);

kal_int32 FGADC_Get_BatteryCapacity_VoltageMothod(void);
extern kal_int32 ha01_FGADC_Get_BatteryCapacity_VoltageMothod(void);
extern kal_int32 ha03_FGADC_Get_BatteryCapacity_VoltageMothod(void);

kal_int32 FGADC_Get_FG_Voltage(void);
extern kal_int32 ha01_FGADC_Get_FG_Voltage(void);
extern kal_int32 ha03_FGADC_Get_FG_Voltage(void);

void FGADC_Reset_SW_Parameter(void);
extern void ha01_FGADC_Reset_SW_Parameter(void);
extern void ha03_FGADC_Reset_SW_Parameter(void);

kal_int32 get_dynamic_period(int first_use, int first_wakeup_time, int battery_capacity_level);
extern kal_int32 ha01_get_dynamic_period(int first_use, int first_wakeup_time, int battery_capacity_level);
extern kal_int32 ha03_get_dynamic_period(int first_use, int first_wakeup_time, int battery_capacity_level);

void fg_voltage_mode(void);
extern void ha01_fg_voltage_mode(void);
extern void ha03_fg_voltage_mode(void);

void FGADC_thread_kthread(void);
extern void ha01_FGADC_thread_kthread(void);
extern void ha03_FGADC_thread_kthread(void);

int init_proc_log_fg(void);
extern int ha01_init_proc_log_fg(void);
extern int ha03_init_proc_log_fg(void);

/*weli end***************************************/

#endif /* ***** CCI BSP.SYS ***** */

#endif /* _CCI_DRV_COMMON_H */
