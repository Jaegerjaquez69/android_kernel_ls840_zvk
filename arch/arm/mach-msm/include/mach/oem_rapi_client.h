/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __ASM__ARCH_OEM_RAPI_CLIENT_H
#define __ASM__ARCH_OEM_RAPI_CLIENT_H

/*
 * OEM RAPI CLIENT Driver header file
 */

#include <linux/types.h>
#include <mach/msm_rpcrouter.h>
//LG_UPDATE_S : [sinjo.mattappallil@lge.com] [06 June 2012] [Porting LS840 GB chnages]
#if defined (CONFIG_MACH_LGE_C1_BOARD_SPR)
#include "../../../../../lge/include/lg_diag_cfg.h"
#endif
//LG_UPDATE_E : [sinjo.mattappallil@lge.com] [06 June 2012] [Porting LS840 GB chnages]
enum {
	OEM_RAPI_CLIENT_EVENT_NONE = 0,

	/*
	 * list of oem rapi client events
	 */
#if defined (CONFIG_LGE_SUPPORT_RAPI)
	LG_FW_RAPI_START = 100,
	LG_FW_RAPI_CLIENT_EVENT_GET_LINE_TYPE = LG_FW_RAPI_START,
	LG_FW_TESTMODE_EVENT_FROM_ARM11 = LG_FW_RAPI_START + 1,
	LG_FW_A2M_BATT_INFO_GET = LG_FW_RAPI_START + 2,
	LG_FW_A2M_PSEUDO_BATT_INFO_SET = LG_FW_RAPI_START + 3,
	LG_FW_MEID_GET = LG_FW_RAPI_START + 4,
	LG_FW_SET_OPERATION_MODE = LG_FW_RAPI_START + 5,
	LG_FW_A2M_BLOCK_CHARGING_SET = LG_FW_RAPI_START + 6,
	LG_FW_MANUAL_TEST_MODE = LG_FW_RAPI_START + 8,
	LGE_RPC_HANDLE_REQUEST = LG_FW_RAPI_START + 9,
	LG_FW_REQUEST_SRD_RPC = LG_FW_RAPI_START + 10,
//START FOTA_LGE_BSP miracle.kim@lge.com 2011-08-09 for fota misc info
#ifdef CONFIG_LGE_FOTA_MISC_INFO
        LGE_REMOTE_RPC_REQUEST_VAL = LG_FW_RAPI_START + 11,
#endif	
//END FOTA_LGE_BSP miracle.kim@lge.com 2011-08-09 for fota misc info
	LG_FW_RAPI_ERI_DIAG_WRITE = LG_FW_RAPI_START + 11,
	LGE_REQUEST_ERI_RPC = LG_FW_RAPI_START + 12,
	OEM_RAPI_CLIENT_SET_ROOTING_NV_FROM_KERNEL = LG_FW_RAPI_START + 13,
	LG_FW_WEB_DLOAD_STATUS = LG_FW_RAPI_START + 14,
	LG_FW_SW_VERSION_GET = LG_FW_RAPI_START + 15,
	LG_FW_SUB_VERSION_GET = LG_FW_RAPI_START + 16,
	LG_FW_DID_BACKUP_REQUEST = LG_FW_RAPI_START + 17,
//LGE_CHANGE_S:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>	
// LGE_CHANGE [jaekyung83.lee@lge.com] 2011-09-27, Add ICD OEM RAPI Function [START]
#if defined (CONFIG_MACH_LGE_C1_BOARD_SPR)
	LG_FW_RAPI_ICD_DIAG_EVENT = LG_FW_RAPI_START + 20,
#endif	
//LG_UPDATE_S : [sinjo.mattappallil@lge.com] [06 June 2012] [Porting LS840 GB chnages]
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
#ifdef CONFIG_LGE_USB_ACCESS_LOCK
	LG_FW_SET_USB_LOCK_STATE =   LG_FW_RAPI_START + 25,
	LG_FW_GET_USB_LOCK_STATE =   LG_FW_RAPI_START + 26,
#endif

#ifdef CONFIG_LGE_USB_ACCESS_LOCK
	LG_FW_GET_SPC_CODE =   LG_FW_RAPI_START + 28,
#endif
// LGE_CHANGE [jaekyung83.lee@lge.com] 2011-06-01, Add LED On/Off oem rapi function [START]	
	LG_FW_LED_ON = LG_FW_RAPI_START + 35,
	LG_FW_LED_OFF = LG_FW_RAPI_START + 36,
// LGE_CHANGE [jaekyung83.lee@lge.com] 2011-06-01, Add LED On/Off oem rapi function [END]	
#endif
//LG_UPDATE_E : [sinjo.mattappallil@lge.com] [06 June 2012] [Porting LS840 GB chnages]
// LGE_CHANGE [jaekyung83.lee@lge.com] 2011-09-27, Add ICD OEM RAPI Function [END]
//LGE_CHANGE_E:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
	LG_FW_FELICA_KEY = LG_FW_RAPI_START + 90,
	LG_FW_FELICA_SIGN,
	LG_FW_ANNOYING_FLC_1ST,
	LG_FW_ANNOYING_FLC_2ND,
	LG_FW_FUEL_CAL,

	LG_MSG_UNIFIEDMSGTOOL_FROM_ARM11 = 200,
	LG_FW_GET_MANUAL_MODE = LG_MSG_UNIFIEDMSGTOOL_FROM_ARM11 + 3,
	LG_FW_PUT_MANUAL_MODE = LG_MSG_UNIFIEDMSGTOOL_FROM_ARM11 + 4,
#endif 

	OEM_RAPI_CLIENT_EVENT_MAX
};

//LG_UPDATE_S : [sinjo.mattappallil@lge.com] [06 June 2012] [Porting LS840 GB chnages]
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
//START FOTA_LGE_BSP miracle.kim@lge.com 2011-08-09 for fota misc info
#ifdef CONFIG_LGE_SUPPORT_RAPI
enum {
	LGE_CLIENT_CMD_START = 0,
#ifdef CONFIG_LGE_SYNC_CMD
	LGE_SYNC_REQUEST = 1,
#endif
	LGE_SW_VERSION_INFO = 2,
	LGE_MIN_INFO = 3,
	LGE_TESTMODE_MANUAL_TEST_INFO = 4,
	LGE_MEID_INFO = 5,
	LGE_ESN_INFO = 6,
	LGE_CLIENT_CMD_MAX = 0xF,
};
#endif
#endif
//LG_UPDATE_E : [sinjo.mattappallil@lge.com] [06 June 2012] [Porting LS840 GB chnages]

struct oem_rapi_client_streaming_func_cb_arg {
	uint32_t  event;
	void      *handle;
	uint32_t  in_len;
	char      *input;
	uint32_t out_len_valid;
	uint32_t output_valid;
	uint32_t output_size;
};

struct oem_rapi_client_streaming_func_cb_ret {
	uint32_t *out_len;
	char *output;
};

struct oem_rapi_client_streaming_func_arg {
	uint32_t event;
	int (*cb_func)(struct oem_rapi_client_streaming_func_cb_arg *,
		       struct oem_rapi_client_streaming_func_cb_ret *);
	void *handle;
	uint32_t in_len;
	char *input;
	uint32_t out_len_valid;
	uint32_t output_valid;
	uint32_t output_size;
};

struct oem_rapi_client_streaming_func_ret {
	uint32_t *out_len;
	char *output;
};

int oem_rapi_client_streaming_function(
	struct msm_rpc_client *client,
	struct oem_rapi_client_streaming_func_arg *arg,
	struct oem_rapi_client_streaming_func_ret *ret);

int oem_rapi_client_close(void);

struct msm_rpc_client *oem_rapi_client_init(void);

#endif
