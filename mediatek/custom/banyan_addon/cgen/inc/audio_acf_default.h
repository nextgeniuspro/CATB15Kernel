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
 * audio_acf_default.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 * This file is the header of audio customization related parameters or definition.
 *
 * Author:
 * -------
 * Tina Tsai
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 *
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef AUDIO_ACF_DEFAULT_H
#define AUDIO_ACF_DEFAULT_H

    /* Compensation Filter HSF coeffs: default all pass filter       */
    /* BesLoudness also uses this coeffs    */ 
    #define BES_LOUDNESS_HSF_COEFF \
    0x00000000, 0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, 0x00000000   


    /* Compensation Filter BPF coeffs: default all pass filter      */ 
    #define BES_LOUDNESS_BPF_COEFF \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
\
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
\
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
\
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000, \
    0x00000000, 0x00000000, 0x00000000   
    
    #define BES_LOUDNESS_DRC_FORGET_TABLE \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000, \
    0x00000000, 0x00000000   

    #define BES_LOUDNESS_WS_GAIN_MAX  0
           
    #define BES_LOUDNESS_WS_GAIN_MIN  0
           
    #define BES_LOUDNESS_FILTER_FIRST  0
           
    #define BES_LOUDNESS_GAIN_MAP_IN \
    0, 0, 0, 0, 0     
              
    #define BES_LOUDNESS_GAIN_MAP_OUT \            
    0, 0, 0, 0, 0               

#endif
