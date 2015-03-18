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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   CFG_file_info_custom.h
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *   Configuration File List for Customer
 *
 *
 * Author:
 * -------
 *   Nick Huang (mtk02183)
 *
 ****************************************************************************/

#ifndef __CFG_FILE_INFO_CUSTOM_H__
#define __CFG_FILE_INFO_CUSTOM_H__

#include "CFG_file_public.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"
#include "../../../common/cgen/cfgfileinc/CFG_AUDIO_File.h"
#include "../../../common/cgen/cfgdefault/CFG_Audio_Default.h"
#include "../cfgfileinc/CFG_GPS_File.h"
#include "../cfgdefault/CFG_GPS_Default.h"
#include "../cfgfileinc/CFG_Wifi_File.h"
#include "../cfgdefault/CFG_WIFI_Default.h"
#include "../cfgfileinc/CFG_PRODUCT_INFO_File.h"
#include "../cfgdefault/CFG_PRODUCT_INFO_Default.h"
#include "../cfgfileinc/CFG_CCI_SW_File.h"   //paul
#include "../cfgdefault/CFG_CCI_SW_Default.h"   //paul
#include "../cfgfileinc/CFG_CUST_IN_SW_File.h"   //paul
#include "../cfgdefault/CFG_CUST_IN_SW_Default.h"   //paul
#include "../cfgfileinc/CFG_CUST_EX_SW_File.h"   //paul
#include "../cfgdefault/CFG_CUST_EX_SW_Default.h"   //paul
#include "../cfgfileinc/CFG_HW_File.h"   //paul
#include "../cfgdefault/CFG_HW_Default.h"   //paul
#include "../cfgfileinc/CFG_FTM_TEST_RESULT_File.h"   //paul
#include "../cfgdefault/CFG_FTM_TEST_RESULT_Default.h"   //paul
#include "../cfgfileinc/CFG_OP_File.h"   //paul
#include "../cfgdefault/CFG_OP_Default.h"   //paul
#include "../cfgfileinc/CFG_Warranty_File.h"
#include "../cfgdefault/CFG_Warranty_Default.h"
#include "../cfgfileinc/CFG_P_SENSOR_THRESHOLD_File.h"
#include "../cfgdefault/CFG_P_SENSOR_THRESHOLD_Default.h"
#include <stdio.h>
#ifdef __cplusplus
extern "C"
{
#endif

    const TCFG_FILE g_akCFG_File_Custom[]=
    {
        {
            "/data/nvram/APCFG/APRDCL/Audio_Sph",       VER(AP_CFG_RDCL_FILE_AUDIO_LID),         CFG_FILE_SPEECH_REC_SIZE,
            CFG_FILE_SPEECH_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                                   (char *)&speech_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDEB/GPS",         VER(AP_CFG_CUSTOM_FILE_GPS_LID),	           CFG_FILE_GPS_CONFIG_SIZE,
            CFG_FILE_GPS_CONFIG_TOTAL,                  SIGNLE_DEFUALT_REC,                (char *)&stGPSConfigDefault, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_CompFlt",       VER(AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID),         CFG_FILE_AUDIO_COMPFLT_REC_SIZE,
            CFG_FILE_AUDIO_COMPFLT_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                (char *)&audio_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_Effect",       VER(AP_CFG_RDCL_FILE_AUDIO_EFFECT_LID),         CFG_FILE_AUDIO_EFFECT_REC_SIZE,
            CFG_FILE_AUDIO_EFFECT_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                (char *)&audio_effect_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDEB/WIFI",	    	VER(AP_CFG_RDEB_FILE_WIFI_LID),		    CFG_FILE_WIFI_REC_SIZE,
            CFG_FILE_WIFI_REC_TOTAL,		    	SIGNLE_DEFUALT_REC,				    (char *)&stWifiCfgDefault, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDEB/WIFI_CUSTOM",	VER(AP_CFG_RDEB_WIFI_CUSTOM_LID),	CFG_FILE_WIFI_CUSTOM_REC_SIZE,
            CFG_FILE_WIFI_CUSTOM_REC_TOTAL,		    SIGNLE_DEFUALT_REC,				    (char *)&stWifiCustomDefault, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_Sph_Med",       VER(AP_CFG_RDCL_FILE_AUDIO_PARAM_MED_LID),         CFG_FILE_AUDIO_PARAM_MED_REC_SIZE,
            CFG_FILE_AUDIO_PARAM_MED_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                (char *)&audio_param_med_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_Vol_custom",       VER(AP_CFG_RDCL_FILE_AUDIO_VOLUME_CUSTOM_LID),         CFG_FILE_AUDIO_VOLUME_CUSTOM_REC_SIZE,
            CFG_FILE_AUDIO_VOLUME_CUSTOM_REC_TOTAL,           SIGNLE_DEFUALT_REC,                (char *)&audio_volume_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Sph_Dual_Mic",       VER(AP_CFG_RDCL_FILE_DUAL_MIC_CUSTOM_LID),         CFG_FILE_SPEECH_DUAL_MIC_SIZE,
            CFG_FILE_SPEECH_DUAL_MIC_TOTAL,           SIGNLE_DEFUALT_REC,                (char *)&dual_mic_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_Wb_Sph",       VER(AP_CFG_RDCL_FILE_AUDIO_WB_PARAM_LID),         CFG_FILE_WB_SPEECH_REC_SIZE,
            CFG_FILE_WB_SPEECH_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                                   (char *)&wb_speech_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDEB/PRODUCT_INFO",       VER(AP_CFG_REEB_PRODUCT_INFO_LID),         CFG_FILE_PRODUCT_INFO_SIZE,
            CFG_FILE_PRODUCT_INFO_TOTAL,                   SIGNLE_DEFUALT_REC,                                   (char *)&stPRODUCT_INFOConfigDefault,DataReset, NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Headphone_CompFlt",       VER(AP_CFG_RDCL_FILE_HEADPHONE_COMPFLT_LID),         CFG_FILE_AUDIO_COMPFLT_REC_SIZE,
            CFG_FILE_HEADPHONE_COMPFLT_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                (char *)&audio_hcf_custom_default, DataReset , NULL
        },

        { "/data/nvram/APCFG/APRDCL/Audio_gain_table",   VER(AP_CFG_RDCL_FILE_AUDIO_GAIN_TABLE_LID), CFG_FILE_AUDIO_GAIN_TABLE_CUSTOM_REC_SIZE,
            CFG_FILE_AUDIO_GAIN_TABLE_CUSTOM_REC_TOTAL, SIGNLE_DEFUALT_REC  ,	 (char *)&Gain_control_table_default, DataReset , NULL
        },

        { "/data/nvram/APCFG/APRDCL/Audio_Hd_Record_Param",   VER(AP_CFG_RDCL_FILE_AUDIO_HD_REC_PAR_LID), CFG_FILE_AUDIO_HD_REC_PAR_SIZE,
            CFG_FILE_AUDIO_HD_REC_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Hd_Recrod_Par_default, DataReset , NULL
        },
        { "/data/nvram/APCFG/APRDCL/Audio_Hd_Record_Scene_Table",   VER(AP_CFG_RDCL_FILE_AUDIO_HD_REC_SCENE_LID), CFG_FILE_AUDIO_HD_REC_SCENE_TABLE_SIZE,
            CFG_FILE_AUDIO_HD_REC_SCENE_TABLE_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Hd_Recrod_Scene_Table_default, DataReset , NULL
        },        
	     //paul
	{
            "/data/nvram/APCFG/APRDEB/CCI_SW",         VER(AP_CFG_CUSTOM_FILE_CCI_SW_LID),	           CFG_FILE_CCI_SW_REC_SIZE,
            CFG_FILE_CCI_SW_REC_TOTAL,                  SIGNLE_DEFUALT_REC,                (char *)&stCCI_SWDefault, DataReset , NULL
        },
        
	{
            "/data/nvram/APCFG/APRDEB/CUST_IN_SW",         VER(AP_CFG_CUSTOM_FILE_CUST_IN_SW_LID),	           CFG_FILE_CUST_IN_SW_REC_SIZE,
            CFG_FILE_CUST_IN_SW_REC_TOTAL,                  SIGNLE_DEFUALT_REC,                (char *)&stCUST_IN_SWDefault, DataReset , NULL
        },
	{
            "/data/nvram/APCFG/APRDEB/CUST_EX_SW",         VER(AP_CFG_CUSTOM_FILE_CUST_EX_SW_LID),	           CFG_FILE_CUST_EX_SW_REC_SIZE,
            CFG_FILE_CUST_EX_SW_REC_TOTAL,                  SIGNLE_DEFUALT_REC,                (char *)&stCUST_EX_SWDefault, DataReset , NULL
        },
	{
            "/data/nvram/APCFG/APRDEB/HW",         VER(AP_CFG_CUSTOM_FILE_HW_LID),	           CFG_FILE_HW_REC_SIZE,
            CFG_FILE_HW_REC_TOTAL,                  SIGNLE_DEFUALT_REC,                (char *)&stHWDefault, DataReset , NULL
        },
	{
            "/data/nvram/APCFG/APRDEB/FTM_TEST_RESULT",         VER(AP_CFG_CUSTOM_FILE_FTM_TEST_RESULT_LID),	           CFG_FILE_FTM_TEST_RESULT_REC_SIZE,
            CFG_FILE_FTM_TEST_RESULT_REC_TOTAL,                  SIGNLE_DEFUALT_REC,                (char *)&stFTM_TEST_RESULTDefault, DataReset , NULL
        },
	{
            "/data/nvram/APCFG/APRDEB/OP",         VER(AP_CFG_CUSTOM_FILE_OP_LID),	           CFG_FILE_OP_REC_SIZE,
            CFG_FILE_OP_REC_TOTAL,                  SIGNLE_DEFUALT_REC,                (char *)&stOPDefault, DataReset , NULL
        },
	//paul
        { "/data/nvram/APCFG/APRDEB/Warranty_Info",   VER(AP_CFG_REEB_FILE_WARRANTY_INFO_LID), CFG_FILE_WARRANTY_INFO_SIZE,
            CFG_FILE_WARRANTY_INFO_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&WarrantyInfoDefault, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDEB/P_SENSOR_THRESHOLD",          VER(AP_CFG_CUSTOM_FILE_P_SENSOR_THRESHOLD_RESULT_LID),               CFG_P_SENSOR_THRESHOLD_RESULT_REC_SIZE,
            CFG_P_SENSOR_THRESHOLD_REC_TOTAL,                  SIGNLE_DEFUALT_REC,                (char *)&stP_SENSOR_THRESHOLDDefault, DataReset , NULL
        },
    };

    int iNvRamFileMaxLID=AP_CFG_CUSTOM_FILE_MAX_LID;
    extern int iNvRamFileMaxLID;
    const unsigned int g_i4CFG_File_Custom_Count = sizeof(g_akCFG_File_Custom)/sizeof(TCFG_FILE);

    extern const TCFG_FILE g_akCFG_File_Custom[];

    extern const unsigned int g_i4CFG_File_Custom_Count;

    int iFileWIFILID=AP_CFG_RDEB_FILE_WIFI_LID;
    extern int iFileWIFILID;
    int iFileCustomWIFILID=AP_CFG_RDEB_WIFI_CUSTOM_LID;
    extern int iFileCustomWIFILID;
    int iFilePRODUCT_INFOLID=AP_CFG_REEB_PRODUCT_INFO_LID;
    extern int iFilePRODUCT_INFOLID;

#ifdef __cplusplus
}
#endif

#endif
