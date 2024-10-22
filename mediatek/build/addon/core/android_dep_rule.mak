# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.

ifneq ($(CUSTOM_MODEM),)
MTK_MODEM_PATH = mediatek/custom/common/modem/$(CUSTOM_MODEM)
MTK_MODEM_FILE = $(if $(wildcard $(MTK_MODEM_PATH)/modem_E2.mak),$(MTK_MODEM_PATH)/modem_E2.mak,$(MTK_MODEM_PATH)/modem.mak)
$(shell cat $(MTK_MODEM_FILE) | sed -e '/^[ \t]*#/'d | awk -F# '{print $1}' | sed -n '/^\S\+[ \t]*=[ \t]*\S\+/'p | \
      sed -e 's/^/MODEM_/' > $(MTK_MODEM_PATH)/modem_feature.mak)
include $(MTK_MODEM_PATH)/modem_feature.mak
endif

#######################################################
# dependency check between AP side & modem side


ifeq (yes,$(strip $(MTK_TTY_SUPPORT)))
   ifeq (FALSE, $(strip $(MODEM_CTM_SUPPORT)))
    $(call dep-err-seta-or-setb,MTK_TTY_SUPPORT,no,CTM_SUPPORT,true)
   endif
endif

ifeq (yes,$(strip $(MTK_VT3G324M_SUPPORT)))
   ifeq (FALSE,$(MODEM_SP_VIDEO_CALL_SUPPORT))
    $(call dep-err-ona-or-offb,SP_VIDEO_CALL_SUPPORT,MTK_VT3G324M_SUPPORT)
   endif
endif

ifeq (yes,$(strip $(MTK_VT3G324M_SUPPORT)))
  ifeq ($(findstring modem_3g,$(MTK_MODEM_SUPPORT)),)
     $(call dep-err-seta-or-offb,MTK_MODEM_SUPPORT,modem_3g_tdd/modem_3g_fdd,MTK_VT3G324M_SUPPORT)
  endif
endif

ifneq ($(findstring modem_3g,$(MTK_MODEM_SUPPORT)),)
  ifeq ($(findstring hspa,$(CUSTOM_MODEM)),)
    $(call dep-err-seta-or-setb,CUSTOM_MODEM,xxx_hspa_xxx,MTK_MODEM_SUPPORT,none 3g)
  endif
endif

# temp fpr MT6573
ifeq (MT6573,$(strip $(MTK_PLATFORM)))
  ifeq (UMTS_FDD_MODE_SUPPORT,$(strip $(MODEM_UMTS_MODE_SUPPORT)))
    ifneq (modem_3g_fdd,$(strip $(MTK_MODEM_SUPPORT)))
      $(call dep-err-common,please set MTK_MODEM_SUPPORT=modem_3g_fdd when UMTS_MODE_SUPPORT=$(UMTS_MODE_SUPPORT))
    endif
  endif

  ifeq (UMTS_TDD128_MODE_SUPPORT,$(strip $(MODEM_UMTS_MODE_SUPPORT)))
    ifneq (modem_3g_tdd,$(strip $(MTK_MODEM_SUPPORT)))
      $(call dep-err-common,please set MTK_MODEM_SUPPORT=modem_3g_tdd when UMTS_MODE_SUPPORT=$(UMTS_MODE_SUPPORT))
    endif
  endif
endif

# for dual mic 
ifeq (yes, $(strip $(MTK_DUAL_MIC_SUPPORT)))
  ifneq (TRUE,$(strip $(MODEM_DUAL_MIC_SUPPORT)))
    $(call dep-err-ona-or-offb, DUAL_MIC_SUPPORT, MTK_DUAL_MIC_SUPPORT)
  endif
endif

################################################################
# for LCA feature

ifeq (yes, $(strip $(MTK_LCA_SUPPORT)))
  ifneq (no,$(strip $(MTK_SNS_SUPPORT)))
    $(call dep-err-offa-or-offb,MTK_SNS_SUPPORT,MTK_LCA_SUPPORT)
  endif
endif

ifeq (yes, $(strip $(MTK_LCA_SUPPORT)))
  ifneq (no,$(strip $(MTK_SMS_NATIONAL_LANGUAGE_SUPPORT)))
    $(call dep-err-offa-or-offb,MTK_SMS_NATIONAL_LANGUAGE_SUPPORT,MTK_LCA_SUPPORT)
  endif
endif

ifeq (yes, $(strip $(MTK_LCA_SUPPORT)))
  ifneq (no,$(strip $(MTK_CAMERA_APP_3DHW_SUPPORT)))
    $(call dep-err-offa-or-offb,MTK_CAMERA_APP_3DHW_SUPPORT,MTK_LCA_SUPPORT)
  endif
endif

ifeq (yes, $(strip $(MTK_LCA_SUPPORT)))
  ifneq (no,$(strip $(MTK_LIVEWALLPAPER_APP)))
    $(call dep-err-offa-or-offb,MTK_LIVEWALLPAPER_APP,MTK_LCA_SUPPORT)
  endif
endif

############################################################
# for wapi feature

ifeq (yes,$(strip $(MTK_WAPI_SUPPORT)))
  ifneq (yes, $(strip $(MTK_WLAN_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_WLAN_SUPPORT, MTK_WAPI_SUPPORT)
  endif
endif

############################################################
# for wifi_hotspot feature

ifeq (yes,$(strip $(MTK_WIFI_HOTSPOT_SUPPORT)))
  ifneq (yes, $(strip $(MTK_WLAN_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_WLAN_SUPPORT, MTK_WIFI_HOTSPOT_SUPPORT)
  endif
endif

##############################################################
# for camera feature

ifeq (no,$(strip $(MTK_CAMERA_APP)))
  ifeq (yes,$(strip $(MTK_CAMERA_VIDEO_ZOOM)))
     $(call dep-err-ona-or-offb, MTK_CAMERA_APP, MTK_CAMERA_VIDEO_ZOOM)
  endif
endif

ifeq (yes,$(strip $(MTK_CAMERA_APP)))
  ifeq (,$(strip $(CUSTOM_HAL_CAMERA)))
     $(call dep-err-seta-or-offb, CUSTOM_HAL_CAMERA,camera, MTK_CAMERA_APP)
  endif
endif

##############################################################
# for matv feature

ifeq (yes,$(strip $(HAVE_MATV_FEATURE)))
  ifeq (,$(strip $(CUSTOM_HAL_MATV)))
     $(call dep-err-common, PLEASE turn off HAVE_MATV_FEATURE or set CUSTOM_HAL_MATV)
  endif
endif

ifeq (yes,$(strip $(HAVE_MATV_FEATURE)))
  ifeq (,$(strip $(CUSTOM_KERNEL_MATV)))
     $(call dep-err-common, PLEASE turn off HAVE_MATV_FEATURE or set CUSTOM_KERNEL_MATV)
  endif
endif


##############################################################
# for fm feature

ifeq (yes,$(strip $(MTK_MT519X_FM_SUPPORT)))
  ifeq (no,$(strip $(HAVE_MATV_FEATURE)))
     $(call dep-err-ona-or-offb, HAVE_MATV_FEATURE, MTK_MT519X_FM_SUPPORT)
  endif
endif

##############################################################
# for wcdma feature
ifeq (yes,$(strip $(MTK_WCDMA_SUPPORT)))
   ifeq (2, $(words,$(subst gprs, ,$(CUSTOM_MODEM))))
    $(call dep-err-seta-or-setb,MTK_WCDMA_SUPPORT,no,CUSTOM_MODEM,$(subst gprs,hspa,$(CUSTOM_MODEM)))
   endif
endif

##############################################################
# for gps feature

ifeq (yes,$(strip $(MTK_AGPS_APP)))
  ifeq (no,$(strip $(MTK_GPS_SUPPORT)))
     $(call dep-err-ona-or-offb, MTK_GPS_SUPPORT, MTK_AGPS_APP)
  endif
endif

##############################################################
# for BT feature

ifeq (yes,$(strip $(MTK_WLANBT_SINGLEANT)))
  ifneq (yes,$(strip $(MTK_BT_SUPPORT)))
     $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_WLANBT_SINGLEANT)
  endif
endif

ifeq (yes,$(strip $(MTK_WLANBT_SINGLEANT)))
  ifneq (yes,$(strip $(MTK_WLAN_SUPPORT)))
     $(call dep-err-ona-or-offb, MTK_WLAN_SUPPORT, MTK_WLANBT_SINGLEANT)
  endif
endif

##############################################################
# for GEMINI feature
ifeq (yes, $(strip $(MTK_GEMINI_3G_SWITCH)))
  ifneq (yes, $(strip $(GEMINI)))
    $(call dep-err-ona-or-offb, GEMINI, MTK_GEMINI_3G_SWITCH)
  endif
endif

ifeq (yes, $(strip $(MTK_GEMINI_ENHANCEMENT)))
  ifneq (yes, $(strip $(GEMINI)))
    $(call dep-err-ona-or-offb, GEMINI, MTK_GEMINI_ENHANCEMENT)
  endif
endif

##############################################################
# for DSPIRDBG feature
ifeq (yes, $(strip $(MTK_DSPIRDBG)))
  ifneq (yes, $(strip $(MTK_MDLOGGER_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_MDLOGGER_SUPPORT, MTK_DSPIRDBG)
  endif
endif

##############################################################
# for Log2Server feature

ifeq (no, $(strip $(HAVE_AEE_FEATURE)))
  ifeq (yes, $(strip $(MTK_LOG2SERVER_APP)))
    $(call dep-err-ona-or-offb, HAVE_AEE_FEATURE, MTK_LOG2SERVER_APP)
  endif
endif

##############################################################
# for MTK_FOTA_SUPPORT feature

ifeq (no, $(strip $(MTK_DM_APP)))
  ifeq (yes, $(strip $(MTK_FOTA_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_DM_APP, MTK_FOTA_SUPPORT)
  endif
endif

ifeq (no, $(strip $(MTK_DM_APP)))
  ifeq (yes, $(strip $(MTK_SCOMO_ENTRY)))
    $(call dep-err-ona-or-offb, MTK_DM_APP, MTK_SCOMO_ENTRY)
  endif
endif

ifeq (no, $(strip $(MTK_FOTA_SUPPORT)))
  ifeq (yes, $(strip $(MTK_FOTA_ENTRY)))
    $(call dep-err-ona-or-offb, MTK_FOTA_SUPPORT, MTK_FOTA_ENTRY)
  endif
endif

ifeq (no, $(strip $(MTK_SMSREG_APP)))
  ifeq (yes, $(strip $(MTK_FOTA_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_SMSREG_APP, MTK_FOTA_SUPPORT)
  endif
endif

ifeq (no, $(strip $(MTK_SMSREG_APP)))
  ifeq (yes, $(strip $(MTK_DM_APP)))
    $(call dep-err-ona-or-offb, MTK_SMSREG_APP, MTK_DM_APP)
  endif
endif

ifeq (yes, $(strip $(MTK_DM_APP)))
  ifeq (yes, $(strip $(MTK_MDM_APP)))
    $(call dep-err-offa-or-offb, MTK_DM_APP, MTK_MDM_APP)
  endif
  ifeq (yes, $(strip $(MTK_RSDM_APP)))
    $(call dep-err-offa-or-offb, MTK_DM_APP, MTK_RSDM_APP)
  endif
endif

ifeq (yes, $(strip $(MTK_RSDM_APP)))
  ifeq (yes, $(strip $(MTK_MDM_APP)))
    $(call dep-err-offa-or-offb, MTK_RSDM_APP, MTK_MDM_APP)
  endif
endif

##############################################################
# for IME

ifeq (yes,$(MTK_INTERNAL))
  ifeq (no,$(MTK_INTERNAL_LANG_SET))
     $(call dep-err-ona-or-offb, MTK_INTERNAL_LANG_SET, MTK_INTERNAL)
  endif
endif

ifeq (yes,$(MTK_INTERNAL_LANG_SET))
  ifeq (no,$(MTK_INTERNAL))
     $(call dep-err-ona-or-offb, MTK_INTERNAL, MTK_INTERNAL_LANG_SET)
  endif
endif


ifneq (yes, $(strip $(MTK_IME_SUPPORT)))
  ifeq (yes, $(strip $(MTK_IME_FRENCH_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_FRENCH_SUPPORT)
  endif
  
  ifeq (yes, $(strip $(MTK_IME_RUSSIAN_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_RUSSIAN_SUPPORT)
  endif

  ifeq (yes, $(strip $(MTK_IME_GERMAN_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_GERMAN_SUPPORT)
  endif

  ifeq (yes, $(strip $(MTK_IME_SPANISH_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_SPANISH_SUPPORT)
  endif

  ifeq (yes, $(strip $(MTK_IME_ITALIAN_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_ITALIAN_SUPPORT)
  endif

  ifeq (yes, $(strip $(MTK_IME_PORTUGUESE_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_PORTUGUESE_SUPPORT)
  endif

  ifeq (yes, $(strip $(MTK_IME_TURKISH_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_TURKISH_SUPPORT)
  endif

  ifeq (yes, $(strip $(MTK_IME_MALAY_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_MALAY_SUPPORT)
  endif

  ifeq (yes, $(strip $(MTK_IME_HINDI_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_HINDI_SUPPORT)
  endif

  ifeq (yes, $(strip $(MTK_IME_ARABIC_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_ARABIC_SUPPORT)
  endif

  ifeq (yes, $(strip $(MTK_IME_THAI_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_THAI_SUPPORT)
  endif

  ifeq (yes, $(strip $(MTK_IME_VIETNAM_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_VIETNAM_SUPPORT)
  endif

  ifeq (yes, $(strip $(MTK_IME_INDONESIAN_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_IME_SUPPORT, MTK_IME_INDONESIAN_SUPPORT)
  endif
endif

##############################################################
# for MtkWeatherProvider

ifeq (yes,$(MTK_WEATHER_WIDGET_APP))
  ifeq (no,$(MTK_WEATHER_PROVIDER_APP))
    $(call dep-err-ona-or-offb, MTK_WEATHER_PROVIDER_APP, MTK_WEATHER_WIDGET_APP)
  endif
endif

##############################################################
# for FD feature

ifeq (yes,$(MTK_FD_SUPPORT))
  ifeq ($(findstring modem_3g,$(MTK_MODEM_SUPPORT)),)
     $(call dep-err-seta-or-setb, MTK_FD_SUPPORT,no,MTK_MODEM_SUPPORT,modem_3g_tdd/modem_3g_fdd)
  endif
endif

ifeq (no,$(strip $(MTK_FD_SUPPORT)))
  ifeq (yes,$(strip $(MTK_FD_FORCE_REL_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_FD_SUPPORT, MTK_FD_FORCE_REL_SUPPORT)
  endif
endif

##############################################################
# for SNS feature

ifeq (yes,$(MTK_SNS_SINAWEIBO_APP))
  ifeq (no,$(MTK_SNS_SUPPORT))
     $(call dep-err-ona-or-offb, MTK_SNS_SUPPORT,MTK_SNS_SINAWEIBO_APP)
  endif
endif

##############################################################
# for VT voice answer feature

ifeq (OP01_SPEC0200_SEGC,$(OPTR_SPEC_SEG_DEF))
  ifneq (yes,$(MTK_PHONE_VT_VOICE_ANSWER))
     $(call dep-err-seta-or-onb, OPTR_SPEC_SEG_DEF,non OP01_SPEC0200_SEGC,MTK_PHONE_VT_VOICE_ANSWER)
  endif
endif

ifeq (yes,$(MTK_PHONE_VT_VOICE_ANSWER))
  ifneq (OP01_SPEC0200_SEGC,$(OPTR_SPEC_SEG_DEF))
     $(call dep-err-seta-or-offb, OPTR_SPEC_SEG_DEF,OP01_SPEC0200_SEGC,MTK_PHONE_VT_VOICE_ANSWER)
  endif
endif

##############################################################
# for VT multimedia ringtone

ifeq (OP01_SPEC0200_SEGC,$(OPTR_SPEC_SEG_DEF))
  ifneq (yes,$(MTK_PHONE_VT_MM_RINGTONE))
     $(call dep-err-seta-or-onb, OPTR_SPEC_SEG_DEF,non OP01_SPEC0200_SEGC,MTK_PHONE_VT_MM_RINGTONE)
  endif
endif

ifeq (yes,$(MTK_PHONE_VT_MM_RINGTONE))
  ifneq (OP01_SPEC0200_SEGC,$(OPTR_SPEC_SEG_DEF))
     $(call dep-err-seta-or-offb, OPTR_SPEC_SEG_DEF,OP01_SPEC0200_SEGC,MTK_PHONE_VT_MM_RINGTONE)
  endif
endif

##############################################################
# for BT 

ifeq (no,$(strip $(MTK_BT_SUPPORT)))
  ifeq (yes,$(strip $(MTK_BT_21_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_21_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_BT_30_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_30_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_BT_30_HS_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_30_HS_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_BT_40_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_40_SUPPORT)
  endif
  ifeq (yes,$(strip $(MTK_BT_FM_OVER_BT_VIA_CONTROLLER)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_FM_OVER_BT_VIA_CONTROLLER)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_A2DP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_A2DP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_AVRCP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_AVRCP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_AVRCP14)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_AVRCP14)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_BIP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_BIP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_BPP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_BPP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_DUN)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_DUN)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_FTP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_FTP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_HFP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_HFP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_HIDH)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_HIDH)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_MANAGER)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_MANAGER)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_MAPC)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_MAPC)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_MAPS)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_MAPS)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_OPP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_OPP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PAN)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_PAN)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PBAP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_PBAP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PRXM)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_PRXM)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PRXR)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_PRXR)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_SIMAP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_SIMAP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_SPP)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_SPP)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_TIMEC)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_TIMEC)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_TIMES)))
    $(call dep-err-ona-or-offb, MTK_BT_SUPPORT, MTK_BT_PROFILE_TIMES)
  endif
endif

ifeq (no,$(strip $(MTK_BT_40_SUPPORT)))
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PRXM)))
    $(call dep-err-ona-or-offb, MTK_BT_40_SUPPORT, MTK_BT_PROFILE_PRXM)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_PRXR)))
    $(call dep-err-ona-or-offb, MTK_BT_40_SUPPORT, MTK_BT_PROFILE_PRXR)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_TIMEC)))
    $(call dep-err-ona-or-offb, MTK_BT_40_SUPPORT, MTK_BT_PROFILE_TIMEC)
  endif
  ifeq (yes,$(strip $(MTK_BT_PROFILE_TIMES)))
    $(call dep-err-ona-or-offb, MTK_BT_40_SUPPORT, MTK_BT_PROFILE_TIMES)
  endif
endif

ifeq (yes,$(strip $(MTK_BT_FM_OVER_BT_VIA_CONTROLLER)))
  ifeq (no,$(strip $(MTK_BT_PROFILE_A2DP)))
    $(call dep-err-ona-or-offb, MTK_BT_PROFILE_A2DP, MTK_BT_FM_OVER_BT_VIA_CONTROLLER)
  endif
endif

##############################################################
# for emmc feature
ifeq (no,$(strip $(MTK_EMMC_SUPPORT)))
  ifneq (,$(strip $(EMMC_CHIP)))
    $(call dep-err-common, PLEASE set EMMC_CHIP as NULL when MTK_EMMC_SUPPORT=no)
  endif
endif

##############################################################
# for emmc otp
ifeq (yes,$(strip $(MTK_EMMC_SUPPORT_OTP)))
  ifeq (no,$(strip $(MTK_EMMC_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_EMMC_SUPPORT, MTK_EMMC_SUPPORT_OTP)
  endif
endif

ifeq (yes,$(strip $(MTK_EMMC_SUPPORT_OTP)))
  ifeq (FALSE,$(strip $(MODEM_OTP_SUPPORT)))
    $(call dep-err-ona-or-offb, OTP_SUPPORT, MTK_EMMC_SUPPORT_OTP)
  endif
endif

##############################################################
# for NFC feature
ifeq (yes,$(strip $(MTK_NFC_SUPPORT)))
  ifeq (no,$(strip $(CONFIG_NFC_PN544)))
    $(call dep-err-ona-or-offb, CONFIG_NFC_PN544, MTK_NFC_SUPPORT)
  endif
endif

ifeq (no,$(strip $(MTK_NFC_SUPPORT)))
  ifeq (yes,$(strip $(CONFIG_NFC_PN544)))
    $(call dep-err-ona-or-offb, MTK_NFC_SUPPORT, CONFIG_NFC_PN544)
  endif
endif

##############################################################
# for fm feature
ifeq (no,$(strip $(MTK_FM_SUPPORT)))
  ifeq (yes,$(strip $(MTK_FM_RECORDING_SUPPORT)))
    $(call dep-err-ona-or-offb, MTK_FM_SUPPORT, MTK_FM_RECORDING_SUPPORT)
  endif
endif

##############################################################
# for launcher2 feature
ifeq (2G,$(strip $(CUSTOM_DRAM_SIZE)))
  ifneq (no,$(strip $(MTK_LAUNCHER_ALLAPPSGRID)))
     $(call dep-err-seta-or-offb, CUSTOM_DRAM_SIZE,non $(CUSTOM_DRAM_SIZE), MTK_LAUNCHER_ALLAPPSGRID)
  endif
endif


##############################################################
# for Brazil customization
ifeq (no,$(strip $(MTK_BRAZIL_CUSTOMIZATION)))
  ifeq (yes,$(strip $(MTK_BRAZIL_CUSTOMIZATION_TIM)))
    $(call dep-err-ona-or-offb, MTK_BRAZIL_CUSTOMIZATION, MTK_BRAZIL_CUSTOMIZATION_TIM)
  endif
endif

##############################################################
# for mtk brazil customization feature

ifeq (no,$(strip $(MTK_BRAZIL_CUSTOMIZATION)))
  ifeq (yes,$(strip $(MTK_BRAZIL_CUSTOMIZATION_VIVO)))
    $(call dep-err-common, please set MTK_BRAZIL_CUSTOMIZATION_VIVO=no when MTK_BRAZIL_CUSTOMIZATION=no)
  endif
  ifeq (yes,$(strip $(MTK_BRAZIL_CUSTOMIZATION_CLARO)))
    $(call dep-err-common, please set MTK_BRAZIL_CUSTOMIZATION_CLARO=no when MTK_BRAZIL_CUSTOMIZATION=no)
  endif
endif

##############################################################
# for 6575 display quality enhancement
ifeq (,$(filter MT6575 MT6577, $(MTK_PLATFORM)))
  ifdef MTK_75DISPLAY_ENHANCEMENT_SUPPORT
    $(call dep-err-common, MTK_75DISPLAY_ENHANCEMENT_SUPPORT could only be defined when MTK_PLATFORM = MT6575/MT6577)
  endif
endif

##############################################################
# for TDD

ifeq (yes,$(MODEM_UMTS_TDD128_MODE))
  ifneq (modem_3g,$(MTK_MODEM_SUPPORT))
     $(call dep-err-seta-or-setb, MODEM_UMTS_TDD128_MODE,no,MTK_MODEM_SUPPORT,modem_3g)
  endif
endif

##############################################################
# for HAVE_CMMB_FEATURE and CUSTOM_KERNEL_CMMB

ifeq (yes,$(HAVE_CMMB_FEATURE))
  ifneq (cmmb,$(CUSTOM_KERNEL_CMMB))
    $(call dep-err-seta-or-offb,CUSTOM_KERNEL_CMMB,cmmb,HAVE_CMMB_FEATURE)
  endif
endif

ifeq (no,$(HAVE_CMMB_FEATURE))
  ifdef CUSTOM_KERNEL_CMMB
    $(call dep-err-common,CUSTOM_KERNEL_CMMB could only be defined when HAVE_CMMB_FEATURE = yes)
  endif
endif

##############################################################
# for MD DB
ifeq (no,$(strip $(MTK_MDLOGGER_SUPPORT)))
  ifeq (yes,$(strip $(MTK_INCLUDE_MODEM_DB_IN_IMAGE)))
     $(call dep-err-ona-or-offb, MTK_MDLOGGER_SUPPORT, MTK_INCLUDE_MODEM_DB_IN_IMAGE)
  endif
endif

##############################################################
# for phone number geo-description
ifneq ($(filter OP01% OP02%, $(OPTR_SPEC_SEG_DEF)),)
  ifeq (no,$(strip $(MTK_PHONE_NUMBER_GEODESCRIPTION)))
    $(call dep-err-seta-or-onb, OPTR_SPEC_SEG_DEF,none OP01/OP02,MTK_PHONE_NUMBER_GEODESCRIPTION)
  endif
endif

#############################################################
# MTK_MDM_APP, MTK_DM_APP and MTK_RSDM_APP are exclusive. (Only one can be enabled at the same time.)
ifeq ($(strip $(MTK_MDM_APP)),yes)
  ifeq ($(strip $(MTK_DM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_MDM_APP, MTK_DM_APP)
  endif
  ifeq ($(strip $(MTK_RSDM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_MDM_APP, MTK_RSDM_APP)
  endif
endif

ifeq ($(strip $(MTK_DM_APP)),yes)
  ifeq ($(strip $(MTK_MDM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_DM_APP, MTK_MDM_APP)
  endif
  ifeq ($(strip $(MTK_RSDM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_DM_APP, MTK_RSDM_APP)
  endif
endif

ifeq ($(strip $(MTK_RSDM_APP)),yes)
  ifeq ($(strip $(MTK_MDM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_RSDM_APP, MTK_MDM_APP)
  endif
  ifeq ($(strip $(MTK_DM_APP)),yes)
     $(call dep-err-offa-or-offb, MTK_RSDM_APP, MTK_DM_APP)
  endif
endif

#############################################################
# for MTK_APKINSTALLER_APP

ifeq (OP02_SPEC0200_SEGC,$(strip $(OPTR_SPEC_SEG_DEF)))
  ifeq ($(strip $(MTK_APKINSTALLER_APP)),no)
     $(call dep-err-seta-or-onb, OPTR_SPEC_SEG_DEF,non OP02_SPEC0200_SEGC,MTK_APKINSTALLER_APP)  
  endif
endif

#############################################################

ifneq (yes,$(strip $(MTK_TABLET_PLATFORM)))
  ifeq (240,$(strip $(LCM_WIDTH)))
    ifeq (320,$(strip $(LCM_HEIGHT)))
      ifeq ($(filter ldpi,$(MTK_PRODUCT_LOCALES)),)
        $(call dep-err-common, Please add ldpi to MTK_PRODUCT_LOCALES or set different LCM_WIDTH and LCM_HEIGHT)
      endif       
    endif
  endif
  ifeq (320,$(strip $(LCM_WIDTH)))
    ifeq (480,$(strip $(LCM_HEIGHT)))
      ifeq ($(filter mdpi,$(MTK_PRODUCT_LOCALES)),)
        $(call dep-err-common, Please add mdpi to MTK_PRODUCT_LOCALES or set different LCM_WIDTH and LCM_HEIGHT)
      endif       
    endif
  endif
  ifeq (480,$(strip $(LCM_WIDTH)))
    ifeq (800,$(strip $(LCM_HEIGHT)))
      ifeq ($(filter hdpi,$(MTK_PRODUCT_LOCALES)),)
        $(call dep-err-common, Please add hdpi to MTK_PRODUCT_LOCALES or set different LCM_WIDTH and LCM_HEIGHT)
      endif       
    endif
  endif
  ifeq (540,$(strip $(LCM_WIDTH)))
    ifeq (960,$(strip $(LCM_HEIGHT)))
      ifeq ($(filter hdpi,$(MTK_PRODUCT_LOCALES)),)
        $(call dep-err-common, Please add hdpi to MTK_PRODUCT_LOCALES or set different LCM_WIDTH and LCM_HEIGHT)
      endif       
    endif
  endif
  ifeq (720,$(strip $(LCM_WIDTH)))
    ifeq (1280,$(strip $(LCM_HEIGHT)))
      ifeq ($(filter hdpi,$(MTK_PRODUCT_LOCALES)),)
        $(call dep-err-common, Please add hdpi to MTK_PRODUCT_LOCALES or set different LCM_WIDTH and LCM_HEIGHT)
      endif  
      ifeq ($(filter xhdpi,$(MTK_PRODUCT_LOCALES)),)
        $(call dep-err-common, Please add xhdpi to MTK_PRODUCT_LOCALES or set different LCM_WIDTH and LCM_HEIGHT)
      endif 
    endif
  endif
endif

############################################################
ifeq (yes,$(MTK_WEATHER3D_WIDGET))
  ifeq (no,$(MTK_WEATHER_PROVIDER_APP))
        $(call dep-err-ona-or-offb, MTK_WEATHER_PROVIDER_APP, MTK_WEATHER3D_WIDGET)
  endif
endif

############################################################
ifeq ($(strip $(MTK_COMBO_CHIP)),MT6628)
  ifneq ($(strip $(MTK_BT_FM_OVER_BT_VIA_CONTROLLER)),no)
    $(call dep-err-seta-or-setb,MTK_BT_FM_OVER_BT_VIA_CONTROLLER,no,MTK_COMBO_CHIP,none MT6628)
  endif
endif
ifeq ($(strip $(MTK_BT_CHIP)),MTK_MT6628)
  ifneq ($(strip $(MTK_COMBO_CHIP)),MT6628)
     $(call dep-err-seta-or-setb,MTK_BT_CHIP, none MTK_MT6628,MTK_COMBO_CHIP,MT6628)
  endif
endif
ifeq ($(strip $(MTK_FM_CHIP)),MT6628_FM)  
  ifneq ($(strip $(MTK_COMBO_CHIP)),MT6628)
    $(call dep-err-seta-or-setb,MTK_FM_CHIP, none MT6628_FM,MTK_COMBO_CHIP,MT6628)
  endif
endif
ifeq ($(strip $(MTK_WLAN_CHIP)),MT6628)
  ifneq ($(strip $(MTK_COMBO_CHIP)),MT6628)
    $(call dep-err-seta-or-setb,MTK_WLAN_CHIP, none MT6628,MTK_COMBO_CHIP,MT6628)
  endif
endif
ifeq ($(strip $(MTK_GPS_CHIP)),MTK_GPS_MT6628)
  ifneq ($(strip $(MTK_COMBO_CHIP)),MT6628)
    $(call dep-err-seta-or-setb,MTK_GPS_CHIP, none MTK_GPS_MT6628,MTK_COMBO_CHIP,MT6628)
  endif
endif

############################################################
ifeq ($(strip $(MTK_AP_SPEECH_ENHANCEMENT)),yes)
  ifneq ($(strip $(MTK_AUDIO_HD_REC_SUPPORT)),yes)
    $(call dep-err-ona-or-offb, MTK_AUDIO_HD_REC_SUPPORT, MTK_AP_SPEECH_ENHANCEMENT)
  endif
endif
############################################################
ifneq ($(strip $(MTK_LOG2SERVER_APP)),yes)
  ifeq ($(strip $(MTK_LOG2SERVER_INTERNAL)),yes)
    $(call dep-err-ona-or-offb, MTK_LOG2SERVER_APP, MTK_LOG2SERVER_INTERNAL)
  endif
endif
############################################################
ifeq ($(strip $(MTK_MEDIA3D_APP)),yes)
  ifneq ($(strip $(MTK_TABLET_PLATFORM)),yes)
    ifeq ($(filter xhdpi hdpi ,$(MTK_PRODUCT_LOCALES)),)
      $(call dep-err-common,MTK_MEDIA3D_APP can set to yes only if MTK_TABLET_PLATFORM is yes or MTK_PRODUCT_LOCALES contains xhdpi hdpi) 
    endif
  endif
  ifeq ($(filter xhdpi hdpi ,$(MTK_PRODUCT_LOCALES)),)
    ifneq ($(strip $(MTK_TABLET_PLATFORM)),yes)
      $(call dep-err-common,MTK_MEDIA3D_APP can set to yes only if MTK_TABLET_PLATFORM is yes or MTK_PRODUCT_LOCALES contains xhdpi hdpi) 
    endif
  endif
endif
############################################################
ifeq ($(strip $(MTK_AP_SPEECH_ENHANCEMENT)),yes)
  ifneq ($(strip $(MTK_AUDIO_HD_REC_SUPPORT)),yes)
     $(call dep-err-ona-or-offb, MTK_AUDIO_HD_REC_SUPPORT, MTK_AP_SPEECH_ENHANCEMENT)
  endif
endif

############################################################
ifeq ($(strip $(MTK_HDMI_SUPPORT)),yes)
  ifeq ($(strip $(CUSTOM_KERNEL_HDMI)),)
    $(call dep-err-common, CUSTOM_KERNEL_HDMI should not be NULL when MTK_HDMI_SUPPORT=yes)
  endif
endif
############################################################
ifneq ($(strip $(OPTR_SPEC_SEG_DEF)), NONE)
  ifneq ($(strip $(MTK_NETWORK_TYPE_ALWAYS_ON)), no)
     $(call dep-err-common, Please set OPTR_SPEC_SEG_DEF as NONE or set MTK_NETWORK_TYPE_ALWAYS_ON as no)
  endif
endif
############################################################
ifeq ($(strip $(MTK_S3D_SUPPORT)),yes)
  ifneq ($(strip $(MTK_STEREO3D_WALLPAPER_APP)),yes)
    $(call dep-err-ona-or-offb, MTK_STEREO3D_WALLPAPER_APP, MTK_S3D_SUPPORT)
  endif
endif
ifneq ($(strip $(MTK_S3D_SUPPORT)),yes)
  ifeq ($(strip $(MTK_STEREO3D_WALLPAPER_APP)),yes)
    $(call dep-err-ona-or-offb, MTK_S3D_SUPPORT, MTK_STEREO3D_WALLPAPER_APP)
  endif
endif
############################################################
ifeq ($(strip $(MTK_S3D_SUPPORT)),yes)
  ifneq ($(strip $(MTK_3DWORLD_APP)),yes)
    $(call dep-err-ona-or-offb, MTK_3DWORLD_APP, MTK_S3D_SUPPORT)
  endif
endif
ifneq ($(strip $(MTK_S3D_SUPPORT)),yes)
  ifeq ($(strip $(MTK_3DWORLD_APP)),yes)
    $(call dep-err-ona-or-offb, MTK_S3D_SUPPORT, MTK_3DWORLD_APP)
  endif
endif
