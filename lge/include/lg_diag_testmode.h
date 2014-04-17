#ifndef LG_DIAG_TESTMODE_H
#define LG_DIAG_TESTMODE_H
// LG_FW_DIAG_KERNEL_SERVICE

#include "lg_comdef.h"


/*********************** BEGIN PACK() Definition ***************************/
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
/********************** END PACK() Definition *****************************/


/* BEGIN: 0014654 jihoon.lee@lge.com 20110124 */
/* MOD 0014654: [TESTMODE] SYNC UP TESTMODE PACKET STRUCTURE TO KERNEL */
//#define MAX_KEY_BUFF_SIZE    200
#define MAX_KEY_BUFF_SIZE    201
/* END: 0014654 jihoon.lee@lge.com 2011024 */

typedef enum
{
    VER_SW=0,
    VER_DSP,
    VER_MMS,
    VER_CONTENTS,
    VER_PRL,
    VER_ERI,
    VER_BREW,
    VER_MODEL,
    VER_HW,
    REV_DSP=9,
    CONTENTS_SIZE,
    JAVA_FILE_CNT=13,
    JAVA_FILE_SIZE,
    VER_JAVA,
    BANK_ON_CNT=16,
    BANK_ON_SIZE,
    MODULE_FILE_CNT,
    MODULE_FILE_SIZE,
    MP3_DSP_OS_VER=21,
    VER_TOUCH_FW=22,
    VER_LCD_REVISION=24,
    VER_SMART_OS=26
} test_mode_req_version_type;
/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode function [START] */
#define WIFI_MAC_ADDR_CNT 12

typedef enum
{
	WLAN_TEST_MODE_CHK=3,
	WLAN_TEST_MODE_54G_ON=4,
	WLAN_TEST_MODE_OFF,
	WLAN_TEST_MODE_RX_START,
	WLAN_TEST_MODE_RX_RESULT=9,
	WLAN_TEST_MODE_TX_START=10,
	WLAN_TEST_MODE_TXRX_STOP=13,
	WLAN_TEST_MODE_LF_RX_START=31,
	WLAN_TEST_MODE_MF_TX_START=44,
	WLAN_TEST_MODE_11B_ON=57,
	WLAN_TEST_MODE_11N_MIXED_LONG_GI_ON=69,
	WLAN_TEST_MODE_11N_MIXED_SHORT_GI_ON=77,
	WLAN_TEST_MODE_11N_GREEN_LONG_GI_ON=85,
	WLAN_TEST_MODE_11N_GREEN_SHORT_GI_ON=93,
	WLAN_TEST_MODE_11A_CH_RX_START=101, // not support
	WLAN_TEST_MODE_11BG_CH_TX_START=128,
	WLAN_TEST_MODE_11A_ON=155,
	WLAN_TEST_MODE_11AN_MIXED_LONG_GI_ON=163,
	WLAN_TEST_MODE_MAX=195,
} test_mode_req_wifi_type;

typedef enum
{
	WLAN_TEST_MODE_CTGRY_ON,
	WLAN_TEST_MODE_CTGRY_OFF,
	WLAN_TEST_MODE_CTGRY_RX_START,
	WLAN_TEST_MODE_CTGRY_RX_STOP,
	WLAN_TEST_MODE_CTGRY_TX_START,
	WLAN_TEST_MODE_CTGRY_TX_STOP,
	WLAN_TEST_MODE_CTGRY_CHK,
	WLAN_TEST_MODE_CTGRY_NOT_SUPPORTED,
} test_mode_ret_wifi_ctgry_t;


typedef enum
{
	WIFI_MAC_ADDRESS_WRITE = 0,
	WIFI_MAC_ADDRESS_READ = 1,
} test_mode_req_wifi_addr_req_type;

typedef struct {
	//test_mode_req_wifi_addr_req_type req_type;
	byte req_type;
	byte wifi_mac_addr[WIFI_MAC_ADDR_CNT];
} test_mode_req_wifi_addr_type;

/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode function [END] */

#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)	// [LG_BTUI] '12.05.07 : LG_BTUI_TEST_MODE For VZW
typedef enum
{
  BT_GET_ADDR, //no use anymore
  BT_TEST_MODE_1=1,
  BT_TEST_MODE_CHECK=2,
  BT_TEST_MODE_RELEASE=5,
  BT_TEST_MODE_11=11 // 11~42
}test_mode_req_bt_type;

typedef enum
{
  BT_ADDR_WRITE=0,
  BT_ADDR_READ
}test_mode_req_bt_rw_type;

#define BT_RW_CNT 20

#endif //LG_BTUI_TEST_MODE



typedef enum
{
  	MOTOR_OFF,
 	MOTOR_ON
}test_mode_req_motor_type;

typedef enum
{
  	ACOUSTIC_OFF=0,
  	ACOUSTIC_ON,
  	HEADSET_PATH_OPEN,
  	HANDSET_PATH_OPEN,
  	ACOUSTIC_LOOPBACK_ON,
  	ACOUSTIC_LOOPBACK_OFF
}test_mode_req_acoustic_type;
typedef enum
{
  	MP3_128KHZ_0DB,
  	MP3_128KHZ_0DB_L,
  	MP3_128KHZ_0DB_R,
  	MP3_MULTISINE_20KHZ,
  	MP3_PLAYMODE_OFF,
  	MP3_SAMPLE_FILE,
  	MP3_NoSignal_LR_128k
}test_mode_req_mp3_test_type;
typedef enum
{
  	SPEAKER_PHONE_OFF,
  	SPEAKER_PHONE_ON,
  	NOMAL_Mic1,
  	NC_MODE_ON,
  	ONLY_MIC2_ON_NC_ON,
  	ONLY_MIC1_ON_NC_ON
}test_mode_req_speaker_phone_type;
typedef enum
{
  	VOL_LEV_OFF,
  	VOL_LEV_MIN,
  	VOL_LEV_MEDIUM,
  	VOL_LEV_MAX
}test_mode_req_volume_level_type;

#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
typedef enum
{
	CAM_TEST_MODE_OFF = 0,
	CAM_TEST_MODE_ON,
	CAM_TEST_SHOT,
	CAM_TEST_SAVE_IMAGE,
	CAM_TEST_CALL_IMAGE,
	CAM_TEST_ERASE_IMAGE,
	CAM_TEST_FLASH_ON,
	CAM_TEST_FLASH_OFF = 9,
	CAM_TEST_CAMCORDER_MODE_OFF,
	CAM_TEST_CAMCORDER_MODE_ON,
	CAM_TEST_CAMCORDER_SHOT,
	CAM_TEST_CAMCORDER_SAVE_MOVING_FILE,
	CAM_TEST_CAMCORDER_PLAY_MOVING_FILE,
	CAM_TEST_CAMCORDER_ERASE_MOVING_FILE,
	CAM_TEST_CAMCORDER_FLASH_ON,
	CAM_TEST_CAMCORDER_FLASH_OFF,
	CAM_TEST_STROBE_LIGHT_ON,
	CAM_TEST_STROBE_LIGHT_OFF,
	CAM_TEST_CAMERA_SELECT = 22,
}test_mode_req_cam_type;
#endif

typedef enum
{
    EXTERNAL_SOCKET_MEMORY_CHECK,
    EXTERNAL_FLASH_MEMORY_SIZE,
    EXTERNAL_SOCKET_ERASE,
    EXTERNAL_FLASH_MEMORY_USED_SIZE = 4,
    EXTERNAL_FLASH_MEMORY_CONTENTS_CHECK,
    EXTERNAL_FLASH_MEMORY_ERASE,
    EXTERNAL_SOCKET_ERASE_SDCARD_ONLY = 0xE,
    EXTERNAL_SOCKET_ERASE_FAT_ONLY = 0xF,
}test_mode_req_socket_memory;

typedef enum
{
    FIRST_BOOTING_COMPLETE_CHECK,
    FIRST_BOOTING_CHG_MODE_CHECK=0xF,
}test_mode_req_fboot;

typedef enum
{
    FIRST_BOOTING_IN_CHG_MODE,
    FIRST_BOOTING_NOT_IN_CHG_MODE
}test_mode_first_booting_chg_mode_type;

typedef enum
{
    MEMORY_TOTAL_CAPA_TEST,
    MEMORY_USED_CAPA_TEST,
    MEMORY_REMAIN_CAPA_TEST
}test_mode_req_memory_capa_type;

typedef enum
{
    MEMORY_TOTAL_SIZE_TEST = 0 ,
    MEMORY_FORMAT_MEMORY_TEST = 1,
}test_mode_req_memory_size_type;

typedef enum
{
    FACTORY_RESET_CHECK,
    FACTORY_RESET_COMPLETE_CHECK,
    FACTORY_RESET_STATUS_CHECK,
    FACTORY_RESET_COLD_BOOT,
    FACTORY_RESET_FORCE_CHANGE_STATUS,
    FACTORY_RESET_ERASE_USERDATA = 0x0F,
}test_mode_req_factory_reset_mode_type;

typedef enum{
    FACTORY_RESET_START = 0,
    FACTORY_RESET_INITIAL = 1,
    FACTORY_RESET_ARM9_END = 2,
    FACTORY_RESET_COLD_BOOT_START = 3,
    FACTORY_RESET_COLD_BOOT_END = 5,
    FACTORY_RESET_HOME_SCREEN_END = 6,
    FACTORY_RESET_NA = 7,
}test_mode_factory_reset_status_type;

typedef enum
{
    SLEEP_MODE_ON,
    SLEEP_FLIGHT_MODE_ON,
    FLIGHT_KERNEL_MODE_ON,
    FLIGHT_MODE_OFF,
} test_mode_sleep_mode_type;
/* LGE_FACTORY_TEST_MODE for Photo Sensor(ALC) */
typedef enum
{
	ALC_TEST_MODE_OFF=0,
	ALC_TEST_MODE_ON,
	ALC_TEST_CHECK_STATUS,
	ALC_TEST_AUTOTEST
} test_mode_req_alc_type;

typedef enum
{
    TEST_SCRIPT_ITEM_SET,
    TEST_SCRIPT_MODE_CHECK,
    CAL_DATA_BACKUP,
    CAL_DATA_RESTORE,
    CAL_DATA_ERASE,
    CAL_DATA_INFO
}test_mode_req_test_script_mode_type;

#define PID_CNT 30

typedef enum
{
    PID_WRITE,
    PID_READ
}test_mode_req_pid_req_type;

typedef struct {
    test_mode_req_pid_req_type req_type;
    byte bt_addr[PID_CNT];
}test_mode_req_pid_type;

typedef enum
{
    SW_VERSION,
    SW_OUTPUT_VERSION,
    SW_COMPLETE_VERSION,
    SW_VERSION_CHECK
} test_mode_req_sw_version_type;

typedef enum
{
    CAL_CHECK,
    CAL_DATA_CHECK,
} test_mode_req_cal_check_type;

typedef enum
{
    MULTIMODE,
    LTEONLY,
    CDMAONLY,
} test_mode_req_lte_mdoe_seletion_type;

typedef enum
{
    CALL_CONN,
    CALL_DISCONN,
} test_mode_req_lte_call_type;

typedef enum
{
    VIRTUAL_SIM_OFF,
    VIRTUAL_SIM_ON,
    VIRTUAL_SIM_STATUS,
    CAMP_CHECK = 3,
    AUTO_CAMP_REQ = 20,
    DETACH = 21,
}test_mode_req_virtual_sim_type;

typedef enum
{
    CHANGE_MODEM,
    CHANGE_MASS,
} test_mode_req_change_usb_driver_type;

typedef enum
{
    DB_INTEGRITY_CHECK=0,
    FPRI_CRC_CHECK=1,
    FILE_CRC_CHECK=2,
    CODE_PARTITION_CRC_CHECK=3,
    TOTAL_CRC_CHECK=4,
    DB_DUMP_CHECK=5,
    DB_COPY_CHECK=6,    
} test_mode_req_db_check;

typedef enum 
{
    IMEI_WRITE=0, 
    IMEI_READ=1,
    IMEI2_WRITE=2,
    IMEI2_READ=3
} test_mode_req_imei_req_type;

//20110930, addy.kim@lge.com,Add NFC TestCommand  [START]
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)  
typedef enum
{
  NFC_TEST_MODE_ON=0,
  NFC_TEST_MODE_OFF,
  NFC_TEST_MODE_SWP,
  NFC_TEST_MODE_ANT,
  NFC_TEST_MODE_READER,
  NFC_TEST_MODE_FIRMWARE_FILE_VERSION,
  NFC_TEST_MODE_FIMEWARE_UPDATE,
  NFC_TEST_MODE_FIRMWARE_CHIP_VERSION
}test_mode_req_nfc_type;
//20110930, addy.kim@lge.com [END]
#endif

typedef enum
{
    VCO_SELF_TUNNING_ITEM_SET,
    VCO_SELF_TUNNING_ITEM_READ
}test_mode_req_vco_self_tunning_type;

typedef enum 
{
    BATTERY_THERM_ADC=0,
    BATTERY_VOLTAGE_LEVEL=1,
    BATTERY_CHARGING_COMPLETE,
    BATTERY_CHARGING_MODE_TEST,
    BATTERY_FUEL_GAUGE_RESET=5,
    BATTERY_FUEL_GAUGE_SOC=6,
#ifdef CONFIG_LGE_BATT_SOC_FOR_NPST
    BATTERY_FUEL_GAUGE_SOC_NPST =99,
#endif
} test_mode_req_batter_bar_type;

typedef enum 
{
    MANUAL_TEST_ON,
    MANUAL_TEST_OFF,
    MANUAL_MODE_CHECK,
	 MANUAL_MODE_COMPLETE_CHECK
} test_mode_req_manual_test_mode_type;

typedef enum
{
    CDMA_WCDMA_MAX_POWER_ON = 0,
    CDMA_WCDMA_MAX_POWER_OFF =1,
    LTE_TESTMODE_ON = 2,
    LTE_RF_ON = 3,
    LTE_FAKE_SYNC = 4,
    LTE_RX_SETUP = 5,
    LTE_SCHEDULE = 6,
    LTE_TX_SETUP = 7,
    LTE_TX_POWER_SETUP = 8,
    LTE_MAX_POWER_OFF = 9,
    LTE_RF_OFF = 10,
} test_mode_rep_max_current_check_type; 

typedef enum
{
    LTE_OFF_AND_CDMA_ON = 0,
    LTE_ON_AND_CDMA_OFF = 1,
    RF_MODE_CHECK = 2
} test_mode_rep_change_rf_cal_mode_type;

typedef enum
{
    DUAL_ANT = 0,
    SECONDARY_ANT_ONLY = 1,
    PRIMARY_ANT_ONLY = 2
} test_mode_rep_select_mimo_ant_type;

typedef enum
{
    MODE_OFF,
    MODE_ON,
    STATUS_CHECK,
    IRDA_AUTO_TEST_START,
    IRDA_AUTO_TEST_RESULT,
    EXT_CARD_AUTO_TEST,
} test_mode_req_irda_fmrt_finger_uim_type;

typedef enum
{
    FELICA_CAL_CHECK,
    IDM_READ,
} test_mode_req_felica_type;

typedef enum
{
    RESET_FIRST_PRODUCTION,
    RESET_FIRST_PRODUCTION_CHECK,
    RESET_REFURBISH=2,
    RESET_REFURBISH_CHECK,
}test_mode_req_reset_production_type;

#define BT_ADDR_CNT 12

typedef enum
{
    BLUETOOTH_ADDRESS_WRITE,
    BLUETOOTH_ADDRESS_READ,
}test_mode_req_bt_addr_req_type;

typedef struct {
    test_mode_req_bt_addr_req_type req_type;
    byte bt_addr[BT_ADDR_CNT];
}test_mode_req_bt_addr_type;

typedef enum {
    XO_FIELD_CALIBRATION,
    XO_FACTORY_CALIBRATION,
} test_mode_req_XOCalDataBackup_Type;

typedef enum
{
    FOTA_ID_CHECK,
    FOTA_ID_READ,
}test_mode_req_fota_type;

typedef enum
{
    KEY_LOCK,
    KEY_UNLOCK,
}test_mode_req_key_lock_unlock_type;

typedef enum
{
    MLT_DISABLE,
    MLT_ENABLE,
}test_mode_req_mlt_enable_type;
//LGE_CHANGE_S:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
extern int key_lock;

typedef enum
{
	KEY_LOCK_REQ=0,
	KEY_UNLOCK_REQ=1,	
}test_mode_req_key_lock_type;

typedef enum
{
	KEY_LOCK_RSP=0,
	KEY_UNLOCK_RSP=1,
	KEY_LOCK_NOT_SUPPORT=2,
}test_mode_rsp_key_lock_type;
#endif
//LGE_CHANGE_E:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
typedef enum
{
    OTP_WRITE,
    OTP_READ,
    PLAYREADY_KEY_VERIFY
}test_mode_req_blow_command_type;

typedef enum
{
    WV_ERASE,
    WV_WRITE,
    WV_CHECK,
    WV_WINDEX,
    WV_RINDEX
}test_mode_wv_command_type;

typedef struct
{
    byte sub2;
    word type;
    byte data[256];
}PACKED test_mode_req_wv_type;

#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
typedef struct
{
	byte MaxRGB[10];
}test_mode_req_lcd_cal;
#endif

#define MAX_NTCODE_SIZE 800

typedef union
{
    byte NTCode[MAX_NTCODE_SIZE];
    test_mode_req_version_type version;
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)	// [LG_BTUI] '12.05.07 : LG_BTUI_TEST_MODE For VZW
	test_mode_req_bt_type	bt;
	byte					bt_rw[BT_RW_CNT];
	byte 					read_bd_addr[BT_RW_CNT];
#endif //LG_BTUI_TEST_MODE
//20110930, addy.kim@lge.com,Add NFC TestCommand  [START]
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)  
	test_mode_req_nfc_type 			nfc;	
#endif
//20110930, addy.kim@lge.com,Add NFC TestCommand  [END]
    test_mode_req_socket_memory esm;
    test_mode_req_fboot fboot;
    test_mode_req_memory_capa_type mem_capa;
    test_mode_req_memory_size_type memory_format;
    word key_data;
  	test_mode_req_motor_type motor;
  	test_mode_req_acoustic_type acoustic;
    test_mode_req_mp3_test_type mp3_play;
  	test_mode_req_speaker_phone_type speaker_phone;
  	test_mode_req_volume_level_type	volume_level;
    boolean key_test_start;
    test_mode_req_factory_reset_mode_type factory_reset;
    test_mode_sleep_mode_type sleep_mode;
    test_mode_req_test_script_mode_type test_mode_test_scr_mode;
    test_mode_req_pid_type pid;
    test_mode_req_vco_self_tunning_type vco_self_tunning;
    test_mode_req_factory_reset_mode_type test_factory_mode;
    byte CGPSTest;
    test_mode_req_batter_bar_type batt;
    test_mode_req_manual_test_mode_type test_manual_mode; 
    test_mode_req_sw_version_type sw_version;
    test_mode_req_cal_check_type cal_check;
    test_mode_req_db_check db_check;
    test_mode_req_lte_mdoe_seletion_type mode_seletion;
    test_mode_req_lte_call_type lte_call;
    test_mode_req_virtual_sim_type lte_virtual_sim;
    test_mode_rep_max_current_check_type max_current;
    test_mode_rep_change_rf_cal_mode_type rf_mode;
    test_mode_rep_select_mimo_ant_type select_mimo;
    test_mode_req_change_usb_driver_type change_usb_driver;
    test_mode_req_irda_fmrt_finger_uim_type ext_device_cmd;
    test_mode_req_reset_production_type reset_production_cmd;
    test_mode_req_bt_addr_type bt_ad;
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
	test_mode_req_lcd_cal lcd_cal;
//HELLG ........ is this is my job????????????? 
    int  resetproduction_sub2;
    int  irdafmrtfingerUIM_sub2;
	test_mode_req_cam_type		   camera;
#endif
	/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode [START] */
	test_mode_req_wifi_type wifi;
	test_mode_req_wifi_addr_type wifi_mac_ad;
	/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode [END] */
	test_mode_req_fota_type fota;
//LGE_CHANGE_S:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
  test_mode_req_key_lock_type req_key_lock;
#else
    test_mode_req_key_lock_unlock_type key_lock_unlock;
#endif
//LGE_CHANGE_E:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
    test_mode_req_mlt_enable_type mlt_enable;
    test_mode_req_blow_command_type otp_command;
    test_mode_req_felica_type felica_cmd;
    test_mode_req_wv_type wv_command;
    test_mode_req_XOCalDataBackup_Type XOCalDataBackup;
} test_mode_req_type;

typedef struct diagpkt_header
{
    byte opaque_header;
}PACKED diagpkt_header_type;

typedef struct DIAG_TEST_MODE_F_req_tag {
    diagpkt_header_type xx_header;
    word sub_cmd_code;
    test_mode_req_type test_mode_req;
} PACKED DIAG_TEST_MODE_F_req_type;

typedef enum
{
    TEST_OK_S,
    TEST_FAIL_S,
    TEST_NOT_SUPPORTED_S
} PACKED (test_mode_ret_stat_type);

/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode [START] */
typedef struct
{
	int packet;
	int per;
} PACKED WlRxResults;
/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode [END] */
typedef struct
{
    byte SVState;
    uint8 SV;
    uint16 MeasuredCNo;
} PACKED CGPSResultType;

typedef union
{
    test_mode_req_version_type version;
    byte str_buf[17];
    CGPSResultType TestResult[16];
    test_mode_req_motor_type motor;
    test_mode_req_acoustic_type acoustic;
  	test_mode_req_mp3_test_type mp3_play;
    test_mode_req_speaker_phone_type speaker_phone;
    test_mode_req_volume_level_type volume_level;
    char key_pressed_buf[MAX_KEY_BUFF_SIZE];
    char memory_check;
    uint32 socket_memory_size;
    uint32 socket_memory_usedsize;
    unsigned int mem_capa;
	/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode [START] */
	byte wlan_status;
	WlRxResults wlan_rx_results;
	byte read_wifi_mac_addr[12];
	/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode [END] */
    char batt_voltage[5];
    byte chg_stat;
    int manual_test;
    test_mode_req_pid_type pid;
    test_mode_req_sw_version_type sw_version;
    byte hkadc_value;
    byte uim_state;
//HELLG ........ is this is my job????????????? 
  int uim_state2;    
    byte vco_value;
    test_mode_req_cal_check_type cal_check;
    test_mode_req_factory_reset_mode_type factory_reset;
    test_mode_req_test_script_mode_type test_mode_test_scr_mode;
    byte read_bd_addr[BT_ADDR_CNT];
    byte fota_id_read[20];
    byte felica_cal_check;
    byte idm_read[20];
// LGE_CHANGE_S  [UICC] Testmode SIM ID READ - minyi.kim@lge.com 2011-10-18
	byte read_sim_id[21];
// LGE_CHANGE_E  [UICC] Testmode SIM ID READ - minyi.kim@lge.com 2011-10-18

    test_mode_req_XOCalDataBackup_Type XOCalDataBackup;
//LGE_CHANGE_S:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
 	test_mode_rsp_key_lock_type key_lock;
#endif	
//LGE_CHANGE_E:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
    byte mlt_enable;
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
	test_mode_req_lcd_cal lcd_cal;
	test_mode_req_cam_type		   camera;

#endif
	uint32 wv_index;
//20110930, addy.kim@lge.com,Add NFC TestCommand  [START]
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)  
	byte read_nfc_data[100]; //addy.kim@lge.com
#endif
//20110930, addy.kim@lge.com,Add NFC TestCommand  [END]

} PACKED test_mode_rsp_type;

typedef struct DIAG_TEST_MODE_F_rsp_tag {
    diagpkt_header_type xx_header;
    word sub_cmd_code;
    test_mode_ret_stat_type ret_stat_code;
    test_mode_rsp_type test_mode_rsp;
} PACKED DIAG_TEST_MODE_F_rsp_type;

typedef enum
{
    TEST_MODE_VERSION = 0,
    TEST_MODE_LCD,
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
	TEST_MODE_LCD_CAL = 2, 
#endif    
	TEST_MODE_MOTOR = 3,
    TEST_MODE_ACOUSTIC,
    TEST_MODE_CAM = 7,
    TEST_MODE_EFS_INTEGRITY = 11,
    TEST_MODE_IRDA_FMRT_FINGER_UIM_TEST = 13,
    TEST_MODE_BREW_CNT = 20,

    TEST_MODE_BREW_SIZE = 21,
    TEST_MODE_KEY_TEST,
    TEST_MODE_EXT_SOCKET_TEST,
    TEST_MODE_BLUETOOTH_TEST,
    TEST_MODE_BATT_LEVEL_TEST,
    TEST_MODE_MP3_TEST = 27,
    TEST_MODE_FM_TRANCEIVER_TEST,
    TEST_MODE_ISP_DOWNLOAD_TEST,
    //TEST_MODE_COMPASS_SENSOR_TEST = 30,

    TEST_MODE_ACCEL_SENSOR_TEST = 31,
    TEST_MODE_ALCOHOL_SENSOR_TEST = 32, //[2012-06-12] addy.kim@lge.com, NFC Test Number
    TEST_MODE_TDMB_TEST = 33,
    TEST_MODE_WIFI_TEST = 33,
    TEST_MODE_TV_OUT_TEST = 33,
    TEST_MODE_SDMB_TEST = 33,
    TEST_MODE_MANUAL_TEST_MODE = 36,
    TEST_MODE_FORMAT_MEMORY_TEST = 38,
    TEST_MODE_3D_ACCELERATOR_SENSOR_TEST = 39,
    TEST_MODE_KEY_DATA_TEST = 40,  

    TEST_MODE_MEMORY_CAPA_TEST = 41,  
    TEST_MODE_SLEEP_MODE_TEST,
    TEST_MODE_SPEAKER_PHONE_TEST,
    TEST_MODE_VIRTUAL_SIM_TEST = 44,
    TEST_MODE_PHOTO_SENSER_TEST,
    TEST_MODE_VCO_SELF_TUNNING_TEST,
    TEST_MODE_MRD_USB_TEST = 47,
    TEST_MODE_TEST_SCRIPT_MODE = 48,
    TEST_MODE_PROXIMITY_SENSOR_TEST = 49,
    TEST_MODE_FACTORY_RESET_CHECK_TEST = 50,

    TEST_MODE_VOLUME_TEST = 51,
    TEST_MODE_HANDSET_FREE_ACTIVATION_TEST,
    TEST_MODE_MOBILE_SYSTEM_CHANGE_TEST,
    TEST_MODE_STANDALONE_GPS_TEST,
    TEST_MODE_PRELOAD_INTEGRITY_TEST,
    TEST_MODE_FIRST_BOOT_COMPLETE_TEST = 58,
    TEST_MODE_MAX_CURRENT_CHECK = 59,
    TEST_MODE_LED_TEST = 60,

    TEST_MODE_CHANGE_RFCALMODE = 61,
    TEST_MODE_SELECT_MIMO_ANT = 62,
    TEST_MODE_LTE_MODE_SELECTION = 63,
    TEST_MODE_LTE_CALL = 64,
    TEST_MODE_CHANGE_USB_DRIVER = 65,
    TEST_MODE_GET_HKADC_VALUE = 66,
    TEST_MODE_PID_TEST = 70,

    TEST_MODE_SW_VERSION = 71,
    TEST_MODE_IME_TEST = 72,
    TEST_MODE_IMPL_TEST,
    TEST_MODE_SIM_LOCK_TYPE_TEST,
    TEST_MODE_UNLOCK_CODE_TEST,
    TEST_MODE_IDDE_TEST,
    TEST_MODE_FULL_SIGNATURE_TEST,
    TEST_MODE_NT_CODE_TEST,
    TEST_MODE_SIM_ID_TEST = 79,
  // LGE_CHANGE_S  [UICC] Testmode SIM ID READ - minyi.kim@lge.com 2011-10-18
  TEST_MODE_SIM_ID_READ = 81,
  // LGE_CHANGE_E  [UICC] Testmode SIM ID READ - minyi.kim@lge.com 2011-10-18
    TEST_MODE_CAL_CHECK = 82,
    TEST_MODE_BLUETOOTH_RW = 83,
    TEST_MODE_SKIP_WELCOM_TEST = 87,
    TEST_MODE_WIFI_MAC_RW = 88,

    TEST_MODE_DB_INTEGRITY_CHECK = 91,
    TEST_MODE_NVCRC_CHECK = 92,
    TEST_MODE_GYRO_SENSOR_TEST = 93,
    TEST_MODE_RESET_PRODUCTION = 96,
    TEST_MODE_FOTA = 98,
//LGE_CHANGE_S:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
	TEST_MODE_KEY_LOCK = 99,
#else		
    TEST_MODE_KEY_LOCK_UNLOCK = 99,
#endif	
//LGE_CHANGE_E:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
    TEST_MODE_ACCELERATOR_SENSOR_TEST = 100,
    TEST_MODE_GYRO_SENSOR_TEST_TEST = 101,
    TEST_MODE_COMPASS_SENSOR_TEST = 102,
    TEST_MODE_POWER_RESET = 105,
    TEST_MODE_MLT_ENABLE = 106,
    TEST_MODE_BLOW_COMMAND = 109,
    TEST_MODE_FELICA = 111,
    TEST_MODE_WV_PROVISIONING_COMMAND = 114,
    TEST_MODE_XO_CAL_DATA_COPY = 250,

    MAX_TEST_MODE_SUBCMD = 0xFFFF
} PACKED test_mode_sub_cmd_type;

#define TESTMODE_MSTR_TBL_SIZE   128

#define ARM9_PROCESSOR       0
#define ARM11_PROCESSOR     1

typedef void*(* testmode_func_type)(test_mode_req_type * , DIAG_TEST_MODE_F_rsp_type * );

typedef struct
{
    word cmd_code;
    testmode_func_type func_ptr;
    byte  which_procesor;             // to choose which processor will do act.
}testmode_user_table_entry_type;

typedef struct
{
    uint16 countresult;
    uint16 wlan_status;
    uint16 g_wlan_status;
    uint16 rx_channel;
    uint16 rx_per;
    uint16 tx_channel;
    uint32 goodFrames;
    uint16 badFrames;
    uint16 rxFrames;
    uint16 wlan_data_rate;
    uint16 wlan_payload;
    uint16 wlan_data_rate_recent;
    unsigned long pktengrxducast_old;
    unsigned long pktengrxducast_new;
    unsigned long rxbadfcs_old;
    unsigned long rxbadfcs_new;
    unsigned long rxbadplcp_old;
    unsigned long rxbadplcp_new;
}wlan_status;

typedef struct DIAG_TEST_MODE_KEY_F_rsp_tag {
    diagpkt_header_type xx_header;
    word sub_cmd_code;
    test_mode_ret_stat_type ret_stat_code;
    char key_pressed_buf[MAX_KEY_BUFF_SIZE];
} PACKED DIAG_TEST_MODE_KEY_F_rsp_type;

// LGE_CHANGE_S moses.son@lge.com
#ifdef CONFIG_LGE_USB_ACCESS_LOCK
#define PPE_UART_KEY_LENGTH 6
#define PPE_DES3_KEY_LENGTH 128

typedef enum {
  TF_SUB_CHECK_PORTLOCK = 0,
  TF_SUB_LOCK_PORT,
  TF_SUB_UNLOCK_PORT,
  TF_SUB_KEY_VERIFY,  
  TF_SUB_GET_CARRIER,
  TF_SUB_PROD_FLAG_STATUS,  
  TF_SUB_PROD_KEY_VERIFY,  
} nvdiag_tf_sub_cmd_type;

// oskwon 090606 : Lock ???? SUB ??? 3?? ??, ?? ??? 3?? ? ?? 
typedef struct	
{
  byte cmd_code;                      /* Command code */
  byte sub_cmd;                       /* Sub Command */
} PACKED DIAG_TF_F_req_type1;

typedef struct
{
  byte cmd_code;                      /* Command code */
  byte sub_cmd;                       /* Sub Command */
  byte keybuf[PPE_UART_KEY_LENGTH];   /* Uart Lock Key - 6 Digit */
} PACKED DIAG_TF_F_req_type2;

typedef struct
{
  byte cmd_code;                      /* Command code */
  byte sub_cmd;                       /* Sub Command */
  union {
  byte keybuf[PPE_UART_KEY_LENGTH];   /* Uart Lock Key - 16 Digit */
  byte probuf[PPE_DES3_KEY_LENGTH];   /* Production Key - 128 Byte */
  } PACKED buf;
} PACKED DIAG_TF_F_req_type;

typedef struct
{
  byte cmd_code;                      /* Command code */ 
  byte sub_cmd;                       /* Sub Command */
  byte result;                        /* Status of operation */
} PACKED DIAG_TF_F_rsp_type;

typedef enum {
  TF_STATUS_FAIL = 0,       	//Fail Unknown Reason
  TF_STATUS_SUCCESS,        	//General Success
  TF_STATUS_PORT_LOCK = 12,    	//TF_SUB_CHECK_PORTLOCK -> LOCK
  TF_STATUS_PORT_UNLOCK,    	//TF_SUB_CHECK_PORTLOCK -> UNLOCK
  TF_STATUS_VER_KEY_OK,     	//TF_SUB_KEY_VERIFY -> OK 
  TF_STATUS_VER_KEY_NG,     	//TF_SUB_KEY_VERIFY -> NG
  TF_STATUS_P_FLAG_ENABLE,   	//PRODUCTION FLAG 1 
  TF_STATUS_P_FLAG_DISABLE,		//PRODUCTION FLAG 0
  TF_STATUS_VER_P_KEY_OK,		// PPE KEY OK
  TF_STATUS_VER_P_KEY_NG,		// PPE KEY NG
} DIAG_TF_F_sub_cmd_result_type;

typedef struct
{
  byte cmd_code;                      /* Command code */
  dword seccode;                       /* security code */
} PACKED DIAG_TF_SB_F_req_type;

typedef struct
{
  byte cmd_code;                      /* Command code */ 
} PACKED DIAG_TF_SB_F_rsp_type;

#endif
// LGE_CHANGE_E moses.son@lge.com

//LG_UPDATE_S : [sinjo.mattappallil@lge.com] [06 June 2012] [Porting LS840 GB chnages]
byte CheckHWRev(byte *pStr);
void delayed_recharging_cmd(int hour);
//LG_UPDATE_E : [sinjo.mattappallil@lge.com] [06 June 2012] [Porting LS840 GB chnages]
#endif /* LG_DIAG_TESTMODE_H */
