/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of MediaTek Inc. (C) 2008
 *
 *  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
 *  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 *  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 *  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 *  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
 *  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
 *  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 *  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 *  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 *  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
 *  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
 *  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
 *  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
 *  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
 *  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
 *
 *****************************************************************************/
#ifdef BUILD_LK
    #ifndef bool
    typedef int bool;
    #endif
#include <platform/cci_drv_common.h>
#include <platform/mt_gpio.h>
#else
#include <cci_drv_common.h>
#include <linux/string.h>
#include <mach/mt_gpio.h>

#ifndef BUILD_UBOOT
// Temporarily remove, Ken
//#include <linux/cci_drv_common.h>
#endif

#endif

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(800)
#define LCM_ID_HX8369  (0x69)
#define LCM_ID_NT35510 (5510)
#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if 0 // Unused code, Ken
static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
#endif
#define LCM_DSI_CMD_MODE									1

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
	unsigned char cmd;
	unsigned char count;
	unsigned char para_list[64];
};

#if 0 // Temporarily remove, Ken
static struct LCM_setting_table lcm_initialization_setting_tianma[] = {

{0xB9,  3,      {0xFF, 0x83, 0x69}},
{REGFLAG_DELAY, 10, {}},

{0xB1, 19,      {0x01, 0x00, 0x34, 0x0A, 0x00,
                 0x11, 0x11, 0x32, 0x2F, 0x3F,
                 0x3F, 0x01, 0x3A, 0x01, 0xE6,
                 0xE6, 0xE6, 0xE6, 0xE6}},
{REGFLAG_DELAY, 10, {}},


{0xB2, 15,      {0x00, 0x20, 0x05, 0x0A, 0x70,
                 0x00, 0xff, 0x00, 0x00, 0x00,
                 0x00, 0x03, 0x03, 0x00, 0x01}},
{REGFLAG_DELAY, 10, {}},

{0xB4,  5,      {0x02, 0x1d, 0x80, 0x06, 0x02}},
{REGFLAG_DELAY, 10, {}},

{0x36,  1,      {0x00}},
{REGFLAG_DELAY, 10, {}},

{0xcc,  1,      {0x00}},
{REGFLAG_DELAY, 10, {}},

{0xB6,  2,      {0x4D, 0x4D}},
{REGFLAG_DELAY, 10, {}},

{0xD5,  26,     {0x00, 0x01, 0x03, 0x28, 0x01,
                 0x06, 0x00, 0x70, 0x11, 0x13,
                 0x00, 0x00, 0x60, 0x24, 0x71,
                 0x35, 0x00, 0x00, 0x71, 0x05,
                 0x60, 0x24, 0x07, 0x0F, 0x04,
                 0x04}},
{REGFLAG_DELAY, 10, {}},

{0xE0, 34,      {0x00, 0x05, 0x0b, 0x0a, 0x07,
                 0x37, 0x20, 0x2f, 0x09, 0x13,
                 0x0e, 0x15, 0x16, 0x14, 0x15,
                 0x11, 0x17, 0x00, 0x05, 0x0b,
                 0x0a, 0x07, 0x37, 0x20, 0x2f,
                 0x09, 0x13, 0x0e, 0x15, 0x16,
                 0x14, 0x15, 0x11, 0x17}},
{REGFLAG_DELAY, 10, {}},

{0xBA, 13,      {0x00, 0xa0, 0xc6, 0x00, 0x0a,
                 0x00, 0x10, 0x30, 0x6f, 0x02,
                 0x11, 0x18, 0x40}},
{REGFLAG_DELAY, 10, {}},

{0x44,  2,      {((FRAME_HEIGHT/2)>>8), ((FRAME_HEIGHT/2)&0xFF)}},
{0x35,  1,      {0x00}},
{REGFLAG_DELAY, 10, {}},
        // Note
        // Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


        // Setting ending by predefined flag
{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static struct LCM_setting_table lcm_initialization_setting_nt35510[] = {
{0x35,  1,  {0x00}},
{0xf0,  5,  {0x55,0xaa,0x52,0x08,0x01}},
{0xb0, 3,   {0x10,0x10,0x10}},
{0xb6, 3,   {0x44,0x44,0x44}},
{0xb1, 3,   {0x10,0x10,0x10}},
{0xb7, 3,   {0x34,0x34,0x34}},
{0xb2, 3,   {0x00,0x00,0x00}},
{0xb8, 3,   {0x34,0x34,0x34}},
{0xbf, 4,   {0x01,0x0f,0x0f,0x0f}},

{0xd1, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
                0x4a,0x00,0x5c,0x00,
                0x81,0x00,0xa6,0x00,
                0xe5,0x01,0x13,0x01,
                0x54,0x01,0x82,0x01,
                0xca,0x02,0x00,0x02,
                0x01,0x02,0x34,0x02,
                0x67,0x02,0x84,0x02,
                0xa4,0x02,0xb7,0x02,
                0xcf,0x02,0xde,0x02,
                0xf2,0x02,0xfe,0x03,
                0x10,0x03,0x33,0x03,
                0x6d}},
{0xd2, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
                0x4a,0x00,0x5c,0x00,
                0x81,0x00,0xa6,0x00,
                0xe5,0x01,0x13,0x01,
                0x54,0x01,0x82,0x01,
                0xca,0x02,0x00,0x02,
                0x01,0x02,0x34,0x02,
                0x67,0x02,0x84,0x02,
                0xa4,0x02,0xb7,0x02,
                0xcf,0x02,0xde,0x02,
                0xf2,0x02,0xfe,0x03,
                0x10,0x03,0x33,0x03,
                0x6d}},
{0xd3, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
                0x4a,0x00,0x5c,0x00,
                0x81,0x00,0xa6,0x00,
                0xe5,0x01,0x13,0x01,
                0x54,0x01,0x82,0x01,
                0xca,0x02,0x00,0x02,
                0x01,0x02,0x34,0x02,
                0x67,0x02,0x84,0x02,
                0xa4,0x02,0xb7,0x02,
                0xcf,0x02,0xde,0x02,
                0xf2,0x02,0xfe,0x03,
                0x10,0x03,0x33,0x03,
                0x6d}},
{0xd4, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
                0x4a,0x00,0x5c,0x00,
                0x81,0x00,0xa6,0x00,
                0xe5,0x01,0x13,0x01,
                0x54,0x01,0x82,0x01,
                0xca,0x02,0x00,0x02,
                0x01,0x02,0x34,0x02,
                0x67,0x02,0x84,0x02,
                0xa4,0x02,0xb7,0x02,
                0xcf,0x02,0xde,0x02,
                0xf2,0x02,0xfe,0x03,
                0x10,0x03,0x33,0x03,
                0x6d}},
{0xd5, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
                0x4a,0x00,0x5c,0x00,
                0x81,0x00,0xa6,0x00,
                0xe5,0x01,0x13,0x01,
                0x54,0x01,0x82,0x01,
                0xca,0x02,0x00,0x02,
                0x01,0x02,0x34,0x02,
                0x67,0x02,0x84,0x02,
                0xa4,0x02,0xb7,0x02,
                0xcf,0x02,0xde,0x02,
                0xf2,0x02,0xfe,0x03,
                0x10,0x03,0x33,0x03,
                0x6d}},
{0xd6, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
                0x4a,0x00,0x5c,0x00,
                0x81,0x00,0xa6,0x00,
                0xe5,0x01,0x13,0x01,
                0x54,0x01,0x82,0x01,
                0xca,0x02,0x00,0x02,
                0x01,0x02,0x34,0x02,
                0x67,0x02,0x84,0x02,
                0xa4,0x02,0xb7,0x02,
                0xcf,0x02,0xde,0x02,
                0xf2,0x02,0xfe,0x03,
                0x10,0x03,0x33,0x03,
                0x6d}},
{REGFLAG_DELAY, 10, {}},
{0xb9, 3,  {0x34,0x34,0x34}},
{0xb5, 3,  {0x08,0x08,0x08}},
{0xba, 3,  {0x14,0x14,0x14}},
{0xbc, 3,  {0x00,0x78,0x00}},
{0xbd, 3,  {0x00,0x78,0x00}},
{0xbe, 2,  {0x00,0x64}},
{0xf0, 5,  {0x55,0xaa,0x52,0x08,0x00}},
{0xb1, 2,  {0xfc,0x00}},
{0xb6, 1,  {0x05}},
{0xb8, 4,  {0x01,0x03,0x03,0x03}},
{0xbc, 3,  {0x01,0x00,0x00}},

	//{0x11, 1, {0x00}},
	//{REGFLAG_DELAY, 200, {}},

{0x2a, 4,  {0x00,0x00,0x01,0xdf}},
{0x2b, 4,  {0x00,0x00,0x03,0x55}},
	//{0x29, 1, {0x00}},
    //{REGFLAG_DELAY, 200, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_initialization_setting_truly[] = {

	/*
Note :

Data ID will depends on the following rule.

count of parameters > 1	=> Data ID = 0x39
count of parameters = 1	=> Data ID = 0x15
count of parameters = 0	=> Data ID = 0x05

Structure Format :

{DCS command, count of parameters, {parameter list}}
{REGFLAG_DELAY, milliseconds of time, {}},

...

Setting ending by predefined flag

{REGFLAG_END_OF_TABLE, 0x00, {}}
*/

#if 1	/* P74 settings */
{0xB9, 	3,	{0xFF, 0x83, 0x69}},
//{REGFLAG_DELAY, 10, {}},

{0xB1,  19,	{0x01, 0x00, 0x34, 0x07, 0x00,
	0x0E, 0x0E, 0x21, 0x29, 0x3F,
	0x3F, 0x01, 0x23, 0x01, 0xE6,
	0xE6, 0xE6, 0xE6, 0xE6}},
//{REGFLAG_DELAY, 10, {}},

{0xB2,	15,	{0x00, 0x20, 0x08, 0x08, 0x70,
	0x00, 0xFF, 0x00, 0x00, 0x00,
	0x00, 0x03, 0x03, 0x00, 0x01}},
//{REGFLAG_DELAY, 10, {}},

{0xB4,	5,	{0x0A, 0x0C, 0x84, 0x0C, 0x01}},
//{REGFLAG_DELAY, 10, {}},

{0xB6,	2,	{0x1F, 0x1F}},
//{REGFLAG_DELAY, 10, {}},

	//[BUG28] jashe add start
{0x44,  2,      {((FRAME_HEIGHT/2)>>8), ((FRAME_HEIGHT/2)&0xFF)}},
{0x35,  1,      {0x00}},
	//[BUG28] jashe add end

{0xCC,	1,	{0x00}},
//{REGFLAG_DELAY, 10, {}},

{0xD5,	26,	{0x00, 0x03, 0x00, 0x00, 0x01,
	0x07, 0x10, 0x80, 0x33, 0x37,
	0x23, 0x01, 0xB9, 0x75, 0xA8,
	0x64, 0x00, 0x00, 0x41, 0x06,
	0x50, 0x07, 0x07, 0x0F, 0x07,
	0x00}},
//{REGFLAG_DELAY, 10, {}},

{0xE0,	34,	{0x00, 0x03, 0x00, 0x09, 0x09,
	0x21, 0x1B, 0x2D, 0x06, 0x0C,
	0x10, 0x15, 0x16, 0x14, 0x16,
	0x12, 0x18, 0x00, 0x03, 0x00,
	0x09, 0x09, 0x21, 0x1B, 0x2D,
	0x06, 0x0C, 0x10, 0x15, 0x16,
	0x14, 0x16, 0x12, 0x18}},
//{REGFLAG_DELAY, 10, {}},

{0x3A,	1,	{0x77}},
//{REGFLAG_DELAY, 10, {}},

{0xBA,	13,	{0x00, 0xA0, 0xC6, 0x00, 0x0A,
	0x00, 0x10, 0x30, 0x6C, 0x02,
	0x11, 0x18, 0x40}},
//{REGFLAG_DELAY, 10, {}},

	//jashe end
#else /* MP setting */
{0xB9,	3,	{0xFF, 0x83, 0x69}},
//{REGFLAG_DELAY, 10, {}},

{0xB0,	2,	{0x01, 0x0B}},
//{REGFLAG_DELAY, 10, {}},

{0xB2,	15,	{0x00, 0x20, 0x05, 0x05,
	0x70, 0x00, 0xFF, 0x00,
	0x00, 0x00, 0x00, 0x03,
	0x03, 0x00, 0x01}},
//{REGFLAG_DELAY, 10, {}},

{0xB4, 	5,	{0x00, 0x18, 0x80, 0x06,
	0x02}},
//{REGFLAG_DELAY, 10, {}},

{0xD5,	26, {0x00, 0x04, 0x03, 0x00,
	0x01, 0x05, 0x28, 0x70,
	0x01, 0x03, 0x00, 0x00,
	0x40, 0x06, 0x51, 0x07,
	0x00, 0x00, 0x41, 0x06,
	0x50, 0x07, 0x07, 0x0F,
	0x04, 0x00}},
//{REGFLAG_DELAY, 10, {}},

{0xB1,	19,	{0x85, 0x00, 0x34, 0x07,
	0x00, 0x0f, 0x0f, 0x2A,
	0x32, 0x3F, 0x3F, 0x01,
	0x3A, 0x01, 0xE6, 0xE6,
	0xE6, 0xE6, 0xE6}},
//{REGFLAG_DELAY, 10, {}},


{0x3A,	1,	{0x07}},
{0xCC,	1,	{0x02}},

{0xB6,	2,	{0x42, 0x42}},      ///apply it as ESD indication
//{REGFLAG_DELAY, 10, {}},

	// ENABLE FMARK
{0x44,	2,	{((FRAME_HEIGHT/2)>>8), ((FRAME_HEIGHT/2)&0xFF)}},
{0x35,	1,	{0x00}},

	// SET GAMMA
{0xE0,	34,	{0x00, 0x31, 0x19, 0x38,
	0x3D, 0x3F, 0x28, 0x46,
	0x07, 0x0D, 0x0E, 0x12,
	0x15, 0x12, 0x14, 0x0F,
	0x17, 0x00, 0x13, 0x19,
	0x38, 0x3D, 0x3F, 0x28,
	0x46, 0x07, 0x0D, 0x0E,
	0x12, 0x15, 0x12, 0x14,
	0x0F, 0x17}},
//{REGFLAG_DELAY, 10, {}}, 

{0xBA,	13,	{0x00, 0xA0, 0xC6, 0x00,
	0x0A, 0x02, 0x10, 0x30,
	0x6F, 0x02, 0x11, 0x18,
	0x40}},

{0x51,	1,	{0x00}},
//{REGFLAG_DELAY, 10, {}},

{0x53,	1,	{0x24}},
//{REGFLAG_DELAY, 10, {}},

{0x55,	1,	{0x00}},
//{REGFLAG_DELAY, 10, {}},

{0x5E,	1,	{0x70}},
//{REGFLAG_DELAY, 10, {}},
#endif

	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
{REGFLAG_END_OF_TABLE, 0x00, {}}
};


#if 0
static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static struct LCM_setting_table lcm_sleep_out_setting[] = {
	// Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	// Display ON
	{0x29, 1, {0x00}},
    {REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},

	// Sleep Mode On
	{0x10, 1, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xB9,	3,	{0xFF, 0x83, 0x69}},
	{REGFLAG_DELAY, 10, {}},

	// Sleep Mode On
	//	{0xC3, 1, {0xFF}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#if 0 // Unused code, Ken
static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for(i = 0; i < count; i++) {

		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {

			case REGFLAG_DELAY :
				MDELAY(table[i].count);
				break;

			case REGFLAG_END_OF_TABLE :
				break;

			default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}

}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	// enable tearing-free
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
#else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count=480*3;	

	params->dsi.vertical_sync_active=3;
	params->dsi.vertical_backporch=12;
	params->dsi.vertical_frontporch=2;
	params->dsi.vertical_active_line=800;

	params->dsi.line_byte=2048;		// 2256 = 752*3
	params->dsi.horizontal_sync_active_byte=26;
	params->dsi.horizontal_backporch_byte=146;
	params->dsi.horizontal_frontporch_byte=146;	
	params->dsi.rgb_byte=(480*3+6);	

	params->dsi.horizontal_sync_active_word_count=20;	
	params->dsi.horizontal_backporch_word_count=140;
	params->dsi.horizontal_frontporch_word_count=140;

	// Bit rate calculation
	params->dsi.pll_div1=38;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
	params->dsi.pll_div2=1;			// div2=0~15: fout=fvo/(2*div2)

}


static void lcm_init(void)
{
    int project_id;
    int isSecondLCM;
#ifdef BUILD_LK
    printf("[LCM][%s] ENTER.\n",__func__);
#else
	printk("[LCM][%s] ENTER.\n",__func__);
#endif

    project_id = cci_project_id();  //1:VT57, 0:VT58
    isSecondLCM = cci_platform_is_pvt_sl_sb_sc();
#ifdef BUILD_LK
    printf("[LCM][%s] project_id = %d, isSecondLCM = %d, LCD_ID = %d.\n",__func__, project_id,     isSecondLCM, mt_get_gpio_in(GPIO162));
#else
    printk("[LCM][%s] project_id = %d, isSecondLCM = %d, LCD_ID = %d.\n",__func__, project_id, isSecondLCM, mt_get_gpio_in(GPIO162));
#endif
    if(project_id)
    {
        if(mt_get_gpio_in(GPIO162)==1)//2nd LCM
        {
            goto nt35510_setting;
        }
        else
        {
            goto hx8369_setting;
        }
    }
    else
    {
        if(isSecondLCM)
        {
            goto nt35510_setting;
        }
        else
        {
            goto hx8369_setting;
        }
    }

nt35510_setting:
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);

    push_table(lcm_initialization_setting_nt35510, sizeof(lcm_initialization_setting_nt35510) / sizeof(struct LCM_setting_table), 1);

#ifdef BUILD_LK
    printf("[LCM][%s]nt35510 EXIT.\n",__func__);
#else
    printk("[LCM][%s]nt35510 EXIT.\n",__func__);
#endif
    return;

hx8369_setting:
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(10);

	push_table(lcm_initialization_setting_truly, sizeof(lcm_initialization_setting_truly) / sizeof(struct LCM_setting_table), 1);
#ifdef BUILD_LK
    printf("[LCM][%s]hx8369 EXIT.\n",__func__);
#else
	printk("[LCM][%s]hx8369 EXIT.\n",__func__);
#endif
    return;
}


static void lcm_suspend(void)
{
    int project_id;
    int isSecondLCM;
#ifdef BUILD_LK
	printf("[LCM][%s] ENTER.\n",__func__);
#else
	printk("[LCM][%s] ENTER.\n",__func__);
#endif
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

    project_id = cci_project_id();  //1:VT57, 0:VT58
    goto nt35510_setting;
    if(project_id)
    {
        if(mt_get_gpio_in(GPIO162)==1)//2nd LCM
        {
            goto nt35510_setting;
        }
        else
        {
            goto hx8369_setting;
        }
    }
    else
    {
        isSecondLCM = cci_platform_is_pvt_sl_sb_sc();
        if(isSecondLCM)
        {
            goto nt35510_setting;
        }
        else
        {
            goto hx8369_setting;
        }
    }

nt35510_setting:
#ifdef BUILD_LK
	printf("[LCM][%s]nt35510 LEAVE.\n",__func__);
#else
	printk("[LCM][%s]nt35510 LEAVE.\n",__func__);
#endif

    return;
hx8369_setting:
	//[OKOK]Himax LCM sleep mode workaround start
        MDELAY(10);
        SET_RESET_PIN(1);
        SET_RESET_PIN(0);
        MDELAY(1);
        SET_RESET_PIN(1);
        MDELAY(10);
        //[OKOK]Himax LCM sleep mode workaround end
#ifdef BUILD_LK
	printf("[LCM][%s]hx8369 LEAVE.\n",__func__);
#else
	printk("[LCM][%s]hx8369 LEAVE.\n",__func__);
#endif
    return;
}


static void lcm_resume(void)
{
	lcm_init();
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_update(unsigned int x, unsigned int y,
		unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(data_array, 7, 0);

}

#if 0 // Unused code, Ken
static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 145;
	unsigned int mapped_level = 0;

	//for LGE backlight IC mapping table
	if(level > 255) 
		level = 255;

	if(level >0) 
		mapped_level = default_level+(level)*(255-default_level)/(255);
	else
		mapped_level=0;

	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_UBOOT
	if(lcm_esd_test)
	{
		lcm_esd_test = FALSE;
		return TRUE;
	}

	/// please notice: the max return packet size is 1
	/// if you want to change it, you can refer to the following marked code
	/// but read_reg currently only support read no more than 4 bytes....
	/// if you need to read more, please let BinHan knows.
	/*
	   unsigned int data_array[16];
	   unsigned int max_return_size = 1;

	   data_array[0]= 0x00003700 | (max_return_size << 16);    

	   dsi_set_cmdq(&data_array, 1, 1);
	   */

	if(read_reg(0xB6) == 0x42)
	{
		return FALSE;
	}
	else
	{            
		return TRUE;
	}
#endif
}

static unsigned int lcm_esd_recover(void)
{
	/*
	unsigned char para = 0;

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(120);
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	MDELAY(10);
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
	MDELAY(10);
	dsi_set_cmdq_V2(0x35, 1, &para, 1);     ///enable TE
	MDELAY(10);

	return TRUE;
	*/
	return TRUE;
}
#endif // End of Unused code, Ken

static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0;
	unsigned char buffer[2];
	unsigned int array[16];
    int project_id;
    int isSecondLCM;
#ifdef BUILD_LK
	printf("[LCM][%s] ENTER.\n",__func__);
#else
	printk("[LCM][%s] ENTER.\n",__func__);
#endif
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

    project_id = cci_project_id();  //1:VT57, 0:VT58
    goto nt35510_setting;
    if(project_id)
    {
        if(mt_get_gpio_in(GPIO162)==1)//2nd LCM
        {
            goto nt35510_setting;
        }
        else
        {
            goto hx8369_setting;
        }
    }
    else
    {
        isSecondLCM = cci_platform_is_pvt_sl_sb_sc();
        if(isSecondLCM)
        {
            goto nt35510_setting;
        }
        else
        {
            goto hx8369_setting;
        }
    }

nt35510_setting:
//Do reset here
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(25);
    SET_RESET_PIN(1);
    MDELAY(50);

    array[0]=0x00063902;
    array[1]=0x52aa55f0;
    array[2]=0x00000108;
    dsi_set_cmdq(array, 3, 1);
    MDELAY(10);

    array[0] = 0x00083700;
    dsi_set_cmdq(array, 1, 1);
    //read_reg_v2(0x04, buffer, 3);//if read 0x04,should get 0x008000,that is both OK.
    read_reg_v2(0xc5, buffer,2);
    id = buffer[0]<<8 |buffer[1];

#ifdef BUILD_LK
    printf("[LCM][%s], id = 0x%08x \n", __func__, id);
#else
    printk("[LCM][%s], id = 0x%08x \n", __func__, id);
#endif

    return (LCM_ID_NT35510 == id)?1:0;

hx8369_setting:
	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(10);

	push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	//	id = read_reg(0xF4);
	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; //we only need ID

#if defined(BUILD_LK)
	printf("[LCM][%s], id = 0x%08x \n", __func__, id);
#else
    printk("[LCM][%s], id = 0x%08x \n", __func__, id);
#endif
	return (LCM_ID_HX8369 == id)?1:0;

}
// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hx8369_dsi_6575_lcm_drv = 
{
	.name			= "hx8369_dsi_6575",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
	//	.set_backlight	= lcm_setbacklight,
	//	.set_pwm        = lcm_setpwm,
	//	.get_pwm        = lcm_getpwm,
	/* [VT] mark for solving white screen. */
	/*        .esd_check     = lcm_esd_check,*/
	/*        .esd_recover   = lcm_esd_recover,*/
	.compare_id    = lcm_compare_id,
#endif
};
