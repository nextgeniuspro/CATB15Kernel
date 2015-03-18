#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/suspend.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/xlog.h>
#include <linux/aee.h>

#include <asm/io.h>
#include <mach/mt_boot.h>
#include <mach/mt_sc.h>
#include <mach/mt_clock_manager.h>
#include <mach/mt_dcm.h>
#include <mach/mtk_rtc.h>
#include <mach/mt_gpio.h>

#include <mt_battery.h>
/*Bug90 Hsipo modify for power debug mechanism start*/
#include <linux/debugfs.h>
#include <linux/module.h>
#include <mach/pmic_mt6329_sw.h>
/*Bug90 Hsipo modify for power debug mechanism end*/
#define XLOG_MYTAG		"Power/Sleep"

#define slp_read16(addr)	(*(volatile u16 *)(addr))
#define slp_write16(addr, val)	(*(volatile u16 *)(addr) = (u16)(val))

#define slp_read32(addr)	(*(volatile u32 *)(addr))
#define slp_write32(addr, val)	(*(volatile u32 *)(addr) = (u32)(val))

#define slp_write_sync()	\
do {				\
	dsb();			\
	outer_sync();		\
} while (0)

#define slp_xverb(fmt, args...)		\
	xlog_printk(ANDROID_LOG_VERBOSE, XLOG_MYTAG, fmt, ##args)

#define slp_xinfo(fmt, args...)		\
	xlog_printk(ANDROID_LOG_INFO, XLOG_MYTAG, fmt, ##args)

#define slp_xerror(fmt, args...)	\
	xlog_printk(ANDROID_LOG_ERROR, XLOG_MYTAG, fmt, ##args)

extern void pmic_status_for_sleep(void);

static wake_reason_t slp_wake_reason;

/*
 * 0: CPU standby + AP sleep => legacy sleep mode
 * 1: CPU dormant (L1/L2 sleep) + AP sleep
 * 2: CPU shutdown (L1/L2 power down) + AP sleep
 */
static unsigned int slp_pwr_level = 2;

static bool slp_dump_pmic = 0;
static bool slp_dump_gpio = 0;
static bool slp_dump_regs = 1;

static unsigned long RM_CLK_SETTLE = 0;
static unsigned long RM_TMR_SSTA = 0;
/*Bug90 Hsipo modify for power debug mechanism start*/

struct gpio_struct{
	int pin;
	int mode;
	int pullsel;
	int din;
	int dout;
	int pullen;
	int dir;
	int dinv;
};
struct regulator_struct{
	char name[10];
	kal_uint8 reg;
};
#define MAX_GPIO_REG_BITS	16
#define MAX_GPIO_MODE_PER_REG	5
#define GPIO_MODE_BITS		3
#define REGULATOR_NUM	LDO_MAX+5
unsigned int suspend_info_enable = 0;
static unsigned int is_regulator_suspend_valid = 0;
static struct dentry *suspend_info_debugfs_dir;
static struct gpio_struct gpio_suspend_data[MAX_GPIO_PIN];
static struct gpio_struct gpio_current_data[MAX_GPIO_PIN];
static struct regulator_struct regulator_suspend_data[REGULATOR_NUM];
static struct regulator_struct regulator_current_data[REGULATOR_NUM];
extern char cci_wakeup_reason[10][128];
extern unsigned int is_wakeup_reason_valid;

void regulator_dump_info(int suspend, struct regulator_struct reg_val[])
{
	kal_uint32 ret=0;

	//BUCK
	strcpy(reg_val[LDO_VRTC+1].name, "VAPROC");
	ret = pmic_read_interface(0x42, &reg_val[LDO_VRTC+1].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] BUCK_VAPROC_STATUS[0x42] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VRTC+2].name, "VCORE");
	ret = pmic_read_interface(0x55, &reg_val[LDO_VRTC+2].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] BUCK_VCORE_STATUS[0x55] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VRTC+3].name, "VRF18");
	ret = pmic_read_interface(0x60, &reg_val[LDO_VRTC+3].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] BUCK_VRF18_STATUS[0x60] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VRTC+4].name, "VIO18");
	ret = pmic_read_interface(0x6A, &reg_val[LDO_VRTC+4].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] BUCK_VIO18_STATUS[0x6A] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VRTC+5].name, "VPA");
	ret = pmic_read_interface(0x73, &reg_val[LDO_VRTC+5].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] BUCK_VPA_STATUS[0x73] : 0x%x\r\n", reg_val);

	//LDO
	strcpy(reg_val[LDO_VM12_1].name, "VM12_1");
	ret = pmic_read_interface(0x83, &reg_val[LDO_VM12_1].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VM12_1_STATUS[0x83] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VM12_2].name, "VM12_2");
	ret = pmic_read_interface(0x87, &reg_val[LDO_VM12_2].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VM12_2_STATUS[0x87] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VM12_INT].name, "VM12_INT");
	ret = pmic_read_interface(0x8D, &reg_val[LDO_VM12_INT].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VM12_INT_STATUS[0x8D] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VIO28].name, "VIO28");
	ret = pmic_read_interface(0x92, &reg_val[LDO_VIO28].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VIO28_STATUS[0x92] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VSIM].name, "VSIM");
	ret = pmic_read_interface(0x95, &reg_val[LDO_VSIM].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VSIM_STATUS[0x95] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VSIM2].name, "VSIM2");
	ret = pmic_read_interface(0x98, &reg_val[LDO_VSIM2].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VSIM2_STATUS[0x98] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VUSB].name, "VUSB");
	ret = pmic_read_interface(0x9B, &reg_val[LDO_VUSB].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VUSB_STATUS[0x9B] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VCAMD].name, "VCAMD");
	ret = pmic_read_interface(0x9E, &reg_val[LDO_VCAMD].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VCAMD_STATUS[0x9E] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VCAM_IO].name, "VCAM_IO");
	ret = pmic_read_interface(0xA1, &reg_val[LDO_VCAM_IO].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VCAM_IO_STATUS[0xA1] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VCAM_AF].name, "VCAM_AF");
	ret = pmic_read_interface(0xA4, &reg_val[LDO_VCAM_AF].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VCAM_AF_STATUS[0xA4] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VMC].name, "VMC");
	ret = pmic_read_interface(0xA7, &reg_val[LDO_VMC].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VMC_STATUS[0xA7] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VMCH].name, "VMCH");
	ret = pmic_read_interface(0xAB, &reg_val[LDO_VMCH].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VMCH_STATUS[0xAB] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VGP].name, "VGP");
	ret = pmic_read_interface(0xAF, &reg_val[LDO_VGP].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VGP_STATUS[0xAF] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VGP2].name, "VGP2");
	ret = pmic_read_interface(0xB2, &reg_val[LDO_VGP2].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VGP2_STATUS[0xB2] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VIBR].name, "VIBR");
	ret = pmic_read_interface(0xB5, &reg_val[LDO_VIBR].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VIBR_STATUS[0xB5] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VRF].name, "VRF");
	ret = pmic_read_interface(0xBE, &reg_val[LDO_VRF].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VRF_STATUS[0xBE] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VTCXO].name, "VTCXO");
	ret = pmic_read_interface(0xC1, &reg_val[LDO_VTCXO].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VTCXO_STATUS[0xC1] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VA1].name, "VA1");
	ret = pmic_read_interface(0xC4, &reg_val[LDO_VA1].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VA1_STATUS[0xC4] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VA2].name, "VA2");
	ret = pmic_read_interface(0xC8, &reg_val[LDO_VA2].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VA2_STATUS[0xC8] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VCAMA].name, "VCAMA");
	ret = pmic_read_interface(0xCB, &reg_val[LDO_VCAMA].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VCAMA_STATUS[0xCB] : 0x%x\r\n", reg_val);

	strcpy(reg_val[LDO_VRTC].name, "VRTC");
	ret = pmic_read_interface(0xCF, &reg_val[LDO_VRTC].reg, 0xFF, 0x0);
	//    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] LDO_VRTC_STATUS[0xCF] : 0x%x\r\n", reg_val);
	if(suspend)
		is_regulator_suspend_valid = 1;

}

void gpio_dump_info(int suspend, struct gpio_struct gpio_setting[])
{
	int idx = 0;
	int num, bit;
	int mode, pullsel, din, dout, pullen, dir, dinv;
	u32 mask = (1L << GPIO_MODE_BITS) - 1;
	GPIO_REGS *reg = (GPIO_REGS*)GPIO_BASE;
	for (idx = 0; idx < MAX_GPIO_PIN; idx++) {
		num = idx / MAX_GPIO_REG_BITS;
		bit = idx % MAX_GPIO_REG_BITS;
		gpio_setting[idx].pin     = idx;
		gpio_setting[idx].pullsel = (reg->pullsel[num].val & (1L << bit)) ? (1) : (0);
		gpio_setting[idx].din     = (reg->din[num].val & (1L << bit)) ? (1) : (0);
		gpio_setting[idx].dout    = (reg->dout[num].val & (1L << bit)) ? (1) : (0);
		gpio_setting[idx].pullen  = (reg->pullen[num].val & (1L << bit)) ? (1) : (0);
		gpio_setting[idx].dir     = (reg->dir[num].val & (1L << bit)) ? (1) : (0);
		gpio_setting[idx].dinv    = (reg->dinv[num].val & (1L << bit)) ? (1) : (0);
		num = idx / MAX_GPIO_MODE_PER_REG;
		bit = idx % MAX_GPIO_MODE_PER_REG;
		gpio_setting[idx].mode = (reg->mode[num].val >> (GPIO_MODE_BITS*bit)) & mask;
	}
	if(suspend)
		is_regulator_suspend_valid = 1;

}
/*Bug90 Hsipo modify for power debug mechanism end*/
static int slp_md_sta_show(struct seq_file *m, void *v)
{
	u32 ssta, mdssta = 0;
	int i;

	if (!RM_TMR_SSTA)
		return -ENOMEM;

	sc_request_mdbus_clk(SC_MDBUS_USER_SLEEP);
	slp_xinfo("RM_CLK_SETTLE = 0x%x\n", RM_CLK_SETTLE ? slp_read32(RM_CLK_SETTLE) : 0);
	ssta = slp_read32(RM_TMR_SSTA);
	sc_unrequest_mdbus_clk(SC_MDBUS_USER_SLEEP);

	for (i = 0; i < 4; i++) {
		/* check if timer is in Pause or Pre-Pause state */
		if ((ssta & 0xf) == 0x2 || (ssta & 0xf) == 0x4)
			mdssta |= (1U << (i * 4));
		ssta >>= 4;
	}

	seq_printf(m, "0x%x\n", mdssta);

	return 0;
}

static int slp_md_sta_open(struct inode *inode, struct file *file)
{
	return single_open(file, slp_md_sta_show, NULL);
}

static struct file_operations slp_md_sta_fops = {
	.open		= slp_md_sta_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static void slp_dump_pm_regs(void)
{
#if 0
	/* TOPSM registers */
	sc_request_mdbus_clk(SC_MDBUS_USER_SLEEP);
	slp_xverb("RM_CLK_SETTLE   0x%x = 0x%x\n", 0xd1090000, RM_CLK_SETTLE ? slp_read32(RM_CLK_SETTLE) : 0);
	sc_unrequest_mdbus_clk(SC_MDBUS_USER_SLEEP);
#endif

	/* ACP related registers */
	slp_xverb("PERIAXIBUS_CTL1 0x%x = 0x%x\n", 0xf1000200, slp_read32(0xf1000200));
	slp_xverb("PG_DMA_CACHE    0x%x = 0x%x\n", 0xf1000100, slp_read32(0xf1000100));
	slp_xverb("VDEC_DMACFG     0x%x = 0x%x\n", 0xf2084024, slp_read32(0xf2084024));
	slp_xverb("VENC_MC_CTRL    0x%x = 0x%x\n", 0xf208506c, slp_read32(0xf208506c));

	/* PLL registers */
	slp_xverb("PLL_CON1        0x%x = 0x%x\n", PLL_CON1   , slp_read16(PLL_CON1));
	slp_xverb("PLL_CON2        0x%x = 0x%x\n", PLL_CON2   , slp_read16(PLL_CON2));
	slp_xverb("IPLL_CON0       0x%x = 0x%x\n", IPLL_CON0  , slp_read16(IPLL_CON0));
	slp_xverb("UPLL_CON0       0x%x = 0x%x\n", UPLL_CON0  , slp_read16(UPLL_CON0));
	slp_xverb("MDPLL_CON0      0x%x = 0x%x\n", MDPLL_CON0 , slp_read16(MDPLL_CON0));
	slp_xverb("WPLL_CON0       0x%x = 0x%x\n", WPLL_CON0  , slp_read16(WPLL_CON0));
	slp_xverb("AUDPLL_CON0     0x%x = 0x%x\n", AUDPLL_CON0, slp_read16(AUDPLL_CON0));

	/* TOPCKGEN/PERICFG/MMSYS/MCUSYS registers */
	slp_xverb("TOP_CKMUXSEL    0x%x = 0x%x\n", TOP_CKMUXSEL       , slp_read32(TOP_CKMUXSEL));
	slp_xverb("TOP_DCMCTL      0x%x = 0x%x\n", TOP_DCMCTL         , slp_read32(TOP_DCMCTL));
	slp_xverb("TOP_MISC        0x%x = 0x%x\n", TOP_MISC           , slp_read32(TOP_MISC));
	slp_xverb("GLOBALCON_PDN0  0x%x = 0x%x\n", PERI_GLOBALCON_PDN0, slp_read32(PERI_GLOBALCON_PDN0));
	slp_xverb("GLOBALCON_PDN1  0x%x = 0x%x\n", PERI_GLOBALCON_PDN1, slp_read32(PERI_GLOBALCON_PDN1));
	slp_xverb("GLOBALCON_PDN2  0x%x = 0x%x\n", PERI_GLOBALCON_PDN2, slp_read32(PERI_GLOBALCON_PDN2));
	slp_xverb("GLOBALCON_PDN3  0x%x = 0x%x\n", PERI_GLOBALCON_PDN3, slp_read32(PERI_GLOBALCON_PDN3));
	slp_xverb("MMSYS1_CG_CON0  0x%x = 0x%x\n", MMSYS1_CG_CON0     , slp_read32(MMSYS1_CG_CON0));
	slp_xverb("MMSYS1_CG_CON1  0x%x = 0x%x\n", MMSYS1_CG_CON1     , slp_read32(MMSYS1_CG_CON1));
	slp_xverb("MMSYS1_CG_CON2  0x%x = 0x%x\n", MMSYS1_CG_CON2     , slp_read32(MMSYS1_CG_CON2));
	slp_xverb("MMSYS2_CG_CON   0x%x = 0x%x\n", MMSYS2_CG_CON      , slp_read32(MMSYS2_CG_CON));
	slp_xverb("PWR_CTL1        0x%x = 0x%x\n", 0xf0009024         , slp_read32(0xf0009024));
	slp_xverb("PWR_MON         0x%x = 0x%x\n", 0xf00090a8         , slp_read32(0xf00090a8));

	/* SLPCTRL registers */
	slp_xverb("SC_CLK_SETTLE   0x%x = 0x%x\n", SC_CLK_SETTLE  , slp_read32(SC_CLK_SETTLE));
	slp_xverb("SC_PWR_SETTLE   0x%x = 0x%x\n", SC_PWR_SETTLE  , slp_read32(SC_PWR_SETTLE));
	slp_xverb("SC_PWR_CON0     0x%x = 0x%x\n", SC_PWR_CON0    , slp_read32(SC_PWR_CON0));
	slp_xverb("SC_PWR_CON1     0x%x = 0x%x\n", SC_PWR_CON1    , slp_read32(SC_PWR_CON1));
	slp_xverb("SC_PWR_CON2     0x%x = 0x%x\n", SC_PWR_CON2    , slp_read32(SC_PWR_CON2));
	slp_xverb("SC_PWR_CON3     0x%x = 0x%x\n", SC_PWR_CON3    , slp_read32(SC_PWR_CON3));
	slp_xverb("SC_PWR_CON4     0x%x = 0x%x\n", SC_PWR_CON4    , slp_read32(SC_PWR_CON4));
	slp_xverb("SC_PWR_CON5     0x%x = 0x%x\n", SC_PWR_CON5    , slp_read32(SC_PWR_CON5));
	slp_xverb("SC_PWR_CON6     0x%x = 0x%x\n", SC_PWR_CON6    , slp_read32(SC_PWR_CON6));
	slp_xverb("SC_PWR_CON7     0x%x = 0x%x\n", SC_PWR_CON7    , slp_read32(SC_PWR_CON7));
	slp_xverb("SC_PWR_CON8     0x%x = 0x%x\n", SC_PWR_CON8    , slp_read32(SC_PWR_CON8));
	slp_xverb("SC_PWR_CON9     0x%x = 0x%x\n", SC_PWR_CON9    , slp_read32(SC_PWR_CON9));
	slp_xverb("SC_CLK_CON      0x%x = 0x%x\n", SC_CLK_CON     , slp_read32(SC_CLK_CON));
	slp_xverb("SC_MD_CLK_CON   0x%x = 0x%x\n", SC_MD_CLK_CON  , slp_read32(SC_MD_CLK_CON));
	slp_xverb("SC_MD_INTF_CON  0x%x = 0x%x\n", SC_MD_INTF_CON , slp_read32(SC_MD_INTF_CON));
	slp_xverb("SC_MD_INTF_STS  0x%x = 0x%x\n", SC_MD_INTF_STS , slp_read32(SC_MD_INTF_STS));
	slp_xverb("SC_TMR_PWR      0x%x = 0x%x\n", SC_TMR_PWR     , slp_read32(SC_TMR_PWR));
	slp_xverb("SC_PERI_CON     0x%x = 0x%x\n", SC_PERI_CON    , slp_read32(SC_PERI_CON));
	slp_xverb("SC_PWR_STA      0x%x = 0x%x\n", SC_PWR_STA     , slp_read32(SC_PWR_STA));
	slp_xverb("SC_APMCU_PWRCTL 0x%x = 0x%x\n", SC_APMCU_PWRCTL, slp_read32(SC_APMCU_PWRCTL));
	slp_xverb("SC_AP_DVFS_CON  0x%x = 0x%x\n", SC_AP_DVFS_CON , slp_read32(SC_AP_DVFS_CON));
}

static int slp_suspend_ops_valid(suspend_state_t pm_state)
{
	return pm_state == PM_SUSPEND_MEM;
}

static int slp_suspend_ops_begin(suspend_state_t state)
{
	/* legacy log */
	slp_xinfo("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	slp_xinfo("_Chip_pm_begin (%u) @@@@@@@@@@@@@@@@@@\n", slp_pwr_level);
	slp_xinfo(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	slp_wake_reason = WR_NONE;

	return 0;
}

static int slp_suspend_ops_prepare(void)
{
	/* legacy log */
	aee_sram_printk("_Chip_pm_prepare\n");
	slp_xinfo("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	slp_xinfo("_Chip_pm_prepare @@@@@@@@@@@@@@@@@@@@\n");
	slp_xinfo(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

/*Bug90 Hsipo modify for power debug mechanism start*/
	if (suspend_info_enable)
		regulator_dump_info(1, regulator_suspend_data);
/*Bug90 Hsipo modify for power debug mechanism end*/

	if (slp_dump_pmic)
		pmic_status_for_sleep();

	/* ccci callback is removed and moved to ccci_helper module */

	return 0;
}

static int slp_suspend_ops_enter(suspend_state_t state)
{
	u32 topmisc;
	unsigned int pwrlevel;

	/* legacy log */
	aee_sram_printk("_Chip_pm_enter\n");
	slp_xinfo("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	slp_xinfo("_Chip_pm_enter @@@@@@@@@@@@@@@@@@@@@@\n");
	slp_xinfo(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

/*Bug90 Hsipo modify for power debug mechanism start*/
	if (suspend_info_enable)
		gpio_dump_info(1, gpio_suspend_data);
/*Bug90 Hsipo modify for power debug mechanism end*/
	if (slp_dump_gpio)
		gpio_dump_regs();

	/* keep CA9 clock frequency when WFI to sleep */
	topmisc = slp_read32(TOP_MISC);
	slp_write32(TOP_MISC, topmisc & ~(1U << 0));
	slp_write_sync();

	if (slp_dump_regs)
		slp_dump_pm_regs();

	rtc_disable_writeif();

	pwrlevel = slp_pwr_level;
	if ((pwrlevel != 0) && (slp_read32(0xf0009024) & 0x8000)) {
		slp_xerror("!!! WILL NOT POWER DOWN CPUSYS DUE TO CPU1 ON !!!\n");
		pwrlevel = 0;
	}
	slp_wake_reason = sc_go_to_sleep(pwrlevel);

	rtc_enable_writeif();

	/* restore TOP_MISC */
	slp_write32(TOP_MISC, topmisc);
	slp_write_sync();

	return 0;
}

static void slp_suspend_ops_finish(void)
{
	/* legacy log */
	aee_sram_printk("_Chip_pm_finish\n");
	slp_xinfo("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	slp_xinfo("_Chip_pm_finish @@@@@@@@@@@@@@@@@@@@@\n");
	slp_xinfo(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	/* debug help */
	slp_xinfo("Battery_Voltage = %lu\n", BAT_Get_Battery_Voltage(0));
}

static void slp_suspend_ops_end(void)
{
	/* legacy log */
	slp_xinfo("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	slp_xinfo("_Chip_pm_end @@@@@@@@@@@@@@@@@@@@@@@@\n");
	slp_xinfo(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
}

static struct platform_suspend_ops slp_suspend_ops = {
	.valid		= slp_suspend_ops_valid,
	.begin		= slp_suspend_ops_begin,
	.prepare	= slp_suspend_ops_prepare,
	.enter		= slp_suspend_ops_enter,
	.finish		= slp_suspend_ops_finish,
	.end		= slp_suspend_ops_end,
};

/*Bug90 Hsipo modify for power debug mechanism start*/
void print_regualtor_setting(struct seq_file *s, struct regulator_struct regulator_setting[])
{
	unsigned int i;
	seq_printf(s, "Name	   Register\n");
	for(i = 0; i < REGULATOR_NUM; i++)
	{
		seq_printf(s, "%-10s 0x%2x\n", regulator_setting[i].name, regulator_setting[i].reg);
	}
}

void regulator_suspend_info_show(struct seq_file *s)
{
	if(is_regulator_suspend_valid)
	{

		print_regualtor_setting(s, regulator_suspend_data);
	}else{
		seq_printf(s, "Not get into suspend state yet\n");
	}

}

static int regulator_suspend_open(struct inode *inode, struct file *file)
{
       return single_open(file, regulator_suspend_info_show, NULL);
}

static const struct file_operations regulator_suspend_fops = {
       .open   = regulator_suspend_open,
       .read           = seq_read,
       .llseek = seq_lseek,
       .release        = single_release,
};
void regulator_current_info_show(struct seq_file *s)
{
	regulator_dump_info(0, regulator_current_data);
	print_regualtor_setting(s, regulator_current_data);
}

static int regulator_current_open(struct inode *inode, struct file *file)
{
       return single_open(file, regulator_current_info_show, NULL);
}

static const struct file_operations regulator_current_fops = {
       .open   = regulator_current_open,
       .read           = seq_read,
       .llseek = seq_lseek,
       .release        = single_release,
};
static int get_enable(void *data, u64 * val)
{
       int ret;

       ret = suspend_info_enable;
       *val = ret;
       return 0;
}

static int set_enable(void *data, u64 val)
{
       suspend_info_enable = (unsigned int)val;
       return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(enable_fops, get_enable, set_enable, "%llu\n");

static void print_gpio_setting(struct seq_file *s, struct gpio_struct gpio_setting)
{
	char buf[20];

		seq_printf(s, "%3d %2d", gpio_setting.pin, gpio_setting.mode);
		switch(gpio_setting.pullsel)
		{
			case 0:
				sprintf(buf,"PD");
				break;
			case 1:
				sprintf(buf,"PU");
				break;
			default:
				sprintf(buf,"na");

		}
		seq_printf(s, "%3s", buf);
		switch(gpio_setting.din)
		{
			case 0:
				sprintf(buf,"L");
				break;
			case 1:
				sprintf(buf,"H");
				break;
			default:
				sprintf(buf,"na");

		}
		seq_printf(s, "%3s", buf);
		switch(gpio_setting.dout)
		{
			case 0:
				sprintf(buf,"L");
				break;
			case 1:
				sprintf(buf,"H");
				break;
			default:
				sprintf(buf,"na");

		}
		seq_printf(s, "%3s", buf);

		switch(gpio_setting.pullen)
		{
			case 0:
				sprintf(buf,"DIS");
				break;
			case 1:
				sprintf(buf,"EN");
				break;
			default:
				sprintf(buf,"na");

		}
		seq_printf(s, "%4s", buf);
		switch(gpio_setting.dir)
		{
			case 0:
				sprintf(buf,"IN");
				break;
			case 1:
				sprintf(buf,"OUT");
				break;
			default:
				sprintf(buf,"na");

		}
		seq_printf(s, "%4s", buf);

		switch(gpio_setting.dinv)
		{
			case 0:
				sprintf(buf,"un");
				break;
			case 1:
				sprintf(buf,"inv");
				break;
			default:
				sprintf(buf,"na");

		}
		seq_printf(s, "%4s", buf);

		seq_printf(s, "\n");

}

static void gpio_suspend_info_show(struct seq_file *s)
{
       unsigned int i;

       if(is_regulator_suspend_valid)
       {
               for(i = 0 ; i < 100 ; i++)
               {
		print_gpio_setting(s, gpio_suspend_data[i]);

               }
       }else{
       seq_printf(s, "Not get into suspend state yet\n");
       }
}

static int gpio_suspend_open(struct inode *inode, struct file *file)
{
       return single_open(file, gpio_suspend_info_show, NULL);
}

static const struct file_operations gpio_suspend_fops = {
       .open   = gpio_suspend_open,
       .read           = seq_read,
       .llseek = seq_lseek,
       .release        = single_release,
};

static void gpio_suspend_info_show_100(struct seq_file *s)
{
	unsigned int i;

	if(is_regulator_suspend_valid)
	{
		for(i = 100 ; i < MAX_GPIO_PIN ; i++)
		{
			print_gpio_setting(s, gpio_suspend_data[i]);

		}
	}else{
		seq_printf(s, "Not get into suspend state yet\n");
	}
}

static int gpio_suspend_open_100(struct inode *inode, struct file *file)
{
       return single_open(file, gpio_suspend_info_show_100, NULL);
}

static const struct file_operations gpio_suspend_fops_100 = {
       .open   = gpio_suspend_open_100,
       .read           = seq_read,
       .llseek = seq_lseek,
       .release        = single_release,
};

static void gpio_current_info_show(struct seq_file *s)
{
	unsigned int i;
	gpio_dump_info(0, gpio_current_data);
	for(i = 0 ; i < 100 ; i++)
	{
		print_gpio_setting(s, gpio_current_data[i]);
	}
}

static int gpio_current_open(struct inode *inode, struct file *file)
{
       return single_open(file, gpio_current_info_show, NULL);
}

static const struct file_operations gpio_current_fops = {
       .open   = gpio_current_open,
       .read           = seq_read,
       .llseek = seq_lseek,
       .release        = single_release,
};

static void gpio_current_info_show_100(struct seq_file *s)
{
	unsigned int i;
	gpio_dump_info(0, gpio_current_data);
	for(i = 100 ; i < MAX_GPIO_PIN ; i++)
	{
		print_gpio_setting(s, gpio_current_data[i]);
	}
}

static int gpio_current_open_100(struct inode *inode, struct file *file)
{
       return single_open(file, gpio_current_info_show_100, NULL);
}

static const struct file_operations gpio_current_fops_100 = {
       .open   = gpio_current_open_100,
       .read           = seq_read,
       .llseek = seq_lseek,
       .release        = single_release,
};

static void wakeup_reason_show(struct seq_file *s)
{
	if(is_wakeup_reason_valid)
	{
		unsigned int i;
		for(i=0;i<10;i++){
				seq_printf(s, "%s\n", cci_wakeup_reason[i]);
		}
	}else{
		seq_printf(s, "Not get into suspend state yet\n");
	}

}

static int wakeup_reason_open(struct inode *inode, struct file *file)
{
       return single_open(file, wakeup_reason_show, NULL);
}

static const struct file_operations wakeup_reason_fops = {
       .open   = wakeup_reason_open,
       .read           = seq_read,
       .llseek = seq_lseek,
       .release        = single_release,
};
/*Bug90 Hsipo modify for power debug mechanism end*/

void slp_mod_init(void)
{
	sc_mod_init();

	suspend_set_ops(&slp_suspend_ops);

	RM_CLK_SETTLE = (unsigned long)ioremap_nocache(0xd1090000, 4);
	if (!RM_CLK_SETTLE)
		slp_xerror("ioremap RM_CLK_SETTLE failed\n");

	RM_TMR_SSTA = (unsigned long)ioremap_nocache(0xd1090040, 4);
	if (!RM_TMR_SSTA)
		slp_xerror("ioremap RM_TMR_SSTA failed\n");
	proc_create_data("slp_md_sta", 0444, NULL, &slp_md_sta_fops, NULL);

/*Bug90 Hsipo modify for power debug mechanism start*/
       suspend_info_debugfs_dir = debugfs_create_dir("cci_power_info", NULL);
       if (IS_ERR_OR_NULL(suspend_info_debugfs_dir))
               pr_err("%s():Cannot create debugfs dir\n", __func__);

       debugfs_create_file("dump_suspend_enable", 0644, suspend_info_debugfs_dir, NULL, &enable_fops);
       debugfs_create_file("gpio_suspend_info_0-100", 0644, suspend_info_debugfs_dir, NULL, &gpio_suspend_fops);
       debugfs_create_file("gpio_suspend_info_100-end", 0644, suspend_info_debugfs_dir, NULL, &gpio_suspend_fops_100);
       debugfs_create_file("gpio_current_info_0-100", 0644, suspend_info_debugfs_dir, NULL, &gpio_current_fops);
       debugfs_create_file("gpio_current_info_100-end", 0644, suspend_info_debugfs_dir, NULL, &gpio_current_fops_100);
       debugfs_create_file("regulator_suspend_info", 0644, suspend_info_debugfs_dir, NULL, &regulator_suspend_fops);
       debugfs_create_file("regulator_current_info", 0644, suspend_info_debugfs_dir, NULL, &regulator_current_fops);
       debugfs_create_file("wakeup_reason", 0644, suspend_info_debugfs_dir, NULL, &wakeup_reason_fops);
/*Bug90 Hsipo modify for power debug mechanism end*/
	//console_suspend_enabled = 0;
}

module_param(slp_pwr_level, uint, 0644);

module_param(slp_dump_pmic, bool, 0644);
module_param(slp_dump_gpio, bool, 0644);
module_param(slp_dump_regs, bool, 0644);

MODULE_AUTHOR("Terry Chang <terry.chang@mediatek.com>");
MODULE_DESCRIPTION("MT6577/75 Sleep Driver v3.7");
