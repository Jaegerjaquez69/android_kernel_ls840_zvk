/* arch/arm/mach-msm/lge/lg_fw_diag_icd.c
 *
 * Copyright (C) 2009,2010 LGE, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <lg_diagcmd.h>
#include <lge_diag_icd.h>
#include <lge_base64.h>
#include <lg_fw_diag_communication.h>

#include <linux/unistd.h>	/*for open/close */
#include <linux/fcntl.h>	/*for O_RDWR */

#include <linux/fb.h>		/* to handle framebuffer ioctls */
#include <linux/ioctl.h>
#include <linux/uaccess.h>

#include <linux/syscalls.h>	//for sys operations

#include <linux/input.h>	// for input_event
#include <linux/fs.h>		// for file struct
#include <linux/types.h>	// for ssize_t
#include <linux/input.h>	// for event parameters
#include <linux/jiffies.h>
#include <linux/delay.h>

#include <lg_diag_screen_shot.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/leds-pmic8058.h>
#include <linux/timer.h>
#include <linux/msm-charger.h>
#include <lg_diag_testmode.h>
#include <linux/syscalls.h>
#include <linux/input.h>

#include <linux/jiffies.h>
/* ==========================================================================
===========================================================================*/
/*  jihye.ahn   2010-10-01    convert RGBA8888 to RGB565 */
#define LCD_BUFFER_SIZE ICD_LCD_MAIN_WIDTH * ICD_LCD_MAIN_HEIGHT * 4
#define CONVERT565(r, g, b) ( ((r >> 3) << 11) | (g >> 2) << 5 | (b >> 3) )
/* ==========================================================================
===========================================================================*/
extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);
extern PACK(void *) diagpkt_alloc2 (diagpkt_cmd_code_type code, unsigned int length, unsigned int packet_length);
/*==========================================================================*/
/*==========================================================================*/

/*
 * EXTERNAL FUNCTION AND VARIABLE DEFINITIONS
 */
extern PACK(void *) diagpkt_free(PACK(void *)pkt);

extern int icd_send_to_arm9(void* pReq, void* pRsp, unsigned int output_length);

extern icd_user_table_entry_type icd_mstr_tbl[ICD_MSTR_TBL_SIZE];

unsigned char g_diag_slate_capture_rsp_num = 0;
unsigned char g_diag_latitude_longitude = 0;
unsigned char g_slate_status = 0x0;

extern int lge_bd_rev;
extern KERNEL_TIMER_MANAGER * ptiming;
void* icd_app_handler(DIAG_ICD_F_req_type*pReq);
/*
 * LOCAL DEFINITIONS AND DECLARATIONS FOR MODULE
 *
 * This section contains local definitions for constants, macros, types,
 * variables and other items needed by this module.
 */
static struct diagcmd_dev *diagpdev;
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET KEYBACKLIGHT SETTINGS_10JUL_sandeep.pawar@lge.com*/
//static int saveKeyLedSetValue=0;

char process_status[10];
EXPORT_SYMBOL(process_status);

char process_value[100];
EXPORT_SYMBOL(process_value);

int key_touch_logging_status = 0;
int key_logging_count = 0;
EXPORT_SYMBOL(key_touch_logging_status);
EXPORT_SYMBOL(key_logging_count);
EXPORT_SYMBOL(g_slate_status);
extern uint16_t max_x;
extern uint16_t max_y;
extern char mtc_running;

static void large_mdelay(unsigned int msec)
{
	unsigned long endtime = 0;
	unsigned long msec_to_jiffies = 0;
	msec_to_jiffies = msecs_to_jiffies(msec);
	endtime = jiffies + msec_to_jiffies + 1;

	//printk("%s: =================== large_delay %d sec(%d jiffies) , Start\n", __func__, msec, msec_to_jiffies);
	while(jiffies<endtime) ;
	//printk("%s: =================== END\n", __func__);

	return ;
}


#if 1 //SPRINT_SLATE_KEYPRESS_TEST
extern int diagchar_ioctl(struct file *filp,unsigned int iocmd, unsigned long ioarg);
#endif //SPRINT_SLATE_KEYPRESS_TEST

int
inet_pton4(const char *src, u_char *dst)
{
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
	u_char tmp[4], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr(digits, ch)) != NULL) {
			u_int new = *tp * 10 + (pch - digits);

			if (new > 255)
				return (0);
			if (! saw_digit) {
				if (++octets > 4)
					return (0);
				saw_digit = 1;
			}
			*tp = new;
		} else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return (0);
			*++tp = 0;
			saw_digit = 0;
		} else
			return (0);
	}
	if (octets < 4)
		return (0);

	memcpy(dst, tmp, 4);
	return (1);
}

int
inet_pton6(const char *src, u_char *dst)
{
	static const char xdigits_l[] = "0123456789abcdef",
			  xdigits_u[] = "0123456789ABCDEF";
	u_char tmp[16], *tp, *endp, *colonp;
	const char *xdigits, *curtok;
	int ch, saw_xdigit, count_xdigit;
	u_int val;

	memset((tp = tmp), '\0', 16);
	endp = tp + 16;
	colonp = NULL;
	/* Leading :: requires some special handling. */
	if (*src == ':')
		if (*++src != ':')
			return (0);
	curtok = src;
	saw_xdigit = count_xdigit = 0;
	val = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr((xdigits = xdigits_u), ch);
		if (pch != NULL) {
			if (count_xdigit >= 4)
				return (0);
			val <<= 4;
			val |= (pch - xdigits);
			if (val > 0xffff)
				return (0);
			saw_xdigit = 1;
			count_xdigit++;
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!saw_xdigit) {
				if (colonp)
					return (0);
				colonp = tp;
				continue;
			} else if (*src == '\0') {
				return (0);
			}
			if (tp + 2 > endp)
				return (0);
			*tp++ = (u_char) (val >> 8) & 0xff;
			*tp++ = (u_char) val & 0xff;
			saw_xdigit = 0;
			count_xdigit = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + 4) <= endp) &&
		    inet_pton4(curtok, tp) > 0) {
			tp += 4;
			saw_xdigit = 0;
			count_xdigit = 0;
			break;	/* '\0' was seen by inet_pton4(). */
		}
		return (0);
	}
	if (saw_xdigit) {
		if (tp + 2 > endp)
			return (0);
		*tp++ = (u_char) (val >> 8) & 0xff;
		*tp++ = (u_char) val & 0xff;
	}
	if (colonp != NULL) {
		/*
		 * Since some memmove()'s erroneously fail to handle
		 * overlapping regions, we'll do the shift by hand.
		 */
		const int n = tp - colonp;
		int i;

		for (i = 1; i <= n; i++) {
			endp[- i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		return (0);
	memcpy(dst, tmp, 16);
	return (1);
}

/*
 * INTERNAL FUNCTION DEFINITIONS
 */
PACK(void *) LGE_ICDProcess(PACK(void *)req_pkt_ptr,	/* pointer to request packet  */
			    		unsigned short pkt_len			/* length of request packet   */)
{
	DIAG_ICD_F_req_type *req_ptr = (DIAG_ICD_F_req_type *) req_pkt_ptr;
	DIAG_ICD_F_rsp_type *rsp_ptr = NULL;
	icd_func_type func_ptr = NULL;
	int is_valid_arm9_command = 1;
	unsigned int rsp_ptr_len;

	int nIndex = 0;
	if(!g_slate_status)
		g_slate_status = 0x1;
	
	diagpdev = diagcmd_get_dev();

	for (nIndex = 0; nIndex < ICD_MSTR_TBL_SIZE; nIndex++) 
	{
		if(icd_mstr_tbl[nIndex].cmd_code == req_ptr->hdr.sub_cmd) 
		{
			if (icd_mstr_tbl[nIndex].which_procesor == ICD_ARM11_PROCESSOR)
			{
				func_ptr = icd_mstr_tbl[nIndex].func_ptr;
			}
			
			break;
		} 
		else if (icd_mstr_tbl[nIndex].cmd_code == ICD_MAX_REQ_CMD)
		{
			break;
		}
		else
		{
			continue;
		}
	}

	if (func_ptr != NULL) 
	{
		printk(KERN_INFO "[ICD] cmd_code : [0x%X], sub_cmd : [0x%X]\n",req_ptr->hdr.cmd_code, req_ptr->hdr.sub_cmd);
		rsp_ptr = func_ptr((DIAG_ICD_F_req_type *) req_ptr);
	} 
	else
	{
		printk(KERN_INFO "[ICD] cmd_code : [0x%X], sub_cmd : [0x%X]\n",req_ptr->hdr.cmd_code, req_ptr->hdr.sub_cmd);
		switch(req_ptr->hdr.sub_cmd) {
			case ICD_GETDEVICEINFO_REQ_CMD:
				rsp_ptr_len = sizeof(icd_device_info_rsp_type);
				break;
			case ICD_GETGPSSTATUS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_gps_status_rsp_type);
				break;
			case ICD_SETGPSSTATUS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_set_gps_status_rsp_type);
				break;
			case ICD_GETROAMINGMODE_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_roamingmode_rsp_type);
				break;
			case ICD_GETSTATEANDCONNECTIONATTEMPTS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_state_connect_rsp_type);
				break;
			case ICD_GETBATTERYCHARGINGSTATE_REQ_CMD:
				rsp_ptr_len = sizeof(icd_set_battery_charging_state_rsp_type);
				break;
			case ICD_GETBATTERYLEVEL_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_battery_level_rsp_type);
				break;
/*			case ICD_GETRSSI_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_rssi_rsp_type);
				break;
*/			case ICD_SETDISCHARGING_REQ_CMD:
				rsp_ptr_len = sizeof(icd_set_discharger_rsp_type);
				break;

			case ICD_EMULATEPENACTION_REQ_CMD:
				rsp_ptr_len = sizeof(icd_emulate_pen_action_rsp_type);				
				break;
				
			case ICD_GET4GSTATUS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_4g_status_rsp_type);								
				break;
				
			case ICD_SET4GSTATUS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_set_4g_status_rsp_type);				
				break;

			case ICD_GETUSBDEBUGSTATUSSTATUS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_usbdebug_status_rsp_type);				
				break;
				
			case ICD_SETUSBDEBUGSTATUSSTATUS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_set_usbdebug_status_rsp_type);				
				break;
				
			case ICD_GETLATITUDELONGITUDEVALUES_REQ_CMD:
				g_diag_latitude_longitude = 1;
				rsp_ptr_len = sizeof(icd_get_latitude_longitude_rsp_type);				
				break;
				
			case ICD_GETSCREENLOCKSTATUS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_screenlock_status_rsp_type);				
				break;
				
			case ICD_SETSCREENLOCKSTATUS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_set_screenlock_status_rsp_type);				
				break;
				
			case ICD_GETAPNLIST_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_apnlist_rsp_type );								
				break;

			case ICD_GETTECHNOLOGYPRIORITY_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_technology_priority_rsp_type);				
				break;
				
			case ICD_SETTECHNOLOGYPRIORITY_REQ_CMD:
				rsp_ptr_len = sizeof(icd_set_technology_priority_rsp_type);				
				break;
				
			case ICD_GETHRPDSTATUS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_hrpd_status_rsp_type);				
				break;
				
			case ICD_SETHRPDSTATUS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_set_hrpd_status_rsp_type);								
				break;
				
			case ICD_GETANDORIDIDENTIFIER_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_android_identifier_rsp_type);				
				break;
				
 			case ICD_STARTIPCAPTURE_REQ_CMD:
				rsp_ptr_len = sizeof(icd_start_ip_capture_rsp_type);				
				break;
				
			case ICD_STOPIPCAPTURE_REQ_CMD:
				rsp_ptr_len = sizeof(icd_stop_ip_capture_rsp_type);				
				break;

#if 0				
			case ICD_INCOMMINGIPMESSAGE_REQ_CMD	:
				rsp_ptr_len = sizeof(icd_emulate_pen_action_req_proc);				
				break;
#endif				
			case ICD_GETINTERFACEIPADDRESS_REQ_CMD:
				rsp_ptr_len = sizeof(icd_get_interface_ip_address_rsp_type);				
				break;
			
			default:
				is_valid_arm9_command = 0;
				printk(KERN_INFO "[ICD] %s : invalid sub command : 0x%x\n",__func__,req_ptr->hdr.sub_cmd);
				break;
		}
		
		if(is_valid_arm9_command == 1)
		{
			int result;
			
			rsp_ptr = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_ptr_len);
			if (rsp_ptr == NULL) {
				printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
				return rsp_ptr;
			}
				
			printk(KERN_INFO "[ICD] cmd_code : [0x%X], sub_cmd : [0x%X] --> goto MODEM through oem rapi\n",
				req_ptr->hdr.cmd_code, req_ptr->hdr.sub_cmd);
			result = icd_send_to_arm9((void *)req_ptr, (void *)rsp_ptr, rsp_ptr_len);
			if( result > 0)
			{
				printk(KERN_INFO "%s(), icd_send_to_arm9 success\n",__func__);
			}
			else
			{
				printk(KERN_ERR "%s(), icd_send_to_arm9 fail \n",__func__);
			}
			
			
		}
	}

	return (rsp_ptr);
}
EXPORT_SYMBOL(LGE_ICDProcess);

void* icd_app_handler(DIAG_ICD_F_req_type*pReq)
{
	return NULL;
}

/** SAR : Sprint Automation Requirement - START **/
extern int lge_bd_rev;
extern void msm_get_SW_VER_type(char* sw_ver);
extern byte CheckHWRev(byte *pStr);

DIAG_ICD_F_rsp_type* icd_info_req_proc(DIAG_ICD_F_req_type * pReq)
{
	char* sw_ver = NULL;
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_device_info_rsp_type);

	printk(KERN_INFO "[ICD] get icd_info_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);

	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETDEVICEINFO_REQ_CMD;

	//get manufacture info
	strcpy(pRsp->icd_rsp.dev_rsp_info.manf_string, "LGE");

	//get model name info
	strcpy(pRsp->icd_rsp.dev_rsp_info.model_string, "LS840");	

	CheckHWRev((byte*)pRsp->icd_rsp.dev_rsp_info.hw_ver_string);
	
	msm_get_SW_VER_type(sw_ver);
	strcpy(pRsp->icd_rsp.dev_rsp_info.sw_ver_string, sw_ver);
	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_extended_info_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_extended_info_rsp_type);

	printk(KERN_INFO "[ICD] icd_extended_info_req_proc\n");
	printk(KERN_INFO "[ICD] icd_extended_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_EXTENDEDVERSIONINFO_REQ_CMD;

	// get extended version info
	memset(pRsp->icd_rsp.extended_rsp_info.ver_string, 0x00, sizeof(pRsp->icd_rsp.extended_rsp_info.ver_string));
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETEXTENDEDVERSION", 1);
		mdelay(100);
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			strcpy(pRsp->icd_rsp.extended_rsp_info.ver_string,process_value);
			printk(KERN_INFO "[ICD] %s was successful\n",__func__);
		}
		else
		{
			strcpy(pRsp->icd_rsp.extended_rsp_info.ver_string,"UNKNOWN");
			printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
		}
	}
	else
	{
		strcpy(pRsp->icd_rsp.extended_rsp_info.ver_string,"UNKNOWN");
		printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
	}
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_handset_disp_text_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_handset_disp_text_rsp_type);

	printk(KERN_INFO "[ICD] icd_handset_disp_text_req_proc\n");
	printk(KERN_INFO "[ICD] icd_handset_disp_text_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_HANDSETDISPLAYTEXT_REQ_CMD;

	// get handset display text
	
	return pRsp;
}



static void add_hdlc_packet(struct slate_data_buffer *mb, char data)
{
	mb->data[mb->data_length++] = data;

	if (mb->data_length >= BUFFER_MAX_SIZE) {
		mb->data_length = BUFFER_MAX_SIZE;
		msleep(10);
		if (diagchar_ioctl (NULL,DIAG_IOCTL_BULK_DATA, (unsigned long)mb)) {
			printk(KERN_ERR "[SLATE] %s: diagchar_ioctl error\n", __func__);
		} 
		mb->data_length = 0;
	}

	return;
}

/*
 * FUNCTION	add_hdlc_esc_packet.
 */
static void add_hdlc_esc_packet(struct slate_data_buffer *mb, char data)
{
	if (data == ESC_CHAR || data == CONTROL_CHAR) {
		add_hdlc_packet(mb, ESC_CHAR);
		add_hdlc_packet(mb, (data ^ ESC_MASK));
	} 
	else {
		add_hdlc_packet(mb, data);
	}

	return;
}

/*
 * FUNCTION	mtc_send_hdlc_packet.
 */
static void slate_send_hdlc_packet(byte * pBuf, int len)
{
	int i;
	struct slate_data_buffer *mb = NULL;
	word crc = CRC_16_L_SEED;

	mb = (struct slate_data_buffer *)kzalloc(sizeof(struct slate_data_buffer), GFP_KERNEL);
	if (mb == NULL) {
		printk(KERN_ERR "[SLATE] %s: failed to alloc memory\n", __func__);
		return;
	}

	//Generate crc data.
	for (i = 0; i < len; i++) {
		add_hdlc_esc_packet(mb, pBuf[i]);
		crc = CRC_16_L_STEP(crc, (word) pBuf[i]);
	}

	crc ^= CRC_16_L_SEED;
	add_hdlc_esc_packet(mb, ((unsigned char)crc));
	add_hdlc_esc_packet(mb, ((unsigned char)((crc >> 8) & 0xFF)));
	add_hdlc_packet(mb, CONTROL_CHAR);

	if (diagchar_ioctl(NULL,DIAG_IOCTL_BULK_DATA, (unsigned long)mb)) {
		printk(KERN_ERR "[SLATE] %s: ioctl ignored\n", __func__);
	}

	if(mb != NULL)
	{
		printk(KERN_INFO "%s(), slate_data_buffer free\n",__func__);
		kfree(mb);
		mb = NULL;
	}

	return;
}

void slate_send_key_log_packet(unsigned long keycode, unsigned long state)
{
	key_msg_type msg;
	dword sendKeyValue = 0;

	if(mtc_running)
		return ;

	memset(&msg, 0, sizeof(key_msg_type));

	sendKeyValue = keycode;

	msg.cmd_code = 121;
	msg.ts_type = 0;	//2;
	msg.num_args = 2;
	msg.drop_cnt = 0;
	//ts_get(&msg.time[0]); 
	msg.time[0] = 0;
	msg.time[1] = 0;
	msg.line_number = 261;
	msg.ss_id = LGE_DIAG_ICD_LOGGING_SSID;
	msg.ss_mask = LGE_DIAG_ICD_LOGGING_SSID_MASK;
	msg.args[0] = sendKeyValue;
	msg.args[1] = state;

	printk(KERN_INFO "%s(), sendKeyValue:%ld, state:%ld \n",__func__,sendKeyValue,state);

	memcpy(&msg.code[0], "Debounced %d", sizeof("Debounced %d"));
	//msg.code[12] = '\0';

	memcpy(&msg.file_name[0], "ckpd_daemon.c", sizeof("ckpd_daemon.c"));
	//msg.fle_name[13] = '\0';

	slate_send_hdlc_packet((byte *) & msg, sizeof(key_msg_type));

	return;
}
EXPORT_SYMBOL(slate_send_key_log_packet);
#if 1
void slate_send_touch_log_packet(unsigned long touch_x, unsigned long touch_y, unsigned long status)
{
	touch_msg_type msg;
	
	/* LGE_CHANGE [dojip.kim@lge.com] 2010-06-04 [LS670]
	 * don't send a raw diag packet in running MTC
	 */
	if(mtc_running)
		return ;
	memset(&msg, 0, sizeof(touch_msg_type));

	msg.cmd_code = 121;
	msg.ts_type = 0;	//2;
	msg.num_args = 3;
	msg.drop_cnt = 0;
	//ts_get(&msg.time[0]); 
	msg.time[0] = 0;
	msg.time[1] = 0;
	msg.line_number = 261;
	msg.ss_id = LGE_DIAG_ICD_LOGGING_SSID;
	msg.ss_mask = LGE_DIAG_ICD_LOGGING_SSID_MASK;
	printk(KERN_INFO "%s(), status:%ld\n",__func__, status);
	if(status == 1) // push - "DWN"
	{
		printk(KERN_INFO "%s(), Down\n",__func__);
		msg.args[0] = 0x004E5744;
	}
	else	// release - "UP"
	{
		printk(KERN_INFO "%s(), Up\n",__func__);		
		msg.args[0] = 0x00005055;
	}
	msg.args[1] = touch_x*480/max_x;
	msg.args[2] = touch_y*800/max_y;
	printk(KERN_INFO "%s(), touch_x:%ld, touch_y:%ld\n",__func__,touch_x,touch_y);
	memcpy(&msg.code[0], "PenEvent %d,%d", sizeof("PenEvent %d,%d"));
	//msg.code[12] = '\0';

	memcpy(&msg.file_name[0], "ckpd_daemon.c", sizeof("ckpd_daemon.c"));
	//msg.fle_name[13] = '\0';

	slate_send_hdlc_packet((byte *) & msg, sizeof(touch_msg_type));
	
	return;
}
EXPORT_SYMBOL(slate_send_touch_log_packet);
#else
void slate_send_touch_log_packet(unsigned long touch_x, unsigned long touch_y, unsigned long status)
{
	touch_msg_type* msg = NULL;
	
	/* LGE_CHANGE [dojip.kim@lge.com] 2010-06-04 [LS670]
	 * don't send a raw diag packet in running MTC
	 */
	if(mtc_running)
		return ;

	msg = vmalloc(sizeof(touch_msg_type));


	if(NULL == msg)
	{
		printk(KERN_ERR "%s(), msg is NULL\b");
		return;
	}
	
	memset(msg, 0, sizeof(touch_msg_type));

	msg->cmd_code = 121;
	msg->ts_type = 0;	//2;
	msg->num_args = 3;
	msg->drop_cnt = 0;
	//ts_get(&msg.time[0]); 
	msg->time[0] = 0;
	msg->time[1] = 0;
	msg->line_number = 261;
	msg->ss_id = LGE_DIAG_ICD_LOGGING_SSID;
	msg->ss_mask = LGE_DIAG_ICD_LOGGING_SSID_MASK;
	printk(KERN_INFO "%s(), status:%ld\n",__func__, status);
	if(status == 1) // push - "DWN"
	{
		printk(KERN_INFO "%s(), Down\n",__func__);
		msg->args[0] = 0x004E5744;
	}
	else	// release - "UP"
	{
		printk(KERN_INFO "%s(), Up\n",__func__);		
		msg->args[0] = 0x00005055;
	}
	msg->args[1] = touch_x*320/max_x;
	msg->args[2] = touch_y*480/max_y;
	printk(KERN_INFO "%s(), touch_x:%ld, touch_y:%ld\n",__func__,touch_x,touch_y);
	memcpy(&(msg->code[0]), "PenEvent %d,%d", sizeof("PenEvent %d,%d"));
	//msg.code[12] = '\0';

	memcpy(&(msg->file_name[0]), "ckpd_daemon.c", sizeof("ckpd_daemon.c"));
	//msg.fle_name[13] = '\0';

	slate_send_hdlc_packet((byte *) msg, sizeof(touch_msg_type));

	vfree(msg);
	return;
}
EXPORT_SYMBOL(slate_send_touch_log_packet);
#endif
typedef struct
{
	short 	bfType;
	long 	bfSize;
	short 	bfReserved1;
	short 	bfReserved2;
	long 	bfOffBits;
	long 	biSize;
	long 	biWidth;
	long 	biHeight;
	short 	biPlanes;
	short 	biBitCount;
	long 	biCompression;
	long 	biSizeImage;
	long 	biXPelsPerMeter;
	long 	biYPelsPerMeter;
	long 	biClrUsed;
	long 	biClrImportant;
} PACKED BMPHEAD;


typedef struct tagBITMAPFILEHEADER {
	short bfType;    
	long bfSize;      
	short bfReserved1;
	short bfReserved2;
	long bfOffBits;  
} PACKED BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	long biSize;
	long biWidth;
	long biHeight;
	short biPlanes;
	short biBitCount;
	long biCompression;
	long biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	long biClrUsed;
	long biClrImportant;
} PACKED BITMAPINFOHEADER;

int removefile( char const *filename )
{
             char *argv[4] = { NULL, NULL, NULL, NULL };
             char *envp[3] = { NULL, NULL, NULL };

             if ( !filename )
                          return -EINVAL;
 
             argv[0] = "/system/bin/rm";
             argv[1] = (char *)filename;
 
             envp[0] = "HOME=/";
             envp[1] = "TERM=linux";
 
             return call_usermodehelper( argv[0], argv, envp, UMH_WAIT_PROC);
}
EXPORT_SYMBOL(removefile);

int makepix(void)
{
             char *argv[4] = { NULL, NULL, NULL, NULL };
             char *envp[3] = { NULL, NULL, NULL };
 
             argv[0] = "/system/bin/slate_screencap";
 
             envp[0] = "HOME=/";
             envp[1] = "TERM=linux";

             return call_usermodehelper( argv[0], argv, envp, UMH_WAIT_PROC );
}

static void read_Framebuffer(int x_start, int y_start, int x_end, int y_end)
{
	byte *fb_buffer;
	byte *save_buffer;
	int fbfd, i, j;
	mm_segment_t old_fs=get_fs();
	int point = 0;
	BITMAPFILEHEADER map_header;
	BITMAPINFOHEADER info_header;
	byte header[12] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00};

	set_fs(get_ds());
	
	fb_buffer = vmalloc(LCD_BUFFER_SIZE);
	memset(fb_buffer, 0x00, LCD_BUFFER_SIZE);

	makepix();

	if( (fbfd = sys_open("/data/img/img.raw", O_RDONLY, 0)) == -1 )
	{
		printk(KERN_ERR "%s, Can't open %s\n",__func__,"/data/img/img.raw");
		return;
	}
	
	sys_read(fbfd, fb_buffer, LCD_BUFFER_SIZE) ;

	sys_close(fbfd);

	removefile("/data/img/img.raw");
	
	if( (fbfd = sys_open("/data/img/image.bmp", O_CREAT | O_LARGEFILE | O_WRONLY, 0)) == -1)
	{
		printk(KERN_ERR "%s, Can't open %s\n",__func__,"/data/img/image.bmp");
		return;
	}
	
	save_buffer = vmalloc(LCD_BUFFER_SIZE);	
	memset(save_buffer, 0x00, LCD_BUFFER_SIZE);

	for(j=0;j<ICD_LCD_MAIN_HEIGHT;j++)
	{
		for(i=0;i<ICD_LCD_MAIN_WIDTH;i++)
		{
			if(((i>=(x_start)) && (i<(x_end+1))) && ((j>=(y_start)) && (j<(y_end+1))))
			{
				memcpy(&save_buffer[point], &fb_buffer[(j*ICD_LCD_MAIN_WIDTH*4)+i*4], sizeof(byte) * 4);
				point += 4;
			}
		}
	}

	map_header.bfType = 0x4d42;
	map_header.bfSize = point + 66;
	map_header.bfReserved1 = 0x0;
	map_header.bfReserved2 = 0x0;
	map_header.bfOffBits = 0x42;

	info_header.biSize = 0x28;
	info_header.biWidth = x_end - x_start + 1; 
	info_header.biHeight =  (y_end - y_start + 1) * -1;

	info_header.biPlanes = 0x1;
	info_header.biBitCount = 0x20;
	info_header.biCompression = 0x3;
	info_header.biSizeImage = point;
	info_header.biXPelsPerMeter = 0xec4;
	info_header.biYPelsPerMeter = 0xec4;
	info_header.biClrUsed = 0x0;
	info_header.biClrImportant = 0x0;

	printk(KERN_INFO "%s(), sizeof(BITMAPFILEHEADER):%d\n",__func__,sizeof(BITMAPFILEHEADER));
	printk(KERN_INFO "%s(), map_header.bfType:0x%x\n", __func__,map_header.bfType);
	printk(KERN_INFO "%s(), map_header.bfSize:0x%lx\n", __func__,map_header.bfSize);	
	printk(KERN_INFO "%s(), map_header.bfReserved1:0x%x\n", __func__,map_header.bfReserved1);	
	printk(KERN_INFO "%s(), map_header.bfReserved2:0x%x\n", __func__,map_header.bfReserved2);
	printk(KERN_INFO "%s(), map_header.bfOffBits:0x%lx\n", __func__,map_header.bfOffBits);
	
	printk(KERN_INFO "%s(), sizeof(BITMAPINFOHEADER):%d\n",__func__,sizeof(BITMAPINFOHEADER));
	printk(KERN_INFO "%s(), info_header.biSize:0x%lx \n",__func__,info_header.biSize);
	printk(KERN_INFO "%s(), info_header.biWidth:0x%lx\n",__func__,info_header.biWidth);
	printk(KERN_INFO "%s(), info_header.biHeight:0x%lx\n",__func__,info_header.biHeight);
	printk(KERN_INFO "%s(), info_header.biPlanes:0x%d\n",__func__,info_header.biPlanes);
	printk(KERN_INFO "%s(), info_header.biBitCount:0x%d\n",__func__,info_header.biBitCount);
	printk(KERN_INFO "%s(), info_header.biCompression:0x%ld\n",__func__,info_header.biCompression);
	printk(KERN_INFO "%s(), info_header.biSizeImage:0x%ld\n",__func__,info_header.biSizeImage);
	printk(KERN_INFO "%s(), info_header.biXPelsPerMeter:0x%ld\n",__func__,info_header.biXPelsPerMeter);
	printk(KERN_INFO "%s(), info_header.biYPelsPerMeter:0x%ld\n",__func__,info_header.biYPelsPerMeter);
	printk(KERN_INFO "%s(), info_header.biClrUsed:0x%ld\n",__func__,info_header.biClrUsed);
	printk(KERN_INFO "%s(), info_header.biClrImportant:0x%ld\n",__func__,info_header.biClrImportant);

	sys_write(fbfd, (const char __user *)&map_header, sizeof(BITMAPFILEHEADER));
	sys_write(fbfd, (const char __user *)&info_header, sizeof(BITMAPINFOHEADER));
	sys_write(fbfd, (const char __user *)&header, sizeof(byte)*12);
	
	sys_write(fbfd, (const char __user *)save_buffer, point);
	sys_close(fbfd);
	
	set_fs(old_fs);	

	if(save_buffer != NULL)
		vfree(save_buffer);
	
	if(fb_buffer != NULL)
		vfree(fb_buffer);
}


DIAG_ICD_F_rsp_type *icd_capture_img_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len, packet_len;	
	static short x_start=0, y_start=0, x_end=0, y_end=0;
	static short seq_num = 0;
	DIAG_ICD_F_rsp_type *pRsp;	

	g_diag_slate_capture_rsp_num = 0x01;

	if(pReq->icd_req.capture_req_info.seq_num == 0)
	{
		x_start = pReq->icd_req.capture_req_info.upper_left_x;
		y_start = pReq->icd_req.capture_req_info.upper_left_y;
		x_end = pReq->icd_req.capture_req_info.lower_right_x;
		y_end = pReq->icd_req.capture_req_info.lower_right_y;
		seq_num = pReq->icd_req.capture_req_info.seq_num;
	}
	
	rsp_len = sizeof(icd_screen_capture_rsp_type);
	packet_len = rsp_len;

	printk(KERN_INFO "%s(), x_start:%d, y_start:%d, x_end:%d, y_end:%d, seq_num:%d \n",
		__func__,x_start ,y_start ,x_end ,y_end , seq_num);

	pRsp = (DIAG_ICD_F_rsp_type *)diagpkt_alloc2(DIAG_ICD_F, rsp_len, packet_len);
  	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return NULL;
  	}
	memset(pRsp, 0, rsp_len);


	if(pReq->icd_req.capture_req_info.seq_num == 0)
	{
		read_Framebuffer( x_start,y_start,x_end, y_end);
	}

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_CAPTUREIMAGE_REQ_CMD;
	pRsp->icd_rsp.capture_rsp_info.screen_id = 0;
	pRsp->icd_rsp.capture_rsp_info.upper_left_x = x_start;
	pRsp->icd_rsp.capture_rsp_info.upper_left_y = y_start;		
	pRsp->icd_rsp.capture_rsp_info.lower_right_x = x_end;
	pRsp->icd_rsp.capture_rsp_info.lower_right_y = y_end;
	pRsp->icd_rsp.capture_rsp_info.bit_per_pixel = 16;
	pRsp->icd_rsp.capture_rsp_info.actual_width = x_end - x_start + 1;
	pRsp->icd_rsp.capture_rsp_info.actual_height = y_end - y_start + 1;
	pRsp->icd_rsp.capture_rsp_info.seq_num = 0;
	pRsp->icd_rsp.capture_rsp_info.flags = LastImageNoHeader;
	memset(pRsp->icd_rsp.capture_rsp_info.image_data_block, 0x0, sizeof(char) * ICD_SEND_SAVE_IMG_PATH_LEN);
	sprintf(pRsp->icd_rsp.capture_rsp_info.image_data_block, "/data/img/image.bmp");

	sys_chmod("/data/img/image.bmp", 0664);

	return pRsp;
}

/** ICDR : ICD Implementation Recommendation  - START **/
DIAG_ICD_F_rsp_type *icd_get_airplanemode_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_get_airplane_mode_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_get_airplanemode_req_proc\n");
	printk(KERN_INFO "[ICD] icd_get_airplanemode_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETAIRPLANEMODE_REQ_CMD;
	
	// get airplane mode info
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETAIRPLANEMODE", 1);
		mdelay(100);
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"GETAIRPLANE_0") == 0)
			{
				pRsp->icd_rsp.get_airplane_mode_rsp_info.airplane_mode = 0;
				printk(KERN_INFO "[ICD] %s was successful : airplan mode is on\n",__func__);
			}
			else if(strcmp(process_value,"GETAIRPLANE_1") == 0)
			{
				pRsp->icd_rsp.get_airplane_mode_rsp_info.airplane_mode = 1;
				printk(KERN_INFO "[ICD] %s was successful : airplan mode is off\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.get_airplane_mode_rsp_info.airplane_mode = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.get_airplane_mode_rsp_info.airplane_mode = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.get_airplane_mode_rsp_info.airplane_mode = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
		
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_set_airplanemode_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_set_airplane_mode_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_set_airplanemode_req_proc : req mode = %d\n",pReq->icd_req.set_aiplane_mode_req_info.airplane_mode);
	printk(KERN_INFO "[ICD] icd_set_airplanemode_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SETAIRPLANEMODE_REQ_CMD;
	
	// set airplane mode info
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_SETAIRPLANEMODE", pReq->icd_req.set_aiplane_mode_req_info.airplane_mode);
		mdelay(300);

		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"SETAIRPLANE_0") == 0)
			{
				pRsp->icd_rsp.set_airplane_mode_rsp_info.cmd_status = 0;
				printk(KERN_INFO "[ICD] %s was successful\n",__func__);
			}
			else if(strcmp(process_value,"SETAIRPLANE_1") == 0)
			{
				pRsp->icd_rsp.set_airplane_mode_rsp_info.cmd_status = 1;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.set_airplane_mode_rsp_info.cmd_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.set_airplane_mode_rsp_info.cmd_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.set_airplane_mode_rsp_info.cmd_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_get_backlight_setting_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_get_backlight_setting_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_get_backlight_setting_req_proc\n");
	printk(KERN_INFO "[ICD] icd_get_backlight_setting_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETBACKLIGHTSETTING_REQ_CMD;
	
	// get backlight setting info
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETBACKLIGHTSETTING", 1);
		mdelay(100);

		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"GETBACKLIGHT_15") == 0)
			{
				pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data = 15;
				printk(KERN_INFO "[ICD] %s was successful : %dsec\n",__func__,pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data);
			}
			else if(strcmp(process_value,"GETBACKLIGHT_30") == 0)
			{
				pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data = 30;
				printk(KERN_INFO "[ICD] %s was successful : %dsec\n",__func__,pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data);
			}
			else if(strcmp(process_value,"GETBACKLIGHT_60") == 0)
			{
				pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data = 60;
				printk(KERN_INFO "[ICD] %s was successful : %dsec\n",__func__,pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data);
			}
			else if(strcmp(process_value,"GETBACKLIGHT_120") == 0)
			{
				pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data = 120;
				printk(KERN_INFO "[ICD] %s was successful : %dsec\n",__func__,pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data);
			}
			else if(strcmp(process_value,"GETBACKLIGHT_600") == 0)
			{
				pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data = 600;
				printk(KERN_INFO "[ICD] %s was successful : %dsec\n",__func__,pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data);
			}
			//20120709 mukesh83.kumar@lge.com LGE_CHANGE_CAYMAN_ICS_FOR_SLATE_BEGIN
			else if(strcmp(process_value,"GETBACKLIGHT_300") == 0)
			{
				pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data = 300;
				printk(KERN_INFO "[ICD] %s was successful : %dsec\n",__func__,pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data);
			}
			//20120709 mukesh83.kumar@lge.com LGE_CHANGE_CAYMAN_ICS_FOR_SLATE_END
			else if(strcmp(process_value,"GETBACKLIGHT_ALWAY") == 0)
			{
				pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data = 100;
				printk(KERN_INFO "[ICD] %s was successful : %dsec\n",__func__,pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data);
			}
			else
			{
				pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data = 0x00;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data = 0x00;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data = 0x00;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_set_backlight_setting_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	int retry_num = 0;
	
	rsp_len = sizeof(icd_set_backlight_setting_rsp_type);
	
	printk(KERN_INFO "[ICD_BRIGHTNESS] icd_set_backlight_setting_req_proc, req = %d\n",pReq->icd_req.set_backlight_setting_req_info.item_data);
	printk(KERN_INFO "[ICD_BRIGHTNESS] icd_set_backlight_setting_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD_BRIGHTNESS] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SETBACKLIGHTSETTING_REQ_CMD;
	
	// set backlight setting info
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD_BRIGHTNESS] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_SETBACKLIGHTSETTING", pReq->icd_req.set_backlight_setting_req_info.item_data);
		mdelay(200); /*SLATE: delay increased_sandeep.pawar_18AUG*/

		while(strcmp(process_status,"START") == 0)
		{
			mdelay(100);			
			printk(KERN_INFO "[ICD_BRIGHTNESS] %s wait again, Retry_num =%d \n",__func__,retry_num);
			retry_num++;
			
			if(retry_num==3)	
				break;
		}
		
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"SETBACKLIGHT_0") == 0)
			{
				pRsp->icd_rsp.set_backlight_setting_rsp_info.cmd_status = 0;
				printk(KERN_INFO "[ICD_BRIGHTNESS] %s was successful\n",__func__);
			}
			else if(strcmp(process_value,"SETBACKLIGHT_1") == 0)
			{
				pRsp->icd_rsp.set_backlight_setting_rsp_info.cmd_status = 1;
				printk(KERN_INFO "[ICD_BRIGHTNESS] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.set_backlight_setting_rsp_info.cmd_status = 0xFF;
				printk(KERN_INFO "[ICD_BRIGHTNESS] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.set_backlight_setting_rsp_info.cmd_status = 0xFF;
			printk(KERN_INFO "[ICD_BRIGHTNESS] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.set_backlight_setting_rsp_info.cmd_status = 0xFF;
		printk(KERN_INFO "[ICD_BRIGHTNESS] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}


DIAG_ICD_F_rsp_type *icd_get_batterycharging_state_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	char status[14];
	int rt;
	extern int battery_status_read(char*);	
	
	memset(status, 0, sizeof(char) * 14);
	rt = battery_status_read(status);

	rsp_len = sizeof(icd_get_battery_charging_state_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_get_batterycharging_state_req_proc\n");
	printk(KERN_INFO "[ICD] icd_get_batterycharging_state_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETBATTERYCHARGINGSTATE_REQ_CMD;
	
	if((!strcmp(status, "Charging")) || (!strcmp(status, "Full")))
	{
		pRsp->icd_rsp.get_battery_charging_state_rsp_info.charging_state = 1;
	}
	else
	{
		pRsp->icd_rsp.get_battery_charging_state_rsp_info.charging_state = 0;
	}
			
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_set_batterycharging_state_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_set_battery_charging_state_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_set_batterycharging_state_req_proc\n");
	printk(KERN_INFO "[ICD] icd_set_batterycharging_state_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SETBATTERYCHARGINGSTATE_REQ_CMD;
	
	// set battery charging info
	testmode_charging_mode_test();
	pRsp->icd_rsp.set_battery_charging_state_rsp_info.cmd_status = 0;

	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_get_battery_level_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	int batt_level;
	
	extern int battery_capacity_read(void);
	rsp_len = sizeof(icd_get_battery_level_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_get_battery_level_req_proc\n");
	printk(KERN_INFO "[ICD] icd_get_battery_level_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETBATTERYLEVEL_REQ_CMD;

	batt_level = battery_capacity_read();
	printk(KERN_INFO "[ICD] icd_get_battery_level : %d\n", batt_level);

	pRsp->icd_rsp.get_battery_level_rsp_info.battery_level = (unsigned char)batt_level;
		
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_get_bluetooth_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_get_bluetooth_status_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_get_bluetooth_status_req_proc\n");
	printk(KERN_INFO "[ICD] icd_get_bluetooth_status_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETBLUETOOTHSTATUS_REQ_CMD;
	
	// get bluetooth status info
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETBLUETOOTH", 1);
		mdelay(100);
		//large_mdelay(700);

		if(strcmp(process_status,"COMPLETED") == 0)
		{		
			if(strcmp(process_value,"GETBLUETOOTH_0") == 0)
			{
				pRsp->icd_rsp.get_bluetooth_status_rsp_info.bluetooth_status = 0;
				printk(KERN_INFO "[ICD] %s was successful : status = %d\n",__func__,pRsp->icd_rsp.get_bluetooth_status_rsp_info.bluetooth_status);
			}
			else if(strcmp(process_value,"GETBLUETOOTH_1") == 0)
			{
				pRsp->icd_rsp.get_bluetooth_status_rsp_info.bluetooth_status = 1;
				printk(KERN_INFO "[ICD] %s was successful : status = %d\n",__func__,pRsp->icd_rsp.get_bluetooth_status_rsp_info.bluetooth_status);
			}
			else if(strcmp(process_value,"GETBLUETOOTH_2") == 0)
			{
				pRsp->icd_rsp.get_bluetooth_status_rsp_info.bluetooth_status = 3;
				printk(KERN_INFO "[ICD] %s was successful : status = %d\n",__func__,pRsp->icd_rsp.get_bluetooth_status_rsp_info.bluetooth_status);
			}
			else
			{
				pRsp->icd_rsp.get_bluetooth_status_rsp_info.bluetooth_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.get_bluetooth_status_rsp_info.bluetooth_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.get_bluetooth_status_rsp_info.bluetooth_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_set_bluetooth_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_set_bluetooth_status_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_set_bluetooth_status_req_proc\n");
	printk(KERN_INFO "[ICD] icd_set_bluetooth_status_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SETBLUETOOTHSTATUS_REQ_CMD;
	
	// set bluetooth status info
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_SETBLUETOOTH", pReq->icd_req.set_bluetooth_status_req_info.bluetooth_status);

//		mdelay(100);
		large_mdelay(700);
		
		if(strcmp(process_status,"COMPLETED") == 0)
		{		
			if(strcmp(process_value,"SETBLUETOOTH_0") == 0)
			{
				pRsp->icd_rsp.set_bluetooth_status_rsp_info.cmd_status = 0;
				printk(KERN_INFO "[ICD] %s was successful \n",__func__);
			}
			else if(strcmp(process_value,"SETBLUETOOTH_1") == 0)
			{
				pRsp->icd_rsp.set_bluetooth_status_rsp_info.cmd_status = 1;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.set_bluetooth_status_rsp_info.cmd_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.set_bluetooth_status_rsp_info.cmd_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.set_bluetooth_status_rsp_info.cmd_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_get_gps_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_get_gps_status_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_get_gps_status_req_proc\n");
	printk(KERN_INFO "[ICD] icd_get_gps_status_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETGPSSTATUS_REQ_CMD;
	
	// get gps status info
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET GPS STATUS_10JUL_sandeep.pawar@lge.com_START*/	/*LGSI_CMF_LS840_ICS_SLATE GPS Sandeep Pawar:Start*/	
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETGPS", 1);
		mdelay(100);

		if(strcmp(process_status,"COMPLETED") == 0)
		{		
			if(strcmp(process_value,"GETGPS_0") == 0)
			{
				pRsp->icd_rsp.get_gps_status_rsp_info.gps_status = 0;
				printk(KERN_INFO "[ICD] %s was successful : status = %d\n",__func__,pRsp->icd_rsp.get_gps_status_rsp_info.gps_status);
			}
			else if(strcmp(process_value,"GETGPS_1") == 0)
			{
				pRsp->icd_rsp.get_gps_status_rsp_info.gps_status = 1;
				printk(KERN_INFO "[ICD] %s was successful : status = %d\n",__func__,pRsp->icd_rsp.get_gps_status_rsp_info.gps_status);
			}
			else
			{
				pRsp->icd_rsp.get_gps_status_rsp_info.gps_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.get_gps_status_rsp_info.gps_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.get_gps_status_rsp_info.gps_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET GPS STATUS_10JUL_sandeep.pawar@lge.com_END*/	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_set_gps_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET GPS STATUS_10JUL_sandeep.pawar@lge.com_START*/
	int retry_num = 0;
	unsigned char result = 0;
	result = pReq->icd_req.set_gps_status_req_info.gps_status;
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET GPS STATUS_10JUL_sandeep.pawar@lge.com_START*/
	
	rsp_len = sizeof(icd_set_gps_status_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_set_gps_status_req_proc\n");
	printk(KERN_INFO "[ICD] icd_set_gps_status_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SETGPSSTATUS_REQ_CMD;
	
	// set gps status info
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET GPS STATUS_10JUL_sandeep.pawar@lge.com_START*/
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_SETGPS", pReq->icd_req.set_gps_status_req_info.gps_status);
		mdelay(100);
		
		while(strcmp(process_status,"START") == 0)
		{
			mdelay(100);			
			printk(KERN_INFO "[ICD_SETGPSSTATUS] %s wait again, Retry_num =%d \n",__func__,retry_num);
			retry_num++;
			
			if(retry_num==3)	
				break;
		}

		if(strcmp(process_status,"COMPLETED") == 0)
		{		
			if(strcmp(process_value,"SETGPS_0") == 0)
			{
				pRsp->icd_rsp.set_gps_status_rsp_info.cmd_status = 0;
				printk(KERN_INFO "[ICD] %s was successful \n",__func__);
			}
			else if(strcmp(process_value,"SETGPS_1") == 0)
			{
				pRsp->icd_rsp.set_gps_status_rsp_info.cmd_status = 0; //sandeep.pawar_modified value_18JUL
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.set_gps_status_rsp_info.cmd_status = 0;//result;//0xFF; //sandeep.pawar_modified value_18JUL
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.set_gps_status_rsp_info.cmd_status = 0;//result;//0xFF; //sandeep.pawar_modified value_18JUL
			printk(KERN_INFO "[ICD] %s process_status is not equal to COMPLETED \n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.set_gps_status_rsp_info.cmd_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET GPS STATUS_10JUL_sandeep.pawar@lge.com_START*/
	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_get_keypadbacklight_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_get_keypadbacklight_rsp_type);
	
	printk(KERN_INFO "[ICD_KEY_BRIGHTNESS] icd_get_keypadbacklight_req_proc\n");
	printk(KERN_INFO "[ICD_KEY_BRIGHTNESS] icd_get_keypadbacklight_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD_KEY_BRIGHTNESS] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETKEYPADBACKLIGHT_REQ_CMD;
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET KEYBACKLIGHT SETTINGS_10JUL_sandeep.pawar@lge.com_START*/
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETKEYPADBACKLIGHT", 1);
		mdelay(100);
	
		if(strcmp(process_status,"COMPLETED") == 0)
		{
		if(strcmp(process_value,"GETKEYBACKLIGHT_1.5") == 0)
			{
				pRsp->icd_rsp.get_keypadbacklight_rsp_info.keypad_backlight = 15;
				printk(KERN_INFO "[ICD] %s was successful : %dsec\n",__func__,pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data);
			}
			else if(strcmp(process_value,"GETKEYBACKLIGHT_3") == 0)
			{
				pRsp->icd_rsp.get_keypadbacklight_rsp_info.keypad_backlight = 30;
				printk(KERN_INFO "[ICD] %s was successful : %dsec\n",__func__,pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data);
			}
			else if(strcmp(process_value,"GETKEYBACKLIGHT_5") == 0)
			{
				pRsp->icd_rsp.get_keypadbacklight_rsp_info.keypad_backlight = 50;
				printk(KERN_INFO "[ICD] %s was successful : %dsec\n",__func__,pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data);
			}
			else if(strcmp(process_value,"GETKEYBACKLIGHT_ALWAY") == 0)
			{
				pRsp->icd_rsp.get_keypadbacklight_rsp_info.keypad_backlight = 100;
				printk(KERN_INFO "[ICD] %s was successful : %dsec\n",__func__,pRsp->icd_rsp.get_backlight_setting_rsp_info.item_data);
			}
			else
			{
				pRsp->icd_rsp.get_keypadbacklight_rsp_info.keypad_backlight = 0;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispacher is invalid\n",__func__);
			}
			
		}
		else
		{
			pRsp->icd_rsp.get_keypadbacklight_rsp_info.keypad_backlight = 0;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.get_keypadbacklight_rsp_info.keypad_backlight = 0;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	// get keypadbacklight status info
/*	switch(saveKeyLedSetValue){
		case 0 : //off
			pRsp->icd_rsp.get_keypadbacklight_rsp_info.keypad_backlight = 0;
			break;
		case 100 : //on
			pRsp->icd_rsp.get_keypadbacklight_rsp_info.keypad_backlight = 100;
			break;
		default :
			pRsp->icd_rsp.get_keypadbacklight_rsp_info.keypad_backlight = 0;
			break;
	}		
*/
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET KEYBACKLIGHT SETTINGS_10JUL_sandeep.pawar@lge.com_END*/	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_set_keypadbacklight_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	int retry_num = 0; /*LGSI_CMF_LS840_ICS_SLATE_SET/GET KEYBACKLIGHT SETTINGS_10JUL_sandeep.pawar@lge.com_START*/
	
	rsp_len = sizeof(icd_set_keypadbacklight_rsp_type);
	
	printk(KERN_INFO "[ICD_KEY_BRIGHTNESS] icd_set_keypadbacklight_req_proc, req = %d\n",pReq->icd_req.set_backlight_setting_req_info.item_data);
	printk(KERN_INFO "[ICD_KEY_BRIGHTNESS] icd_set_keypadbacklight_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD_KEY_BRIGHTNESS] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SETKEYPADBACKLIGHT_REQ_CMD;

/*LGSI_CMF_LS840_ICS_SLATE_SET/GET KEYBACKLIGHT SETTINGS_10JUL_sandeep.pawar@lge.com_START*/
	// set Keybacklight setting info
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD_BRIGHTNESS] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_SETKEYPADBACKLIGHT", pReq->icd_req.set_backlight_setting_req_info.item_data);
		mdelay(100);

		while(strcmp(process_status,"START") == 0)
		{
			mdelay(100);			
			printk(KERN_INFO "[ICD_SETKEYPADBACKLIGHT] %s wait again, Retry_num =%d \n",__func__,retry_num);
			retry_num++;
			
			if(retry_num==3)	
				break;
		}
		
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"SETBACKLIGHT_0") == 0)
			{
				pRsp->icd_rsp.set_keypadbacklight_rsp_info.cmd_status = 0;
				printk(KERN_INFO "[ICD_BRIGHTNESS] %s was successful\n",__func__);
			}
			else if(strcmp(process_value,"SETBACKLIGHT_1") == 0)
			{
				pRsp->icd_rsp.set_keypadbacklight_rsp_info.cmd_status = 1;
				printk(KERN_INFO "[ICD_BRIGHTNESS] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.set_keypadbacklight_rsp_info.cmd_status = 1;
				printk(KERN_INFO "[ICD_BRIGHTNESS] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.set_keypadbacklight_rsp_info.cmd_status = 1;
			printk(KERN_INFO "[ICD_BRIGHTNESS] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.set_backlight_setting_rsp_info.cmd_status = 1;
		printk(KERN_INFO "[ICD_BRIGHTNESS] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
		
/*	switch(pReq->icd_req.set_keypadbacklight_req_info.keypad_backlight){
		
		case 0 : //off
			 pm8058_set_led_current(PMIC8058_ID_LED_KB_LIGHT,0);
			pRsp->icd_rsp.set_keypadbacklight_rsp_info.cmd_status = 0;
			printk(KERN_ERR "[ICD_KEY_BRIGHTNESS] set OFF\n");
			saveKeyLedSetValue=0;
			break;
		case 100 : //on
			pm8058_set_led_current(PMIC8058_ID_LED_KB_LIGHT,15*20); //max brightness
			pRsp->icd_rsp.set_keypadbacklight_rsp_info.cmd_status = 0;
			printk(KERN_ERR "[ICD_KEY_BRIGHTNESS] set ON\n");
			saveKeyLedSetValue=100;
			break;
		default :
			pRsp->icd_rsp.set_keypadbacklight_rsp_info.cmd_status = 1;
			printk(KERN_ERR "[ICD_KEY_BRIGHTNESS]request valus is invalid\n");
			break;
	}
*/	
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET KEYBACKLIGHT SETTINGS_10JUL_sandeep.pawar@lge.com_END*/
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_get_roamingmode_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_get_roamingmode_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_get_roamingmode_req_proc\n");
	printk(KERN_INFO "[ICD] icd_get_roamingmode_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETROAMINGMODE_REQ_CMD;
	
	// get roaming mode info
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETROAMMONGMODE", 1);
		mdelay(100);
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"GETICDROAMING_1") == 0)
			{
				pRsp->icd_rsp.get_roamingmode_rsp_info.roaming_mode= 1;
				printk(KERN_INFO "[ICD] %s was successful : roaming mode is sprint only\n",__func__);
			}
			else if(strcmp(process_value,"GETICDROAMING_2") == 0)
			{
				pRsp->icd_rsp.get_roamingmode_rsp_info.roaming_mode = 2;
				printk(KERN_INFO "[ICD] %s was successful : roaming mode is include roaming\n",__func__);
			}
			else if(strcmp(process_value,"GETICDROAMING_0") == 0)
			{
				pRsp->icd_rsp.get_roamingmode_rsp_info.roaming_mode = 0;
				printk(KERN_INFO "[ICD] %s was successful : roaming mode is not in the list\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.get_roamingmode_rsp_info.roaming_mode = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.get_roamingmode_rsp_info.roaming_mode = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.get_roamingmode_rsp_info.roaming_mode = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
		
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_get_rssi_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	int rssi, ecio, no_of_bar, status;
	
	rsp_len = sizeof(icd_get_rssi_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_get_rssi_req_proc\n");
	printk(KERN_INFO "[ICD] icd_get_rssi_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETRSSI_REQ_CMD;
	
	// get RSSI info
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETRSSI", 0);
		mdelay(100);
	
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			// printk(KERN_INFO "[ICD] %s process_value = %s\n",__func__, process_value);

			sscanf(process_value, "%d %d %d %d", &rssi, &ecio, &no_of_bar, &status);
			pRsp->icd_rsp.get_rssi_rsp_info.rx_power = rssi*100; /* multiplied by 100 due to req. */
			pRsp->icd_rsp.get_rssi_rsp_info.rx_ec_io = ecio*10;	/* multiplied by 100 due to req. & already multiplied by 10 */
			pRsp->icd_rsp.get_rssi_rsp_info.numbar = no_of_bar;
			pRsp->icd_rsp.get_rssi_rsp_info.status = status;

			printk(KERN_INFO "[ICD] %s rssi = %d(%d)\n",__func__, pRsp->icd_rsp.get_rssi_rsp_info.rx_power, rssi);
			printk(KERN_INFO "[ICD] %s ecio = %d(%d)\n",__func__, pRsp->icd_rsp.get_rssi_rsp_info.rx_ec_io, ecio);
			printk(KERN_INFO "[ICD] %s bars = %d(%d)\n",__func__, pRsp->icd_rsp.get_rssi_rsp_info.numbar, no_of_bar);
			printk(KERN_INFO "[ICD] %s status = %d(%d)\n",__func__, pRsp->icd_rsp.get_rssi_rsp_info.status, status);
		}
		else
		{			
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{		
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_get_state_connect_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_get_state_connect_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_get_state_connect_req_proc\n");
	printk(KERN_INFO "[ICD] icd_get_state_connect_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETSTATEANDCONNECTIONATTEMPTS_REQ_CMD;
	
	// get state and connection attempts info
	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_get_ui_screen_id_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_get_ui_screen_id_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_get_ui_screen_id_req_proc\n");
	printk(KERN_INFO "[ICD] icd_get_ui_screen_id_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETUISCREENID_REQ_CMD;
	
	// get ui screen id 
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETUISCREENID", pReq->icd_req.get_ui_srceen_id_req_info.physical_screen);
		mdelay(100);

		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"GETUISCREENID_1") == 0)
			{
				pRsp->icd_rsp.get_ui_screen_id_rsp_info.ui_screen_id = 1;
				printk(KERN_INFO "[ICD] %s was successful\n",__func__);
			}
			else if(strcmp(process_value,"GETUISCREENID_2") == 0)
			{
				pRsp->icd_rsp.get_ui_screen_id_rsp_info.ui_screen_id = 2;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else if(strcmp(process_value,"GETUISCREENID_3") == 0)
			{
				pRsp->icd_rsp.get_ui_screen_id_rsp_info.ui_screen_id = 3;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else if(strcmp(process_value,"GETUISCREENID_4") == 0)
			{
				pRsp->icd_rsp.get_ui_screen_id_rsp_info.ui_screen_id = 4;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else if(strcmp(process_value,"GETUISCREENID_5") == 0)
			{
				pRsp->icd_rsp.get_ui_screen_id_rsp_info.ui_screen_id = 5;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else if(strcmp(process_value,"GETUISCREENID_6") == 0)
			{
				pRsp->icd_rsp.get_ui_screen_id_rsp_info.ui_screen_id = 6;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else if(strcmp(process_value,"GETUISCREENID_7") == 0)
			{
				pRsp->icd_rsp.get_ui_screen_id_rsp_info.ui_screen_id = 7;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.get_ui_screen_id_rsp_info.ui_screen_id = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.get_ui_screen_id_rsp_info.ui_screen_id = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.get_ui_screen_id_rsp_info.ui_screen_id = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	pRsp->icd_rsp.get_ui_screen_id_rsp_info.physical_screen = 0;
		
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_get_wifi_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_get_wifi_status_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_get_wifi_status_req_proc\n");
	printk(KERN_INFO "[ICD] icd_get_wifi_status_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETWIFISTATUS_REQ_CMD;

	// get wifi status
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETWIFISTATUS", 1);
		mdelay(100);

		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"GETWIFISTATUS_0") == 0)
			{
				pRsp->icd_rsp.get_wifi_status_rsp_info.wifi_status = 0;
				printk(KERN_INFO "[ICD] %s was successful : wifi status = %d\n",__func__,pRsp->icd_rsp.get_wifi_status_rsp_info.wifi_status);
			}
			else if(strcmp(process_value,"GETWIFISTATUS_1") == 0)
			{
				pRsp->icd_rsp.get_wifi_status_rsp_info.wifi_status = 1;
				printk(KERN_INFO "[ICD] %s was successful : wifi status = %d\n",__func__,pRsp->icd_rsp.get_wifi_status_rsp_info.wifi_status);
			}
			else
			{
				pRsp->icd_rsp.get_wifi_status_rsp_info.wifi_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.get_wifi_status_rsp_info.wifi_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.get_wifi_status_rsp_info.wifi_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_set_wifi_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_set_wifi_status_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_set_wifi_status_req_proc, req = %d\n", pReq->icd_req.set_wifi_status_req_info.wifi_status);
	printk(KERN_INFO "[ICD] icd_set_wifi_status_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SETWIFISTATUS_REQ_CMD;
	
	// set wifi status
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_SETWIFISTATUS", pReq->icd_req.set_wifi_status_req_info.wifi_status);
		mdelay(100);

		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"SETWIFISTATUS_0") == 0)
			{
				pRsp->icd_rsp.set_wifi_status_rsp_info.cmd_status = 0;
				printk(KERN_INFO "[ICD] %s was successful \n",__func__);
			}
			else if(strcmp(process_value,"SETWIFISTATUS_1") == 0)
			{
				pRsp->icd_rsp.set_wifi_status_rsp_info.cmd_status = 1;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.set_wifi_status_rsp_info.cmd_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.set_wifi_status_rsp_info.cmd_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.set_wifi_status_rsp_info.cmd_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}

DIAG_ICD_F_rsp_type *icd_set_discharge_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;
	
	rsp_len = sizeof(icd_set_discharger_rsp_type);
	
	printk(KERN_INFO "[ICD] icd_set_discharging_status_req_proc, req = %ld\n", pReq->icd_req.set_discharger_req_info.off_time);
	printk(KERN_INFO "[ICD] icd_set_discharging_status_req_proc rsp_len :(%d)\n", rsp_len);
	
	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SETDISCHARGING_REQ_CMD;

    // off_time input ?Ö¼? ??À§?? hour
	testmode_discharging_mode_test();
	delayed_recharging_cmd(pReq->icd_req.set_discharger_req_info.off_time);
	printk(KERN_INFO "[ICD] icd_set_discharging \n");
	pRsp->icd_rsp.set_discharger_rsp_info.cmd_status = 0; // successful: 0 / fail: 1
	return pRsp;
}


DIAG_ICD_F_rsp_type *icd_set_screenorientationlock_req_proc(DIAG_ICD_F_req_type * pReq)	
{
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_set_screenorientationlock_rsp_type);
		
	printk(KERN_INFO "[ICD] %s req = %d\n", __func__,pReq->icd_req.set_screenorientationlock_req_info.orientation_mode);
	printk(KERN_INFO "[ICD] %s rsp_len :(%d)\n", __func__, rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SETSCREENORIENTATIONLOCK_REQ_CMD;
		
	// set screen orientation lock
	
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_SETORIENTATIONLOCK", pReq->icd_req.set_screenorientationlock_req_info.orientation_mode);
		mdelay(100);
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"SETORIENTATION_0") == 0)
			{
				pRsp->icd_rsp.set_screenorientation_rsp_info.cmd_status = 0;
				printk(KERN_INFO "[ICD] %s was successful\n",__func__);
			}
			else if(strcmp(process_value,"SETORIENTATION_1") == 0)
			{
				pRsp->icd_rsp.set_screenorientation_rsp_info.cmd_status = 1;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.set_screenorientation_rsp_info.cmd_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.set_screenorientation_rsp_info.cmd_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.set_screenorientation_rsp_info.cmd_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}

#if 0
extern int lge_bd_rev;
extern void msm_get_SW_VER_type(char* sw_ver);
byte CheckHWRev(byte *pStr)
{
	char *rev_str[] = {"evb1", "evb2", "A", "B", "C", "D",
		"E", "F", "G", "H", "1.0", "1.1", "1.2",
		"revserved"};
	
	strcpy((char *)pStr ,(char *)rev_str[lge_bd_rev]);
	return pStr[0];
}

DIAG_ICD_F_rsp_type* icd_info_req_proc(DIAG_ICD_F_req_type * pReq)
{
	byte* hw_rev;
	char* sw_ver;
	unsigned int rsp_len;
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_device_info_rsp_type);

	printk(KERN_INFO "[ICD] get icd_info_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);

	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETDEVICEINFO_REQ_CMD;

	//get manufacture info
	strcpy(pRsp->icd_rsp.dev_rsp_info.manf_string, "LGE");

	//get model name info
	strcpy(pRsp->icd_rsp.dev_rsp_info.model_string, "LS840");	

	CheckHWRev((byte*)pRsp->icd_rsp.dev_rsp_info.hw_ver_string);
	
	msm_get_SW_VER_type(sw_ver);
	strcpy(pRsp->icd_rsp.dev_rsp_info.sw_ver_string, sw_ver);
	
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_get_batterycharging_state_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_get_battery_charging_state_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterycharging_state_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_set_batterycharging_state_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_set_battery_charging_state_rsp_type);

	printk(KERN_INFO "[ICD] icd_set_batterycharging_state_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_get_batterylevel_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_get_battery_level_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	
	return pRsp;
}
#endif

DIAG_ICD_F_rsp_type* icd_emulate_pen_action_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_emulate_pen_action_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_get_4g_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_get_4g_status_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);

	//allen.song ADD
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GET4GSTATUS_REQ_CMD;

    // get 4G Status info
	// Customer Equipment shall return the integer value that indicates whether the 4G radio is turned on or off.(ICD requirement)
	// Set 4G Status ?Þ´??? ???? Set?? ?????Ï¸?  4G?? service???????? ?????Ï´? ???? ?Æ´Ï¶? 4G?? ??À» ?? ?Öµ??? ??Á¤?Ç¾? ?Ö´????? È®???Ï´? ??À¸?? ?Ç´Üµ?. 
	
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GET4GSTATUS", 1);
		mdelay(100);
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"GET4GSTATUS_0") == 0)
			{
				pRsp->icd_rsp.get_4g_status_rsp_info.status_4g = 0;
				printk(KERN_INFO "[ICD] %s was successful : 4GSTATUS is Off\n",__func__);
			}
			else if(strcmp(process_value,"GET4GSTATUS_1") == 0)
			{
				pRsp->icd_rsp.get_4g_status_rsp_info.status_4g = 1;
				printk(KERN_INFO "[ICD] %s was successful : 4GSTATUS is On\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.get_4g_status_rsp_info.status_4g = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.get_4g_status_rsp_info.status_4g = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.get_4g_status_rsp_info.status_4g = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}


	//allen.song END
	
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_set_4g_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_set_4g_status_rsp_type);

	printk(KERN_INFO "[ICD] icd_set_4g_status_req_proc \n");
	printk(KERN_INFO "[ICD] icd_set_4g_status_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);

    //allen.song ADD

	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SET4GSTATUS_REQ_CMD;

	// set airplane mode info
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);

		update_diagcmd_state(diagpdev, "ICD_SET4GSTATUS", pReq->icd_req.set_4g_status_req_info.status_4g);
		//mdelay(100);
        large_mdelay(700); //mukesh83.kumar@lge.com modified for set4GStatus failure

		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"SET4GSTATUS_0") == 0)
			{
				pRsp->icd_rsp.set_4g_status_rsp_info.cmd_status = 0;
				printk(KERN_INFO "[ICD] %s was successful\n",__func__);
			}
			else if(strcmp(process_value,"SET4GSTATUS_1") == 0)
			{
				pRsp->icd_rsp.set_4g_status_rsp_info.cmd_status = 1;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.set_4g_status_rsp_info.cmd_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.set_4g_status_rsp_info.cmd_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.set_4g_status_rsp_info.cmd_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}



	//allen.song END

	
	return pRsp;
}


DIAG_ICD_F_rsp_type* icd_get_usbdebug_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_get_usbdebug_status_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_usbdebug_status_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}	

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETUSBDEBUGSTATUSSTATUS_REQ_CMD;

	// get usbdebug status
	
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETUSBDEBUGSTATUS", 1); 
		mdelay(100);
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"GETUSBDEBUGSTATUS_0") == 0)
			{
				pRsp->icd_rsp.get_usbdebug_status_rsp_info.usbdebug_status = 0;
				printk(KERN_INFO "[ICD] %s was successful\n",__func__);
			}
			else if(strcmp(process_value,"GETUSBDEBUGSTATUS_1") == 0)
			{
				pRsp->icd_rsp.get_usbdebug_status_rsp_info.usbdebug_status = 1;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.get_usbdebug_status_rsp_info.usbdebug_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.get_usbdebug_status_rsp_info.usbdebug_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.get_usbdebug_status_rsp_info.usbdebug_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
		
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_set_usbdebug_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_set_usbdebug_status_rsp_type);

	printk(KERN_INFO "[ICD] icd_set_usbdebug_status_req_proc : req mode = %d\n",pReq->icd_req.set_usbdebug_status_req_info.usbdebug_status);
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}	

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SETUSBDEBUGSTATUSSTATUS_REQ_CMD;

	// set usbdebug status
	
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_SETUSBDEBUGSTATUS",  pReq->icd_req.set_usbdebug_status_req_info.usbdebug_status); 
		mdelay(100);
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"SETUSBDEBUGSTATUS_0") == 0)
			{
				pRsp->icd_rsp.set_usbdebug_status_rsp_info.cmd_status = 0;
				printk(KERN_INFO "[ICD] %s was successful\n",__func__);
			}
			else if(strcmp(process_value,"SETUSBDEBUGSTATUS_1") == 0)
			{
				pRsp->icd_rsp.set_usbdebug_status_rsp_info.cmd_status = 1;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.set_usbdebug_status_rsp_info.cmd_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid 1\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.set_usbdebug_status_rsp_info.cmd_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid 2\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.set_usbdebug_status_rsp_info.cmd_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_get_screenlock_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_get_screenlock_status_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_screenlock_status_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}	

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETSCREENLOCKSTATUS_REQ_CMD;

	// get screenlock status
	
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETSCREENLOCKSTATUS", 1); 
		mdelay(100);
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"GETSCREENLOCKSTATUS_0") == 0)
			{
				pRsp->icd_rsp.get_screenlock_status_rsp_info.screenlock_status = 0;
				printk(KERN_INFO "[ICD] %s was successful\n",__func__);
			}
			else if(strcmp(process_value,"GETSCREENLOCKSTATUS_1") == 0)
			{
				pRsp->icd_rsp.get_screenlock_status_rsp_info.screenlock_status = 1;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.get_screenlock_status_rsp_info.screenlock_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.get_screenlock_status_rsp_info.screenlock_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.get_screenlock_status_rsp_info.screenlock_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
		
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_set_screenlock_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_set_screenlock_status_rsp_type);

	printk(KERN_INFO "[ICD] icd_set_screenlock_status_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_set_screenlock_status_req_proc : req mode = %d\n",pReq->icd_req.set_screenlock_status_req_info.screenlock_status);
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}	

	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_SETSCREENLOCKSTATUS_REQ_CMD;

	// set screenlock status
	
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_SETSCREENLOCKSTATUS",  pReq->icd_req.set_screenlock_status_req_info.screenlock_status); 
		mdelay(100);
		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strcmp(process_value,"SETSCREENLOCKSTATUS_0") == 0)
			{
				pRsp->icd_rsp.set_screenlock_status_rsp_info.cmd_status = 0;
				printk(KERN_INFO "[ICD] %s was successful\n",__func__);
			}
			else if(strcmp(process_value,"SETSCREENLOCKSTATUS_1") == 0)
			{
				pRsp->icd_rsp.set_screenlock_status_rsp_info.cmd_status = 1;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else if(strcmp(process_value,"SETSCREENLOCKSTATUS_2") == 0)
			{
				pRsp->icd_rsp.set_screenlock_status_rsp_info.cmd_status = 2;
				printk(KERN_INFO "[ICD] %s was unsuccessful\n",__func__);
			}
			else
			{
				pRsp->icd_rsp.set_screenlock_status_rsp_info.cmd_status = 0xFF;
				printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
			}
		}
		else
		{
			pRsp->icd_rsp.set_screenlock_status_rsp_info.cmd_status = 0xFF;
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{
		pRsp->icd_rsp.set_screenlock_status_rsp_info.cmd_status = 0xFF;
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
			
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_get_apnlist_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_get_apnlist_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
   if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETAPNLIST_REQ_CMD;

   // get APN List
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETAPNLIST", 0);
		mdelay(100);

		if(strcmp(process_status,"COMPLETED") == 0)
		{
		   strcpy(pRsp->icd_rsp.get_apnlist_rsp_info.apnlist_string, process_value);	
		}
		else
		{			
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{		
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_get_latitude_longitude_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_get_latitude_longitude_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_get_technology_priority_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_get_technology_priority_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_set_technology_priority_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_set_technology_priority_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_get_hrpd_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_get_hrpd_status_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_set_hrpd_status_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_set_hrpd_status_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	
	return pRsp;
}

/* START 2011.11.13 jaeho.cho@lge.com to get USB serial number */
#if 1//def CONFIG_LGE_USB_EXPORT_SERIAL_NUMBER
extern void android_get_serial_number(char *snum);
#endif
/* END 2011.11.13 jaeho.cho@lge.com to get USB serial number */
DIAG_ICD_F_rsp_type* icd_get_android_identifier_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;
/* START 2011.11.13 jaeho.cho@lge.com to get USB serial number */
#if 1//def CONFIG_LGE_USB_EXPORT_SERIAL_NUMBER
	char serial_number[256];

	android_get_serial_number(serial_number);
#endif
/* END 2011.11.13 jaeho.cho@lge.com to get USB serial number */
	rsp_len = sizeof(icd_get_android_identifier_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
//[LS840_ICS]::Fix for WBT Issue::TD:388291
	if (pRsp == NULL) {
                printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
                return pRsp;
        }
//[LS840_ICS]::Fix for WBT Issue::TD:388291

/* START 2011.11.13 jaeho.cho@lge.com to get USB serial number */
#if 1//def CONFIG_LGE_USB_EXPORT_SERIAL_NUMBER
	pRsp->hdr.cmd_code = 0xF6;
	pRsp->hdr.sub_cmd = 0x47;
	snprintf(pRsp->icd_rsp.get_android_identifier_rsp_info.android_id_string, VARIABLE, serial_number);	
#endif
/* END 2011.11.13 jaeho.cho@lge.com to get USB serial number */
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_start_ip_capture_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_start_ip_capture_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
   if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_STARTIPCAPTURE_REQ_CMD;
   
	pRsp->icd_rsp.start_ip_capture_rsp_info.cmd_status = 0;
	return pRsp;
}

DIAG_ICD_F_rsp_type* icd_stop_ip_capture_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_stop_ip_capture_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
   if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_STOPIPCAPTURE_REQ_CMD;
	
	return pRsp;
}

#if 0
DIAG_ICD_F_rsp_type* icd_incomming_ip_message_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_incomming_ip_message_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
	
	return pRsp;
}
#endif
DIAG_ICD_F_rsp_type* icd_get_interface_ip_address_req_proc(DIAG_ICD_F_req_type * pReq)
{
	unsigned int rsp_len;	
	DIAG_ICD_F_rsp_type *pRsp;

	rsp_len = sizeof(icd_get_interface_ip_address_rsp_type);

	printk(KERN_INFO "[ICD] icd_get_batterylevel_req_proc req \n");
	printk(KERN_INFO "[ICD] icd_info_req_proc rsp_len :(%d)\n", rsp_len);

	pRsp = (DIAG_ICD_F_rsp_type *) diagpkt_alloc(DIAG_ICD_F, rsp_len);
   if (pRsp == NULL) {
		printk(KERN_ERR "[ICD] diagpkt_alloc failed\n");
		return pRsp;
	}
	
	pRsp->hdr.cmd_code = DIAG_ICD_F;
	pRsp->hdr.sub_cmd = ICD_GETINTERFACEIPADDRESS_REQ_CMD;

	// get interface IP address
	if(diagpdev != NULL)
	{
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather\n",__func__);
		update_diagcmd_state(diagpdev, "ICD_GETINTERFACEIPADDRESS", pReq->icd_req.get_interface_ip_address_req_info.type);
		mdelay(100);

		if(strcmp(process_status,"COMPLETED") == 0)
		{
			if(strchr(process_value,'.') != NULL)
			{
			    if(inet_pton4(process_value, pRsp->icd_rsp.get_interface_ip_address_rsp_info.ip_address) < 1) {
                  printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);              
              }
              else
              {
                  printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is successful\n",__func__); 
              }			    
           }
           else if(strchr(process_value,':') != NULL)
           {
              if(inet_pton6(process_value, pRsp->icd_rsp.get_interface_ip_address_rsp_info.ip_address) < 1) {
                  printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);              
              }
              else
              {
                  printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is successful\n",__func__); 
              }		
           }
           else
           {              
			    printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);              
           }
		}
		else
		{			
			printk(KERN_INFO "[ICD] %s return value from DiagCommandDispather is invalid\n",__func__);
		}
	}
	else
	{		
		printk(KERN_INFO "[ICD] %s goto DiagCommandDispather : Error cannot open diagpdev\n",__func__);
	}
	
	return pRsp;
}

/** ICDR : ICD Implementation Recommendation  - END **/

/*  USAGE (same as testmode
 *    1. If you want to handle at ARM9 side, 
 *       you have to insert fun_ptr as NULL and mark ARM9_PROCESSOR
 *    2. If you want to handle at ARM11 side , 
 *       you have to insert fun_ptr as you want and mark AMR11_PROCESSOR.
 */
icd_user_table_entry_type icd_mstr_tbl[ICD_MSTR_TBL_SIZE] = {
	/*sub_command								fun_ptr									which procesor*/
	/** SAR : Sprint Automation Requirement - START **/
//	{ICD_GETDEVICEINFO_REQ_CMD, 				NULL, 									ICD_ARM9_PROCESSOR},
	{ICD_GETDEVICEINFO_REQ_CMD, 				icd_info_req_proc, 						ICD_ARM11_PROCESSOR},
	{ICD_EXTENDEDVERSIONINFO_REQ_CMD,			icd_extended_info_req_proc, 			ICD_ARM11_PROCESSOR},
	{ICD_HANDSETDISPLAYTEXT_REQ_CMD,			icd_handset_disp_text_req_proc,		 	ICD_ARM11_PROCESSOR},
	{ICD_CAPTUREIMAGE_REQ_CMD,					icd_capture_img_req_proc,				ICD_ARM11_PROCESSOR},
	/** SAR : Sprint Automation Requirement - END **/

	/** ICDR : ICD Implementation Recommendation  - START **/
	{ICD_GETAIRPLANEMODE_REQ_CMD,				icd_get_airplanemode_req_proc,			ICD_ARM11_PROCESSOR},
	{ICD_SETAIRPLANEMODE_REQ_CMD,				icd_set_airplanemode_req_proc,			ICD_ARM11_PROCESSOR},
	{ICD_GETBACKLIGHTSETTING_REQ_CMD,			icd_get_backlight_setting_req_proc,		ICD_ARM11_PROCESSOR},
	{ICD_SETBACKLIGHTSETTING_REQ_CMD,			icd_set_backlight_setting_req_proc,		ICD_ARM11_PROCESSOR},
//	{ICD_GETBATTERYCHARGINGSTATE_REQ_CMD,		NULL,									ICD_ARM9_PROCESSOR},
//	{ICD_SETBATTERYCHARGINGSTATE_REQ_CMD,		NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETBATTERYCHARGINGSTATE_REQ_CMD,		icd_get_batterycharging_state_req_proc,	ICD_ARM11_PROCESSOR},
	{ICD_SETBATTERYCHARGINGSTATE_REQ_CMD,		icd_set_batterycharging_state_req_proc,	ICD_ARM11_PROCESSOR},
//	{ICD_GETBATTERYLEVEL_REQ_CMD,				NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETBATTERYLEVEL_REQ_CMD,				icd_get_battery_level_req_proc,			ICD_ARM11_PROCESSOR},
	{ICD_GETBLUETOOTHSTATUS_REQ_CMD,			icd_get_bluetooth_status_req_proc,		ICD_ARM11_PROCESSOR},
	{ICD_SETBLUETOOTHSTATUS_REQ_CMD,			icd_set_bluetooth_status_req_proc,		ICD_ARM11_PROCESSOR},
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET GPS STATUS_10JUL_sandeep.pawar@lge.com_START*/
	{ICD_GETGPSSTATUS_REQ_CMD,					icd_get_gps_status_req_proc,			ICD_ARM11_PROCESSOR},
	{ICD_SETGPSSTATUS_REQ_CMD,					icd_set_gps_status_req_proc,			ICD_ARM11_PROCESSOR},
/*LGSI_CMF_LS840_ICS_SLATE_SET/GET GPS STATUS_10JUL_sandeep.pawar@lge.com_START*/
	{ICD_GETKEYPADBACKLIGHT_REQ_CMD,			icd_get_keypadbacklight_req_proc,						ICD_ARM11_PROCESSOR},
	{ICD_SETKEYPADBACKLIGHT_REQ_CMD,			icd_set_keypadbacklight_req_proc,						ICD_ARM11_PROCESSOR},
	//{ICD_GETROAMINGMODE_REQ_CMD,				NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETROAMINGMODE_REQ_CMD,				icd_get_roamingmode_req_proc,			ICD_ARM11_PROCESSOR},
	{ICD_GETSTATEANDCONNECTIONATTEMPTS_REQ_CMD,	NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETUISCREENID_REQ_CMD,					icd_get_ui_screen_id_req_proc,			ICD_ARM11_PROCESSOR},
	{ICD_GETWIFISTATUS_REQ_CMD,					icd_get_wifi_status_req_proc,			ICD_ARM11_PROCESSOR},
	{ICD_SETWIFISTATUS_REQ_CMD,					icd_set_wifi_status_req_proc,			ICD_ARM11_PROCESSOR},
	{ICD_SETDISCHARGING_REQ_CMD, 				icd_set_discharge_status_req_proc,		ICD_ARM11_PROCESSOR},	
	{ICD_SETSCREENORIENTATIONLOCK_REQ_CMD,		icd_set_screenorientationlock_req_proc,	ICD_ARM11_PROCESSOR},
	{ICD_GETRSSI_REQ_CMD,						icd_get_rssi_req_proc,					ICD_ARM11_PROCESSOR},

#if 0
	{ICD_EMULATEPENACTION_REQ_CMD,				NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GET4GSTATUS_REQ_CMD,					NULL,									ICD_ARM9_PROCESSOR},
	{ICD_SET4GSTATUS_REQ_CMD,					NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETUSBDEBUGSTATUSSTATUS_REQ_CMD,		NULL,									ICD_ARM9_PROCESSOR},
	{ICD_SETUSBDEBUGSTATUSSTATUS_REQ_CMD,		NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETLATITUDELONGITUDEVALUES_REQ_CMD,	NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETSCREENLOCKSTATUS_REQ_CMD,			NULL,									ICD_ARM9_PROCESSOR},
	{ICD_SETSCREENLOCKSTATUS_REQ_CMD,			NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETAPNLIST_REQ_CMD,					NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETTECHNOLOGYPRIORITY_REQ_CMD,			NULL,									ICD_ARM9_PROCESSOR},
	{ICD_SETTECHNOLOGYPRIORITY_REQ_CMD,			NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETHRPDSTATUS_REQ_CMD,					NULL,									ICD_ARM9_PROCESSOR},
	{ICD_SETHRPDSTATUS_REQ_CMD,					NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETANDORIDIDENTIFIER_REQ_CMD,			NULL,									ICD_ARM9_PROCESSOR},
	{ICD_STARTIPCAPTURE_REQ_CMD,				NULL,									ICD_ARM9_PROCESSOR},
	{ICD_STOPIPCAPTURE_REQ_CMD,					NULL,									ICD_ARM9_PROCESSOR},
	{ICD_INCOMMINGIPMESSAGE_REQ_CMD,			NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETINTERFACEIPADDRESS_REQ_CMD,			NULL,									ICD_ARM9_PROCESSOR},
#else
	{ICD_EMULATEPENACTION_REQ_CMD,				icd_emulate_pen_action_req_proc,		ICD_ARM11_PROCESSOR},
	{ICD_GET4GSTATUS_REQ_CMD,					icd_get_4g_status_req_proc,				ICD_ARM11_PROCESSOR},
	{ICD_SET4GSTATUS_REQ_CMD,					icd_set_4g_status_req_proc,				ICD_ARM11_PROCESSOR},
	{ICD_GETUSBDEBUGSTATUSSTATUS_REQ_CMD,		icd_get_usbdebug_status_req_proc,		ICD_ARM11_PROCESSOR},
	{ICD_SETUSBDEBUGSTATUSSTATUS_REQ_CMD,		icd_set_usbdebug_status_req_proc,		ICD_ARM11_PROCESSOR},
	//{ICD_GETLATITUDELONGITUDEVALUES_REQ_CMD,	icd_get_latitude_longitude_req_proc,	ICD_ARM11_PROCESSOR},
	{ICD_GETLATITUDELONGITUDEVALUES_REQ_CMD,	NULL								,	ICD_ARM9_PROCESSOR},
	{ICD_GETSCREENLOCKSTATUS_REQ_CMD,			icd_get_screenlock_status_req_proc,		ICD_ARM11_PROCESSOR},
	{ICD_SETSCREENLOCKSTATUS_REQ_CMD,			icd_set_screenlock_status_req_proc,		ICD_ARM11_PROCESSOR},
	{ICD_GETAPNLIST_REQ_CMD,					icd_get_apnlist_req_proc,				ICD_ARM11_PROCESSOR},
	{ICD_GETTECHNOLOGYPRIORITY_REQ_CMD,			icd_get_technology_priority_req_proc,	ICD_ARM11_PROCESSOR},
	{ICD_SETTECHNOLOGYPRIORITY_REQ_CMD,			icd_set_technology_priority_req_proc,	ICD_ARM11_PROCESSOR},
	//{ICD_GETHRPDSTATUS_REQ_CMD,					icd_get_hrpd_status_req_proc,			ICD_ARM11_PROCESSOR},
	//{ICD_SETHRPDSTATUS_REQ_CMD,					icd_set_hrpd_status_req_proc,			ICD_ARM11_PROCESSOR},
	{ICD_GETHRPDSTATUS_REQ_CMD,					NULL,									ICD_ARM9_PROCESSOR},
	{ICD_SETHRPDSTATUS_REQ_CMD,					NULL,									ICD_ARM9_PROCESSOR},
	{ICD_GETANDORIDIDENTIFIER_REQ_CMD,			icd_get_android_identifier_req_proc,	ICD_ARM11_PROCESSOR},
	{ICD_STARTIPCAPTURE_REQ_CMD,				icd_start_ip_capture_req_proc,			ICD_ARM11_PROCESSOR},
	{ICD_STOPIPCAPTURE_REQ_CMD,					icd_stop_ip_capture_req_proc,			ICD_ARM11_PROCESSOR},
//	{ICD_INCOMMINGIPMESSAGE_REQ_CMD,			NULL,									ICD_ARM11_PROCESSOR},
	{ICD_GETINTERFACEIPADDRESS_REQ_CMD,			icd_get_interface_ip_address_req_proc,	ICD_ARM11_PROCESSOR},
#endif
	/** ICDR : ICD Implementation Recommendation  - END **/
};
