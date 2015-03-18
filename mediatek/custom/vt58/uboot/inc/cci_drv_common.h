#ifndef _CCI_DRV_COMMON_H
#define _CCI_DRV_COMMON_H



/*
 * owner: brook start
 */
//extern bool cci_platform_is_ha01(void);
//extern bool cci_platform_is_ha03(void);
//extern bool cci_platform_is_ha01_dvt1(void);
//extern bool cci_platform_is_ha03_dvt1(void);
//extern bool cci_platform_is_ha01_dvt2(void);
//extern bool cci_platform_is_ha03_dvt2(void);
//extern bool cci_platform_is_ha01_pvt(void);
//extern bool cci_platform_is_ha03_pvt(void);
//extern bool cci_platform_is_ha05_dvt(void);
//extern bool cci_platform_is_ha05_pvt(void);
//extern bool cci_platform_is_ha05_mp(void);
extern bool cci_platform_is_pvt(void);
extern bool cci_platform_is_dvt(void);
extern bool cci_platform_is_band_1_8(void);
extern bool cci_platform_is_band_2_5(void);
/*
 * owner: brook end
 */
#if 0 /* ***** CCI BSP.SYS ***** */

/***********************************************/
/*
 * owner: weli start
 */
void bmt_charger_ov_check(void);
extern void ha01_bmt_charger_ov_check(void);
extern void ha03_bmt_charger_ov_check(void);

kal_bool pmic_chrdet_status(void);
kal_bool ha01_pmic_chrdet_status(void);
kal_bool ha03_pmic_chrdet_status(void);
/*
 * owner: weli end
 */
/*************************************************/

#endif /* ***** CCI BSP.SYS ***** */

#endif /* _CCI_DRV_COMMON_H */
