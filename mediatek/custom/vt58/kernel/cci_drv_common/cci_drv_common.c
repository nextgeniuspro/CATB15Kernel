#include <mach/mt_gpio.h>
#include <cci_drv_common.h>

//#if 0 /* ***** CCI BSP.SYS ***** */

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


#include <linux/init.h>
#include <asm/string.h>
#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */

/*
HA01.DVT1
HA01.DVT2
HA01.PVT
HA03.DVT1
HA03.DVT2
HA03.PVT
*/
#define boot_reason_string "cust_bootreason="
//#define adb_boot_string "cust_adb_boot="

int is_vt57 =0;
int is_vt58 =0;
char emcp_id[19];
int is_pvt = 0;
int is_pvt_sb = 0;
int is_pvt_sc = 0;
int is_pvt_sl_sb_sc = 0;
int is_pvt_sb_sc = 0;
int is_dvt = 0;
//int is_mp = 0;
int is_band_2_5 = 0;
int is_band_1_8 = 0;
int is_unknown_band = 0;
int is_unknown_phase = 0;
char boot_reason ='\0';
char adb_boot ='\0';

static int cci_project_id_init = 0;
static int cci_platform_init = 0;

int setup_cci_project_id(char *str)
{
    if(strstr(str,"VT57")) 
    {
		printk("-----< PROJ = VT57 >-----\n");
   		cci_project_id_init = 1;
		is_vt57 = 1;
	}

    else
    {
		printk("-----< PROJ = VT58 >-----\n");
        cci_project_id_init = 1;
		is_vt58 = 1;
    }

    return;
}

int setup_cci_emcp_id(char *str)
{
    char* tmp;
    tmp = strstr(str,"###");
    if(NULL != tmp) 
    {
	tmp+=3;
        memcpy(emcp_id, tmp, sizeof(emcp_id));
    }
    else
    {
        strcpy(emcp_id,"NULL");
    }
    printk("-----< EMCP = %s >-----\n",emcp_id);
    emcp_id[18]=(char)0x0;
    return;
}

int setup_cci_project(char *str)
{
    if(strstr(str,"PVT_SL_SB_SC")) {
		printk("-----< HW= PVT_SL_SB_SC >-----\n");
		cci_platform_init = 1;
		is_pvt_sl_sb_sc = 1;
	}
    else if(strstr(str,"PVT_SB_SC")) {
		printk("-----< HW= PVT_SB_SC >-----\n");
		cci_platform_init = 1;
		is_pvt_sb_sc = 1;
	}
    else if(strstr(str,"PVT_SB")) {
		printk("-----< HW= PVT_SB>-----\n");
		cci_platform_init = 1;
		is_pvt_sb = 1;
	}
    else if(strstr(str,"PVT_SC")) {
		printk("-----< HW= PVT_SC >-----\n");
		cci_platform_init = 1;
		is_pvt_sc = 1;
	}

    else if(strstr(str,"PVT")) {
		printk("-----< HW= PVT >-----\n");
		cci_platform_init = 1;
		is_pvt = 1;
	}
	else if(strstr(str,"DVT")) {
		printk("-----< HW= DVT >-----\n");
		cci_platform_init = 1;
		is_dvt = 1;
	}
    else if(strstr(str,"UNKNOWN_PHASE")) {
		printk("-----< UNKNOWN_PHASE >-----\n");
		cci_platform_init = 1;
		is_unknown_phase = 1;
	}

    if(strstr(str,"BAND_1_8")) {
		printk("-----< BAND= 1,8 >-----\n");
		cci_platform_init = 1;
		is_band_1_8 = 1;
	}
	else if(strstr(str,"BAND_2_5")) {
		printk("-----< BAND= 2,5 >-----\n");
		cci_platform_init = 1;
		is_band_2_5 = 1;
	}	
	else if(strstr(str,"UNKNOWN_BAND")) {
		printk("-----< UNKNOWN_BAND >-----\n");
		cci_platform_init = 1;
		is_unknown_band = 1;
	}
	/*if(strstr(str,"MP")) {
		printk("-----< HW= MP >-----\n");
		cci_platform_init = 1;
		is_mp = 1;
	}*/
	return ;
}
int setup_cci_boot_reason(char *str)
{
	char *ptr = NULL;
	if(ptr = strstr(str, boot_reason_string)) 
	{
		ptr+=strlen(boot_reason_string);
		printk("boot_reason_string = %d\n", strlen(boot_reason_string));				
		boot_reason = ptr[0];
	}	
	/*char *ptr2 = NULL;
	if(ptr2 = strstr(str, adb_boot_string)) 
	{
		ptr2+=strlen(adb_boot_string);
		printk("adb_boot = %d\n", strlen(adb_boot_string));				
		adb_boot = ptr2[0];
	}*/	
	return ;
}

int cci_project_id(void)
{
    if(!cci_platform_init)
    {
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}

    if(is_vt57 == 1 && is_vt58 == 0)
    {
        return is_vt57;
    }
    else if(is_vt57 == 0 && is_vt58 == 1)
    {
        return is_vt57;
    }
    else
    {
	    panic("CCI GET PROJECT ID VT5X ERROR! \n");
	}
    
}

int cci_platform_is_band_1_8(void)
{
    if(!cci_platform_init){
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}
	return is_band_1_8;
}
int cci_platform_is_band_2_5(void)
{
    if(!cci_platform_init){
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}
	return is_band_2_5;
}
int cci_platform_is_pvt(void)
{
    if(!cci_platform_init){
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}
	return is_pvt;
}
int cci_platform_is_pvt_sb(void)
{
    if(!cci_platform_init){
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}
	return is_pvt_sb;
}
int cci_platform_is_pvt_sc(void)
{
    if(!cci_platform_init){
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}
	return is_pvt_sc;
}
int cci_platform_is_pvt_sl_sb_sc(void)
{
    if(!cci_platform_init){
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}
	return is_pvt_sl_sb_sc;
}
int cci_platform_is_pvt_sb_sc(void)
{
    if(!cci_platform_init){
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}
	return is_pvt_sb_sc;
}
int cci_platform_is_dvt(void)
{
    if(!cci_platform_init){
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}
	return is_dvt;
}
int cci_platform_is_unknown_band(void)
{
    if(!cci_platform_init){
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}
	return is_unknown_band;
}
int cci_platform_is_unknown_phase(void)
{
    if(!cci_platform_init){
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}
	return is_unknown_phase;
}
/*int cci_platform_is_mp(void)
{
    if(!cci_platform_init){
	    panic("CCI GET PLATFORM ID ERROR! \n");
	}
	return is_mp;
}*/

/*
 * owner: brook end
 */
/*********************************************************/

/*
 * owner: Rudolph start
 */


////
//CL test
///
#if 0
#include <mach/mt_reg_base.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_wdt.h>

#define procfs_watchdog "cust_watchdog"

/*
 * This structure hold information about the /proc file
 */
struct proc_dir_entry *cci_proc_watchdog;

int proc_watchdog_read(
		char *buffer,
		char **buffer_location,
		off_t offset,
		int buffer_length,
		int *eof,
		void *data)
{
	int ret;

	printk(KERN_INFO "procfile_read (/proc/%s) called\n", procfs_watchdog);

	
	if (DRV_Reg16(MTK_WDT_MODE) & MTK_WDT_MODE_ENABLE)
	{
		ret = sprintf(buffer, "Enable\n");
	}else {
		ret = sprintf(buffer, "disable\n");
	}

	return ret;
}

static int build_proc_watchdog(void) 
{
	

	cci_proc_watchdog = create_proc_entry(procfs_watchdog, 0644, NULL);
	
	if (cci_proc_watchdog == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
		       procfs_watchdog);
		return -ENOMEM;
	}

	cci_proc_watchdog->read_proc = proc_watchdog_read;
	cci_proc_watchdog->mode 	 = S_IFREG | S_IRUGO;
	cci_proc_watchdog->uid 	 = 0;
	cci_proc_watchdog->gid 	 = 0;
	cci_proc_watchdog->size 	 = 37;

	
	return 0;
}
#endif
//
//CL
//
#define procfs_id_name "cust_project_id"
#define procfs_name "cust_project"
#define procfs_emcp_name "cust_emcp_id"
/*
 * This structure hold information about the /proc file
 */
struct proc_dir_entry *cci_proc_project_id;
struct proc_dir_entry *cci_proc_emcp_id;
struct proc_dir_entry *cci_proc_project;

int procfile_read_id(
		char *buffer,
		char **buffer_location,
		off_t offset,
		int buffer_length,
		int *eof,
		void *data)
{
    int ret = 0;
    
	printk(KERN_INFO "procfile_read (/proc/%s) called\n", procfs_id_name);

	if(is_vt57 == 1 && is_vt58 == 0)
	{
        ret = sprintf(buffer, "VT57\n");
    }
	else if(is_vt57 == 0 && is_vt58 == 1)
	{
        ret = sprintf(buffer, "VT58\n");
    }
    else
    {
        ret = sprintf(buffer, "UNKNOWN PROJECT ID\n");
    }
    return ret;
}

int procfile_read_emcp(
		char *buffer,
		char **buffer_location,
		off_t offset,
		int buffer_length,
		int *eof,
		void *data)
{
    int ret = 0;
    
	printk(KERN_ERR "procfile_read_emcp (/proc/%s) called\n", procfs_emcp_name);
    ret = sprintf(buffer, emcp_id);
    //memcpy(buffer, &emcp_id[3], sizeof(emcp_id)-3);
    printk(KERN_ERR "procfile_read_emcp: buffer = %s\n", buffer);
    return ret;
}

int procfile_read(
		char *buffer,
		char **buffer_location,
		off_t offset,
		int buffer_length,
		int *eof,
		void *data)
{
	int ret = 0;

	printk(KERN_INFO "procfile_read (/proc/%s) called\n", procfs_name);

	
	if(cci_platform_is_band_1_8())
	{
        if(cci_platform_is_pvt_sl_sb_sc())
		{
			ret = sprintf(buffer, "PVT_SL_SB_SC.BAND_1_8\n");
		}	
        else if(cci_platform_is_pvt_sb_sc())
		{
			ret = sprintf(buffer, "PVT_SB_SC.BAND_1_8\n");
		}
        else if(cci_platform_is_pvt_sb())
		{
			ret = sprintf(buffer, "PVT_SB.BAND_1_8\n");
		}
        else if(cci_platform_is_pvt_sc())
		{
			ret = sprintf(buffer, "PVT_SC.BAND_1_8\n");
		}
        else if(cci_platform_is_pvt())
		{
			ret = sprintf(buffer, "PVT.BAND_1_8\n");
		}
		else if(cci_platform_is_dvt())
		{
			ret = sprintf(buffer, "DVT.BAND_1_8\n");
		}
		else if(cci_platform_is_unknown_phase())
		{
			ret = sprintf(buffer, "UNKNOWN_PHASE.BAND_1_8\n");
		}
	}
	else if(cci_platform_is_band_2_5())
	{	
        if(cci_platform_is_pvt_sl_sb_sc())
		{
			ret = sprintf(buffer, "PVT_SL_SB_SC.BAND_2_5\n");
		}
        else if(cci_platform_is_pvt_sb_sc())
		{
			ret = sprintf(buffer, "PVT_SB_SC.BAND_2_5\n");
		}
        else if(cci_platform_is_pvt_sb())
		{
			ret = sprintf(buffer, "PVT_SB.BAND_2_5\n");
		}
        else if(cci_platform_is_pvt_sc())
		{
			ret = sprintf(buffer, "PVT_SC.BAND_2_5\n");
		}

        else if(cci_platform_is_pvt())
		{
			ret = sprintf(buffer, "PVT.BAND_2_5\n");
		}
		else if(cci_platform_is_dvt())
		{
			ret = sprintf(buffer, "DVT.BAND_2_5\n");
		}
		else if(cci_platform_is_unknown_phase())
		{
			ret = sprintf(buffer, "UNKNOWN_PHASE.BAND_2_5\n");
		}
	}
	else if(cci_platform_is_unknown_band())
	{
        if(cci_platform_is_pvt_sl_sb_sc())
		{
			ret = sprintf(buffer, "PVT_SL_SB_SC.UNKNOWN_BAND\n");
		}
        if(cci_platform_is_pvt_sb_sc())
		{
			ret = sprintf(buffer, "PVT_SB_SC.UNKNOWN_BAND\n");
		}
        else if(cci_platform_is_pvt_sb())
		{
			ret = sprintf(buffer, "PVT_SB.UNKNOWN_BAND\n");
		}
        else if(cci_platform_is_pvt_sc())
		{
			ret = sprintf(buffer, "PVT_SC.UNKNOWN_BAND\n");
		}

        else if(cci_platform_is_pvt())
		{
			ret = sprintf(buffer, "PVT.UNKNOWN_BAND\n");
		}
		else if(cci_platform_is_dvt())
		{
			ret = sprintf(buffer, "DVT.UNKNOWN_BAND\n");
		}
		else if(cci_platform_is_unknown_phase())
		{
			ret = sprintf(buffer, "UNKNOWN_PHASE.UNKNOWN_BAND\n");
		}
	}
		/*else if(cci_platform_is_mp())
		{
			ret = sprintf(buffer, "MP\n");
		}*/
		//ret = sprintf(buffer, "DVT.BAND_2_5\n");
	
	
	return ret;
}

static int __init build_proc_project_id(void) 
{
	printk("***********************************\n");
	printk("*                                 *\n");
	printk("*    build_proc_project_id enter  *\n");
	printk("*                                 *\n");
	printk("***********************************\n");

	cci_proc_project_id = create_proc_entry(procfs_id_name, 0644, NULL);
	
	if (cci_proc_project_id == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
		       procfs_id_name);
		return -ENOMEM;
	}

	cci_proc_project_id->read_proc = procfile_read_id;
	cci_proc_project_id->mode 	 = S_IFREG | S_IRUGO;
	cci_proc_project_id->uid 	 = 0;
	cci_proc_project_id->gid 	 = 0;
	cci_proc_project_id->size 	 = 37;

	printk(KERN_INFO "/proc/%s created\n", procfs_id_name);	

	printk("***********************************\n");
	printk("*                                 *\n");
	printk("*    build_proc_project_id exit   *\n");
	printk("*                                 *\n");
	printk("***********************************\n");
	//build_proc_watchdog(); // CL add for watchdog status

	return 0;
}
module_init(build_proc_project_id);

static int __init build_proc_emcp_id(void) 
{
	printk("***********************************\n");
	printk("*                                 *\n");
	printk("*    build_proc_emcp_id enter     *\n");
	printk("*                                 *\n");
	printk("***********************************\n");

	cci_proc_emcp_id = create_proc_entry(procfs_emcp_name, 0644, NULL);
	
	if (cci_proc_emcp_id == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
		       procfs_emcp_name);
		return -ENOMEM;
	}

	cci_proc_emcp_id->read_proc = procfile_read_emcp;
	cci_proc_emcp_id->mode 	 = S_IFREG | S_IRUGO;
	cci_proc_emcp_id->uid 	 = 0;
	cci_proc_emcp_id->gid 	 = 0;
	cci_proc_emcp_id->size 	 = 37;

	printk(KERN_INFO "/proc/%s created\n", procfs_emcp_name);	

	printk("***********************************\n");
	printk("*                                 *\n");
	printk("*    build_proc_emcp_id exit      *\n");
	printk("*                                 *\n");
	printk("***********************************\n");
	//build_proc_watchdog(); // CL add for watchdog status

	return 0;
}
module_init(build_proc_emcp_id);

static int __init build_proc_project(void) 
{
	printk("***********************************\n");
	printk("*                                 *\n");
	printk("*    build_proc_project enter     *\n");
	printk("*                                 *\n");
	printk("***********************************\n");

	cci_proc_project = create_proc_entry(procfs_name, 0644, NULL);
	
	if (cci_proc_project == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
		       procfs_name);
		return -ENOMEM;
	}

	cci_proc_project->read_proc = procfile_read;
	cci_proc_project->mode 	 = S_IFREG | S_IRUGO;
	cci_proc_project->uid 	 = 0;
	cci_proc_project->gid 	 = 0;
	cci_proc_project->size 	 = 37;

	printk(KERN_INFO "/proc/%s created\n", procfs_name);	

	printk("***********************************\n");
	printk("*                                 *\n");
	printk("*    build_proc_project exit      *\n");
	printk("*                                 *\n");
	printk("***********************************\n");
	//build_proc_watchdog(); // CL add for watchdog status

	return 0;
}
module_init(build_proc_project);
/*
 * owner: Rudolph end
 */
/*********************************************************/

#define procfs_adb_boot "cust_adb_boot"

/*
 * This structure hold information about the /proc file
 */
struct proc_dir_entry *cci_proc_adb_boot;

int procfile_adb_bootread(
		char *buffer,
		char **buffer_location,
		off_t offset,
		int buffer_length,
		int *eof,
		void *data)
{
	int ret;
	printk(KERN_INFO "procfile_read (/proc/%s) called\n", procfs_adb_boot);	
	printk("adb_boot = %c\n", adb_boot);
	ret = sprintf(buffer, "%c", adb_boot);	
	return ret;
}

static int __init build_proc_adb_boot(void) 
{

	cci_proc_adb_boot = create_proc_entry(procfs_adb_boot, 0644, NULL);
	
	if (cci_proc_adb_boot == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
		       procfs_adb_boot);
		return -ENOMEM;
	}

	cci_proc_adb_boot->read_proc = procfile_adb_bootread;
	cci_proc_adb_boot->mode 	 = S_IFREG | S_IRUGO;
	cci_proc_adb_boot->uid 	 = 0;
	cci_proc_adb_boot->gid 	 = 0;
	cci_proc_adb_boot->size 	 = 37;

	printk(KERN_INFO "/proc/%s created\n", procfs_adb_boot);	

	printk("***********************************\n");
	printk("*                                 *\n");
	printk("*    build_proc_boot_reason exit  *\n");
	printk("*                                 *\n");
	printk("***********************************\n");

	return 0;
}
module_init(build_proc_adb_boot);


#define procfs_name_2 "cust_boot_reason"
struct proc_dir_entry *cci_proc_boot_reason;

int procfile_read_2(
		char *buffer,
		char **buffer_location,
		off_t offset,
		int buffer_length,
		int *eof,
		void *data)
{
	int ret;
	printk(KERN_INFO "procfile_read (/proc/%s) called\n", procfs_name_2);	
	printk("boot_reason = %c\n", boot_reason);
	ret = sprintf(buffer, "%c", boot_reason);	
	return ret;
}

static int __init build_proc_boot_reason(void) 
{
	printk("***********************************\n");
	printk("*                                 *\n");
	printk("*    build_proc_boot_reason enter *\n");
	printk("*                                 *\n");
	printk("***********************************\n");

	cci_proc_boot_reason = create_proc_entry(procfs_name_2, 0644, NULL);
	
	if (cci_proc_boot_reason == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
		       procfs_name_2);
		return -ENOMEM;
	}

	cci_proc_boot_reason->read_proc = procfile_read_2;
	cci_proc_boot_reason->mode 	 = S_IFREG | S_IRUGO;
	cci_proc_boot_reason->uid 	 = 0;
	cci_proc_boot_reason->gid 	 = 0;
	cci_proc_boot_reason->size 	 = 37;

	printk(KERN_INFO "/proc/%s created\n", procfs_name_2);	

	printk("***********************************\n");
	printk("*                                 *\n");
	printk("*    build_proc_boot_reason exit  *\n");
	printk("*                                 *\n");
	printk("***********************************\n");

	return 0;
}
module_init(build_proc_boot_reason);
#if 0 /* ***** CCI BSP.SYS ***** */
/*
 * owner: weli start
 */
void get_pmic_auxadc_reg(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_get_pmic_auxadc_reg();
	}
	else
	{
		return ha03_get_pmic_auxadc_reg();
	}
}

void get_pmic_auxadc_reg_all(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_get_pmic_auxadc_reg_all();
	}
	else
	{
		return ha03_get_pmic_auxadc_reg_all();
	}
}

int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount)
{
	if(cci_platform_is_ha01())
	{
		return ha01_PMIC_IMM_GetOneChannelValue(dwChannel,deCount);
	}
	else
	{
		return ha03_PMIC_IMM_GetOneChannelValue(dwChannel,deCount);;
	}
}

int PMIC_IMM_GetOneChannelValueSleep(int dwChannel, int deCount)
{
	if(cci_platform_is_ha01())
	{
		return ha01_PMIC_IMM_GetOneChannelValueSleep(dwChannel,deCount);
	}
	else
	{
		return ha03_PMIC_IMM_GetOneChannelValueSleep(dwChannel,deCount);
	}
}

kal_bool upmu_is_chr_det(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_upmu_is_chr_det();
	}
	else
	{
		return ha03_upmu_is_chr_det();
	}
}

void wake_up_bat (void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_wake_up_bat ();
	}
	else
	{
		return ha03_wake_up_bat ();
	}
}
EXPORT_SYMBOL(wake_up_bat);

int init_proc_log(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_init_proc_log();
	}
	else
	{
		return ha03_init_proc_log();
	}
}

INT16 BattThermistorConverTemp(INT32 Res)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BattThermistorConverTemp(Res);
	}
	else
	{
		return ha03_BattThermistorConverTemp(Res);
	}
}

INT16 BattVoltToTemp(UINT32 dwVolt)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BattVoltToTemp(dwVolt);
	}
	else
	{
		return ha03_BattVoltToTemp(dwVolt);
	}
}
void BATTERY_SetUSBState(int usb_state_value)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BATTERY_SetUSBState(usb_state_value);
	}
	else
	{
		return ha03_BATTERY_SetUSBState(usb_state_value);
	}
}
EXPORT_SYMBOL(BATTERY_SetUSBState);

kal_bool pmic_chrdet_status(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_pmic_chrdet_status();
	}
	else
	{
		return ha03_pmic_chrdet_status();
	}
}

void select_charging_curret(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_select_charging_curret();
	}
	else
	{
		return ha03_select_charging_curret();
	}
}

void ChargerHwInit(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_ChargerHwInit();
	}
	else
	{
		return ha03_ChargerHwInit();
	}
}

void pchr_turn_off_charging (void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_pchr_turn_off_charging ();
	}
	else
	{
		return ha03_pchr_turn_off_charging ();
	}
}

void pchr_turn_on_charging (void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_pchr_turn_on_charging ();
	}
	else
	{
		return ha03_pchr_turn_on_charging ();
	}
}

int BAT_CheckPMUStatusReg(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_CheckPMUStatusReg();
	}
	else
	{
		return ha03_BAT_CheckPMUStatusReg();
	}
}

unsigned long BAT_Get_Battery_Voltage(int polling_mode)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_Get_Battery_Voltage(polling_mode);
	}
	else
	{
		return ha03_BAT_Get_Battery_Voltage(polling_mode);
	}
}

int g_Get_I_Charging(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_g_Get_I_Charging();
	}
	else
	{
		return ha03_g_Get_I_Charging();
	}
}

void BAT_GetVoltage(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_GetVoltage();
	}
	else
	{
		return ha03_BAT_GetVoltage();
	}
}

UINT32 BattVoltToPercent(UINT16 dwVoltage)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BattVoltToPercent(dwVoltage);
	}
	else
	{
		return ha03_BattVoltToPercent(dwVoltage);
	}
}

int BAT_CheckBatteryStatus(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_CheckBatteryStatus();
	}
	else
	{
		return ha03_BAT_CheckBatteryStatus();
	}
}

PMU_STATUS BAT_BatteryStatusFailAction(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_BatteryStatusFailAction();
	}
	else
	{
		return ha03_BAT_BatteryStatusFailAction();
	}
}

PMU_STATUS BAT_ChargingOTAction(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_ChargingOTAction();
	}
	else
	{
		return ha03_BAT_ChargingOTAction();
	}
}

PMU_STATUS BAT_BatteryFullAction(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_BatteryFullAction();
	}
	else
	{
		return ha03_BAT_BatteryFullAction();
	}
}

PMU_STATUS BAT_PreChargeModeAction(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_PreChargeModeAction();
	}
	else
	{
		return ha03_BAT_PreChargeModeAction();
	}
}

PMU_STATUS BAT_ConstantCurrentModeAction(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_ConstantCurrentModeAction();
	}
	else
	{
		return ha03_BAT_ConstantCurrentModeAction();
	}
}

PMU_STATUS BAT_TopOffModeAction(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_TopOffModeAction();
	}
	else
	{
		return ha03_BAT_TopOffModeAction();
	}
}

PMU_STATUS BAT_PostFullModeAction(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_PostFullModeAction();
	}
	else
	{
		return ha03_BAT_PostFullModeAction();
	}
}

void mt_battery_notify_check(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_mt_battery_notify_check();
	}
	else
	{
		return ha03_mt_battery_notify_check();
	}
}

void BAT_thread(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BAT_thread();
	}
	else
	{
		return ha03_BAT_thread();
	}
}

int bat_thread_kthread(void *x)//error?
{
	if(cci_platform_is_ha01())
	{
		return ha01_bat_thread_kthread(x);
	}
	else
	{
		return ha03_bat_thread_kthread(x);
	}
}

void bat_thread_wakeup(UINT16 i)
{
	if(cci_platform_is_ha01())
	{
		return ha01_bat_thread_wakeup( i);
	}
	else
	{
		return ha03_bat_thread_wakeup( i);
	}
}
void BatThread_XGPTConfig(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_BatThread_XGPTConfig();
	}
	else
	{
		return ha03_BatThread_XGPTConfig();
	}
}

int charger_hv_detect_sw_thread_handler(void *unused)
{
	if(cci_platform_is_ha01())
	{
		return ha01_charger_hv_detect_sw_thread_handler(unused);
	}
	else
	{
		return ha03_charger_hv_detect_sw_thread_handler(unused);
	}
}

void charger_hv_detect_sw_workaround_init(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_charger_hv_detect_sw_workaround_init();
	}
	else
	{
		return ha03_charger_hv_detect_sw_workaround_init();
	}
}
///end of mt6575_battery.c//////////////////////////////////////////////////////////////////////////////
void MTKFG_PLL_Control(kal_bool en)
{
	if(cci_platform_is_ha01())
	{
		return ha01_MTKFG_PLL_Control(en);
	}
	else
	{
		return ha03_MTKFG_PLL_Control(en);
	}
}

int fgauge_get_saddles(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_get_saddles();
	}
	else
	{
		return ha03_fgauge_get_saddles();
	}
}

int fgauge_get_saddles_r_table(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_get_saddles_r_table();
	}
	else
	{
		return ha03_fgauge_get_saddles_r_table();
	}
}

BATTERY_PROFILE_STRUC_P fgauge_get_profile(kal_uint32 temperature)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_get_profile(temperature);
	}
	else
	{
		return ha03_fgauge_get_profile(temperature);
	}
}

R_PROFILE_STRUC_P fgauge_get_profile_r_table(kal_uint32 temperature)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_get_profile_r_table(temperature);
	}
	else
	{
		return ha03_fgauge_get_profile_r_table(temperature);
	}
}

void FGADC_dump_parameter(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_FGADC_dump_parameter();
	}
	else
	{
		return ha03_FGADC_dump_parameter();
	}
}

void FGADC_dump_register(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_FGADC_dump_register();
	}
	else
	{
		return ha03_FGADC_dump_register();
	}
}

kal_uint32 fg_get_data_ready_status(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fg_get_data_ready_status();
	}
	else
	{
		return ha03_fg_get_data_ready_status();
	}
}
kal_uint32 fg_get_sw_clear_status(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fg_get_sw_clear_status();
	}
	else
	{
		return ha03_fg_get_sw_clear_status();
	}
}

void update_fg_dbg_tool_value(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_update_fg_dbg_tool_value();
	}
	else
	{
		return ha03_update_fg_dbg_tool_value();
	}
}

kal_int32 fgauge_read_temperature(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_read_temperature();
	}
	else
	{
		return ha03_fgauge_read_temperature();
	}
}

void dump_nter(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_dump_nter();
	}
	else
	{
		return ha03_dump_nter();
	}
}

void dump_car(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_dump_car();
	}
	else
	{
		return ha03_dump_car();
	}
}

kal_int32 fgauge_read_columb(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_read_columb();
	}
	else
	{
		return ha03_fgauge_read_columb();
	}
}

kal_int32 fgauge_read_columb_reset(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_read_columb_reset();
	}
	else
	{
		return ha03_fgauge_read_columb_reset();
	}
}

kal_int32 fgauge_read_current(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_read_current();
	}
	else
	{
		return ha03_fgauge_read_current();
	}
}

kal_int32 fgauge_read_voltage(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_read_voltage();
	}
	else
	{
		return ha03_fgauge_read_voltage();
	}
}

kal_int32 fgauge_compensate_battery_voltage(kal_int32 ori_voltage)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_compensate_battery_voltage(ori_voltage);
	}
	else
	{
		return ha03_fgauge_compensate_battery_voltage(ori_voltage);
	}
}

kal_int32 fgauge_compensate_battery_voltage_recursion(kal_int32 ori_voltage, kal_int32 recursion_time)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_compensate_battery_voltage_recursion(ori_voltage,recursion_time);
	}
	else
	{
		return ha03_fgauge_compensate_battery_voltage_recursion(ori_voltage,recursion_time);
	}
}

void fgauge_construct_battery_profile(kal_int32 temperature, BATTERY_PROFILE_STRUC_P temp_profile_p)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_construct_battery_profile(temperature,temp_profile_p);
	}
	else
	{
		return ha03_fgauge_construct_battery_profile(temperature,temp_profile_p);
	}
}

void fgauge_construct_r_table_profile(kal_int32 temperature, R_PROFILE_STRUC_P temp_profile_p)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_construct_r_table_profile(temperature,temp_profile_p);
	}
	else
	{
		return ha03_fgauge_construct_r_table_profile(temperature,temp_profile_p);
	}
}

kal_int32 fgauge_get_dod0(kal_int32 voltage, kal_int32 temperature, kal_bool bOcv)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_get_dod0(voltage, temperature,bOcv);
	}
	else
	{
		return ha03_fgauge_get_dod0(voltage,temperature,bOcv);
	}
}

void fg_qmax_update_for_aging(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fg_qmax_update_for_aging();
	}
	else
	{
		return ha03_fg_qmax_update_for_aging();
	}
}

kal_int32 fgauge_update_dod(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_update_dod();
	}
	else
	{
		return ha03_fgauge_update_dod();
	}
}

kal_int32 fgauge_read_capacity(kal_int32 type)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_read_capacity(type);
	}
	else
	{
		return ha03_fgauge_read_capacity(type);
	}
}

kal_int32 fgauge_read_capacity_by_v(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_read_capacity_by_v();
	}
	else
	{
		return ha03_fgauge_read_capacity_by_v();
	}
}

kal_int32 fgauge_read_r_bat_by_v(kal_int32 voltage)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_read_r_bat_by_v(voltage);
	}
	else
	{
		return ha03_fgauge_read_r_bat_by_v(voltage);
	}
}

kal_int32 fgauge_read_v_by_capacity(int bat_capacity)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_read_v_by_capacity(bat_capacity);
	}
	else
	{
		return ha03_fgauge_read_v_by_capacity(bat_capacity);
	}
}

void fgauge_Normal_Mode_Work(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_Normal_Mode_Work();
	}
	else
	{
		return ha03_fgauge_Normal_Mode_Work();
	}
}
kal_int32 fgauge_get_Q_max(kal_int16 temperature)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_get_Q_max(temperature);
	}
	else
	{
		return ha03_fgauge_get_Q_max(temperature);
	}
}

kal_int32 fgauge_get_Q_max_high_current(kal_int16 temperature)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_get_Q_max_high_current(temperature);
	}
	else
	{
		return ha03_fgauge_get_Q_max_high_current(temperature);
	}
}
void fgauge_initialization(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fgauge_initialization();
	}
	else
	{
		return ha03_fgauge_initialization();
	}
}

kal_int32 FGADC_Get_BatteryCapacity_CoulombMothod(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_FGADC_Get_BatteryCapacity_CoulombMothod();
	}
	else
	{
		return ha03_FGADC_Get_BatteryCapacity_CoulombMothod();
	}
}

kal_int32 FGADC_Get_BatteryCapacity_VoltageMothod(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_FGADC_Get_BatteryCapacity_VoltageMothod();
	}
	else
	{
		return ha03_FGADC_Get_BatteryCapacity_VoltageMothod();
	}
}

kal_int32 FGADC_Get_FG_Voltage(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_FGADC_Get_FG_Voltage();
	}
	else
	{
		return ha03_FGADC_Get_FG_Voltage();
	}
}

void FGADC_Reset_SW_Parameter(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_FGADC_Reset_SW_Parameter();
	}
	else
	{
		return ha03_FGADC_Reset_SW_Parameter();
	}
}

kal_int32 get_dynamic_period(int first_use, int first_wakeup_time, int battery_capacity_level)
{
	if(cci_platform_is_ha01())
	{
		return ha01_get_dynamic_period( first_use,  first_wakeup_time,  battery_capacity_level);
	}
	else
	{
		return ha03_get_dynamic_period( first_use,  first_wakeup_time,  battery_capacity_level);
	}
}

void fg_voltage_mode(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_fg_voltage_mode();
	}
	else
	{
		return ha03_fg_voltage_mode();
	}
}

void FGADC_thread_kthread(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_FGADC_thread_kthread();
	}
	else
	{
		return ha03_FGADC_thread_kthread();
	}
}

int init_proc_log_fg(void)
{
	if(cci_platform_is_ha01())
	{
		return ha01_init_proc_log_fg();
	}
	else
	{
		return ha03_init_proc_log_fg();
	}
}
//////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//// Thermal related flags weli for share codebase issue 20120314
///////////////////////////////////////////////////////////////////////////////////////////
int g_battery_thermal_throttling_flag=0;
int battery_cmd_thermal_test_mode=0;
int battery_cmd_thermal_test_mode_value=0;
int g_battery_tt_check_flag=0;
///////////////////////////////////////////////////////////////////////
kal_uint32 g_eco_version = 0;
int g_E1_vbat_sense = 0;//weli revise for share codebase issue
int Enable_BATDRV_LOG = 0;
//static unsigned short batteryVoltageBuffer[BATTERY_AVERAGE_SIZE];
//static unsigned short batteryCurrentBuffer[BATTERY_AVERAGE_SIZE];
//static unsigned short batterySOCBuffer[BATTERY_AVERAGE_SIZE];
//static int batteryIndex = 0;
//static int batteryVoltageSum = 0;
//static int batteryCurrentSum = 0;
//static int batterySOCSum = 0;
kal_bool g_bat_full_user_view = KAL_FALSE;
kal_bool g_Battery_Fail = KAL_FALSE;
kal_bool batteryBufferFirst = KAL_FALSE;

struct wake_lock battery_suspend_lock; 

int g_BatTempProtectEn = 0; /*0:temperature measuring default off*/
//int g_BatTempProtectEn = 1; /*1:temperature measuring default on*/

//int g_PMIC_CC_VTH = PMIC_ADPT_VOLT_03_300000_V;
//int g_PMIC_CV_VTH = PMIC_ADPT_VOLT_04_000000_V;
int V_PRE2CC_THRES = 3400;
int V_CC2TOPOFF_THRES = 4050;
int V_compensate_EVB = 80;

int g_HW_Charging_Done = 0;
int g_Charging_Over_Time = 0;

int g_SW_CHR_OUT_EN = 0;
int g_HW_stop_charging = 0;

// HW CV algorithm
//int g_sw_cv_enable=0;

int CHARGING_FULL_CURRENT=130;	// mA on phone

int gForceADCsolution=0;
//int gForceADCsolution=1;

int gSyncPercentage=0;

unsigned int g_BatteryNotifyCode=0x0000;
unsigned int g_BN_TestMode=0x0000;

kal_uint32 gFGsyncTimer=0;
kal_uint32 DEFAULT_SYNC_TIME_OUT=60; //1mins

int g_Calibration_FG=0;

int g_XGPT_restart_flag=0;

#define CHR_OUT_CURRENT	50

int gSW_CV_prepare_flag=0;

int getVoltFlag = 0;
int g_bat_temperature_pre=0;

int gADC_BAT_SENSE_temp=0;
int gADC_I_SENSE_temp=0;
int gADC_I_SENSE_offset=0;

int g_battery_flag_resume=0;

int gBAT_counter_15=1;

int gFG_can_reset_flag = 1;

////////////////////////////////////////////////////////////////////////////////
// EM
////////////////////////////////////////////////////////////////////////////////
int g_BAT_TemperatureR = 0;
int g_TempBattVoltage = 0;
int g_InstatVolt = 0;
int g_BatteryAverageCurrent = 0;
int g_BAT_BatterySenseVoltage = 0;
int g_BAT_ISenseVoltage = 0;
int g_BAT_ChargerVoltage = 0;

////////////////////////////////////////////////////////////////////////////////
// Definition For GPT
////////////////////////////////////////////////////////////////////////////////
//static int bat_thread_timeout = 0;

//static DEFINE_MUTEX(bat_mutex);
//static DECLARE_WAIT_QUEUE_HEAD(bat_thread_wq);

////////////////////////////////////////////////////////////////////////////////
//Logging System
////////////////////////////////////////////////////////////////////////////////
int g_chr_event = 0;
int bat_volt_cp_flag = 0;
int bat_volt_check_point = 0;
int g_wake_up_bat=0;

////////////////////////////////////////////////////////////////////////////////
// USB-IF
////////////////////////////////////////////////////////////////////////////////
int g_usb_state = USB_UNCONFIGURED;
//int g_usb_state = USB_SUSPEND;

int g_temp_CC_value = Cust_CC_0MA;
int g_soft_start_delay = 1;

#if (CONFIG_USB_IF == 0)
int g_Support_USBIF = 0;
#else
int g_Support_USBIF = 1;
#endif
//////////////////////////////////////////////////////////////////////////////////
int adc_cali_slop[14] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};
int adc_cali_offset[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int adc_cali_cal[1] = {0};

int adc_in_data[2] = {1,1};
int adc_out_data[2] = {1,1};

int battery_in_data[1] = {0};
int battery_out_data[1] = {0};    

int charging_level_data[1] = {0};

kal_bool g_ADC_Cali = KAL_FALSE;
kal_bool g_ftm_battery_flag = KAL_FALSE;
//////////////////////////////////////////////////////////////////////////////
PMU_ChargerStruct BMT_status;
int g_fg_ready_flag=0;
int POSTFULL_safety_timer=0;
int get_pmic_flag=0;
int g_FG_init = 0;
UINT32 bat_thread_timeout_sum=0;
//////////////////////////////////////////////////////////////////////////////////////////////

///end of mt6575battery.c//////////////////////////////////////////////////////////////////////////
int Enable_FGADC_LOG = 1;//weli revise share codebase issue
/////////////////////
kal_int8 gFG_DOD0_update = 0;
kal_int32 gFG_DOD0 = 0;
kal_int32 gFG_DOD1 = 0;
kal_int32 gFG_DOD1_return = 0;
kal_int32 gFG_columb = 0;
kal_int32 gFG_columb_HW_reg = 0;
kal_int32 gFG_voltage = 0;
kal_int32 gFG_voltage_pre = -500;
kal_int32 gFG_current = 0;
kal_int32 gFG_capacity = 0;
kal_int32 gFG_capacity_by_c = 0;
kal_int32 gFG_capacity_by_c_init = 0;
kal_int32 gFG_capacity_by_v = 0;
kal_int32 gFG_columb_init = 0;
kal_int32 gFG_inner_R = 0;
kal_int16 gFG_temp= 100;
kal_int16 gFG_pre_temp=100;
kal_int16 gFG_T_changed=5;
kal_int32 gEstBatCapacity = 0;
kal_int32 gFG_SW_CoulombCounter = 0;
kal_bool gFG_Is_Charging = KAL_FALSE;
kal_int32 gFG_bat_temperature = 0;
kal_int32 gFG_resistance_bat = 0;
kal_int32 gFG_compensate_value = 0;
kal_int32 gFG_ori_voltage = 0;
kal_int32 gFG_booting_counter_I = 0;
kal_int32 gFG_booting_counter_I_FLAG = 0;
kal_int32 gFG_BATT_CAPACITY = 0;
int vchr_kthread_index=0;
kal_int32 gFG_voltage_init=0;
kal_int32 gFG_current_auto_detect_R_fg_total=0;
kal_int32 gFG_current_auto_detect_R_fg_count=0;
kal_int32 gFG_current_auto_detect_R_fg_result=0;
kal_int32 current_get_ori=0;
int gFG_15_vlot=3700;
kal_int32 gfg_percent_check_point=50;
kal_int32 gFG_BATT_CAPACITY_init_high_current = 1200;
kal_int32 gFG_BATT_CAPACITY_aging = 1200;
int volt_mode_update_timer=0;
int volt_mode_update_time_out=6; //1mins
//////////////////////////////////////////////////////////
int g_fg_dbg_bat_volt=0;
int g_fg_dbg_bat_current=0;
int g_fg_dbg_bat_zcv=0;
int g_fg_dbg_bat_temp=0;
int g_fg_dbg_bat_r=0;
int g_fg_dbg_bat_car=0;
int g_fg_dbg_bat_qmax=0;
int g_fg_dbg_d0=0;
int g_fg_dbg_d1=0;
int g_fg_dbg_percentage=0;
int g_fg_dbg_percentage_fg=0;
int g_fg_dbg_percentage_voltmode=0;
int g_update_qmax_flag=1;
kal_bool gFGvbatBufferFirst = KAL_FALSE;
kal_int32 gFG_voltage_AVG = 0;
kal_int32 gFG_vbat_offset=0;
kal_int32 gFG_voltageVBAT=0;
kal_int32 g_car_instant=0;
kal_int32 g_current_instant=0;
kal_int32 g_car_sleep=0;
kal_int32 g_car_wakeup=0;
kal_int32 g_last_time=0;
kal_int32 gFG_current_inout_battery = 0;
kal_uint32 gRTC_time_suspend=0;
kal_uint32 gRTC_time_resume=0;
kal_uint32 gFG_capacity_before=0;
kal_uint32 gFG_capacity_after=0;
kal_uint32 gFG_RTC_time_MAX=3600; //60mins
/////////////////////////////////////////////////////////////////
/*
 * owner: weli end
 */
/*********************************************************/

#endif /* ***** CCI BSP.SYS ***** */
