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
 
/******************************************************************************* 
* 
* Filename: 
* --------- 
* aud_custom_exp.h 
* 
* Project: 
* -------- 
*   DUMA 
* 
* Description: 
* ------------ 
* This file is the header of audio customization related function or definition. 
* 
* Author: 
* ------- 
* JY Huang 
* 
*============================================================================ 
*             HISTORY 
* Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!! 
*------------------------------------------------------------------------------ 
* $Revision:$ 
* $Modtime:$ 
* $Log:$ 
* 
* 05 26 2010 chipeng.chang 
* [ALPS00002287][Need Patch] [Volunteer Patch] ALPS.10X.W10.11 Volunteer patch for audio paramter 
* modify audio parameter. 
* 
* 05 26 2010 chipeng.chang 
* [ALPS00002287][Need Patch] [Volunteer Patch] ALPS.10X.W10.11 Volunteer patch for audio paramter 
* modify for Audio parameter 
* 
*    mtk80306 
* [DUMA00132370] waveform driver file re-structure. 
* waveform driver file re-structure. 
* 
* Jul 28 2009 mtk01352 
* [DUMA00009909] Check in TWO_IN_ONE_SPEAKER and rearrange 
* 
* 
* 
*------------------------------------------------------------------------------ 
* Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!! 
*============================================================================ 
****************************************************************************/ 
#ifndef AUDIO_CUSTOM_H
#define AUDIO_CUSTOM_H
 
/* define Gain For Normal */ 
/* Normal volume: TON, SPK, MIC, FMR, SPH, SID, MED */ 
/* 
#define GAIN_NOR_TON_VOL      8     // reserved 
#define GAIN_NOR_KEY_VOL      43    // TTY_CTM_Mic 
#define GAIN_NOR_MIC_VOL      26    // IN_CALL BuiltIn Mic gain 
// GAIN_NOR_FMR_VOL is used as idle mode record volume 
#define GAIN_NOR_FMR_VOL      0     // Normal BuiltIn Mic gain 
#define GAIN_NOR_SPH_VOL      20     // IN_CALL EARPIECE Volume 
#define GAIN_NOR_SID_VOL      100  // IN_CALL EARPICE sidetone 
#define GAIN_NOR_MED_VOL      25   // reserved 
*/ 
 
#define GAIN_NOR_TON_VOL    8    // reserved 
#define GAIN_NOR_KEY_VOL    43    // TTY_CTM_Mic 
#define GAIN_NOR_MIC_VOL    26    // IN_CALL BuiltIn Mic gain 
// GAIN_NOR_FMR_VOL is used as idle mode record volume 
#define GAIN_NOR_FMR_VOL    0    // Normal BuiltIn Mic gain 
#define GAIN_NOR_SPH_VOL    20    // IN_CALL EARPIECE Volume 
#define GAIN_NOR_SID_VOL    100    // IN_CALL EARPICE sidetone 
#define GAIN_NOR_MED_VOL    25    // reserved 
 
/* define Gain For Headset */ 
/* Headset volume: TON, SPK, MIC, FMR, SPH, SID, MED */ 
/* 
#define GAIN_HED_TON_VOL      8   // reserved 
#define GAIN_HED_KEY_VOL      24  // reserved 
#define GAIN_HED_MIC_VOL      20   // IN_CALL BuiltIn headset gain 
#define GAIN_HED_FMR_VOL      24  // reserved 
#define GAIN_HED_SPH_VOL      12  // IN_CALL Headset volume 
#define GAIN_HED_SID_VOL      100 // IN_CALL Headset sidetone 
#define GAIN_HED_MED_VOL      12   // Idle, headset Audio Buf Gain setting 
*/ 
 
#define GAIN_HED_TON_VOL    8    // reserved 
#define GAIN_HED_KEY_VOL    24    // reserved 
#define GAIN_HED_MIC_VOL    20    // IN_CALL BuiltIn headset gain 
#define GAIN_HED_FMR_VOL    24    // reserved 
#define GAIN_HED_SPH_VOL    12    // IN_CALL Headset volume 
#define GAIN_HED_SID_VOL    100    // IN_CALL Headset sidetone 
#define GAIN_HED_MED_VOL    12    // Idle, headset Audio Buf Gain setting 
 
/* define Gain For Handfree */ 
/* Handfree volume: TON, SPK, MIC, FMR, SPH, SID, MED */ 
/* GAIN_HND_TON_VOL is used as class-D Amp gain*/ 
/* 
#define GAIN_HND_TON_VOL      15  // 
#define GAIN_HND_KEY_VOL      24  // reserved 
#define GAIN_HND_MIC_VOL      20  // IN_CALL LoudSpeak Mic Gain = BuiltIn Gain 
#define GAIN_HND_FMR_VOL      24 // reserved 
#define GAIN_HND_SPH_VOL      6 // IN_CALL LoudSpeak 
#define GAIN_HND_SID_VOL      100// IN_CALL LoudSpeak sidetone 
#define GAIN_HND_MED_VOL      12 // Idle, loudSPK Audio Buf Gain setting 
*/ 
 
#define GAIN_HND_TON_VOL    8    // use for ringtone volume 
#define GAIN_HND_KEY_VOL    24    // reserved 
#define GAIN_HND_MIC_VOL    20    // IN_CALL LoudSpeak Mic Gain = BuiltIn Gain 
#define GAIN_HND_FMR_VOL    24    // reserved 
#define GAIN_HND_SPH_VOL    12    // IN_CALL LoudSpeak 
#define GAIN_HND_SID_VOL    100    // IN_CALL LoudSpeak sidetone 
#define GAIN_HND_MED_VOL    12    // Idle, loudSPK Audio Buf Gain setting 
 
    /* 0: Input FIR coefficients for 2G/3G Normal mode */
    /* 1: Input FIR coefficients for 2G/3G/VoIP Headset mode */
    /* 2: Input FIR coefficients for 2G/3G Handfree mode */
    /* 3: Input FIR coefficients for 2G/3G/VoIP BT mode */
    /* 4: Input FIR coefficients for VoIP Normal mode */
    /* 5: Input FIR coefficients for VoIP Handfree mode */
#define SPEECH_INPUT_FIR_COEFF \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0   
    /* 0: Output FIR coefficients for 2G/3G Normal mode */
    /* 1: Output FIR coefficients for 2G/3G/VoIP Headset mode */
    /* 2: Output FIR coefficients for 2G/3G Handfree mode */
    /* 3: Output FIR coefficients for 2G/3G/VoIP BT mode */
    /* 4: Output FIR coefficients for VoIP Normal mode */
    /* 5: Output FIR coefficients for VoIP Handfree mode */
#define SPEECH_OUTPUT_FIR_COEFF \
     1585,   763,   396,    37,    75,\
     -695, -1512, -2325, -2562, -2599,\
    -3692, -3338, -3935, -1208, -1987,\
      300, -3213,  2314,  1335, 10390,\
    -1858, 23197, 23197, -1858, 10390,\
     1335,  2314, -3213,   300, -1987,\
    -1208, -3935, -3338, -3692, -2599,\
    -2562, -2325, -1512,  -695,    75,\
       37,   396,   763,  1585,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0   
#define   DG_DL_Speech    0xE3D
#define   DG_Microphone    0x1400
#define   FM_Record_Vol    6     /* 0 is smallest. each step increase 1dB.
                            Be careful of distortion when increase too much. 
                            Generally, it's not suggested to tune this parameter */ 
/* 
* The Bluetooth DAI Hardware COnfiguration Parameter 
*/ 
#define   DEFAULT_BLUETOOTH_SYNC_TYPE    0
#define   DEFAULT_BLUETOOTH_SYNC_LENGTH    1
    /* 0: Input FIR coefficients for 2G/3G Normal mode */
    /* 1: Input FIR coefficients for 2G/3G/VoIP Headset mode */
    /* 2: Input FIR coefficients for 2G/3G Handfree mode */
    /* 3: Input FIR coefficients for 2G/3G/VoIP BT mode */
    /* 4: Input FIR coefficients for VoIP Normal mode */
    /* 5: Input FIR coefficients for VoIP Handfree mode */
#define WB_Speech_Input_FIR_Coeff \
      -47,   -39,     2,   -10,   -82,    17,     2,    -7,    19,    45,\
       76,    14,   151,    81,   249,    66,   106,    22,   -34,  -185,\
     -286,   -41,  -140,   -56,  -193,   122,  -165,   260,  -133,   741,\
      -59,  -296,   152,   879,   604, -2067,  1230,  -653,  1178, -5211,\
     2997, -1634,  1452, -5186, 24169, 24169, -5186,  1452, -1634,  2997,\
    -5211,  1178,  -653,  1230, -2067,   604,   879,   152,  -296,   -59,\
      741,  -133,   260,  -165,   122,  -193,   -56,  -140,   -41,  -286,\
     -185,   -34,    22,   106,    66,   249,    81,   151,    14,    76,\
       45,    19,    -7,     2,    17,   -82,   -10,     2,   -39,   -47,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0   
    /* 0: Output FIR coefficients for 2G/3G Normal mode */
    /* 1: Output FIR coefficients for 2G/3G/VoIP Headset mode */
    /* 2: Output FIR coefficients for 2G/3G Handfree mode */
    /* 3: Output FIR coefficients for 2G/3G/VoIP BT mode */
    /* 4: Output FIR coefficients for VoIP Normal mode */
    /* 5: Output FIR coefficients for VoIP Handfree mode */
#define WB_Speech_Output_FIR_Coeff \
        7,   -12,   -58,    24,   -33,    22,   -27,   -11,   -13,   -91,\
       43,   -56,   150,    -1,   111,    31,   128,    21,   -92,    15,\
     -123,   -40,  -172,   -57,  -240,   219,  -347,   446,   -79,   375,\
      157,  -421,   343,  -183,  1529,  -193,  -887, -2591,  1106,  -893,\
     3941, -5012,  5281, -4313, 20495, 20495, -4313,  5281, -5012,  3941,\
     -893,  1106, -2591,  -887,  -193,  1529,  -183,   343,  -421,   157,\
      375,   -79,   446,  -347,   219,  -240,   -57,  -172,   -40,  -123,\
       15,   -92,    21,   128,    31,   111,    -1,   150,   -56,    43,\
      -91,   -13,   -11,   -27,    22,   -33,    24,   -58,   -12,     7,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0   
#endif 
