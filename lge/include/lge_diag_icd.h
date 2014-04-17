#ifndef DIAGICD_H
#define DIAGICD_H


/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
#include "lg_comdef.h"
#include <linux/crc-ccitt.h>
#include <linux/delay.h>

/*===========================================================================

                      EXTERNAL FUNCTION AND VARIABLE DEFINITIONS

===========================================================================*/


/*===========================================================================

            LOCAL DEFINITIONS AND DECLARATIONS FOR MODULE

  This section contains local definitions for constants, macros, types,
  variables and other items needed by this module.

===========================================================================*/
/*********************** BEGIN PACK() Definition ***************************/
#if defined __GNUC__
#define PACK(x)       x __attribute__((__packed__))
#define PACKED        __attribute__((__packed__))
#elif defined __arm
#define PACK(x)       __packed x
#define PACKED        __packed
#else
#error No PACK() macro defined for this compiler
#endif
/********************** END PACK() Definition *****************************/
#if 1 
#define ICD_LCD_MAIN_WIDTH   480 /*Cayman : 480	  VS920 : 736		VS910 : 480	VS660 : 320 */
#define ICD_LCD_MAIN_HEIGHT  800 /*Cayman : 800	  VS920 : 1280	VS910 : 800	VS660 : 480 */
#else
#define ICD_LCD_MAIN_WIDTH   320 /*Cayman : 480	  VS920 : 736		VS910 : 480	VS660 : 320 */
#define ICD_LCD_MAIN_HEIGHT  480 /*Cayman : 800	  VS920 : 1280	VS910 : 800	VS660 : 480 */
#endif
#define ICD_MAX_STRING 10
#define ICD_DISP_TEXT_MAX_STRING 300
#define ICD_MSTR_TBL_SIZE 0xFF
#define ICD_SEND_BUF_SIZE		 65536	
#define ICD_SEND_SAVE_IMG_PATH_LEN 	20
#define ICD_SCRN_BUF_SIZE_MAX	 ICD_LCD_MAIN_WIDTH * ICD_LCD_MAIN_HEIGHT * 2 // icd_pixel_16_type, 2bytes

#define VARIABLE		50

typedef enum
{
	/** SAR : Sprint Automation Requirement - START **/
	ICD_GETDEVICEINFO_REQ_CMD						=0x01,	//Auto-025, Auto-027, Auto-030, Auto-040
	ICD_EXTENDEDVERSIONINFO_REQ_CMD					=0x02,	//Auto-222, Auto-223
	ICD_HANDSETDISPLAYTEXT_REQ_CMD					=0x03,	//Auto-224, Auto-225
	ICD_CAPTUREIMAGE_REQ_CMD						=0x04,	//Auto-015, Auto-226
	/** SAR : Sprint Automation Requirement - END **/

	/** ICDR : ICD Implementation Recommendation  - START **/
	ICD_GETAIRPLANEMODE_REQ_CMD						=0x20,	//Auto-016
	ICD_SETAIRPLANEMODE_REQ_CMD						=0x21,	//Auto-051
	ICD_GETBACKLIGHTSETTING_REQ_CMD					=0x22,	//Auto-017
	ICD_SETBACKLIGHTSETTING_REQ_CMD					=0x23,	//Auto-052
	ICD_GETBATTERYCHARGINGSTATE_REQ_CMD				=0x24,	//Auto-018
	ICD_SETBATTERYCHARGINGSTATE_REQ_CMD				=0x25,	//Auto-054
	ICD_GETBATTERYLEVEL_REQ_CMD						=0x26,	//Auto-019
	ICD_GETBLUETOOTHSTATUS_REQ_CMD					=0x27,	//Auto-020
	ICD_SETBLUETOOTHSTATUS_REQ_CMD					=0x28,	//Auto-053
	ICD_GETGPSSTATUS_REQ_CMD						=0x29,	//Auto-024
	ICD_SETGPSSTATUS_REQ_CMD						=0x2A,	//Auto-055
	ICD_GETKEYPADBACKLIGHT_REQ_CMD					=0x2B,	//Auto-026
	ICD_SETKEYPADBACKLIGHT_REQ_CMD					=0x2C,	//Auto-056
	ICD_GETROAMINGMODE_REQ_CMD						=0x30,	//Auto-037
	ICD_GETSTATEANDCONNECTIONATTEMPTS_REQ_CMD		=0x32,	//Auto-042
	ICD_GETUISCREENID_REQ_CMD						=0x33,	//Auto-204 ~ Auto214
	ICD_GETWIFISTATUS_REQ_CMD						=0x35,	//Auto-045
	ICD_SETWIFISTATUS_REQ_CMD						=0x36,	//Auto-059
	ICD_SETDISCHARGING_REQ_CMD						=0x37,	
	ICD_SETSCREENORIENTATIONLOCK_REQ_CMD			=0x38,	//
	ICD_GETRSSI_REQ_CMD								=0x39,	//Auto-038
	/** ICDR : ICD Implementation Recommendation  - END **/
	ICD_EMULATEPENACTION_REQ_CMD					=0x3A,
	ICD_GET4GSTATUS_REQ_CMD							=0x3B,
	ICD_SET4GSTATUS_REQ_CMD							=0x3C,
	ICD_GETUSBDEBUGSTATUSSTATUS_REQ_CMD				=0x3D,	
	ICD_SETUSBDEBUGSTATUSSTATUS_REQ_CMD				=0x3E,
	ICD_GETLATITUDELONGITUDEVALUES_REQ_CMD			=0x3F,
	ICD_GETSCREENLOCKSTATUS_REQ_CMD					=0x40,
	ICD_SETSCREENLOCKSTATUS_REQ_CMD					=0x41,
	ICD_GETAPNLIST_REQ_CMD							=0x42,
	ICD_GETTECHNOLOGYPRIORITY_REQ_CMD				=0x43,
	ICD_SETTECHNOLOGYPRIORITY_REQ_CMD				=0x44,
	ICD_GETHRPDSTATUS_REQ_CMD						=0x45,
	ICD_SETHRPDSTATUS_REQ_CMD						=0x46,
	ICD_GETANDORIDIDENTIFIER_REQ_CMD				=0x47,
	ICD_STARTIPCAPTURE_REQ_CMD						=0x48,
	ICD_STOPIPCAPTURE_REQ_CMD						=0x49,
	ICD_INCOMMINGIPMESSAGE_REQ_CMD					=0x50,
	ICD_GETINTERFACEIPADDRESS_REQ_CMD				=0x51,	

	ICD_MAX_REQ_CMD									=0xff,
}icd_sub_cmd_type;

/** ICD requset type**/
typedef struct
{
	unsigned char cmd_code;
	unsigned char sub_cmd;
} PACKED icd_req_hdr_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char screen_id;
} PACKED icd_handset_disp_text_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char screen_id;
	unsigned short expected_width;
	unsigned short expected_height;
	unsigned short upper_left_x;
	unsigned short upper_left_y;
	unsigned short lower_right_x;
	unsigned short lower_right_y;
	unsigned char bit_per_pixel;
	unsigned short seq_num;
} PACKED icd_screen_capture_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char airplane_mode;
} PACKED icd_set_airplane_mode_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned short item_data;
} PACKED icd_set_backlight_setting_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char charging_state;
} PACKED icd_set_battery_charging_state_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char bluetooth_status;
} PACKED icd_set_bluetooth_status_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char gps_status;
} PACKED icd_set_gps_status_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned short keypad_backlight;
} PACKED icd_set_keypadbacklight_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char physical_screen;
} PACKED icd_get_ui_screen_id_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char wifi_status;
} PACKED icd_set_wifi_status_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char orientation_mode;
} PACKED icd_set_screen_orientationlock_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	uint32 off_time;
}PACKED icd_set_discharger_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char display_index;
	unsigned short x;
	unsigned short y;
	unsigned char action;
}PACKED icd_emulate_pen_action_req_type;

typedef struct {
	icd_req_hdr_type hdr;
}PACKED icd_get_4g_status_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char status_4g;
}PACKED icd_set_4g_status_req_type;	

typedef struct {
	icd_req_hdr_type hdr;
}PACKED icd_get_usbdebug_status_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char usbdebug_status;
}PACKED icd_set_usbdebug_status_req_type;

typedef struct {
	icd_req_hdr_type hdr;
}PACKED icd_get_screenlock_status_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char msl[6];
	unsigned char screenlock_status;
}PACKED icd_set_screenlock_status_req_type;

typedef struct {
	icd_req_hdr_type hdr;
}PACKED icd_get_apnlist_req_type;

typedef struct {
	icd_req_hdr_type hdr;
}PACKED icd_get_latitude_longitude_req_type;

typedef struct {
	icd_req_hdr_type hdr;
}PACKED icd_get_technology_priority_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	char priority_string[VARIABLE];
}PACKED icd_set_technology_priority_req_type;

typedef struct {
	icd_req_hdr_type hdr;
}PACKED icd_get_hrpd_status_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char hrpd_status;
}PACKED icd_set_hrpd_status_req_type;

typedef struct {
	icd_req_hdr_type hdr;
}PACKED icd_get_android_identifier_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char msl[6];
	unsigned char type;
}PACKED icd_start_ip_capture_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char type;
}PACKED icd_stop_ip_capture_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char type;
	unsigned char time_stamp[8];
	char ip_message[VARIABLE];
}PACKED icd_incomming_ip_message_req_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char msl[8];
	unsigned char type;
}PACKED icd_get_interface_ip_address_req_type;

typedef union
{
	icd_handset_disp_text_req_type disp_req_info;
	icd_screen_capture_req_type capture_req_info;
	icd_set_airplane_mode_req_type set_aiplane_mode_req_info;
	icd_set_backlight_setting_req_type set_backlight_setting_req_info;
	icd_set_battery_charging_state_req_type set_battery_charging_state_req_info;
	icd_set_bluetooth_status_req_type set_bluetooth_status_req_info;
	icd_set_gps_status_req_type set_gps_status_req_info;
	icd_set_keypadbacklight_req_type set_keypadbacklight_req_info;
	icd_get_ui_screen_id_req_type get_ui_srceen_id_req_info;
	icd_set_wifi_status_req_type set_wifi_status_req_info;
	icd_set_screen_orientationlock_req_type set_screenorientationlock_req_info;
	icd_set_discharger_req_type set_discharger_req_info;
	
	icd_emulate_pen_action_req_type emulate_pen_action_req_info;
	icd_get_4g_status_req_type get_4g_status_req_info;
	icd_set_4g_status_req_type set_4g_status_req_info;
	icd_get_usbdebug_status_req_type get_usbdebug_status_req_info;
	icd_set_usbdebug_status_req_type set_usbdebug_status_req_info;
	icd_get_screenlock_status_req_type get_screenlock_status_req_info;
	icd_set_screenlock_status_req_type set_screenlock_status_req_info;
	icd_get_apnlist_req_type get_apnlist_req_info;
	icd_get_latitude_longitude_req_type get_latitude_longitude_req_info;
	icd_get_technology_priority_req_type get_technology_priority_req_info;
	icd_set_technology_priority_req_type set_technology_priority_req_info;
	icd_get_hrpd_status_req_type get_hrpd_status_req_info;
	icd_set_hrpd_status_req_type set_hrpd_status_req_info;
	icd_get_android_identifier_req_type get_android_identifier_req_info;
	icd_start_ip_capture_req_type start_ip_capture_req_info;
	icd_stop_ip_capture_req_type stop_ip_capture_req_info;
	icd_incomming_ip_message_req_type incomming_ip_message_req_info;
	icd_get_interface_ip_address_req_type get_interface_ip_address_req_info;
	
} PACKED icd_req_type;

typedef union{
	icd_req_hdr_type hdr;
	icd_req_type icd_req;
} PACKED DIAG_ICD_F_req_type;
/** ICD requset type**/

/** ICD response type**/
typedef struct {
	icd_req_hdr_type hdr;
	char manf_string[ICD_MAX_STRING];
	char model_string[ICD_MAX_STRING];
	char hw_ver_string[ICD_MAX_STRING];
	char sw_ver_string[ICD_MAX_STRING];
} PACKED icd_device_info_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	char ver_string[ICD_DISP_TEXT_MAX_STRING];
} PACKED icd_extended_info_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char screen_id;
	//char text_string[ICD_DISP_TEXT_MAX_STRING];
} PACKED icd_handset_disp_text_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char screen_id;
	unsigned short actual_width;
	unsigned short actual_height;	
	unsigned short upper_left_x;
	unsigned short upper_left_y;
	unsigned short lower_right_x;
	unsigned short lower_right_y;
	unsigned char flags;
	unsigned char bit_per_pixel;
	unsigned short seq_num;
//	byte image_data_block[ICD_SEND_BUF_SIZE];
	byte image_data_block[ICD_SEND_SAVE_IMG_PATH_LEN];
} PACKED icd_screen_capture_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char airplane_mode;
} PACKED icd_get_airplane_mode_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
} PACKED icd_set_airplane_mode_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned short item_data;
} PACKED icd_get_backlight_setting_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
} PACKED icd_set_backlight_setting_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char charging_state;
} PACKED icd_get_battery_charging_state_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
} PACKED icd_set_battery_charging_state_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char battery_level;
} PACKED icd_get_battery_level_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char bluetooth_status;
} PACKED icd_get_bluetooth_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
} PACKED icd_set_bluetooth_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char gps_status;
} PACKED icd_get_gps_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
} PACKED icd_set_gps_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned short keypad_backlight;
} PACKED icd_get_keypadbacklight_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
} PACKED icd_set_keypadbacklight_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char roaming_mode;
} PACKED icd_get_roamingmode_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char status;
	unsigned char numbar;
	unsigned short rx_power;
	unsigned short rx_ec_io;
} PACKED icd_get_rssi_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char at_state;
	unsigned char session_state;
	unsigned char almp_state;
	unsigned char init_state;
	unsigned char idle_state;
	unsigned char conn_state;
	unsigned char rup_state;
	unsigned char ovhd_state;
	char hybrid_mode;
	
	unsigned char trans_id;
	unsigned char msg_seq;
	unsigned char result;
	unsigned short duration;
	unsigned int success_count;
	unsigned int failure_count;
	unsigned int attempts_count;
	unsigned short pn;
	unsigned short sector_id_lsw;
	unsigned char sector_id_usw;
	unsigned char color_code;
	unsigned char num_ho;
} PACKED icd_get_state_connect_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char physical_screen;
	unsigned char ui_screen_id;
} PACKED icd_get_ui_screen_id_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char wifi_status;
} PACKED icd_get_wifi_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
} PACKED icd_set_wifi_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
} PACKED icd_set_screenorientationlock_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
} PACKED icd_set_discharger_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
}PACKED icd_emulate_pen_action_rsp_type;
	
typedef struct {
	icd_req_hdr_type hdr;
	unsigned char status_4g;
}PACKED icd_get_4g_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
}PACKED icd_set_4g_status_rsp_type;	

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char usbdebug_status;
}PACKED icd_get_usbdebug_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
}PACKED icd_set_usbdebug_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char screenlock_status;	
}PACKED icd_get_screenlock_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
}PACKED icd_set_screenlock_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	char apnlist_string[VARIABLE];
}PACKED icd_get_apnlist_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
#if 1 //LG_SYS_SPRINT_ICD heeyoung.lee - 2011.12.05
	char lati_longi_value[50];
#endif
}PACKED icd_get_latitude_longitude_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	char priority_string[VARIABLE];	
}PACKED icd_get_technology_priority_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
}PACKED icd_set_technology_priority_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char hrpd_status;	
}PACKED icd_get_hrpd_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
}PACKED icd_set_hrpd_status_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	char android_id_string[VARIABLE];
}PACKED icd_get_android_identifier_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
	unsigned char cmd_status;
}PACKED icd_start_ip_capture_rsp_type;

typedef struct {
	icd_req_hdr_type hdr;
}PACKED icd_stop_ip_capture_rsp_type;

#if 0
typedef struct {
	icd_req_hdr_type hdr;
	unsigned char type;
	unsigned char time_stamp[8];
	char ip_message[VARIABLE];
}PACKED icd_incomming_ip_message_rsp_type;
#endif

typedef struct {
	icd_req_hdr_type hdr;
	char ip_address[16];
}PACKED icd_get_interface_ip_address_rsp_type;

typedef union
{
	icd_device_info_rsp_type dev_rsp_info;
	icd_extended_info_rsp_type extended_rsp_info;
	icd_handset_disp_text_rsp_type disp_rsp_info;
	icd_screen_capture_rsp_type capture_rsp_info;
	icd_get_airplane_mode_rsp_type get_airplane_mode_rsp_info;
	icd_set_airplane_mode_rsp_type set_airplane_mode_rsp_info;
	icd_get_backlight_setting_rsp_type get_backlight_setting_rsp_info;
	icd_set_backlight_setting_rsp_type set_backlight_setting_rsp_info;
	icd_get_battery_charging_state_rsp_type get_battery_charging_state_rsp_info;
	icd_set_battery_charging_state_rsp_type set_battery_charging_state_rsp_info;
	icd_get_battery_level_rsp_type get_battery_level_rsp_info;
	icd_get_bluetooth_status_rsp_type get_bluetooth_status_rsp_info;
	icd_set_bluetooth_status_rsp_type set_bluetooth_status_rsp_info;
	icd_get_gps_status_rsp_type get_gps_status_rsp_info;
	icd_set_gps_status_rsp_type set_gps_status_rsp_info;
	icd_get_keypadbacklight_rsp_type get_keypadbacklight_rsp_info;
	icd_set_keypadbacklight_rsp_type set_keypadbacklight_rsp_info;
	icd_get_roamingmode_rsp_type get_roamingmode_rsp_info;
	icd_get_rssi_rsp_type get_rssi_rsp_info;
	icd_get_state_connect_rsp_type get_state_connect_rsp_info;
	icd_get_ui_screen_id_rsp_type get_ui_screen_id_rsp_info;
	icd_get_wifi_status_rsp_type get_wifi_status_rsp_info;
	icd_set_wifi_status_rsp_type set_wifi_status_rsp_info;
	icd_set_screenorientationlock_rsp_type set_screenorientation_rsp_info;
	icd_set_discharger_rsp_type set_discharger_rsp_info;

	icd_emulate_pen_action_rsp_type emulate_pen_action_rsp_info;
	icd_get_4g_status_rsp_type get_4g_status_rsp_info;
	icd_set_4g_status_rsp_type set_4g_status_rsp_info;
	icd_get_usbdebug_status_rsp_type get_usbdebug_status_rsp_info;
	icd_set_usbdebug_status_rsp_type set_usbdebug_status_rsp_info;
	icd_get_screenlock_status_rsp_type get_screenlock_status_rsp_info;
	icd_set_screenlock_status_rsp_type set_screenlock_status_rsp_info;
	icd_get_apnlist_rsp_type get_apnlist_rsp_info;
	icd_get_latitude_longitude_rsp_type get_latitude_longitude_rsp_info;
	icd_get_technology_priority_rsp_type get_technology_priority_rsp_info;
	icd_set_technology_priority_rsp_type set_technology_priority_rsp_info;
	icd_get_hrpd_status_rsp_type get_hrpd_status_rsp_info;
	icd_set_hrpd_status_rsp_type set_hrpd_status_rsp_info;
	icd_get_android_identifier_rsp_type get_android_identifier_rsp_info;
	icd_start_ip_capture_rsp_type start_ip_capture_rsp_info;
	icd_stop_ip_capture_rsp_type stop_ip_capture_rsp_info;
	icd_get_interface_ip_address_rsp_type get_interface_ip_address_rsp_info;
} PACKED icd_rsp_type;

typedef union
{
	icd_req_hdr_type hdr;
	icd_rsp_type icd_rsp;
} PACKED DIAG_ICD_F_rsp_type;
/** ICD response type**/

// define which processor will handle the sub commands
#if !defined (ARM9_PROCESSOR) && !defined (ARM11_PROCESSOR)
typedef enum{
	ICD_ARM9_PROCESSOR = 0,
	ICD_ARM11_PROCESSOR = 1,
	ICD_ARM9_ARM11_BOTH = 2,
	ICD_NOT_SUPPORTED = 0xFF
}icd_which_processor_type;
#endif

typedef DIAG_ICD_F_rsp_type*(* icd_func_type)(DIAG_ICD_F_req_type *);

typedef struct
{
	unsigned short cmd_code;
	icd_func_type func_ptr;
	unsigned char which_procesor;             // to choose which processor will do act.
}icd_user_table_entry_type;

/*===========================================================================
                      INTERNAL FUNCTION DEFINITIONS
===========================================================================*/
typedef enum
{
	LastImageNoHeader				= 0x00,
	LastImageHeader					= 0x01,		
	ContinueImageNoHeader			= 0x02,
	ContinueImageHeader				= 0x03,	
}icd_rsp_flag;

#if 1 //SPRINT_SLATE_KEYPRESS_TEST
typedef struct key_message{
	byte cmd_code ;
	byte ts_type;
	byte num_args;
	byte drop_cnt;
	dword time[2];
	word line_number;
	word ss_id;
	//word ss_mask;
	dword ss_mask;
	dword args[2];
	char code[13];
	char file_name[13];
} PACKED key_msg_type;

typedef struct touch_message{
	byte cmd_code ;
	byte ts_type;
	byte num_args;
	byte drop_cnt;
	dword time[2];
	word line_number;
	word ss_id;
	//word ss_mask;
	dword ss_mask;
	dword args[3];
	char code[15];
	char file_name[13];
} PACKED touch_msg_type;

#define LGE_DIAG_ICD_LOGGING_SSID			4604
#define LGE_DIAG_ICD_LOGGING_SSID_MASK		1
#endif //SPRINT_SLATE_KEYPRESS_TEST


#if 1 //SPRINT_SLATE_KEYPRESS_TEST
#define ESC_CHAR     0x7D
#define CONTROL_CHAR 0x7E
#define ESC_MASK     0x20

#define CRC_16_L_SEED           0xFFFF

#define CRC_16_L_STEP(xx_crc, xx_c) \
	crc_ccitt_byte(xx_crc, xx_c)

#endif //SPRINT_SLATE_KEYPRESS_TEST

#endif /* DIAGMTC_H */
