#include <linux/module.h>
#include <lg_diagcmd.h>
#include <linux/input.h>
#include <linux/syscalls.h>

#include <lg_fw_diag_communication.h>
#include <lg_diag_testmode.h>
#include <mach/qdsp5v2/audio_def.h>
#include <linux/delay.h>

#ifndef SKW_TEST
#include <linux/fcntl.h> 
#include <linux/fs.h>
#include <linux/uaccess.h>
#endif

#include <userDataBackUpDiag.h>
#include <userDataBackUpTypeDef.h> 
#include <../../kernel/arch/arm/mach-msm/smd_private.h>
#include <linux/slab.h>

#if defined(CONFIG_LGE_PM_CAYMAN_VZW) || defined(CONFIG_LGE_PM_CAYMAN_MPCS)
#include <linux/msm-charger.h>
#endif /* (CONFIG_LGE_PM_CAYMAN_VZW) || (CONFIG_LGE_PM_CAYMAN_MPCS) */

#include <board_lge.h>
#include <lg_backup_items.h>

#include <linux/gpio.h>
#include <linux/mfd/pmic8058.h>
#include <mach/irqs.h>
/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode [START] */
#include <linux/parser.h>
#define WL_IS_WITHIN(min,max,expr)         (((min)<=(expr))&&((max)>(expr)))
/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode [END] */

#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
#define SYS_GPIO_SD_DET	21
#endif
#define NFC_RESULT_PATH 	"/sys/class/lg_fw_diagclass/lg_fw_diagcmd/nfc_testmode_result"
#define PMIC_GPIO_SDC3_DET 22
#define PM8058_GPIO_BASE NR_MSM_GPIOS
#define PM8058_GPIO_PM_TO_SYS(pm_gpio) (pm_gpio + PM8058_GPIO_BASE)

static struct diagcmd_dev *diagpdev;

#ifdef CONFIG_LGE_DIAGTEST
int allow_usb_switch = 0;
#endif
extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);
extern PACK(void *) diagpkt_free (PACK(void *)pkt);
extern void send_to_arm9( void * pReq, void * pRsp);
extern testmode_user_table_entry_type testmode_mstr_tbl[TESTMODE_MSTR_TBL_SIZE];
extern int diag_event_log_start(void);
extern int diag_event_log_end(void);
extern void set_operation_mode(boolean isOnline);
extern struct input_dev* get_ats_input_dev(void);
extern unsigned int LGF_KeycodeTrans(word input);
extern void LGF_SendKey(word keycode);
extern int boot_info;
extern unsigned char testmode_result[20];

extern void remote_rpc_srd_cmmand(void * pReq, void * pRsp );
extern void *smem_alloc(unsigned id, unsigned size);


#ifdef CONFIG_LGE_DLOAD_SRD  //kabjoo.choi
#define SIZE_OF_SHARD_RAM  0x60000  //384K
extern int lge_erase_block(int secnum, size_t size);
extern int lge_write_block(int secnum, unsigned char *buf, size_t size);
extern int lge_read_block(int secnum, unsigned char *buf, size_t size);
extern int lge_mmc_scan_partitions(void);

extern unsigned int srd_bytes_pos_in_emmc ;
unsigned char * load_srd_shard_base;
unsigned char * load_srd_kernel_base;
#endif 
#ifdef CONFIG_LGE_DLOAD_SRD  //kabjoo.choi
#define SIZE_OF_SHARD_RAM  0x60000  //384K

extern PACK (void *)LGE_Dload_SRD (PACK (void *)req_pkt_ptr, uint16 pkg_len);
extern void diag_SRD_Init(udbp_req_type * req_pkt, udbp_rsp_type * rsp_pkt);
extern void diag_userDataBackUp_entrySet(udbp_req_type * req_pkt, udbp_rsp_type * rsp_pkt, script_process_type MODEM_MDM );
extern boolean writeBackUpNVdata( char * ram_start_address , unsigned int size);

extern unsigned int srd_bytes_pos_in_emmc ;
unsigned char * load_srd_shard_base;
unsigned char * load_srd_kernel_base;
#endif 

#ifdef CONFIG_LGE_USB_MDM_DIAG_DISABLE
extern void set_mdm_diag_enable(int enable);
extern int get_mdm_diag_enable(void);
#endif
//[20120614]Add NFC Testmode,addy.kim@lge.com [START]
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)  
void* LGF_TestModeNFC( test_mode_req_type* pReq, DIAG_TEST_MODE_F_rsp_type *pRsp);
#endif
//20111006, addy.kim@lge.com,	[END]	

#if defined(CONFIG_MACH_LGE_120_BOARD) || defined(CONFIG_MACH_LGE_I_BOARD_VZW)
#include <mach/scm-io.h>
#include <mach/msm_iomap.h>
#include <mach/scm.h>
#include <linux/random.h>

void* LGF_TestOTPBlowCommand(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp);
void* LGF_TestWVProvisioningCommand(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp);
void* LGF_TestPRProvisioningCommand(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp);
#endif

#if defined(CONFIG_MACH_LGE_I_BOARD_DCM)
void* LGF_TestModeFelica(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp);
#endif

struct statfs_local {
 __u32 f_type;
 __u32 f_bsize;
 __u32 f_blocks;
 __u32 f_bfree;
 __u32 f_bavail;
 __u32 f_files;
 __u32 f_ffree;
 __kernel_fsid_t f_fsid;
 __u32 f_namelen;
 __u32 f_frsize;
 __u32 f_spare[5];
};

extern int get_touch_ts_fw_version(char *fw_ver);
extern int lge_bd_rev;
/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode function [START] */
typedef struct _rx_packet_info 
{
	int goodpacket;
	int badpacket;
} rx_packet_info_t;

enum {
	Param_none = -1,
	Param_goodpacket,
	Param_badpacket,
	Param_end,
	Param_err,
};

static const match_table_t param_tokens = {
	{Param_goodpacket, "good=%d"},
	{Param_badpacket, "bad=%d"},
	{Param_end,	"END"},
	{Param_err, NULL}
};

void* LGF_TestModeWLAN(test_mode_req_type* pReq, DIAG_TEST_MODE_F_rsp_type *pRsp);
/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode function [END] */

char external_memory_after_format_file_test(int order)
{

    char return_value = TEST_FAIL_S;
	char *src = (void *)0;
    char *dest = (void *)0;
	off_t fd_offset;
	int fd = 0;
	mm_segment_t old_fs=get_fs(); //äºŒì‡±???—ë¸³????????¾ë±·???ê¾ªë¹
    set_fs(get_ds());
    

    switch(order){
      case 1:
        if ( (fd = sys_open((const char __user *) "/sdcard/SDTest.txt", O_CREAT | O_RDWR, 0) ) < 0 )
    	{
    		printk(KERN_ERR "[ATCMD_EMT] Can not access SD card\n");
    		goto file_fail;
    	}
printk(KERN_ERR "[ATCMD_EMT] access SD card\n");
    	if ( (src = kmalloc(10, GFP_KERNEL)) )
    	{
    		sprintf(src,"TEST");
    		if ((sys_write(fd, (const char __user *) src, 5)) < 0)
    		{
    			printk(KERN_ERR "[ATCMD_EMT] Can not write SD card \n");
    			goto file_fail;
    		}
    		fd_offset = sys_lseek(fd, 0, 0);
    	}
    	if ( (dest = kmalloc(10, GFP_KERNEL)) )
    	{
    		if ((sys_read(fd, (char __user *) dest, 5)) < 0)
    		{
    			printk(KERN_ERR "[ATCMD_EMT]Can not read SD card \n");
    			goto file_fail;
    		}
    		if ((memcmp(src, dest, 4)) == 0)
                return_value = TEST_OK_S;
    		else
                return_value = TEST_FAIL_S;
    	}

    	kfree(src);
    	kfree(dest);
        break;
    
    case 2:
    if ( (fd = sys_open((const char __user *) "/sdcard/SDTest.txt", O_RDONLY, 0) ) < 0 ){
		printk(KERN_ERR "[ATCMD_EMT] Noraml! File is not exist After format SD card\n");
        return_value = TEST_OK_S;
	}        
	else{   
        printk(KERN_ERR "[ATCMD_EMT] File is exist. After format \n");
        return_value = TEST_FAIL_S;
    }
    break;
  }


file_fail:
    
	sys_close(fd);
    set_fs(old_fs);

    if(order == 2)
	    sys_unlink((const char __user *)"/sdcard/SDTest.txt");



	return return_value;
}


  


/* 2011.4.23 jaeho.cho@lge.com for usb driver testmode */
#ifdef CONFIG_LGE_DIAGTEST
void set_allow_usb_switch_mode(int allow)
{
    if(allow_usb_switch != allow)
    allow_usb_switch = allow;
}
EXPORT_SYMBOL(set_allow_usb_switch_mode);

int get_allow_usb_switch_mode(void)
{
    return allow_usb_switch;
}
EXPORT_SYMBOL(get_allow_usb_switch_mode);
#endif

/* [yk.kim@lge.com] 2011-01-04, change usb driver */
void* LGF_TestModeChangeUsbDriver(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	printk(KERN_DEBUG "%s, req type : %d", __func__, pReq->change_usb_driver);
	pRsp->ret_stat_code = TEST_OK_S;
#ifdef CONFIG_LGE_USB_GADGET_DRIVER
	extern int android_set_pid(const char *val, struct kernel_param *kp);

/* 2011.4.23 jaeho.cho@lge.com for usb driver testmode */
#ifdef CONFIG_LGE_DIAGTEST
    set_allow_usb_switch_mode(1);
#endif

	if (pReq->change_usb_driver == CHANGE_MODEM)
	{
		android_set_pid("6317", NULL);
	}
	else if (pReq->change_usb_driver == CHANGE_MASS)
		android_set_pid("6317", NULL);
	else
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
 #endif
 
	return pRsp;

}




//HELLG ........ is this is my job????????????? 
    
        void* LGF_TestModeIrdaFmrtFingerUIMTest(
                test_mode_req_type* pReq ,
                DIAG_TEST_MODE_F_rsp_type   *pRsp)
        {
         
        /* BEGIN: 0014656 jihoon.lee@lge.com 20110124 */
        /* MOD 0014656: [LG RAPI] OEM RAPI PACKET MISMATCH KERNEL CRASH FIX */
          DIAG_TEST_MODE_F_req_type req_ptr;
        
          req_ptr.sub_cmd_code = TEST_MODE_IRDA_FMRT_FINGER_UIM_TEST;
          pRsp->ret_stat_code = TEST_OK_S;         
          pRsp->test_mode_rsp.uim_state2 = 0;
//          return pRsp;          
    //      req_ptr.test_mode_req.factory_reset = pReq->factory_reset;
        /* END: 0014656 jihoon.lee@lge.com 2011024 */
    
          printk("==============================================================\n");
          printk("LGF_TestModeResetProduction      sub command is %d\n", pReq->resetproduction_sub2);
          printk("LGF_TestModeResetProduction      req_ptr.sub_cmd_code is %d\n", req_ptr.sub_cmd_code );      
          printk("LGF_TestModeResetProduction      pRsp->sub_cmd_code is %d\n", pRsp->sub_cmd_code );            
          printk("LGF_TestModeResetProduction      pReq->irdafmrtfingerUIM_sub2 is %d\n", pReq->irdafmrtfingerUIM_sub2 );                      
          printk("==============================================================\n");
    
    
          if(pReq->irdafmrtfingerUIM_sub2 == 5)
          {
            req_ptr.test_mode_req.irdafmrtfingerUIM_sub2 = 5;
            send_to_arm9((void*)&req_ptr, (void*)pRsp);

            printk("==============================================================\n");
            printk("LGF_TestModeResetProduction      pRsp->ret_stat_code  is %d\n", pRsp->ret_stat_code );
            printk("LGF_TestModeResetProduction      req_ptr.sub_cmd_code is %d\n", req_ptr.sub_cmd_code );            
            printk("LGF_TestModeResetProduction      pRsp->sub_cmd_code is %d\n", pRsp->sub_cmd_code );              
            printk("LGF_TestModeResetProduction      pReq->irdafmrtfingerUIM_sub2 is %d\n", pReq->irdafmrtfingerUIM_sub2 );                                
            printk("LGF_TestModeResetProduction      pRsp->test_mode_rsp.uim_state is %d\n", pRsp->test_mode_rsp.uim_state2);          
            printk("==============================================================\n");


            if(pRsp->ret_stat_code == TEST_FAIL_S)      
            {
              pRsp->ret_stat_code = TEST_OK_S;   
              pRsp->test_mode_rsp.uim_state2 = 1;              
            }
            else if(pRsp->ret_stat_code == TEST_OK_S)      
            {
                pRsp->test_mode_rsp.uim_state2 = 0;
            }
            else
            {
                pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;                         
                pRsp->test_mode_rsp.uim_state2 = 1;            
            }
              
          }
          else
          {
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;             
          }
    
          printk("==============================================================\n");
          printk("LGF_TestModeResetProduction      pRsp->ret_stat_code  is %d\n", pRsp->ret_stat_code );
          printk("LGF_TestModeResetProduction      req_ptr.sub_cmd_code is %d\n", req_ptr.sub_cmd_code );            
          printk("LGF_TestModeResetProduction      pRsp->sub_cmd_code is %d\n", pRsp->sub_cmd_code );              
          printk("LGF_TestModeResetProduction      pReq->irdafmrtfingerUIM_sub2 is %d\n", pReq->irdafmrtfingerUIM_sub2 );                                
          printk("LGF_TestModeResetProduction      pRsp->test_mode_rsp.uim_state is %d\n", pRsp->test_mode_rsp.uim_state2);          
          printk("==============================================================\n");
    
          
          return pRsp;
        
        }



//HELLG ........ is this is my job????????????? 
    void* LGF_TestModeResetProduction(
            test_mode_req_type* pReq ,
            DIAG_TEST_MODE_F_rsp_type   *pRsp)
    {
     
    /* BEGIN: 0014656 jihoon.lee@lge.com 20110124 */
    /* MOD 0014656: [LG RAPI] OEM RAPI PACKET MISMATCH KERNEL CRASH FIX */
      DIAG_TEST_MODE_F_req_type req_ptr;
    
      req_ptr.sub_cmd_code = TEST_MODE_RESET_PRODUCTION;
      pRsp->ret_stat_code = TEST_OK_S;         
//      req_ptr.test_mode_req.factory_reset = pReq->factory_reset;
    /* END: 0014656 jihoon.lee@lge.com 2011024 */

      printk("==============================================================\n");
      printk("LGF_TestModeResetProduction      sub command is %d\n", pReq->resetproduction_sub2);
      printk("LGF_TestModeResetProduction      req_ptr.sub_cmd_code is %d\n", req_ptr.sub_cmd_code );      
      printk("LGF_TestModeResetProduction      pRsp->sub_cmd_code is %d\n", pRsp->sub_cmd_code );            
      printk("==============================================================\n");


      if(pReq->resetproduction_sub2 == 1)
      {
        req_ptr.test_mode_req.resetproduction_sub2 = 1;
        send_to_arm9((void*)&req_ptr, (void*)pRsp);
      }
      else
      {
        req_ptr.test_mode_req.resetproduction_sub2 = 0;      
        send_to_arm9((void*)&req_ptr, (void*)pRsp);
      }

      printk("==============================================================\n");
      printk("LGF_TestModeResetProduction      pRsp->ret_stat_code  is %d\n", pRsp->ret_stat_code );
      printk("LGF_TestModeResetProduction      req_ptr.sub_cmd_code is %d\n", req_ptr.sub_cmd_code );            
      printk("LGF_TestModeResetProduction      pRsp->sub_cmd_code is %d\n", pRsp->sub_cmd_code );                  
      printk("==============================================================\n");

      if(pRsp->ret_stat_code != TEST_OK_S)      
        pRsp->ret_stat_code = TEST_FAIL_S;   
      
      return pRsp;
    
    }

#ifdef CONFIG_LGE_DIAG_EVENT_TO_FW
extern void android_diag_event_work(void);
#endif

#ifdef CONFIG_LGE_USB_ACCESS_LOCK
PACK (void *)LGF_TFProcess (
			PACK (void	*)req_pkt_ptr,	/* pointer to request packet  */
			uint16		pkt_len )			  /* length of request packet	*/
{
	DIAG_TF_F_req_type *req_ptr = (DIAG_TF_F_req_type *) req_pkt_ptr;
	DIAG_TF_F_rsp_type *rsp_ptr;
	unsigned int rsp_len;
	extern int get_usb_lock(void);
	extern void set_usb_lock(int lock);
	extern void get_spc_code(char * spc_code);

	rsp_len = sizeof(DIAG_TF_F_rsp_type);
	rsp_ptr = (DIAG_TF_F_rsp_type *)diagpkt_alloc(DIAG_TF_CMD_F, rsp_len);
	
	switch(req_ptr->sub_cmd)
	{
		case TF_SUB_CHECK_PORTLOCK:
			if (get_usb_lock())
				rsp_ptr->result = TF_STATUS_PORT_LOCK;
			else
				rsp_ptr->result = TF_STATUS_PORT_UNLOCK;

			rsp_ptr->sub_cmd = TF_SUB_CHECK_PORTLOCK;

			break;

		case TF_SUB_LOCK_PORT:
			set_usb_lock(1);
#ifdef CONFIG_LGE_USB_MDM_DIAG_DISABLE
			set_mdm_diag_enable(1);
#endif
#ifdef CONFIG_LGE_DIAG_EVENT_TO_FW
			android_diag_event_work();
#endif
			rsp_ptr->sub_cmd = TF_SUB_LOCK_PORT;
			rsp_ptr->result = TF_STATUS_SUCCESS;
			break;

		case TF_SUB_UNLOCK_PORT:
		{
			char spc_code[6];
			
			get_spc_code(spc_code);
//			printk(KERN_ERR "LG_FW TF_SUB_UNLOCK_PORT spc_code[%s]",spc_code);
			if (memcmp((byte *)spc_code,req_ptr->buf.keybuf, PPE_UART_KEY_LENGTH )==0)
			{
				set_usb_lock(0);
#ifdef CONFIG_LGE_USB_MDM_DIAG_DISABLE
				set_mdm_diag_enable(0);
#endif
				printk(KERN_ERR "LG_FW TF_SUB_UNLOCK_PORT success \n");
				rsp_ptr->result = TF_STATUS_SUCCESS;
			}
			else
				rsp_ptr->result = TF_STATUS_FAIL;
			 rsp_ptr->sub_cmd = TF_SUB_UNLOCK_PORT;
			break;
		}
	}

	return (rsp_ptr);
}
EXPORT_SYMBOL(LGF_TFProcess);
#endif



#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
byte CheckHWRev(byte *pStr)
#else
void CheckHWRev(byte *pStr)
#endif
{
    char *rev_str[] = {"evb1", "evb2", "A", "B", "C", "D",
        "E", "F", "G", "1.0", "1.1", "1.2",
        "revserved"};

#if defined(CONFIG_MACH_LGE_120_BOARD)
    strcpy((char *)pStr ,(char *)rev_str[lge_bd_rev - 9/*CAYMAN_I_DIFF_VALUE*/]);
#else
    strcpy((char *)pStr ,(char *)rev_str[lge_bd_rev]);
#endif
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
	return pStr[0];
#endif
}

static int android_readwrite_file(const char *filename, char *rbuf, const char *wbuf, size_t length);

void get_smartphone_os(byte *pStr)
{
    char buf[1024];
    char *res;
    char *version = "ro.build.version.release=";

    android_readwrite_file("system/build.prop", buf, NULL, sizeof(buf));
    res= strstr(buf,version);
    memcpy((char*)pStr, res + strlen(version), 5);
}

PACK (void *)LGF_TestMode (
        PACK (void *)req_pkt_ptr, /* pointer to request packet */
        uint16 pkt_len )        /* length of request packet */
{
    DIAG_TEST_MODE_F_req_type *req_ptr = (DIAG_TEST_MODE_F_req_type *) req_pkt_ptr;
    DIAG_TEST_MODE_F_rsp_type *rsp_ptr;
    unsigned int rsp_len=0;
    testmode_func_type func_ptr= NULL;
    int nIndex = 0;

    diagpdev = diagcmd_get_dev();

    // DIAG_TEST_MODE_F_rsp_type union type is greater than the actual size, decrease it in case sensitive items
    switch(req_ptr->sub_cmd_code)
    {
        case TEST_MODE_FACTORY_RESET_CHECK_TEST:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type);
            break;

        case TEST_MODE_TEST_SCRIPT_MODE:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_test_script_mode_type);
            break;

        //REMOVE UNNECESSARY RESPONSE PACKET FOR EXTERNEL SOCKET ERASE
        case TEST_MODE_EXT_SOCKET_TEST:
            if((req_ptr->test_mode_req.esm == EXTERNAL_SOCKET_ERASE) || (req_ptr->test_mode_req.esm == EXTERNAL_SOCKET_ERASE_SDCARD_ONLY) \
                    || (req_ptr->test_mode_req.esm == EXTERNAL_SOCKET_ERASE_FAT_ONLY))
                rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type);
            else
                rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type);
            break;

        //Added by jaeopark 110527 for XO Cal Backup
        case TEST_MODE_XO_CAL_DATA_COPY:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_XOCalDataBackup_Type);
            break;

        case TEST_MODE_MANUAL_TEST_MODE:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_manual_test_mode_type);
            break;

        case TEST_MODE_BLUETOOTH_RW:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_bt_addr_type);
            break;

        case TEST_MODE_WIFI_MAC_RW:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_req_wifi_addr_type);
            break;
#ifdef CONFIG_LGE_DIAG_DISABLE_INPUT_DEVICES_ON_SLEEP_MODE 
		case TEST_MODE_SLEEP_MODE_TEST:
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type) - sizeof(test_mode_rsp_type) + sizeof(test_mode_sleep_mode_type);
			break;
#endif
        default :
            rsp_len = sizeof(DIAG_TEST_MODE_F_rsp_type);
            break;
    }

    rsp_ptr = (DIAG_TEST_MODE_F_rsp_type *)diagpkt_alloc(DIAG_TEST_MODE_F, rsp_len);

    printk(KERN_ERR "[LGF_TestMode] rsp_len: %d, sub_cmd_code: %d \n", rsp_len, req_ptr->sub_cmd_code);

    if (!rsp_ptr)
        return 0;

    rsp_ptr->sub_cmd_code = req_ptr->sub_cmd_code;
    rsp_ptr->ret_stat_code = TEST_OK_S; // test ok

    for( nIndex = 0 ; nIndex < TESTMODE_MSTR_TBL_SIZE  ; nIndex++)
    {
        if( testmode_mstr_tbl[nIndex].cmd_code == req_ptr->sub_cmd_code)
        {
            if( testmode_mstr_tbl[nIndex].which_procesor == ARM11_PROCESSOR)
                func_ptr = testmode_mstr_tbl[nIndex].func_ptr;
            break;
        }
    }

    if( func_ptr != NULL)
        return func_ptr( &(req_ptr->test_mode_req), rsp_ptr);
    else
    {
    	memset(rsp_ptr->test_mode_rsp.str_buf, 0 , sizeof(rsp_ptr->test_mode_rsp.str_buf));
        if(req_ptr->test_mode_req.version == VER_HW)
            CheckHWRev((byte *)rsp_ptr->test_mode_rsp.str_buf);
        else if(req_ptr->test_mode_req.version == VER_TOUCH_FW)
            get_touch_ts_fw_version((byte *)rsp_ptr->test_mode_rsp.str_buf);
        else if(req_ptr->test_mode_req.version == VER_SMART_OS)
            get_smartphone_os((byte *)rsp_ptr->test_mode_rsp.str_buf);
        else
            send_to_arm9((void*)req_ptr, (void*)rsp_ptr);
    }

    return (rsp_ptr);
}
EXPORT_SYMBOL(LGF_TestMode);

void* linux_app_handler(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    diagpkt_free(pRsp);
    return 0;
}

void* not_supported_command_handler(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
    return pRsp;
}

#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
/* LCD QTEST */
PACK (void *)LGF_LcdQTest (
        PACK (void	*)req_pkt_ptr,	/* pointer to request packet  */
        uint16		pkt_len )		      /* length of request packet   */
{
	/* Returns 0 for executing lg_diag_app */
	return 0;
}
EXPORT_SYMBOL(LGF_LcdQTest);

void* LGF_TestLCD_Cal(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	char ptr[30];
	
	pRsp->ret_stat_code = TEST_OK_S;
	printk("<6>" "pReq->lcd_cal: (%d)\n", pReq->lcd_cal.MaxRGB[0]);

	if (diagpdev != NULL){
		if (pReq->lcd_cal.MaxRGB[0] != 5)
			update_diagcmd_state(diagpdev, "LCD_Cal", pReq->lcd_cal.MaxRGB[0]);
		else {
			printk("<6>" "pReq->MaxRGB string type : %s\n",pReq->lcd_cal.MaxRGB);
        	sprintf(ptr,"LCD_Cal,%s",&pReq->lcd_cal.MaxRGB[1]);
			printk("<6>" "%s \n", ptr);
			update_diagcmd_state(diagpdev, ptr, pReq->lcd_cal.MaxRGB[0]);
		}
	}
	else
	{
		printk("\n[%s] error LCD_cal", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
	return pRsp;
}
#endif

void* LGF_TestModeMIMO_RFMode(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
#if 0
    pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
		update_diagcmd_state(diagpdev, "LTEMODESELETION", pReq->mode_seletion);
	}
	else
	{
		printk("\n[%s] error LteModeSelection", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
#endif 
  return pRsp;
}

void* LGF_TestModeLTE_Call(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
#if 0
    pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
		update_diagcmd_state(diagpdev, "LTECALL", pReq->lte_call);
	}
	else
	{
		printk("\n[%s] error LteCall", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
#endif
  return pRsp;
}



#ifndef CONFIG_LGE_BD_DOWN_FAIL
extern void arch_reset(char mode, const char *cmd);
static struct workqueue_struct *reset_wq = NULL;
struct __bd_dl_reset {
    struct delayed_work work;
};
static struct __bd_dl_reset bd_dl_reset_data;
static void
reset_wq_func(struct work_struct *work)
{
	arch_reset(0,NULL);

	return;
}

#endif
// BEGIN: 0010557  unchol.park@lge.com 2011-01-12
// 0010557 : [Testmode] added test_mode command for LTE test 
void* LGF_TestModeDetach(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{

	DIAG_TEST_MODE_F_req_type req_ptr; 
#if 0


	printk("\n[%s] First LteCallDetach", __func__ );

	if(pReq->lte_virtual_sim == 21)
	{
		printk("\n[%s] Second LteCallDetach", __func__ );
		pRsp->ret_stat_code = TEST_OK_S;
		
		if (diagpdev != NULL){
			update_diagcmd_state(diagpdev, "LTECALLDETACH", pReq->lte_virtual_sim);
		}
		else
		{
			printk("\n[%s] error LteCallDetach", __func__ );
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		}
		return pRsp;
	}
#endif

	req_ptr.sub_cmd_code = 44;
	req_ptr.test_mode_req.lte_virtual_sim = pReq->lte_virtual_sim;
	printk(KERN_INFO "%s, pReq->lte_virtual_sim : %d\n", __func__, pReq->lte_virtual_sim);

	send_to_arm9((void*)&req_ptr, (void*)pRsp);
	printk(KERN_INFO "%s, pRsp->ret_stat_code : %d\n", __func__, pRsp->ret_stat_code);
 

#ifndef CONFIG_LGE_BD_DOWN_FAIL
	if ((req_ptr.test_mode_req.lte_virtual_sim =  (pReq->lte_virtual_sim == 251))) //satya
		{
		if(reset_wq == NULL)
	{
		
		reset_wq = create_singlethread_workqueue("reset_wq");
		INIT_DELAYED_WORK(&bd_dl_reset_data.work, reset_wq_func);
		queue_delayed_work(reset_wq, &bd_dl_reset_data.work,msecs_to_jiffies(1500));
	}
		//arch_reset(0,NULL);
		}	
#endif

	
  	return pRsp;

}
// END: 0010557 unchol.park@lge.com 2011-01-12

// END: 0010557 unchol.park@lge.com 2010-11-18

#define GET_INODE_FROM_FILEP(filp) \
    (filp)->f_path.dentry->d_inode

static int android_readwrite_file(const char *filename, char *rbuf, const char *wbuf, size_t length)
{
    int ret = 0;
    struct file *filp = (struct file *)-ENOENT;
    mm_segment_t oldfs;
    oldfs = get_fs();
    set_fs(KERNEL_DS);
    do {
        int mode = (wbuf) ? O_RDWR : O_RDONLY;
        filp = filp_open(filename, mode, S_IRUSR);
        if (IS_ERR(filp) || !filp->f_op) {
            printk(KERN_ERR "%s: file %s filp_open error\n", __FUNCTION__, filename);
            ret = -ENOENT;
            break;
        }

        if (length==0) {
            /* Read the length of the file only */
            struct inode    *inode;

            inode = GET_INODE_FROM_FILEP(filp);
            if (!inode) {
                printk(KERN_ERR "%s: Get inode from %s failed\n", __FUNCTION__, filename);
                ret = -ENOENT;
                break;
            }
            ret = i_size_read(inode->i_mapping->host);
            break;
        }

        if (wbuf) {
            if ( (ret=filp->f_op->write(filp, wbuf, length, &filp->f_pos)) < 0) {
                printk(KERN_ERR "%s: Write %u bytes to file %s error %d\n", __FUNCTION__, 
                                length, filename, ret);
                break;
            }
        } else {
            if ( (ret=filp->f_op->read(filp, rbuf, length, &filp->f_pos)) < 0) {
                printk(KERN_ERR "%s: Read %u bytes from file %s error %d\n", __FUNCTION__,
                                length, filename, ret);
                break;
            }
        }
    } while (0);

    if (!IS_ERR(filp)) {
        filp_close(filp, NULL);
    }
    set_fs(oldfs);

    return ret;
}


#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)	// [LG_BTUI] '12.05.07 : LG_BTUI_TEST_MODE For VZW

void* LGF_TestModeBlueTooth(
		test_mode_req_type*	pReq,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	printk(KERN_ERR "[_BTUI_] [%s:%d] BTSubCmd=<%d>\n", __func__, __LINE__, pReq->bt);

	if (diagpdev != NULL){
		update_diagcmd_state(diagpdev, "BT_TEST_MODE", pReq->bt);
		if(pReq->bt==1 || pReq->bt==11) msleep(5900); //6sec timeout
		else if(pReq->bt==2) ssleep(1);
		else ssleep(3);
		pRsp->ret_stat_code = TEST_OK_S;
	}
	else
	{
		printk(KERN_ERR "[_BTUI_] [%s:%d] BTSubCmd=<%d> ERROR\n", __func__, __LINE__, pReq->bt);
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
	return pRsp;
}

byte *pReq_valid_address(byte *pstr)
{
	int pcnt=0;
	byte value_pstr=0, *pstr_tmp;

	pstr_tmp = pstr;
	do
	{
		++pcnt;
		value_pstr = *(pstr_tmp++);
	}while(!('0'<=value_pstr && value_pstr<='9')&&!('a'<=value_pstr && value_pstr<='f')&&!('A'<=value_pstr && value_pstr<='F')&&(pcnt<BT_RW_CNT));

	return (--pstr_tmp);

}
void* LGF_TestModeBlueTooth_RW(
		test_mode_req_type*	pReq,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{

	byte *p_Req_addr;
	int fd = 0;
	int i = 0;
	char *src = (void *)0;	
	mm_segment_t old_fs=get_fs();
	set_fs(get_ds());
	
	p_Req_addr = pReq_valid_address(pReq->bt_rw);

	if(!p_Req_addr)
	{
		pRsp->ret_stat_code = TEST_FAIL_S;
		return pRsp;
	}

	printk(KERN_ERR "[_BTUI_] [%s:%d] BTSubCmd=<%s>\n", __func__, __LINE__, p_Req_addr);

	if (diagpdev != NULL)
	{
		//250-83-0 bluetooth write
		if(strlen(p_Req_addr) > 0)
		{
			if ( (fd = sys_open((const char __user *) "/data/misc/bluetooth/diag_addr", O_CREAT | O_RDWR, 0777) ) < 0 )
			{
		    		printk(KERN_ERR "[_BTUI_] Can not open file.\n");
				pRsp->ret_stat_code = TEST_FAIL_S;
				goto file_bt_fail;
			}
				
			if ( (src = kmalloc(20, GFP_KERNEL)) )
			{
				sprintf( src,"%c%c%c%c%c%c%c%c%c%c%c%c", 
					p_Req_addr[0],	p_Req_addr[1], p_Req_addr[2], p_Req_addr[3], 
					p_Req_addr[4], p_Req_addr[5], p_Req_addr[6], p_Req_addr[7], 
					p_Req_addr[8], p_Req_addr[9], p_Req_addr[10], p_Req_addr[11]);
					
				if ((sys_write(fd, (const char __user *) src, 12)) < 0)
				{
					printk(KERN_ERR "[_BTUI_] Can not write file.\n");
					pRsp->ret_stat_code = TEST_FAIL_S;
					goto file_bt_fail;
				}
			}

			printk(KERN_ERR "[_BTUI_] Wait 1 Sec.\n");
			msleep(1000);			

			update_diagcmd_state(diagpdev, "BT_TEST_MODE_RW", 0);
		}
		else //250-83-1 bluetooth read
		{
			update_diagcmd_state(diagpdev, "BT_TEST_MODE_RW", 1);

			printk(KERN_ERR "[_BTUI_] Wait 2 Sec.\n");
			msleep(2000);

			if ( (fd = sys_open((const char __user *) "/data/misc/bluetooth/diag_addr", O_CREAT | O_RDWR, 0777) ) < 0 )
			{
				printk(KERN_ERR "[_BTUI_] Can not open file.\n");
				pRsp->ret_stat_code = TEST_FAIL_S;
				goto file_bt_fail;
			}
			
			if ( (src = kmalloc(20, GFP_KERNEL)) )
			{
				if ((sys_read(fd, (char __user *) src, 12)) < 0)
				{
					printk(KERN_ERR "[_BTUI_] Can not read file or address is invalid.\n");
					pRsp->ret_stat_code = TEST_FAIL_S;
					goto file_bt_fail;
				}
			}

			for(i = 0; i < BT_RW_CNT; i++)
			{
				pRsp->test_mode_rsp.read_bd_addr[i] = 0;
			}

			for(i = 0; i < 12; i++)
			{
				pRsp->test_mode_rsp.read_bd_addr[i] = src[i];
			}

			sys_unlink((const char __user *)"/data/misc/bluetooth/diag_addr");
		}
		
		pRsp->ret_stat_code = TEST_OK_S;
	}
	else 
	{
		printk(KERN_ERR "[_BTUI_] [%s:%d] BTSubCmd=<%s> ERROR\n", __func__, __LINE__, pReq->bt_rw);
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}

file_bt_fail:
	if (src != NULL) {
		kfree(src);
	}
	
	sys_close(fd);
	set_fs(old_fs);
	
  	return pRsp;
}
#endif //LG_BTUI_TEST_MODE


// LGE_CHANGE_S  [UICC] Testmode SIM ID READ - minyi.kim@lge.com 2011-10-18
void* LGF_TestModeSIMIdRead(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{

	DIAG_TEST_MODE_F_req_type req_ptr; 
//	int i=0;

	printk("\n[%s] LGF_TestModeSIMIdRead", __func__ );
	req_ptr.sub_cmd_code = TEST_MODE_SIM_ID_READ;
	pRsp->ret_stat_code = TEST_FAIL_S;
	memset(pRsp->test_mode_rsp.read_sim_id,0x00,20);

	printk(KERN_INFO "%s, pRsp->ret_stat_code : %x\n", __func__, pRsp->ret_stat_code);
	printk(KERN_INFO "%s, pRsp->test_mode_rsp.read_sim_id : %x\n", __func__, pRsp->test_mode_rsp.read_sim_id[0]);
		
	send_to_arm9((void*)&req_ptr, (void*)pRsp);

	if(pRsp->test_mode_rsp.read_sim_id[0] == 0x00 && 
		pRsp->test_mode_rsp.read_sim_id[1] == 0x00 && 
		 pRsp->test_mode_rsp.read_sim_id[2] == 0x00 &&
		  pRsp->test_mode_rsp.read_sim_id[3] == 0x00){
		pRsp->ret_stat_code = TEST_FAIL_S;
		pRsp->test_mode_rsp.uim_state2 = 1;
	}
	else{
		pRsp->test_mode_rsp.read_sim_id[20] = '\0';
		pRsp->ret_stat_code = TEST_OK_S;
	}
	/*for(i=0;i<30;i++){
		printk(KERN_INFO "%d, pRsp->test_mode_rsp.read_sim_id : %x\n", i, pRsp->test_mode_rsp.read_sim_id[i]);	
		}*/

	return pRsp;

}
// LGE_CHANGE_E  [UICC] Testmode SIM ID READ - minyi.kim@lge.com 2011-10-18





#if defined(CONFIG_MACH_LGE_120_BOARD) || defined(CONFIG_MACH_LGE_I_BOARD_VZW)
int g_sd_status = 0;
#define EXT_SD_WR_TEST_FILE   "/sdcard/_ExternalSD/SDTest.txt"
#define EXT_SD_MOUNT_POINT   "/sdcard/_ExternalSD"
#else
#define EXT_SD_WR_TEST_FILE   "/sdcard/SDTest.txt"
#define EXT_SD_MOUNT_POINT   "/sdcard"
#endif

#if defined(CONFIG_MACH_LGE_120_BOARD)
static int sd_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", g_sd_status);
}

static int sd_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int l_input_val = 0;
	
	sscanf(buf, "%d\n", &l_input_val);
	g_sd_status = l_input_val;

	return count;
}
DEVICE_ATTR(sd_status, 0777, sd_status_show, sd_status_store);
#endif

char external_memory_copy_test(void)
{
    char return_value = TEST_FAIL_S;
    char *src = (void *)0;
    char *dest = (void *)0;
    off_t fd_offset;
    int fd;
    mm_segment_t old_fs=get_fs();
	
#if defined(CONFIG_MACH_LGE_120_BOARD)
	if (g_sd_status == 0)
	{
		printk(KERN_ERR "[Testmode Memory Test] SD card does not mounted\n");
		return return_value;
	}
#endif

    set_fs(get_ds());

    if ( (fd = sys_open((const char __user *) EXT_SD_WR_TEST_FILE, O_CREAT | O_RDWR, 0) ) < 0 )
    {
        printk(KERN_ERR "[Testmode Memory Test] Can not access SD card\n");
        goto file_fail;
    }

    if ( (src = kmalloc(10, GFP_KERNEL)) )
    {
        sprintf(src,"TEST");
        if ((sys_write(fd, (const char __user *) src, 5)) < 0)
        {
            printk(KERN_ERR "[Testmode Memory Test] Can not write SD card \n");
            goto file_fail;
        }

        fd_offset = sys_lseek(fd, 0, 0);
    }

    if ( (dest = kmalloc(10, GFP_KERNEL)) )
    {
        if ((sys_read(fd, (char __user *) dest, 5)) < 0)
        {
            printk(KERN_ERR "[Testmode Memory Test] Can not read SD card \n");
            goto file_fail;
        }

        if ((memcmp(src, dest, 4)) == 0)
            return_value = TEST_OK_S;
        else
            return_value = TEST_FAIL_S;
    }

    kfree(src);
    kfree(dest);

file_fail:
    sys_close(fd);
    set_fs(old_fs);
    sys_unlink((const char __user *)EXT_SD_WR_TEST_FILE);

    return return_value;
}

extern int external_memory_test;

void* LGF_ExternalSocketMemory(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    struct statfs_local sf;
    pRsp->ret_stat_code = TEST_FAIL_S;

    printk(KERN_ERR "will83.lee[Testmode] in Extneral socket memory");
	
#if 0// defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
	printk(KERN_ERR "[Testmode Memory Test]sdcard start\n");
    // ADD: 0013541: 0014142: [Test_Mode] To remove Internal memory information in External memory test when SD-card is not exist
    if(gpio_get_value(SYS_GPIO_SD_DET)) //?�정?�야??kernel panic 발생
    {
        if (pReq->esm == EXTERNAL_SOCKET_MEMORY_CHECK)
        {
            pRsp->test_mode_rsp.memory_check = TEST_FAIL_S;
            pRsp->ret_stat_code = TEST_OK_S;
            printk(KERN_NOTICE "[Testmode Memory Test] Can not detect SD card :TEST_FAIL_S\n");
        }
        
        printk(KERN_ERR "[Testmode Memory Test] Can not detect SD card\n");
        return pRsp;
    }
#endif
    // ADD: 0013541: 0014142: [Test_Mode] To remove Internal memory information in External memory test when SD-card is not exist
    //if(gpio_get_value(PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SDC3_DET - 1)))
    if(external_memory_copy_test())
    {
        if (pReq->esm == EXTERNAL_SOCKET_MEMORY_CHECK)
        {
            pRsp->test_mode_rsp.memory_check = TEST_FAIL_S;
            pRsp->ret_stat_code = TEST_OK_S;
        }
        
        printk(KERN_ERR "[Testmode Memory Test] Can not detect SD card\n");
        return pRsp;
    }

    switch( pReq->esm){
        case EXTERNAL_SOCKET_MEMORY_CHECK:
            pRsp->test_mode_rsp.memory_check = external_memory_copy_test();
            pRsp->ret_stat_code = TEST_OK_S;
            break;

        case EXTERNAL_FLASH_MEMORY_SIZE:
            if (sys_statfs(EXT_SD_MOUNT_POINT, (struct statfs *)&sf) != 0)
            {
                printk(KERN_ERR "[Testmode Memory Test] can not get sdcard infomation \n");
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }

            pRsp->test_mode_rsp.socket_memory_size = ((long long)sf.f_blocks * (long long)sf.f_bsize) >> 20; // needs Mb.
            pRsp->ret_stat_code = TEST_OK_S;
            break;

        case EXTERNAL_SOCKET_ERASE:

	/* ICS 
            testmode_result[0] = 0xFF;

            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "MMCFORMAT", 1);
				printk("MMC format\n");
            }
            else
            {
                printk("\n[%s] error EXTERNAL_SOCKET_ERASE", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }

            for (i =0; i < 60; i++)
            {
				printk("MMC format= %d\n",i);
                if (testmode_result[0] != 0xFF)
                    break;

                msleep(500);
            }

            if(testmode_result[0] != 0xFF)
            {
				printk("MMC format1\n");
LOGS;	
                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
			printk("MMC format2\n");
LOGS;	
                pRsp->ret_stat_code = TEST_FAIL_S;
                printk(KERN_ERR "[MMCFORMAT] DiagCommandObserver returned fail or didn't return in 30000ms.\n");
            }
			if(external_memory_after_format_file_test(2) == TEST_OK_S){
                //pRsp->ret_stat_code = TEST_OK_S;
                printk("\n[%s] FACTORY_RESET OK", __func__ );
                }

*/


           pRsp->test_mode_rsp.memory_check = external_memory_after_format_file_test(1);
            if(pRsp->test_mode_rsp.memory_check == TEST_FAIL_S) break;
            if (diagpdev == NULL){
                  diagpdev = diagcmd_get_dev();
                  printk("\n[%s] diagpdev is Null dy.lee", __func__ );
            }
            
            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "MMCFORMAT", 1);
                msleep(5000);
                if(external_memory_after_format_file_test(2) == TEST_OK_S){
                pRsp->ret_stat_code = TEST_OK_S;
                printk("\n[%s] FACTORY_RESET OK", __func__ );
                }
                else{
                printk("\n[%s] error FACTORY_RESET", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;    
                }
            }
            else
            {
                printk("\n[%s] error FACTORY_RESET", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
            }
		
            break;

        case EXTERNAL_FLASH_MEMORY_USED_SIZE:
	
        /* 
            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "CALCUSEDSIZE", 0);
            }
            else
            {
                printk("\n[%s] error EXTERNAL_FLASH_MEMORY_USED_SIZE", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }

            for (i =0; i < 10; i++)
            {
                if (external_memory_test !=-1)
                    break;

                msleep(500);
            }
#ifdef CONFIG_MACH_LGE_I_BOARD_VZW
            msleep(500);
#endif
		msleep(1000);
            if(external_memory_test != -1)
            {
                pRsp->test_mode_rsp.socket_memory_usedsize = external_memory_test;
                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                pRsp->ret_stat_code = TEST_FAIL_S;
                printk(KERN_ERR "[CALCUSEDSIZE] DiagCommandObserver returned fail or didn't return in 5000ms.\n");
            }
			
	*/
	    external_memory_test = -1;
            update_diagcmd_state(diagpdev, "CALCUSEDSIZE", 0);
            msleep(1000);

            if(external_memory_test != -1)
            {
                pRsp->test_mode_rsp.socket_memory_usedsize = external_memory_test;
                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                pRsp->ret_stat_code = TEST_FAIL_S;
                printk(KERN_ERR "[CALCUSEDSIZE] DiagCommandObserver returned fail or didn't return in 100ms.\n");
            }


            break;
/* Not supported 
        case EXTERNAL_FLASH_MEMORY_CONTENTS_CHECK:
            external_memory_test = -1;

            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "CHECKCONTENTS", 0);
            }
            else
            {
                printk("\n[%s] error EXTERNAL_FLASH_MEMORY_CONTENTS_CHECK", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }

            for (i =0; i < 10; i++)
            {
                if (external_memory_test !=-1)
                    break;

                msleep(500);
            }

            if(external_memory_test != -1)
            {
                if(external_memory_test == 1)
                    pRsp->test_mode_rsp.memory_check = TEST_OK_S;
                else 
                    pRsp->test_mode_rsp.memory_check = TEST_FAIL_S;

                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                pRsp->ret_stat_code = TEST_FAIL_S;
                printk(KERN_ERR "[CHECKCONTENTS] DiagCommandObserver returned fail or didn't return in 5000ms.\n");
            }
            
            break;

        case EXTERNAL_FLASH_MEMORY_ERASE:
            external_memory_test = -1;

            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "ERASEMEMORY", 0);
            }
            else
            {
                printk("\n[%s] error EXTERNAL_FLASH_MEMORY_ERASE", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }


            for (i =0; i < 10; i++)
            {
                if (external_memory_test !=-1)
                    break;

                msleep(500);
            }

            if(external_memory_test != -1)
            {
                if(external_memory_test == 1)
                    pRsp->test_mode_rsp.memory_check = TEST_OK_S;
                else
                    pRsp->test_mode_rsp.memory_check = TEST_FAIL_S;

                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                pRsp->ret_stat_code = TEST_FAIL_S;
                printk(KERN_ERR "[ERASEMEMORY] DiagCommandObserver returned fail or didn't return in 5000ms.\n");
            }
            
            break;

*/
        case EXTERNAL_SOCKET_ERASE_SDCARD_ONLY: /*0xE*/
	
            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "MMCFORMAT", EXTERNAL_SOCKET_ERASE_SDCARD_ONLY);
                msleep(5000);
                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                printk("\n[%s] error EXTERNAL_SOCKET_ERASE_SDCARD_ONLY", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
            }
            break;

        case EXTERNAL_SOCKET_ERASE_FAT_ONLY: /*0xF*/
            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "MMCFORMAT", EXTERNAL_SOCKET_ERASE_FAT_ONLY);
                msleep(5000);
                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                printk("\n[%s] error EXTERNAL_SOCKET_ERASE_FAT_ONLY", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
            }
            break;

        default:
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
            break;
    }

    return pRsp;
}

void* LGF_TestModeBattLevel(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
#if defined(CONFIG_LGE_PM_CAYMAN_VZW) || defined(CONFIG_LGE_PM_CAYMAN_MPCS)
	int batt_thm_adc=0;
	
  	char buf[5];
  	int max17040_vcell = 0;
	int batt_complete_check = 0;
		
	int guage_level = 0;
	int battery_soc = 0;

	extern int testmode_battery_read_temperature_adc(void);
	extern int max17040_get_battery_mvolts(void);
	extern void max17040_quick_start(void);
	extern int max17040_get_battery_capacity_percent(void);
	
	pRsp->ret_stat_code = TEST_OK_S;

	switch(pReq->batt)
	{
		case BATTERY_THERM_ADC:
			batt_thm_adc = testmode_battery_read_temperature_adc();
			printk(KERN_ERR "%s, battery adc : %d\n", __func__, batt_thm_adc);
			if(batt_thm_adc > 2200)
				pRsp->ret_stat_code = TEST_FAIL_S;
			break;

		case BATTERY_VOLTAGE_LEVEL:
			max17040_vcell = max17040_get_battery_mvolts();
		
			if (max17040_vcell >= 4200)
				sprintf(buf, "%s", "4.2");
			else if (max17040_vcell >= 4100 && max17040_vcell < 4200)
				sprintf(buf, "%s", "4.1");
			else if (max17040_vcell >= 4000 && max17040_vcell < 4100)
				sprintf(buf, "%s", "4.0");
			else if (max17040_vcell >= 3900 && max17040_vcell < 4000)
				sprintf(buf, "%s", "3.9");
			else if (max17040_vcell >= 3800 && max17040_vcell < 3900)
				sprintf(buf, "%s", "3.8");
			else if (max17040_vcell >= 3700 && max17040_vcell < 3800)
				sprintf(buf, "%s", "3.7");
			else if (max17040_vcell >= 3600 && max17040_vcell < 3700)
				sprintf(buf, "%s", "3.6");
			else if (max17040_vcell >= 3500 && max17040_vcell < 3600)
				sprintf(buf, "%s", "3.5");
			else if (max17040_vcell >= 3400 && max17040_vcell < 3500)
				sprintf(buf, "%s", "3.4");
			else if (max17040_vcell >= 3300 && max17040_vcell < 3400)
				sprintf(buf, "%s", "3.3");
			else if (max17040_vcell >= 3200 && max17040_vcell < 3300)
				sprintf(buf, "%s", "3.2");
			else if (max17040_vcell >= 3100 && max17040_vcell < 3200)
				sprintf(buf, "%s", "3.1");
			else if (max17040_vcell >= 3000 && max17040_vcell < 3100)
				sprintf(buf, "%s", "3.0");
			else if (max17040_vcell >= 2900 && max17040_vcell < 3000)
				sprintf(buf, "%s", "2.9");

			sprintf((char *)pRsp->test_mode_rsp.batt_voltage, "%s", buf);
			break;

		case BATTERY_CHARGING_COMPLETE:
			testmode_discharging_mode_test();

			msleep(300);
			max17040_quick_start();
			msleep(300);
			batt_complete_check = max17040_get_battery_capacity_percent();
			msleep(100);

			if(batt_complete_check >=95)
				pRsp->test_mode_rsp.chg_stat = 0;
			else
				pRsp->test_mode_rsp.chg_stat = 1;

			if(pRsp->test_mode_rsp.chg_stat)
		  		testmode_charging_mode_test();
			break;

		case BATTERY_CHARGING_MODE_TEST:
			testmode_discharging_mode_test();

			msleep(300);
			max17040_quick_start();
			msleep(300);
			batt_complete_check = max17040_get_battery_capacity_percent();			
			msleep(100);
			
			testmode_charging_mode_test();
			break;

		case BATTERY_FUEL_GAUGE_RESET:
			max17040_quick_start();
			msleep(300);
			max17040_get_battery_mvolts();
			break;

		case BATTERY_FUEL_GAUGE_SOC:
			testmode_discharging_mode_test();
			
			msleep(300);
			max17040_quick_start();
			msleep(300);
			guage_level = max17040_get_battery_capacity_percent();
			msleep(100);
			guage_level = max17040_get_battery_capacity_percent();
			
			if(guage_level > 100)
				guage_level = 100;
			else if (guage_level < 0)
				guage_level = 0;

			sprintf((char *)pRsp->test_mode_rsp.batt_voltage, "%d", guage_level);
			break;

		case BATTERY_FUEL_GAUGE_SOC_NPST: // this is for getting SoC in NPST
			battery_soc = (int)max17040_get_battery_capacity_percent();
			if(battery_soc > 100)
				battery_soc = 100;
			else if (battery_soc < 0)
				battery_soc = 0;
			printk(KERN_ERR "%s, battery_soc : %d\n", __func__, battery_soc);
			sprintf((char *)pRsp->test_mode_rsp.batt_voltage, "%d", battery_soc);
			printk(KERN_ERR "%s, battery_soc : %s\n", __func__, (char *)pRsp->test_mode_rsp.batt_voltage);
			break;

		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			break;	

	}
#elif CONFIG_LGE_BATT_SOC_FOR_NPST
    int battery_soc = 0;
    extern int max17040_get_battery_capacity_percent(void);

    pRsp->ret_stat_code = TEST_OK_S;

    printk(KERN_ERR "%s, pRsp->ret_stat_code : %d\n", __func__, pReq->batt);
    if(pReq->batt == BATTERY_FUEL_GAUGE_SOC_NPST)
    {
        battery_soc = (int)max17040_get_battery_capacity_percent();
    }
    else
    {
        pRsp->ret_stat_code = TEST_FAIL_S;
    }

    if(battery_soc > 100)
        battery_soc = 100;
    else if (battery_soc < 0)
        battery_soc = 0;

    printk(KERN_ERR "%s, battery_soc : %d\n", __func__, battery_soc);

    sprintf((char *)pRsp->test_mode_rsp.batt_voltage, "%d", battery_soc);

    printk(KERN_ERR "%s, battery_soc : %s\n", __func__, (char *)pRsp->test_mode_rsp.batt_voltage);
#endif /* (CONFIG_LGE_PM_CAYMAN_VZW) || (CONFIG_LGE_PM_CAYMAN_MPCS) */

    return pRsp;
}

#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
byte key_buf[MAX_KEY_BUFF_SIZE];
boolean if_condition_is_on_key_buffering = FALSE;
int count_key_buf = 0;

boolean lgf_factor_key_test_rsp (char key_code)
{
    /* sanity check */
    if (count_key_buf>=MAX_KEY_BUFF_SIZE)
        return FALSE;

    key_buf[count_key_buf++] = (byte)key_code;
    return TRUE;
}
EXPORT_SYMBOL(lgf_factor_key_test_rsp);


void* LGT_TestModeKeyTest(test_mode_req_type* pReq, DIAG_TEST_MODE_F_rsp_type *pRsp)
{
  pRsp->ret_stat_code = TEST_OK_S;

  if(pReq->key_test_start){
	memset((void *)key_buf,0x00,MAX_KEY_BUFF_SIZE);
	count_key_buf=0;
	diag_event_log_start();
  }
  else
  {
	memcpy((void *)((DIAG_TEST_MODE_KEY_F_rsp_type *)pRsp)->key_pressed_buf, (void *)key_buf, MAX_KEY_BUFF_SIZE);
	memset((void *)key_buf,0x00,MAX_KEY_BUFF_SIZE);
	diag_event_log_end();
  }  
  return pRsp;
}

void* LGF_TestCam(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
		pRsp->ret_stat_code = TEST_OK_S;

		switch(pReq->camera)
		{
			case CAM_TEST_SAVE_IMAGE:
			//case CAM_TEST_FLASH_ON:
			//case CAM_TEST_FLASH_OFF:
				pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;

			default:
				if (diagpdev != NULL){

					update_diagcmd_state(diagpdev, "CAMERA", pReq->camera);
				}
				else
				{
					printk("\n[%s] error CAMERA", __func__ );
					pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
				}
				break;
		}
	return pRsp;
}
#endif

void* LGF_TestModeKeyData(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{

    pRsp->ret_stat_code = TEST_OK_S;

    LGF_SendKey(LGF_KeycodeTrans(pReq->key_data));

    return pRsp;
}

extern struct device *get_atcmd_dev(void);

void* LGF_TestModeSleepMode(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
#if defined(CONFIG_LGE_PM_CAYMAN_VZW) || defined(CONFIG_LGE_PM_CAYMAN_MPCS)
	pRsp->ret_stat_code = TEST_OK_S;
	pReq->sleep_mode = (pReq->sleep_mode & 0x00FF); 	// 2011.06.21 biglake for power test after cal

	switch(pReq->sleep_mode){	
		case SLEEP_MODE_ON:
			LGF_SendKey(KEY_POWER);
			break;		
		case SLEEP_FLIGHT_MODE_ON:
			LGF_SendKey(KEY_POWER);
			//if_condition_is_on_air_plain_mode = 1;
			update_diagcmd_state(diagpdev, "SLEEP_FLIGHT_MODE_ON", pReq->sleep_mode);
			set_operation_mode(FALSE);
			break;
	  	case FLIGHT_KERNEL_MODE_ON:
			break;
		case FLIGHT_MODE_OFF:
			set_operation_mode(TRUE);
			break;
		default:
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
#else /* (CONFIG_LGE_PM_CAYMAN_VZW) || (CONFIG_LGE_PM_CAYMAN_MPCS) */
    char *envp[3];
    char *atcmd_name = "AT_NAME=AT%FLIGHT";
    char *atcmd_state = "AT_STATE==1";
    struct device * dev = NULL;

    pRsp->ret_stat_code = TEST_FAIL_S;

    switch(pReq->sleep_mode)
    {
        case SLEEP_FLIGHT_MODE_ON:
            dev = get_atcmd_dev();

            if (dev)
            {
                envp[0] = atcmd_name;
                envp[1] = atcmd_state;
                envp[2] = NULL;

                kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, envp);
                pRsp->ret_stat_code = TEST_OK_S;
            }
            break;

        default:
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
            break;
    }
#endif /* (CONFIG_LGE_PM_CAYMAN_VZW) || (CONFIG_LGE_PM_CAYMAN_MPCS) */
    return pRsp;
}

void* LGF_TestModeVirtualSimTest(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    //pRsp->ret_stat_code = TEST_OK_S;
    //return pRsp;
    
	//[start] 2012/05/11 joonyoung84.kim@lge.com VSIM
	DIAG_TEST_MODE_F_req_type req_ptr; 

	printk("\n[%s] First LteCallDetach", __func__ );

	req_ptr.sub_cmd_code = 44;
	req_ptr.test_mode_req.lte_virtual_sim = pReq->lte_virtual_sim;
	printk(KERN_INFO "%s, pReq->lte_virtual_sim : %d\n", __func__, pReq->lte_virtual_sim);

	send_to_arm9((void*)&req_ptr, (void*)pRsp);
	printk(KERN_INFO "%s, pRsp->ret_stat_code : %d\n", __func__, pRsp->ret_stat_code);
	return pRsp;
	//[end] 2012/05/11 joonyoung84.kim@lge.com VSIM
}

void* LGF_TestModeFBoot(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    switch( pReq->fboot)
    {
        case FIRST_BOOTING_COMPLETE_CHECK:
            if (boot_info)
                pRsp->ret_stat_code = TEST_OK_S;
            else
                pRsp->ret_stat_code = TEST_FAIL_S;

            break;

        default:
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
            break;
    }

    return pRsp;
}


void* LGF_MemoryVolumeCheck(	test_mode_req_type* pReq ,DIAG_TEST_MODE_F_rsp_type	*pRsp)
{

  struct statfs_local  sf;
  unsigned int total = 0;
  unsigned int used = 0;
  unsigned int remained = 0;
  pRsp->ret_stat_code = TEST_OK_S;

  if (sys_statfs("/mnt/sdcard", (struct statfs *)&sf) != 0)
  {
    printk(KERN_ERR "[Testmode]can not get sdcard infomation \n");
    pRsp->ret_stat_code = TEST_FAIL_S;
  }
  else
  {

    total = (sf.f_blocks * sf.f_bsize) >> 20;
    remained = (sf.f_bavail * sf.f_bsize) >> 20;
    used = total - remained;

    switch(pReq->mem_capa)
    {
      case MEMORY_TOTAL_CAPA_TEST:
          pRsp->test_mode_rsp.mem_capa = total;
          break;

      case MEMORY_USED_CAPA_TEST:
          pRsp->test_mode_rsp.mem_capa = used;
          break;

      case MEMORY_REMAIN_CAPA_TEST:
          pRsp->test_mode_rsp.mem_capa = remained;
          break;

      default :
          pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
          break;
    }
  }

  return pRsp;

}


extern int db_integrity_ready;
extern int fpri_crc_ready;
extern int file_crc_ready;
extern int db_dump_ready;
extern int db_copy_ready;

typedef struct {
    char ret[32];
} testmode_rsp_from_diag_type;

extern testmode_rsp_from_diag_type integrity_ret;
void* LGF_TestModeDBIntegrityCheck(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    int i;
    unsigned int crc_val;

    memset(integrity_ret.ret, 0, 32);

    if (diagpdev != NULL)
    {
        db_integrity_ready = 0;
        fpri_crc_ready = 0;
        file_crc_ready = 0;
        db_dump_ready = 0;
        db_copy_ready = 0;

        update_diagcmd_state(diagpdev, "DBCHECK", pReq->db_check);

        switch(pReq->db_check)
        {
            case DB_INTEGRITY_CHECK:
                for (i =0; i < 10; i++)
                {
                    if (db_integrity_ready)
                        break;

                    msleep(500);
                }

                msleep(500); // wait until the return value is written to the file

                if(strlen(integrity_ret.ret) == 8)
                    crc_val = (unsigned int)simple_strtoul(integrity_ret.ret,NULL,16);
                else
                    crc_val = (unsigned int)simple_strtoul(integrity_ret.ret+1,NULL,16);

                sprintf(pRsp->test_mode_rsp.str_buf, "0x%08X", crc_val);

                printk(KERN_INFO "%s\n", integrity_ret.ret);
                printk(KERN_INFO "%d\n", crc_val);
                printk(KERN_INFO "%s\n", pRsp->test_mode_rsp.str_buf);

                pRsp->ret_stat_code = TEST_OK_S;
                break;

            case FPRI_CRC_CHECK:
                for (i =0; i < 10; i++)
                {
                    if (fpri_crc_ready)
                        break;

                    msleep(500);
                }

                msleep(500); // wait until the return value is written to the file

                if(strlen(integrity_ret.ret) == 8)
                    crc_val = (unsigned int)simple_strtoul(integrity_ret.ret,NULL,16);
                else
                    crc_val = (unsigned int)simple_strtoul(integrity_ret.ret+1,NULL,16);

                sprintf(pRsp->test_mode_rsp.str_buf, "0x%08X", crc_val);

                printk(KERN_INFO "%s\n", integrity_ret.ret);
                printk(KERN_INFO "%d\n", crc_val);
                printk(KERN_INFO "%s\n", pRsp->test_mode_rsp.str_buf);

                pRsp->ret_stat_code = TEST_OK_S;
                break;

            case FILE_CRC_CHECK:
                for (i =0; i < 20; i++)
                {
                    if (file_crc_ready)
                        break;

                    msleep(500);
                }

                msleep(500); // wait until the return value is written to the file

                if(strlen(integrity_ret.ret) == 8)
                    crc_val = (unsigned int)simple_strtoul(integrity_ret.ret,NULL,16);
                else
                    crc_val = (unsigned int)simple_strtoul(integrity_ret.ret+1,NULL,16);

                sprintf(pRsp->test_mode_rsp.str_buf, "0x%08X", crc_val);

                printk(KERN_INFO "%s\n", integrity_ret.ret);
                printk(KERN_INFO "%d\n", crc_val);
                printk(KERN_INFO "%s\n", pRsp->test_mode_rsp.str_buf);

                pRsp->ret_stat_code = TEST_OK_S;
                break;

            case CODE_PARTITION_CRC_CHECK:
                pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
                break;

            case TOTAL_CRC_CHECK:
                pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
                break;

            case DB_DUMP_CHECK:
                for (i =0; i < 10; i++)
                {
                    if (db_dump_ready)
                        break;

                    msleep(500);
                }

                msleep(500); // wait until the return value is written to the file

                if (integrity_ret.ret[0] == '0')
                    pRsp->ret_stat_code = TEST_OK_S;
                else
                    pRsp->ret_stat_code = TEST_FAIL_S;

                break;

            case DB_COPY_CHECK:
                for (i =0; i < 10; i++)
                {
                    if (db_copy_ready)
                        break;

                    msleep(500);
                }

                msleep(500); // wait until the return value is written to the file

                if (integrity_ret.ret[0] == '0')
                    pRsp->ret_stat_code = TEST_OK_S;
                else
                    pRsp->ret_stat_code = TEST_FAIL_S;

                break;

            default :
                pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
                break;
        }
    }
    else
    {
        printk("\n[%s] error DBCHECK", __func__ );
        pRsp->ret_stat_code = TEST_FAIL_S;
    }

    printk(KERN_ERR "[_DBCHECK_] [%s:%d] DBCHECK Result=<%s>\n", __func__, __LINE__, integrity_ret.ret);

    return pRsp;
}

extern byte fota_id_read[20];

void* LGF_TestModeFOTA(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    int i;

    if (diagpdev != NULL)
    {
        switch( pReq->fota)
        {
            case FOTA_ID_READ:
                for(i=0; i<19; i++)
                    fota_id_read[i] = 0;

                update_diagcmd_state(diagpdev, "FOTAIDREAD", 0);
                msleep(500);

                for(i=0; i<19; i++)
                    pRsp->test_mode_rsp.fota_id_read[i] = fota_id_read[i];

                printk(KERN_ERR "%s, rsp.read_fota_id : %s\n", __func__, (char *)pRsp->test_mode_rsp.fota_id_read);
                pRsp->ret_stat_code = TEST_OK_S;
                break;

            default:
                pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
                break;
        }
    }
    else
        pRsp->ret_stat_code = TEST_FAIL_S;

    return pRsp;
}
//LGE_CHANGE_S[TestMode][jinhwan.do@lge.com] 2012-02-17, Test Mode Porting of WIFI  [Start]
static char wifi_get_rx_packet_info(rx_packet_info_t* rx_info)
{
	const char* src = "/data/misc/wifi/diag_wifi_result";
	char return_value = TEST_FAIL_S;
	char *dest = (void *)0;
	char buf[30];
	off_t fd_offset;
	int fd;
	char *tok, *holder = NULL;
	char *delimiter = ":\r\n";
	substring_t args[MAX_OPT_ARGS];	
	int token;	
	char tmpstr[10];

    mm_segment_t old_fs=get_fs();
    set_fs(get_ds());

    msleep(2000);
	printk(KERN_INFO "[wifi_get_rx_packet_info]");
	if (rx_info == NULL) {
		goto file_fail;
	}
	
	memset(buf, 0x00, sizeof(buf));

    if ( (fd = sys_open((const char __user *)src, O_CREAT | O_RDWR, 0) ) < 0 )
    {
        printk(KERN_ERR "[Testmode Wi-Fi] sys_open() failed!!\n");
        goto file_fail;
    }

    if ( (dest = kmalloc(30, GFP_KERNEL)) )
    {
        fd_offset = sys_lseek(fd, 0, 0);

        if ((sys_read(fd, (char __user *) dest, 30)) < 0)
        {
            printk(KERN_ERR "[Testmode Wi-Fi] can't read path %s \n", src);
            goto file_fail;
        }

#if 0 
		/*	sscanf(dest, "%d:%d", &(rx_info->goodpacket), &(rx_info->badpacket));    */
		strncpy(buf, (const char *)dest, sizeof(buf) - 1) ;

		tok = strtok_r(dest, delimiter, &holder);

		if ( holder != NULL && tok != NULL)
		{
			rx_info->goodpacket = simple_strtoul(tok, (char**)NULL, 10);
			tok = strtok_r(NULL, delimiter, &holder);
			rx_info->badpacket = simple_strtoul(tok, (char**)NULL, 10);
			printk(KERN_ERR "[Testmode Wi-Fi] rx_info->goodpacket %lu, rx_info->badpacket = %lu \n",
				rx_info->goodpacket, rx_info->badpacket);
			return_value = TEST_OK_S;
		}
#else
		if ((memcmp(dest, "30", 2)) == 0) {
			printk(KERN_INFO "rx_packet_cnt read error \n");
			goto file_fail;
		}

		strncpy(buf, (const char *)dest, sizeof(buf) - 1);
		buf[sizeof(buf)-1] = 0;
		holder = &(buf[2]); // skip index, result
		
		while (holder != NULL) {
			tok = strsep(&holder, delimiter);
			
			if (!*tok)
				continue;

			token = match_token(tok, param_tokens, args);
			switch (token) {
			case Param_goodpacket:
				memset(tmpstr, 0x00, sizeof(tmpstr));
				if (0 == match_strlcpy(tmpstr, &args[0], sizeof(tmpstr)))
				{
					printk(KERN_ERR "Error GoodPacket %s", args[0].from);
					continue;
				}
				rx_info->goodpacket = simple_strtol(tmpstr, NULL, 0);
				printk(KERN_INFO "[Testmode Wi-Fi] rx_info->goodpacket = %d", rx_info->goodpacket);
				break;

			case Param_badpacket:
				memset(tmpstr, 0x00, sizeof(tmpstr));
				if (0 == match_strlcpy(tmpstr, &args[0], sizeof(tmpstr)))
				{
					printk(KERN_ERR "Error BadPacket %s\n", args[0].from);
					continue;
				}

				rx_info->badpacket = simple_strtol(tmpstr, NULL, 0);
				printk(KERN_INFO "[Testmode Wi-Fi] rx_info->badpacket = %d", rx_info->badpacket);
				return_value = TEST_OK_S;
				break;

			case Param_end:
			case Param_err:
			default:
				/* silently ignore unknown settings */
				printk(KERN_ERR "[Testmode Wi-Fi] ignore unknown token %s\n", tok);
				break;
			}
		}
#endif
    }

	printk(KERN_INFO "[Testmode Wi-Fi] return_value %d!!\n", return_value);
	
file_fail:    
    kfree(dest);
    sys_close(fd);
    set_fs(old_fs);
    sys_unlink((const char __user *)src);
    return return_value;
}

static char wifi_get_test_results(int index)
{
	const char* src = "/data/misc/wifi/diag_wifi_result";
    char return_value = TEST_FAIL_S;
    char *dest = (void *)0;
	char buf[4]={0};
    off_t fd_offset;
    int fd;
    mm_segment_t old_fs=get_fs();
    set_fs(get_ds());

    msleep(2000);

    if ( (fd = sys_open((const char __user *)src, O_CREAT | O_RDWR, 0644) ) < 0 )
    {
        printk(KERN_ERR "[Testmode Wi-Fi] sys_open() failed!!\n");
        goto file_fail;
    }

    if ( (dest = kmalloc(20, GFP_KERNEL)) )
    {
        fd_offset = sys_lseek(fd, 0, 0);

        if ((sys_read(fd, (char __user *) dest, 20)) < 0)
        {
            printk(KERN_ERR "[Testmode Wi-Fi] can't read path %s \n", src);
            goto file_fail;
        }
        dest[3]='\0';

		sprintf(buf, "%d""1", index);
		buf[3]='\0';
        printk(KERN_INFO "[Testmode Wi-Fi] result %s!!\n", buf);
        printk(KERN_INFO "[Testmode Wi-Fi] resultfile %s!!\n", dest);

        if ((memcmp(dest, buf, 2)) == 0)
            return_value = TEST_OK_S;
        else
            return_value = TEST_FAIL_S;
		
        printk(KERN_ERR "[Testmode Wi-Fi] return_value %d!!\n", return_value);

    }
	
file_fail:
    kfree(dest);
    sys_close(fd);
    set_fs(old_fs);
    sys_unlink((const char __user *)src);

    return return_value;
}

static char wifi_get_test_mode_check(DIAG_TEST_MODE_F_rsp_type *pRsp)
{
    const char *src = "/data/misc/wifi/diag_wifi_result";
    char return_value = TEST_FAIL_S;
    char *dest = (void *)0;
    char buf[4] = {0};
    off_t fd_offset;
    int fd;
    mm_segment_t old_fs=get_fs();
    set_fs(get_ds());

	printk(KERN_INFO "[wifi_get_test_mode_check]");
    if ((fd = sys_open((const char __user *)src, O_CREAT | O_RDWR, 0) ) < 0)  {
        printk(KERN_ERR "[Testmode Wi-Fi] sys_open() failed!!\n");
        goto file_fail;
    }

    if ((dest = kmalloc(20, GFP_KERNEL))) {
        fd_offset = sys_lseek(fd, 0, 0);

        if ((sys_read(fd, (char __user *) dest, 20)) < 0) {
            printk(KERN_ERR "[Testmode Wi-Fi] can't read path %s \n", src);
            goto file_fail;
        }
        memcpy(buf, dest, 2);
        buf[3]='\0';
        printk(KERN_INFO "[Testmode Wi-Fi] result %s!!\n", buf);

        if (buf[0] == '0' && (buf[1] == '0' || buf[1] == '1')) {
            pRsp->ret_stat_code = TEST_OK_S;
        } else {
            pRsp->ret_stat_code = TEST_FAIL_S;
        }

        if (buf[1] == '1') {
            pRsp->test_mode_rsp.wlan_status = 1;
        } else {
            pRsp->test_mode_rsp.wlan_status = 0;
        }

        printk(KERN_ERR "[Testmode Wi-Fi] return_value %d!!\n", pRsp->test_mode_rsp.wlan_status);

    }
	
file_fail:
    kfree(dest);
    sys_close(fd);
    set_fs(old_fs);
    sys_unlink((const char __user *)src);

    return return_value;
}


static test_mode_ret_wifi_ctgry_t divide_into_wifi_category(test_mode_req_wifi_type input)
{
	test_mode_ret_wifi_ctgry_t sub_category = WLAN_TEST_MODE_CTGRY_NOT_SUPPORTED;
	
	if ( input == WLAN_TEST_MODE_CHK) {
		sub_category = WLAN_TEST_MODE_CTGRY_CHK;
	} else if ( input == WLAN_TEST_MODE_54G_ON || 
		WL_IS_WITHIN(WLAN_TEST_MODE_11B_ON, WLAN_TEST_MODE_11A_CH_RX_START, input)) {
		sub_category = WLAN_TEST_MODE_CTGRY_ON;
	} else if ( input == WLAN_TEST_MODE_OFF ) {
		sub_category = WLAN_TEST_MODE_CTGRY_OFF;
	} else if ( input == WLAN_TEST_MODE_RX_RESULT ) {
		sub_category = WLAN_TEST_MODE_CTGRY_RX_STOP;
	} else if ( WL_IS_WITHIN(WLAN_TEST_MODE_RX_START, WLAN_TEST_MODE_RX_RESULT, input) || 
			WL_IS_WITHIN(WLAN_TEST_MODE_LF_RX_START, WLAN_TEST_MODE_MF_TX_START, input)) {
        sub_category = WLAN_TEST_MODE_CTGRY_RX_START;
	} else if ( WL_IS_WITHIN(WLAN_TEST_MODE_TX_START, WLAN_TEST_MODE_TXRX_STOP, input) || 
			WL_IS_WITHIN( WLAN_TEST_MODE_MF_TX_START, WLAN_TEST_MODE_11B_ON, input)) {
		sub_category = WLAN_TEST_MODE_CTGRY_TX_START;
	} else if ( input == WLAN_TEST_MODE_TXRX_STOP) {
		sub_category = WLAN_TEST_MODE_CTGRY_TX_STOP;
	}
	
	printk(KERN_INFO "[divide_into_wifi_category] input = %d, sub_category = %d!!\n", input, sub_category );
	
	return sub_category;	
}

void* LGF_TestModeWLAN(
        test_mode_req_type*	pReq,
        DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	int i;
	static int first_on_try = 10;
	test_mode_ret_wifi_ctgry_t wl_category;

	if (diagpdev != NULL)
	{
		update_diagcmd_state(diagpdev, "WIFI_TEST_MODE", pReq->wifi);

		printk(KERN_ERR "[WI-FI] [%s:%d] WiFiSubCmd=<%d>\n", __func__, __LINE__, pReq->wifi);

		wl_category = divide_into_wifi_category(pReq->wifi);

		/* Set Test Mode */
		switch (wl_category) {
			case WLAN_TEST_MODE_CTGRY_CHK:
				msleep(1000);
				wifi_get_test_mode_check(pRsp);
				printk(KERN_INFO "[WI-FI] ret_stat_code = %d, wlan_status = %d", pRsp->ret_stat_code, pRsp->test_mode_rsp.wlan_status);

				break;
			case WLAN_TEST_MODE_CTGRY_ON:
				//[10sec timeout] when wifi turns on, it takes about 9seconds to bring up FTM mode.
				msleep(7000);
				
				first_on_try = 5;

				pRsp->ret_stat_code = wifi_get_test_results(wl_category);
				pRsp->test_mode_rsp.wlan_status = !(pRsp->ret_stat_code);
				break;

			case WLAN_TEST_MODE_CTGRY_OFF:
				//5sec timeout
				msleep(3000);
				pRsp->ret_stat_code = wifi_get_test_results(wl_category);
				break;

			case WLAN_TEST_MODE_CTGRY_RX_START:
				msleep(2000);
				pRsp->ret_stat_code = wifi_get_test_results(wl_category);
				pRsp->test_mode_rsp.wlan_status = !(pRsp->ret_stat_code);
				break;

			case WLAN_TEST_MODE_CTGRY_RX_STOP:
			{
				rx_packet_info_t rx_info;
				int total_packet = 0;
				int m_rx_per = 0;
				// init
				rx_info.goodpacket = 0;
				rx_info.badpacket = 0;
				// wait 3 sec
				msleep(3000);
				
				pRsp->test_mode_rsp.wlan_rx_results.packet = 0;
				pRsp->test_mode_rsp.wlan_rx_results.per = 0;

				pRsp->ret_stat_code = wifi_get_rx_packet_info(&rx_info);
				if (pRsp->ret_stat_code == TEST_OK_S) {
					total_packet = rx_info.badpacket + rx_info.goodpacket;
					if(total_packet > 0) {
						m_rx_per = (rx_info.badpacket * 1000 / total_packet);
						printk(KERN_INFO "[WI-FI] per = %d, rx_info.goodpacket = %d, rx_info.badpacket = %d ",
							m_rx_per, rx_info.goodpacket, rx_info.badpacket);
					}
					pRsp->test_mode_rsp.wlan_rx_results.packet = rx_info.goodpacket;
					pRsp->test_mode_rsp.wlan_rx_results.per = m_rx_per;
				}				
				break;
			}

			case WLAN_TEST_MODE_CTGRY_TX_START:
				for (i = 0; i< 2; i++)
					msleep(1000);
				pRsp->ret_stat_code = wifi_get_test_results(wl_category);
				pRsp->test_mode_rsp.wlan_status = !(pRsp->ret_stat_code);
				break;

			case WLAN_TEST_MODE_CTGRY_TX_STOP:
				for (i = 0; i< 2; i++)
					msleep(1000);
				pRsp->ret_stat_code = wifi_get_test_results(wl_category);
				pRsp->test_mode_rsp.wlan_status = !(pRsp->ret_stat_code);
				break;

			default:
				pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
				break;
		}
	}
	else
	{
		printk(KERN_ERR "[WI-FI] [%s:%d] diagpdev %d ERROR\n", __func__, __LINE__, pReq->wifi);
		pRsp->ret_stat_code = TEST_FAIL_S;
	}

	return pRsp;
}
/* 2011-10-13, dongseok.ok@lge.com, Add Wi-Fi Testmode function [END] */
//LGE_CHANGE_S[TestMode][jinhwan.do@lge.com] 2012-02-17, Test Mode Porting of WIFI  [END]

// LGE_CHANGE_S, bill.jung@lge.com, 20110808, WiFi MAC R/W Function by DIAG
void* LGF_TestModeWiFiMACRW(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    int fd=0;
    int i=0;
    char *src = (void *)0;
    mm_segment_t old_fs=get_fs();
    set_fs(get_ds());

    printk(KERN_ERR "[LGF_TestModeWiFiMACRW] req_type=%d, wifi_mac_addr=[%s]\n", pReq->wifi_mac_ad.req_type, pReq->wifi_mac_ad.wifi_mac_addr);

    if (diagpdev != NULL)
    {
        if( pReq->wifi_mac_ad.req_type == 0 )
        {
            printk(KERN_ERR "[LGF_TestModeWiFiMACRW] WIFI_MAC_ADDRESS_WRITE.\n");

            //if ( (fd = sys_open((const char __user *) "/data/misc/wifi/diag_mac", O_CREAT | O_RDWR, 0777) ) < 0 )
            if ( (fd = sys_open((const char __user *) "/data/misc/diag_mac", O_CREAT | O_RDWR, 0777) ) < 0 )
            {
                printk(KERN_ERR "[LGF_TestModeWiFiMACRW] Can not open file.\n");
                pRsp->ret_stat_code = TEST_FAIL_S;
                goto file_fail;
            }

            if ( (src = kmalloc(20, GFP_KERNEL)) )
            {
                sprintf( src,"%c%c%c%c%c%c%c%c%c%c%c%c", pReq->wifi_mac_ad.wifi_mac_addr[0],
                    pReq->wifi_mac_ad.wifi_mac_addr[1], pReq->wifi_mac_ad.wifi_mac_addr[2],
                    pReq->wifi_mac_ad.wifi_mac_addr[3], pReq->wifi_mac_ad.wifi_mac_addr[4],
                    pReq->wifi_mac_ad.wifi_mac_addr[5], pReq->wifi_mac_ad.wifi_mac_addr[6],
                    pReq->wifi_mac_ad.wifi_mac_addr[7], pReq->wifi_mac_ad.wifi_mac_addr[8],
                    pReq->wifi_mac_ad.wifi_mac_addr[9], pReq->wifi_mac_ad.wifi_mac_addr[10],
                    pReq->wifi_mac_ad.wifi_mac_addr[11]
                    );

                if ((sys_write(fd, (const char __user *) src, 12)) < 0)
                {
                    printk(KERN_ERR "[LGF_TestModeWiFiMACRW] Can not write file.\n");
                    pRsp->ret_stat_code = TEST_FAIL_S;
                    goto file_fail;
                }
            }

            for( i=0; i< 5; i++ )
            {
                msleep(500);
            }

            update_diagcmd_state(diagpdev, "WIFIMACWRITE", 0);
            pRsp->ret_stat_code = TEST_OK_S;
        }
        else if(  pReq->wifi_mac_ad.req_type == 1 )
        {
            printk(KERN_ERR "[LGF_TestModeWiFiMACRW] WIFI_MAC_ADDRESS_READ.\n");

            update_diagcmd_state(diagpdev, "WIFIMACREAD", 0);

            for( i=0; i< 12; i++ )
            {
                msleep(500);
            }

            //if ( (fd = sys_open((const char __user *) "/data/misc/wifi/diag_mac", O_CREAT | O_RDWR, 0777) ) < 0 )
            if ( (fd = sys_open((const char __user *) "/data/misc/diag_mac", O_CREAT | O_RDWR, 0777) ) < 0 )
            {
                printk(KERN_ERR "[LGF_TestModeWiFiMACRW] Can not open file.\n");
                pRsp->ret_stat_code = TEST_FAIL_S;
                goto file_fail;
            }

            if ( (src = kmalloc(20, GFP_KERNEL)) )
            {
                if ((sys_read(fd, (char __user *) src, 12)) < 0)
                {
                    printk(KERN_ERR "[LGF_TestModeWiFiMACRW] Can not read file.\n");
                    pRsp->ret_stat_code = TEST_FAIL_S;
                    goto file_fail;
                }
            }

            for( i=0; i<14; i++)
            {
                pRsp->test_mode_rsp.key_pressed_buf[i] = 0;
            }

            for( i=0; i< 12; i++ )
            {
                pRsp->test_mode_rsp.read_wifi_mac_addr[i] = src[i];

                if( (src[i]>='0' && src[i]<= '9') || (src[i]>='a' && src[i]<= 'f') || (src[i]>='A' && src[i]<= 'F') )
                {}
                else
                {
                    pRsp->ret_stat_code = TEST_FAIL_S;
                    goto file_fail;
                }
            }

            printk(KERN_ERR "[LGF_TestModeWiFiMACRW] WIFI_MAC_ADDRESS_READ Result : %c%c%c%c%c%c%c%c%c%c%c%c", pRsp->test_mode_rsp.read_wifi_mac_addr[0],
                    pRsp->test_mode_rsp.read_wifi_mac_addr[1], pRsp->test_mode_rsp.read_wifi_mac_addr[2],
                    pRsp->test_mode_rsp.read_wifi_mac_addr[3], pRsp->test_mode_rsp.read_wifi_mac_addr[4],
                    pRsp->test_mode_rsp.read_wifi_mac_addr[5], pRsp->test_mode_rsp.read_wifi_mac_addr[6],
                    pRsp->test_mode_rsp.read_wifi_mac_addr[7], pRsp->test_mode_rsp.read_wifi_mac_addr[8],
                    pRsp->test_mode_rsp.read_wifi_mac_addr[9], pRsp->test_mode_rsp.read_wifi_mac_addr[10],
                    pRsp->test_mode_rsp.read_wifi_mac_addr[11]
            );

            //sys_unlink((const char __user *)"/data/misc/wifi/diag_mac");
            sys_unlink((const char __user *)"/data/misc/diag_mac");

            pRsp->ret_stat_code = TEST_OK_S;
        }
        else
        {
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
        }
    }
    else
    {
        pRsp->ret_stat_code = TEST_FAIL_S;
    }

    file_fail:
    kfree(src);

    sys_close(fd);
    set_fs(old_fs);

    return pRsp;
}
// LGE_CHANGE_E, bill.jung@lge.com, 20110808, WiFi MAC R/W Function by DIAG

void* LGF_TestModePowerReset(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    if (diagpdev != NULL)
    {
        update_diagcmd_state(diagpdev, "REBOOT", 0);
        pRsp->ret_stat_code = TEST_OK_S;
    }
    else
        pRsp->ret_stat_code = TEST_FAIL_S;

    return pRsp;
}

void* LGF_Testmode_ext_device_cmd(test_mode_req_type *pReq, DIAG_TEST_MODE_F_rsp_type *pRsp)
{
    if (diagpdev != NULL)
    {
        switch (pReq->ext_device_cmd)
        {
            case EXT_CARD_AUTO_TEST:
                testmode_result[0] = 0xFF;
                update_diagcmd_state(diagpdev, "EXT_CARD_AUTO_TEST", 0);
                msleep(500);

                if (testmode_result[0] != 0xFF)
                {
                    pRsp->ret_stat_code = TEST_OK_S;
                    pRsp->test_mode_rsp.uim_state = testmode_result[0] - '0';
                }
                else
                    pRsp->ret_stat_code = TEST_FAIL_S;
                break;

            default:
                pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
                break;
        }
    }
    else
        pRsp->ret_stat_code = TEST_FAIL_S;

    return pRsp;
}

static int test_mode_disable_input_devices = 0;
void LGF_TestModeSetDisableInputDevices(int value)
{
    test_mode_disable_input_devices = value;
}
int LGF_TestModeGetDisableInputDevices(void)
{
    return test_mode_disable_input_devices;
}
EXPORT_SYMBOL(LGF_TestModeGetDisableInputDevices);

#if !defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
void* LGF_TestModeKeyLockUnlock(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    char buf[32];
    int len;
    pRsp->ret_stat_code = TEST_FAIL_S;

    switch(pReq->key_lock_unlock)
    {
        case KEY_LOCK:
            LGF_TestModeSetDisableInputDevices(1);

            len = sprintf(buf, "%d", 0);
            android_readwrite_file("/sys/class/leds/lcd-backlight/brightness", NULL, buf, len);

            pRsp->ret_stat_code = TEST_OK_S;
            break;

        case KEY_UNLOCK:
            LGF_TestModeSetDisableInputDevices(0);

            len = sprintf(buf, "%d", 100);
            android_readwrite_file("/sys/class/leds/lcd-backlight/brightness", NULL, buf, len);

            pRsp->ret_stat_code = TEST_OK_S;
            break;

        default:
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
            break;
    }

    return pRsp;
}
#endif
void* LGF_TestModeMP3 (
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	pRsp->ret_stat_code = TEST_OK_S;
//	printk("\n[%s] diagpdev 0x%x", __func__, diagpdev );

	if (diagpdev != NULL){
		if(pReq->mp3_play == MP3_SAMPLE_FILE)
		{
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		}
		else
		{
			update_diagcmd_state(diagpdev, "MP3", pReq->mp3_play);
		}
	}
	else
	{
		printk("\n[%s] error MP3", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
  return pRsp;

}
void* LGT_TestModeVolumeLevel (
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type *pRsp)
{
#if 1
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
		update_diagcmd_state(diagpdev, "VOLUMELEVEL", pReq->volume_level);
	}
	else
	{
		printk("\n[%s] error VOLUMELEVEL", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
#endif 
  return pRsp;

}

void* LGF_TestModeSpeakerPhone(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
#if 1
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
/*BEGIN: 0011452 kiran.kanneganti@lge.com 2010-11-26*/
/*ADD 0011452: Noice cancellation check support for testmode*/	
//#ifdef SINGLE_MIC_PHONE		
#if 0
		if((pReq->speaker_phone == NOMAL_Mic1) || (pReq->speaker_phone == NC_MODE_ON)
			|| (pReq->speaker_phone == ONLY_MIC2_ON_NC_ON) || (pReq->speaker_phone == ONLY_MIC1_ON_NC_ON)
		)
		{
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		}
#endif
//#else
		if(pReq->speaker_phone == NOMAL_Mic1)
		{
			update_diagcmd_state(diagpdev, "SPEAKERPHONE", pReq->speaker_phone);
		}
		else if( ONLY_MIC1_ON_NC_ON == pReq->speaker_phone )
		{	
//			lge_connect_disconnect_back_mic_path_inQTR(0);
//			lge_connect_disconnect_main_mic_path_inQTR(1);
			update_diagcmd_state(diagpdev, "SPEAKERPHONE", pReq->speaker_phone);
		}
		else if( ONLY_MIC2_ON_NC_ON == pReq->speaker_phone )
		{
//			lge_connect_disconnect_back_mic_path_inQTR(1);
//			lge_connect_disconnect_main_mic_path_inQTR(0);
			update_diagcmd_state(diagpdev, "SPEAKERPHONE", pReq->speaker_phone);
		}		
		else if( NC_MODE_ON == pReq->speaker_phone )
		{
//			lge_connect_disconnect_back_mic_path_inQTR(1);
//			lge_connect_disconnect_main_mic_path_inQTR(1);
			update_diagcmd_state(diagpdev, "SPEAKERPHONE", pReq->speaker_phone);
		}

/* END: 0011452 kiran.kanneganti@lge.com 2010-11-26 */
		else
		{
			update_diagcmd_state(diagpdev, "SPEAKERPHONE", pReq->speaker_phone);
		}
	}
	else
	{
		printk("\n[%s] error SPEAKERPHONE", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
#endif
  return pRsp;
 
}
void* LGF_TestMotor(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
#if 1
	pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
		update_diagcmd_state(diagpdev, "MOTOR", pReq->motor);
	}
	else
	{
		printk("\n[%s] error MOTOR", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
#endif 
  return pRsp;

}
//LG_CHANGE_S: [sinjo.mattappallil@lge.com] [2012-06-05], Porting of slate feature from LS840 GB
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
int lgf_key_lock = 0;
#endif
uint8_t if_condition_is_on_air_plain_mode = 0;
//LG_CHANGE_E: [sinjo.mattappallil@lge.com] [2012-06-05], Porting of slate feature from LS840 GB
void* LGF_TestAcoustic(
		test_mode_req_type* pReq ,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
#if 1
    pRsp->ret_stat_code = TEST_OK_S;

	if (diagpdev != NULL){
		if(pReq->acoustic > ACOUSTIC_LOOPBACK_OFF)
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
		#if 0
		if(pReq->acoustic == ACOUSTIC_ON)
			lgf_key_lock = 0x55;
		else if(pReq->acoustic == ACOUSTIC_OFF)
			lgf_key_lock = 0;
		#endif
		update_diagcmd_state(diagpdev, "ACOUSTIC", pReq->acoustic);
	}
	else
	{
		printk("\n[%s] error ACOUSTIC", __func__ );
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
#endif 
  return pRsp;

}
//LGE_CHANGE_S:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
extern void lm3537_backlight_off(void);
extern void lm3537_backlight_on( int level);
void* LGF_TestModeKEYLOCK(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
	pRsp->ret_stat_code = TEST_OK_S;
    if (diagpdev != NULL)
    {
        switch( pReq->req_key_lock)
        {
            case KEY_LOCK_REQ:				
				pRsp->ret_stat_code = TEST_OK_S;
				printk("[Testmode KEY_LOCK] key lock on\n");
				lgf_key_lock =0x55;
				lm3537_backlight_off();				
				break;
			case KEY_UNLOCK_REQ:
				pRsp->ret_stat_code = TEST_OK_S;
				printk("[Testmode KEY_LOCK] key lock off\n");
				lgf_key_lock =0;
				lm3537_backlight_on(55); //level 55 fix...	
				break;
			default:
				printk("[Testmode KEY_LOCK]not support\n");
				pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
                break;				
        }
    }
	else
        pRsp->ret_stat_code = TEST_FAIL_S;
	
    return pRsp;
}
//LGE_CHANGE_E:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
#endif
//LGE_CHANGE_E:<sinjo.mattappallil@lge.com><03/06/2012><Slate feature porting from LS840 GB>
//kurva.venumadhav@lge.com[LS840-ICS] ported from GB
//BEGIN hongkil.kim 20110907 for... reset... android side... prefernce..... call ui.....
void LGF_TestModeFactoryResetAndroidPrefence(void)
{
	if (diagpdev != NULL){
		update_diagcmd_state(diagpdev, "FRSTPREFERENCE", 0);
	}
	else
	{
		printk("\n[%s] error, diagpdev is null!! ", __func__ );
	}
}
//END hongkil.kim 20110907 for... reset... android side... prefernce..... call ui.....
//kurva.venumadhav@lge.com[LS840-ICS] ported from GB
#ifndef SKW_TEST
static unsigned char test_mode_factory_reset_status = FACTORY_RESET_START;
#define BUF_PAGE_SIZE 2048
// BEGIN: 0010090 sehyuny.kim@lge.com 2010-10-21
// MOD 0010090: [FactoryReset] Enable Recovery mode FactoryReset

#define FACTORY_RESET_STR       "FACT_RESET_"
#define FACTORY_RESET_STR_SIZE 11
#define FACTORY_RESET_BLK 1 // read / write on the first block

#define MSLEEP_CNT 100

typedef struct MmcPartition MmcPartition;

struct MmcPartition {
    char *device_index;
    char *filesystem;
    char *name;
    unsigned dstatus;
    unsigned dtype ;
    unsigned dfirstsec;
    unsigned dsize;
};
// END: 0010090 sehyuny.kim@lge.com 2010-10-21
#endif

extern const MmcPartition *lge_mmc_find_partition_by_name(const char *name);

void* LGF_TestModeFactoryReset(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    unsigned char pbuf[50]; //no need to have huge size, this is only for the flag
    const MmcPartition *pMisc_part;
    unsigned char startStatus = FACTORY_RESET_NA;
    int mtd_op_result = 0;
    unsigned long factoryreset_bytes_pos_in_emmc = 0;
    DIAG_TEST_MODE_F_req_type req_ptr;
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) //|| defined(CONFIG_MACH_LGE_C1_BOARD_SPR) //kurva.venumadhav@lge.com[LS840-ICS] ported from GB
#define ERASING_ISD

#ifdef ERASING_INTERNAL_SD //not definded
 const char *envp[] = {"HOME=/", "TERM=linux", NULL, };
  char *argv[] = {"/system/bin/fsck_msdos",
              "-d",
              "/dev/block/mmcblk0p30",
              NULL,
            };
#endif
int i;
#ifdef ERASING_ISD
int fd_sd;
const char *mpt_path = "/mpt/erasingsd\0";
#endif 
#endif
    req_ptr.sub_cmd_code = TEST_MODE_FACTORY_RESET_CHECK_TEST;
    req_ptr.test_mode_req.factory_reset = pReq->factory_reset;
    pRsp->ret_stat_code = TEST_FAIL_S;
  
    lge_mmc_scan_partitions();
    pMisc_part = lge_mmc_find_partition_by_name("misc");
    factoryreset_bytes_pos_in_emmc = (pMisc_part->dfirstsec*512)+PTN_FRST_PERSIST_POSITION_IN_MISC_PARTITION;
  
    printk("LGF_TestModeFactoryReset> mmc info sec : 0x%x, size : 0x%x type : 0x%x frst sec: 0x%lx\n", \
        pMisc_part->dfirstsec, pMisc_part->dsize, pMisc_part->dtype, factoryreset_bytes_pos_in_emmc);

    switch(pReq->factory_reset)
    {
        case FACTORY_RESET_CHECK :
            #if defined(CONFIG_MACH_LGE_I_BOARD_DCM)
            update_diagcmd_state(diagpdev, "FACTORY_RESET_FROM_TESTMODE", 1);
            #endif

            memset((void*)pbuf, 0, sizeof(pbuf));
            mtd_op_result = lge_read_block(factoryreset_bytes_pos_in_emmc, pbuf, FACTORY_RESET_STR_SIZE+2);

            if( mtd_op_result != (FACTORY_RESET_STR_SIZE+2) )
            {
                printk(KERN_ERR "[Testmode]lge_read_block, read data  = %d \n", mtd_op_result);
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }
            else
            {
                //printk(KERN_INFO "\n[Testmode]factory reset memcmp\n");
                if(memcmp(pbuf, FACTORY_RESET_STR, FACTORY_RESET_STR_SIZE) == 0) // tag read sucess
                {
                    startStatus = pbuf[FACTORY_RESET_STR_SIZE] - '0';
                    printk(KERN_INFO "[Testmode]factory reset backup status = %d \n", startStatus);
                }
                else
                {
                    // if the flag storage is erased this will be called, start from the initial state
                    printk(KERN_ERR "[Testmode] tag read failed :  %s \n", pbuf);
                }
            }

            test_mode_factory_reset_status = FACTORY_RESET_INITIAL;
            memset((void *)pbuf, 0, sizeof(pbuf));
            sprintf(pbuf, "%s%d\n",FACTORY_RESET_STR, test_mode_factory_reset_status);
            printk(KERN_INFO "[Testmode]factory reset status = %d\n", test_mode_factory_reset_status);

            mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc, FACTORY_RESET_STR_SIZE+2);
            if(mtd_op_result!= (FACTORY_RESET_STR_SIZE+2))
            {
                printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }
            else
            {
                mtd_op_result = lge_write_block(factoryreset_bytes_pos_in_emmc, pbuf, FACTORY_RESET_STR_SIZE+2);
                if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
                {
                    printk(KERN_ERR "[Testmode]lge_write_block, error num = %d \n", mtd_op_result);
                    pRsp->ret_stat_code = TEST_FAIL_S;
                    break;
                }
            }

            send_to_arm9((void*)&req_ptr, (void*)pRsp);

            if(pRsp->ret_stat_code != TEST_OK_S)
            {
                printk(KERN_ERR "[Testmode]send_to_arm9 response : %d\n", pRsp->ret_stat_code);
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }

            /*LG_FW khlee 2010.03.04 -If we start at 5, we have to go to APP reset state(3) directly */
#ifdef CONFIG_MACH_LGE_C1_BOARD_SPR
            if((startStatus == FACTORY_RESET_COLD_BOOT_END))// || (startStatus == FACTORY_RESET_HOME_SCREEN_END)) //kurva.venumadhav@lge.com[LS840-ICS] ported from GB
#else
            if((startStatus == FACTORY_RESET_COLD_BOOT_END) || (startStatus == FACTORY_RESET_HOME_SCREEN_END))
#endif
                test_mode_factory_reset_status = FACTORY_RESET_COLD_BOOT_START;
            else
                test_mode_factory_reset_status = FACTORY_RESET_ARM9_END;

            memset((void *)pbuf, 0, sizeof(pbuf));
            sprintf(pbuf, "%s%d\n",FACTORY_RESET_STR, test_mode_factory_reset_status);
            printk(KERN_INFO "[Testmode]factory reset status = %d\n", test_mode_factory_reset_status);

            mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc, FACTORY_RESET_STR_SIZE+2);

            if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
            {
                printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }
            else
            {
                mtd_op_result = lge_write_block(factoryreset_bytes_pos_in_emmc, pbuf, FACTORY_RESET_STR_SIZE+2);
                if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
                {
                    printk(KERN_ERR "[Testmode]lge_write_block, error num = %d \n", mtd_op_result);
                    pRsp->ret_stat_code = TEST_FAIL_S;
                    break;
                }
            }
// f120 testmode by jinsu.park, sungmin.kim
#if defined(CONFIG_MACH_LGE_120_BOARD)	  
	  if( test_mode_factory_reset_status == FACTORY_RESET_ARM9_END )
			{
			  update_diagcmd_state(diagpdev, "ERASEMEMORY", 0);
	  
			}
#endif
#ifndef CONFIG_MACH_LGE_C1_BOARD_SPR  //kurva.venumadhav@lge.com [LS840-ICS] ported from GB
            if((startStatus == FACTORY_RESET_COLD_BOOT_END) || (startStatus == FACTORY_RESET_HOME_SCREEN_END))
            {
                if (diagpdev != NULL)
                    update_diagcmd_state(diagpdev, "REBOOT", 0);
                else
                {
                    printk(KERN_INFO "%s, factory reset reboot failed \n", __func__);
                    pRsp->ret_stat_code = TEST_FAIL_S;
                    break;
                }
            }

            printk(KERN_INFO "%s, factory reset check completed \n", __func__);
            pRsp->ret_stat_code = TEST_OK_S;
#else   //kurva.venumadhav@lge.com [LS840-ICS] ported from GB
//kurva.venumadhav@lge.com [LS840-ICS] ported from GB
      printk(KERN_INFO "%s, factory reset check completed \n", __func__);
      pRsp->ret_stat_code = TEST_OK_S;

      {
         udbp_req_type udbReqType;
         memset(&udbReqType,0x00,sizeof(udbp_req_type));

         udbReqType.header.sub_cmd = SRD_INIT_OPERATION;
         LGE_Dload_SRD((udbp_req_type *)&udbReqType,sizeof(udbReqType));//SRD_INIT_OPERATION
         udbReqType.header.sub_cmd = USERDATA_BACKUP_REQUEST;
         LGE_Dload_SRD((udbp_req_type *)&udbReqType,sizeof(udbReqType));//USERDATA_BACKUP_REQUEST
         //		printk(KERN_INFO "%s,backup_nv_counter %d\n", __func__,userDataBackUpInfo.info.srd_backup_nv_counter);
         udbReqType.header.sub_cmd = USERDATA_BACKUP_REQUEST_MDM;
         LGE_Dload_SRD((udbp_req_type *)&udbReqType,sizeof(udbReqType));//USERDATA_BACKUP_REQUEST_MDM
         //		printk(KERN_INFO "%s,backup_nv_counter %d\n", __func__,userDataBackUpInfo.info.srd_backup_nv_counter);
      }

//BEGIN hongkil.kim 20110907 for... reset... android side... prefernce..... call ui.....
      LGF_TestModeFactoryResetAndroidPrefence();
//END hongkil.kim 20110907 for... reset... android side... prefernce..... call ui.....
#endif
//kurva.venumadhav@lge.com [LS840-ICS] ported from GB
            break;

        case FACTORY_RESET_COMPLETE_CHECK:
            send_to_arm9((void*)&req_ptr, (void*)pRsp);
            if(pRsp->ret_stat_code != TEST_OK_S)
            {
                printk(KERN_ERR "[Testmode]send_to_arm9 response : %d\n", pRsp->ret_stat_code);
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }
            break;

        case FACTORY_RESET_STATUS_CHECK:
            memset((void*)pbuf, 0, sizeof(pbuf));
            mtd_op_result = lge_read_block(factoryreset_bytes_pos_in_emmc, pbuf, FACTORY_RESET_STR_SIZE+2 );
            if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
            {
                printk(KERN_ERR "[Testmode]lge_read_block, error num = %d \n", mtd_op_result);
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }
            else
            {
                if(memcmp(pbuf, FACTORY_RESET_STR, FACTORY_RESET_STR_SIZE) == 0) // tag read sucess
                {
                    test_mode_factory_reset_status = pbuf[FACTORY_RESET_STR_SIZE] - '0';
                    printk(KERN_INFO "[Testmode]factory reset status = %d \n", test_mode_factory_reset_status);
                    pRsp->ret_stat_code = test_mode_factory_reset_status;
                }
                else
                {
                    printk(KERN_ERR "[Testmode]factory reset tag fail, set initial state\n");
                    test_mode_factory_reset_status = FACTORY_RESET_START;
                    pRsp->ret_stat_code = test_mode_factory_reset_status;
                    break;
                }
            }

            break;

        case FACTORY_RESET_COLD_BOOT:

            test_mode_factory_reset_status = FACTORY_RESET_COLD_BOOT_START;
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) // || defined(CONFIG_MACH_LGE_C1_BOARD_SPR) //kurva.venumadhav@lge.com[LS840-ICS] ported from GB

#ifdef ERASING_ISD
		if ((fd_sd = sys_open((const char __user *) mpt_path, O_CREAT | O_RDWR, 0) ) < 0 ){
			printk(KERN_ERR "[Testmode factory reset ] Can not access erasingsd.sh\n");
			pRsp->ret_stat_code = TEST_FAIL_S;
			break;
		}
		else{
			 sys_chmod((const char __user *) mpt_path, 0770);
		}
		sys_close(fd_sd);
#endif 
             testmode_result[0] = 0xFF;
/*
            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "MMCFORMAT", 3);
            }
            else
            {
                printk("\n[%s] error EXTERNAL_SOCKET_ERASE", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }

            for (i =0; i < 60; i++)
            {
                if (testmode_result[0] != 0xFF)
                    break;

                msleep(500);
            }
            
            if(testmode_result[0] != 0xFF)
            {
                pRsp->ret_stat_code = TEST_OK_S;
            }
            else
            {
                pRsp->ret_stat_code = TEST_FAIL_S;
                printk(KERN_ERR "[MMCFORMAT] DiagCommandObserver returned fail or didn't return in 30000ms.\n");
            }
 */           
            
            if (diagpdev != NULL)
            {
                update_diagcmd_state(diagpdev, "MMCFORMAT", 2);
            }
            else
            {
                printk("\n[%s] error EXTERNAL_SOCKET_ERASE", __func__ );
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }

            for (i =0; i < 60; i++)
            {
                if (testmode_result[0] != 0xFF)
                    break;

                msleep(500);
            }

            if(testmode_result[0] != 0xFF)
            {
                pRsp->ret_stat_code = TEST_OK_S;
              //  printk(KERN_ERR "[mmcformat] testmode_result = %s , decimal =%d\n", testmode_result, testmode_result[0]);
            }
            else
            {
                pRsp->ret_stat_code = TEST_FAIL_S;
                printk(KERN_ERR "[MMCFORMAT] DiagCommandObserver returned fail or didn't return in 30000ms.\n");
            }
            
            printk(KERN_ERR "[MMCFORMAT] DiagCommandObserver returned success.\n");

#endif
            
            memset((void *)pbuf, 0, sizeof(pbuf));
            sprintf(pbuf, "%s%d",FACTORY_RESET_STR, test_mode_factory_reset_status);
            printk(KERN_INFO "[Testmode]factory reset status = %d\n", test_mode_factory_reset_status);
            mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc,  FACTORY_RESET_STR_SIZE+2);
            if(mtd_op_result!=( FACTORY_RESET_STR_SIZE+2))
            {
                printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }
            else
            {
                mtd_op_result = lge_write_block(factoryreset_bytes_pos_in_emmc, pbuf,  FACTORY_RESET_STR_SIZE+2);
                if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
                {
                    printk(KERN_ERR "[Testmode]lge_write_block, error num = %d \n", mtd_op_result);
                    pRsp->ret_stat_code = TEST_FAIL_S;
                }
            }
            pRsp->ret_stat_code = TEST_OK_S;
            break;

        case FACTORY_RESET_ERASE_USERDATA:
            test_mode_factory_reset_status = FACTORY_RESET_COLD_BOOT_START;
            memset((void *)pbuf, 0, sizeof(pbuf));
            sprintf(pbuf, "%s%d",FACTORY_RESET_STR, test_mode_factory_reset_status);
            printk(KERN_INFO "[Testmode-erase userdata]factory reset status = %d\n", test_mode_factory_reset_status);
            mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc , FACTORY_RESET_STR_SIZE+2);

            if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
            {
                printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }
            else
            {
                mtd_op_result = lge_write_block(factoryreset_bytes_pos_in_emmc, pbuf, FACTORY_RESET_STR_SIZE+2);
                if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
                {
                    printk(KERN_ERR "[Testmode]lge_write_block, error num = %d \n", mtd_op_result);
                    pRsp->ret_stat_code = TEST_FAIL_S;
                    break;
                }
            }

            pRsp->ret_stat_code = TEST_OK_S;
            break;

        case FACTORY_RESET_FORCE_CHANGE_STATUS:
            test_mode_factory_reset_status = FACTORY_RESET_COLD_BOOT_END;
            memset((void *)pbuf, 0, sizeof(pbuf));
            sprintf(pbuf, "%s%d",FACTORY_RESET_STR, test_mode_factory_reset_status);
            printk(KERN_INFO "[Testmode-force_change]factory reset status = %d\n", test_mode_factory_reset_status);

            mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc , FACTORY_RESET_STR_SIZE+2);
            if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
            {
                printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }
            else
            {
                mtd_op_result = lge_write_block(factoryreset_bytes_pos_in_emmc, pbuf, FACTORY_RESET_STR_SIZE+2);
                if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+2))
                {
                    printk(KERN_ERR "[Testmode]lge_write_block, error num = %d \n", mtd_op_result);
                    pRsp->ret_stat_code = TEST_FAIL_S;
                    break;
                }
            }
            pRsp->ret_stat_code = TEST_OK_S;
            break;

        default:
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
            break;
    }

  return pRsp;
}

void* LGF_TestScriptItemSet(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    DIAG_TEST_MODE_F_req_type req_ptr;
    int mtd_op_result = 0;
    const MmcPartition *pMisc_part;
    unsigned long factoryreset_bytes_pos_in_emmc = 0;
    udbp_req_type nv_req;
    memset(&nv_req,0x0,sizeof(udbp_req_type));
    nv_req.req_data.do_dl_entry.backup_used = 2;
    nv_req.req_data.do_dl_entry.binary_class = 2;
    nv_req.req_data.do_dl_entry.factory_reset_required = 1;
    nv_req.req_data.do_dl_entry.device_srd_reset_required = 1;


    req_ptr.sub_cmd_code = TEST_MODE_TEST_SCRIPT_MODE;
    req_ptr.test_mode_req.test_mode_test_scr_mode = pReq->test_mode_test_scr_mode;

    lge_mmc_scan_partitions();
    pMisc_part = lge_mmc_find_partition_by_name("misc");
    factoryreset_bytes_pos_in_emmc = (pMisc_part->dfirstsec*512)+PTN_FRST_PERSIST_POSITION_IN_MISC_PARTITION;

    switch(pReq->test_mode_test_scr_mode)
    {
        case TEST_SCRIPT_ITEM_SET:
            mtd_op_result = lge_erase_block(factoryreset_bytes_pos_in_emmc, (FACTORY_RESET_STR_SIZE+1) );    
            if(mtd_op_result!=(FACTORY_RESET_STR_SIZE+1))
            {
                printk(KERN_ERR "[Testmode]lge_erase_block, error num = %d \n", mtd_op_result);
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }

            #if defined(CONFIG_MACH_LGE_120_BOARD) || defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
            send_to_arm9((void*)&req_ptr, (void*)pRsp);
            printk(KERN_INFO "%s, result : %s\n", __func__, pRsp->ret_stat_code==TEST_OK_S?"OK":"FALSE");
            #else
            pRsp->ret_stat_code = TEST_OK_S;
            #endif
            break;

        case CAL_DATA_BACKUP:
            #ifdef CONFIG_LGE_DLOAD_SRD
            nv_req.header.sub_cmd = SRD_INIT_OPERATION;
            LGE_Dload_SRD((udbp_req_type *)&nv_req,sizeof(nv_req));//SRD_INIT_OPERATION
            nv_req.header.sub_cmd = USERDATA_BACKUP_REQUEST;
            LGE_Dload_SRD((udbp_req_type *)&nv_req,sizeof(nv_req));//USERDATA_BACKUP_REQUEST
            nv_req.header.sub_cmd = USERDATA_BACKUP_REQUEST_MDM;
            LGE_Dload_SRD((udbp_req_type *)&nv_req,sizeof(nv_req));//USERDATA_BACKUP_REQUEST_MDM
            #endif
            break;

        case CAL_DATA_RESTORE:
            send_to_arm9((void*)&req_ptr, (void*)pRsp);
            printk(KERN_INFO "%s, result : %s\n", __func__, pRsp->ret_stat_code==TEST_OK_S?"OK":"FALSE");
            break;

        default:
            #if defined(CONFIG_MACH_LGE_120_BOARD) || defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
            send_to_arm9((void*)&req_ptr, (void*)pRsp);
            printk(KERN_INFO "%s, cmd : %d, result : %s\n", __func__, pReq->test_mode_test_scr_mode, \
                pRsp->ret_stat_code==TEST_OK_S?"OK":"FALSE");

            if(pReq->test_mode_test_scr_mode == TEST_SCRIPT_MODE_CHECK)
            {
                switch(pRsp->test_mode_rsp.test_mode_test_scr_mode)
                {
                    case 0:
                        printk(KERN_INFO "%s, mode : %s\n", __func__, "USER SCRIPT");
                        break;

                    case 1:
                        printk(KERN_INFO "%s, mode : %s\n", __func__, "TEST SCRIPT");
                        break;

                    default:
                        printk(KERN_INFO "%s, mode : %s, returned %d\n", __func__, "NO PRL", pRsp->test_mode_rsp.test_mode_test_scr_mode);
                        break;
                }
            }
            #else
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
            #endif
            break;

    }

    return pRsp;
}

void* LGF_TestModeMLTEnableSet(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    char *src = (void *)0;
    char *dest = (void *)0;
    off_t fd_offset;
    int fd;

    mm_segment_t old_fs=get_fs();
    set_fs(get_ds());

    pRsp->ret_stat_code = TEST_FAIL_S;

    if (diagpdev != NULL)
    {
        if ( (fd = sys_open((const char __user *) "/mpt/enable", O_CREAT | O_RDWR, 0) ) < 0 )
        {
            printk(KERN_ERR "[Testmode MPT] Can not access MPT\n");
            goto file_fail;
        }

        if ( (src = kmalloc(5, GFP_KERNEL)) )
        {
            sprintf(src, "%d", pReq->mlt_enable);
            if ((sys_write(fd, (const char __user *) src, 2)) < 0)
            {
                printk(KERN_ERR "[Testmode MPT] Can not write MPT \n");
                goto file_fail;
            }

            fd_offset = sys_lseek(fd, 0, 0);
        }

        if ( (dest = kmalloc(5, GFP_KERNEL)) )
        {
            if ((sys_read(fd, (char __user *) dest, 2)) < 0)
            {
                printk(KERN_ERR "[Testmode MPT] Can not read MPT \n");
                goto file_fail;
            }

            if ((memcmp(src, dest, 2)) == 0)
                pRsp->ret_stat_code = TEST_OK_S;
            else
                pRsp->ret_stat_code = TEST_FAIL_S;
        }

        file_fail:
            kfree(src);
            kfree(dest);
            sys_close(fd);
            set_fs(old_fs);
    }

    return pRsp;
}

//====================================================================
// Self Recovery Download Support  diag command 249-XX
//====================================================================
#ifdef CONFIG_LGE_DLOAD_SRD  //kabjoo.choi
PACK (void *)LGE_Dload_SRD (PACK (void *)req_pkt_ptr, uint16 pkg_len)
{
    udbp_req_type *req_ptr = (udbp_req_type *) req_pkt_ptr;
    udbp_rsp_type *rsp_ptr = NULL;
    uint16 rsp_len = pkg_len;
    int write_size=0 , mtd_op_result=0;
    rsp_ptr = (udbp_rsp_type *)diagpkt_alloc(DIAG_USET_DATA_BACKUP, rsp_len);

    // DIAG_TEST_MODE_F_rsp_type union type is greater than the actual size, decrease it in case sensitive items
    switch(req_ptr->header.sub_cmd)
    {
        case  SRD_INIT_OPERATION:
            diag_SRD_Init(req_ptr,rsp_ptr);
            break;

        case USERDATA_BACKUP_REQUEST:
            remote_rpc_srd_cmmand(req_ptr, rsp_ptr);  //userDataBackUpStart() ���⼭ ... shared ram ���� �ϵ���. .. 
            diag_userDataBackUp_entrySet(req_ptr,rsp_ptr,0);  //write info data ,  after rpc respons include write_sector_counter  

            //todo ..  rsp_prt->header.write_sector_counter,  how about checking  no active nv item  ; 
            // write ram data to emmc misc partition  as many as retruned setor counters 
            load_srd_shard_base=smem_alloc(SMEM_ERR_CRASH_LOG, SIZE_OF_SHARD_RAM);  //384K byte 

            if (load_srd_shard_base ==NULL)
            {
                ((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
                break;
                // return rsp_ptr;
            }

            write_size= rsp_ptr->rsp_data.write_sector_counter *256; //return nv backup counters  

            if( write_size >SIZE_OF_SHARD_RAM)
            {
                ((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;  //hue..
                break;
            }

            load_srd_kernel_base=kmalloc((size_t)write_size, GFP_KERNEL);

            memcpy(load_srd_kernel_base,load_srd_shard_base,write_size);
            //srd_bytes_pos_in_emmc+512 means that info data already writed at emmc first sector 
            mtd_op_result = lge_write_block(srd_bytes_pos_in_emmc+512, load_srd_kernel_base, write_size);  //512 info data 

            if(mtd_op_result!= write_size)
            {
                ((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
                kfree(load_srd_kernel_base);
                break;
            }

            kfree(load_srd_kernel_base);
            #if 0
            if ( !writeBackUpNVdata( load_srd_base , write_size))
            {
                ((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
                return rsp_ptr;
            }
            #endif
            break;

        case USERDATA_BACKUP_REQUEST_MDM:
            //MDM backup
            ((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_SUCCESS;
            load_srd_shard_base=smem_alloc(SMEM_ERR_CRASH_LOG, SIZE_OF_SHARD_RAM);  //384K byte 

            if (load_srd_shard_base ==NULL)
            {
                ((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
                break;
                // return rsp_ptr;
            }
            load_srd_shard_base+=1200*256 ; //mdm ram offset

            remote_rpc_srd_cmmand(req_ptr, rsp_ptr); //userDataBackUpStart() ���⼭ ... ram ���� �ϵ���. .. 
            diag_userDataBackUp_entrySet(req_ptr,rsp_ptr,1); //write info data ,  after rpc respons include write_sector_counter  remote_rpc_srd_cmmand(req_ptr, rsp_ptr);  //userDataBackUpStart() ���⼭ ... ram ���� �ϵ���. .. 
            write_size= rsp_ptr->rsp_data.write_sector_counter *256; //return nv backup counters  

            if( write_size >0x15000)  //384K = mode ram (300K) + mdm (80K)
            {
                ((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;  //hue..
                break;
            }
            load_srd_kernel_base=kmalloc((size_t)write_size, GFP_KERNEL);
            memcpy(load_srd_kernel_base,load_srd_shard_base,write_size);

            mtd_op_result = lge_write_block(srd_bytes_pos_in_emmc+0x400000+512, load_srd_kernel_base, write_size); //not sector address > 4M byte offset  

            if(mtd_op_result!= write_size)
            {
                ((udbp_rsp_type*)rsp_ptr)->header.err_code = UDBU_ERROR_CANNOT_COMPLETE;
                kfree(load_srd_kernel_base);
                break;
            }
            kfree(load_srd_kernel_base);
            break;

        case GET_DOWNLOAD_INFO :
            break;

        case EXTRA_NV_OPERATION :
            #ifdef LG_FW_SRD_EXTRA_NV
            diag_extraNv_entrySet(req_ptr,rsp_ptr);
            #endif
            break;

        case PRL_OPERATION :
            #ifdef LG_FW_SRD_PRL
            diag_PRL_entrySet(req_ptr,rsp_ptr);
            #endif
            break;

        default :
            rsp_ptr =NULL; //(void *) diagpkt_err_rsp (DIAG_BAD_PARM_F, req_ptr, pkt_len);
            break;
    }

    /* Execption*/
    if (rsp_ptr == NULL){
        return NULL;
    }

  return rsp_ptr;
}
EXPORT_SYMBOL(LGE_Dload_SRD);
#endif 

/*  USAGE
 *  1. If you want to handle at ARM9 side, you have to insert fun_ptr as NULL and mark ARM9_PROCESSOR
 *  2. If you want to handle at ARM11 side , you have to insert fun_ptr as you want and mark AMR11_PROCESSOR.
 */

testmode_user_table_entry_type testmode_mstr_tbl[TESTMODE_MSTR_TBL_SIZE] =
{
    /* sub_command                          fun_ptr                           which procesor*/
    /* 0 ~ 10 */
    {TEST_MODE_VERSION,                     NULL,                             ARM9_PROCESSOR},
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)	//diag
	{TEST_MODE_LCD,                       linux_app_handler,    ARM11_PROCESSOR},
	{TEST_MODE_LCD_CAL,       				LGF_TestLCD_Cal,  ARM11_PROCESSOR },
#else // ATcommand
	{TEST_MODE_LCD,                       not_supported_command_handler,    ARM11_PROCESSOR},
#endif	
	{ TEST_MODE_MOTOR,                    LGF_TestMotor,            ARM11_PROCESSOR},
	{ TEST_MODE_ACOUSTIC,                 LGF_TestAcoustic,         ARM11_PROCESSOR},
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
    {TEST_MODE_CAM,                         LGF_TestCam,    ARM11_PROCESSOR},
#else
    {TEST_MODE_CAM,                         not_supported_command_handler,    ARM11_PROCESSOR},
#endif
    /* 11 ~ 20 */
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)	// [LG_BTUI] '12.05.07 : LG_BTUI_TEST_MODE For VZW    
	   //HELLG ........ is this is my job?????????????  ---------------------------
    {TEST_MODE_IRDA_FMRT_FINGER_UIM_TEST,       LGF_TestModeIrdaFmrtFingerUIMTest,  ARM11_PROCESSOR },
#else 	
	{TEST_MODE_IRDA_FMRT_FINGER_UIM_TEST,   LGF_Testmode_ext_device_cmd,      ARM11_PROCESSOR},
#endif	
    /* 21 ~ 30 */
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)	//diag
    {TEST_MODE_KEY_TEST,                    LGT_TestModeKeyTest,    ARM11_PROCESSOR},
#else // ATcommand
    {TEST_MODE_KEY_TEST,                    not_supported_command_handler,    ARM11_PROCESSOR},
#endif	
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)	// [LG_BTUI] '12.05.07 : LG_BTUI_TEST_MODE For VZW
	{ TEST_MODE_BLUETOOTH_TEST,           LGF_TestModeBlueTooth,    ARM11_PROCESSOR},
#endif	
    {TEST_MODE_EXT_SOCKET_TEST,             LGF_ExternalSocketMemory,         ARM11_PROCESSOR},
    {TEST_MODE_BLUETOOTH_TEST,              not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_BATT_LEVEL_TEST,             LGF_TestModeBattLevel,            ARM11_PROCESSOR},
	{TEST_MODE_MP3_TEST,                    LGF_TestModeMP3,                  ARM11_PROCESSOR},
    /* 31 ~ 40 */
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
    {TEST_MODE_ACCEL_SENSOR_TEST,           linux_app_handler,    ARM11_PROCESSOR},
#else    
    {TEST_MODE_ACCEL_SENSOR_TEST,           not_supported_command_handler,    ARM11_PROCESSOR},
#endif
//[20120614]Add NFC Testmode,addy.kim@lge.com [START]
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)  
	{ TEST_MODE_ALCOHOL_SENSOR_TEST,		LGF_TestModeNFC,				ARM11_PROCESSOR},		
#endif
//20111006, addy.kim@lge.com,	[END]

//20120810, Add Wi-Fi TestMode, sanghyuk.nam@lge.com[START]	
//    {TEST_MODE_WIFI_TEST,                   linux_app_handler,    ARM11_PROCESSOR},
	{TEST_MODE_WIFI_TEST,					LGF_TestModeWLAN,				ARM11_PROCESSOR},
//20120810, sanghyuk.nam@lge.com[END]

    {TEST_MODE_MANUAL_TEST_MODE,            NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_FORMAT_MEMORY_TEST,          not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_KEY_DATA_TEST,               LGF_TestModeKeyData,              ARM11_PROCESSOR},
    /* 41 ~ 50 */
    {TEST_MODE_MEMORY_CAPA_TEST,            LGF_MemoryVolumeCheck,    ARM11_PROCESSOR},
    {TEST_MODE_SLEEP_MODE_TEST,             LGF_TestModeSleepMode,            ARM11_PROCESSOR},
	{ TEST_MODE_SPEAKER_PHONE_TEST,       LGF_TestModeSpeakerPhone, ARM11_PROCESSOR},
    {TEST_MODE_VIRTUAL_SIM_TEST,            LGF_TestModeVirtualSimTest,       ARM11_PROCESSOR},
    {TEST_MODE_PHOTO_SENSER_TEST,           not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_MRD_USB_TEST,                NULL,                             ARM9_PROCESSOR},
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
	{TEST_MODE_PROXIMITY_SENSOR_TEST,       linux_app_handler,    ARM11_PROCESSOR},
#else
    {TEST_MODE_PROXIMITY_SENSOR_TEST,       not_supported_command_handler,    ARM11_PROCESSOR},
#endif    
    {TEST_MODE_TEST_SCRIPT_MODE,            LGF_TestScriptItemSet,            ARM11_PROCESSOR},
    {TEST_MODE_FACTORY_RESET_CHECK_TEST,    LGF_TestModeFactoryReset,         ARM11_PROCESSOR},
    /* 51 ~60 */
	{ TEST_MODE_VOLUME_TEST,              LGT_TestModeVolumeLevel,  ARM11_PROCESSOR},
    {TEST_MODE_FIRST_BOOT_COMPLETE_TEST,    LGF_TestModeFBoot,                ARM11_PROCESSOR},
    {TEST_MODE_MAX_CURRENT_CHECK,           NULL,                             ARM9_PROCESSOR},
    /* 61 ~70 */
    {TEST_MODE_CHANGE_RFCALMODE,            NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_SELECT_MIMO_ANT,             NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_LTE_MODE_SELECTION,          LGF_TestModeMIMO_RFMode,    ARM9_PROCESSOR},
    {TEST_MODE_LTE_CALL,                    LGF_TestModeLTE_Call,             ARM9_PROCESSOR},
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)	//diag

	/* [yk.kim@lge.com] 2011-01-04, change usb driver --------------------*/ 
	{ TEST_MODE_CHANGE_USB_DRIVER, LGF_TestModeChangeUsbDriver, ARM11_PROCESSOR},
// BEGIN: 0010557  unchol.park@lge.com 2011-01-24
#else
	{TEST_MODE_CHANGE_USB_DRIVER,           not_supported_command_handler,    ARM11_PROCESSOR},
#endif
    {TEST_MODE_GET_HKADC_VALUE,             NULL,                             ARM9_PROCESSOR},
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)	//diag
	{TEST_MODE_LED_TEST,                    linux_app_handler,    ARM11_PROCESSOR},
#else
	{TEST_MODE_LED_TEST,                    not_supported_command_handler,    ARM11_PROCESSOR},
#endif
	{TEST_MODE_PID_TEST,                    NULL,                             ARM9_PROCESSOR},
    /* 71 ~ 80 */
    {TEST_MODE_SW_VERSION,                  NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_IME_TEST,                    NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_IMPL_TEST,                   NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_SIM_LOCK_TYPE_TEST,          NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_UNLOCK_CODE_TEST,            NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_IDDE_TEST,                   NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_FULL_SIGNATURE_TEST,         NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_NT_CODE_TEST,                NULL,                             ARM9_PROCESSOR},
    {TEST_MODE_SIM_ID_TEST,                 NULL,                             ARM9_PROCESSOR},
    /* 81 ~ 90*/
// LGE_CHANGE_S  [UICC] Testmode SIM ID READ - minyi.kim@lge.com 2011-10-18 ----------------------------------------------
    { TEST_MODE_SIM_ID_READ,	LGF_TestModeSIMIdRead,							ARM11_PROCESSOR},
// LGE_CHANGE_E  [UICC] Testmode SIM ID READ - minyi.kim@lge.com 2011-10-18
    {TEST_MODE_CAL_CHECK,                   NULL,                             ARM9_PROCESSOR},
//#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)	// [LG_BTUI] '12.05.07 : LG_BTUI_TEST_MODE For VZW
//	{ TEST_MODE_BLUETOOTH_RW,	LGF_TestModeBlueTooth_RW	   , ARM11_PROCESSOR},
//#else
    {TEST_MODE_BLUETOOTH_RW,                NULL,                             ARM9_PROCESSOR},
//#endif	
    {TEST_MODE_SKIP_WELCOM_TEST,            not_supported_command_handler,    ARM11_PROCESSOR},
    {TEST_MODE_WIFI_MAC_RW,                 LGF_TestModeWiFiMACRW,            ARM11_PROCESSOR},
    /* 91 ~ */
    {TEST_MODE_DB_INTEGRITY_CHECK,          LGF_TestModeDBIntegrityCheck,     ARM11_PROCESSOR},
    {TEST_MODE_NVCRC_CHECK,                 NULL,                             ARM9_PROCESSOR},
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
    {TEST_MODE_GYRO_SENSOR_TEST,            linux_app_handler,                ARM11_PROCESSOR},
#endif
/* BEGIN: 0015893 jihoon.lee@lge.com 20110212 */
/* ADD 0015893: [MANUFACTURE] WDC LIFETIMER CLNR LAUNCH_KIT */
//    { TEST_MODE_RESET_PRODUCTION, 		NULL,								ARM9_PROCESSOR},

    //HELLG ........ is this is my job????????????? -----------------------
    { TEST_MODE_RESET_PRODUCTION, 		LGF_TestModeResetProduction,								ARM11_PROCESSOR},

 /* END: 0015893 jihoon.lee@lge.com 20110212 */
    {TEST_MODE_FOTA,                        LGF_TestModeFOTA,                 ARM11_PROCESSOR},
//LG_CHANGE_S: [sinjo.mattappallil@lge.com] [2012-06-05], Porting of slate feature from LS840 GB
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)	
	   {TEST_MODE_KEY_LOCK,			   LGF_TestModeKEYLOCK, 		 ARM11_PROCESSOR},
#else
	   {TEST_MODE_KEY_LOCK_UNLOCK,             LGF_TestModeKeyLockUnlock,        ARM11_PROCESSOR},
#endif	      
//LG_CHANGE_E: [sinjo.mattappallil@lge.com] [2012-06-05], Porting of slate feature from LS840 GB
#if defined(CONFIG_MACH_LGE_I_BOARD_VZW) || defined(CONFIG_MACH_LGE_C1_BOARD_MPS) || defined(CONFIG_MACH_LGE_C1_BOARD_SPR)
   	{TEST_MODE_ACCELERATOR_SENSOR_TEST,     linux_app_handler,                ARM11_PROCESSOR},
    {TEST_MODE_GYRO_SENSOR_TEST_TEST,       linux_app_handler,                ARM11_PROCESSOR},
    {TEST_MODE_COMPASS_SENSOR_TEST,         linux_app_handler,                ARM11_PROCESSOR},
#endif
    {TEST_MODE_POWER_RESET,                 LGF_TestModePowerReset,           ARM11_PROCESSOR},
    {TEST_MODE_MLT_ENABLE,                  LGF_TestModeMLTEnableSet,         ARM11_PROCESSOR},
    #if defined(CONFIG_MACH_LGE_120_BOARD) || defined(CONFIG_MACH_LGE_I_BOARD_VZW) 
    {TEST_MODE_BLOW_COMMAND,                LGF_TestOTPBlowCommand,           ARM11_PROCESSOR},
    {TEST_MODE_WV_PROVISIONING_COMMAND,     LGF_TestWVProvisioningCommand,    ARM11_PROCESSOR},
    #elif defined(CONFIG_MACH_LGE_I_BOARD_DCM)
    {TEST_MODE_FELICA,                      LGF_TestModeFelica,               ARM11_PROCESSOR},
    #endif
    {TEST_MODE_XO_CAL_DATA_COPY,            NULL,                             ARM9_PROCESSOR},
};


#if defined(CONFIG_MACH_LGE_120_BOARD)
static int SD_status_probe(struct platform_device *pdev)
{
	int err;

	err = device_create_file(&pdev->dev, &dev_attr_sd_status);
	if (err < 0) 
		printk("%s : Cannot create the sysfs\n", __func__);

	return 0;
	
}

static struct platform_device SD_status_device = {
	.name = "sd_status_check",
	.id		= -1,
};

static struct platform_driver SD_status_driver = {
	.probe = SD_status_probe,
	.driver = {
		.name = "sd_status_check",
	},
};

int __init sd_status_init(void)
{
	printk("%s\n", __func__);
	platform_device_register(&SD_status_device);

	return platform_driver_register(&SD_status_driver);
}

module_init(sd_status_init);
MODULE_DESCRIPTION("for easy check of SD card status in kernel");
MODULE_AUTHOR("KIMSUNGMIN(smtk.kim@lge.com>");
MODULE_LICENSE("GPL");
#endif


#if defined(CONFIG_MACH_LGE_I_BOARD_DCM)
void* LGF_TestModeFelica(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    int i;

    if (diagpdev != NULL)
    {
        switch (pReq->felica_cmd)
        {
            case FELICA_CAL_CHECK:
                pr_info("%s: FELICA_CAL_CHECK\n", __func__);
                testmode_result[0] = 0xFF;
                update_diagcmd_state(diagpdev, "FELICA_CAL_CHECK", 0);
                for (i = 0; i < 20; i++)
                {
                    if (testmode_result[0] != 0xFF)
                        break;
                    msleep(500);
                }

                if (testmode_result[0] != 0xFF)
                {
                    pr_info("%s: testmode_result(%s)\n", __func__, testmode_result);
                    pRsp->ret_stat_code = testmode_result[0] - '0';
                }
                else
                {
                    pr_info("%s: TEST_FAIL_S\n", __func__);
                    pRsp->ret_stat_code = TEST_FAIL_S;
                }

                break;

            case IDM_READ:
                pr_info("%s: IDM_READ\n", __func__);
                testmode_result[0] = 0xFF;
                update_diagcmd_state(diagpdev, "IDM_READ", 0);
                for (i = 0; i < 20; i++)
                {
                    if (testmode_result[0] != 0xFF)
                        break;
                    msleep(500);
                }
                pr_info("%s: testmode_result(%s)\n", __func__, testmode_result);

                if (testmode_result[0] != 0xFF)
                {
                    pRsp->ret_stat_code = TEST_OK_S;
                    strncpy(pRsp->test_mode_rsp.idm_read, testmode_result, 16);
                }
                else
                {
                    pRsp->ret_stat_code = TEST_FAIL_S;
                    pr_info("%s: TEST_FAIL_S\n", __func__);
                }

                break;

            default:
                pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
                return pRsp;
        }
    }
    else
        pRsp->ret_stat_code = TEST_FAIL_S;

    return pRsp;
}
#endif



//[20120614]Add NFC Testmode,addy.kim@lge.com [START]
#if defined(CONFIG_MACH_LGE_C1_BOARD_SPR)  
static int lg_diag_nfc_result_file_read(int i_testcode ,int *ptrRenCode, char *sz_extra_buff )
{
	int read;
	int read_size;
	char buf[100] = {0,};

	
	mm_segment_t oldfs;
	

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	printk("[NFC] START FILE READ");
	read = sys_open((const char __user *)"/data/misc/diag_nfc", O_CREAT | O_RDWR, 0777);

	if(read < 0) {
		printk(KERN_ERR "%s, NFC Result File Open Fail\n",__func__);
		goto nfc_read_err;
		
	}else {
		printk(KERN_ERR "%s, NFC Result File Open Success\n",__func__);
		 
	}

	read_size = 0;
	printk("[_NFC_] copy read to buf variable From read\n");
	while( sys_read(read, &buf[read_size], 1) == 1){
		//printk("[_NFC_] READ  buf[%d]:%c \n",read_size,buf[read_size]);

		if(buf[read_size] == 0x0A)
		{
				   buf[read_size] = 0x00;
				   break;
		}
		
		read_size++;
	}	

	printk("[_NFC_] READ char %d\n",buf[0]-48);
	
	*ptrRenCode = buf[0]-48; //change ASCII Code to int Number
	
	printk("[_NFC_] lg_diag_nfc_result_file_read : i_result_status == %d\n",*ptrRenCode);

	if((strlen(buf) > 1))
	{
		if(i_testcode == 5 || i_testcode == 7 || i_testcode == 9)
		{
			if(buf == NULL){
				printk(KERN_ERR "[_NFC_] BUFF is NULL\n");
				goto nfc_read_err;
			}
			printk("[_NFC_] lg_diag_nfc_result_file_read : Start Copy From szExtraData -> buf buf\n");
			strcpy( sz_extra_buff,(char*)(&buf[1]) );
		}
		else if( i_testcode == 10)
		{
			if(buf == NULL){
				printk(KERN_ERR "[_NFC_] BUFF is NULL\n");
				goto nfc_read_err;
			}
			printk("[_NFC_] lg_diag_nfc_result_file_read : Start Copy From szExtraData -> buf buf\n");
			strcpy( sz_extra_buff,(char*)(&buf[1]) );
		}
	}

	sys_unlink((const char __user *)"/data/misc/nfc/diag_nfc");
	set_fs(oldfs);
	sys_close(read);

	return 1;

	nfc_read_err:
		set_fs(oldfs);
		sys_close(read);
		return 0;
		
}

void* LGF_TestModeNFC(
		test_mode_req_type*	pReq,
		DIAG_TEST_MODE_F_rsp_type	*pRsp)
{
	int nfc_result = 0;
	
	char szExtraData[100] = {0,};
	pRsp->ret_stat_code = TEST_FAIL_S;
	printk(KERN_ERR "[_NFC_] [%s:%d] SubCmd=<%d>\n", __func__, __LINE__, pReq->nfc);
/*
	if(pReq->nfc == 7)
	{
		pReq->nfc = 10;
	}
*/
	if (diagpdev != NULL && (pReq->nfc != 3) && (pReq->nfc != 2)){ 
		update_diagcmd_state(diagpdev, "NFC_TEST_MODE", pReq->nfc);
		
		if(pReq->nfc == 0)
			msleep(5000);
		else if(pReq->nfc == 1)
			msleep(6000);
		else if(pReq->nfc == 2)
			msleep(5000);
		else if(pReq->nfc == 3)
			msleep(4000);
		else if(pReq->nfc == 4)
			msleep(7000);
		else if(pReq->nfc == 5)
			msleep(5000);
		else if(pReq->nfc == 6)
			msleep(9000);
		else if(pReq->nfc == 7)
			msleep(5000);
		else
			msleep(7000);		
	
		if( (lg_diag_nfc_result_file_read(pReq->nfc,&nfc_result, szExtraData)) == 0 ){
			pRsp->ret_stat_code = TEST_FAIL_S;
			printk(KERN_ERR "[NFC] FIle Open Error");
			goto nfc_test_err;
		}
/*
		if(pReq->nfc == 10)
		{
			pReq->nfc = 7;
		}			
*/
		printk(KERN_ERR "[_NFC_] [%s:%d] Result Value=<%d>\n", __func__, __LINE__, nfc_result);

		if( nfc_result == 0 || nfc_result == 9 ){
			pRsp->ret_stat_code = TEST_OK_S;
			printk(KERN_ERR "[_NFC_] [%s:%d] DAIG RETURN VALUE=<%s>\n", __func__, __LINE__, "TEST_OK");
		}else if(nfc_result == 1){
			pRsp->ret_stat_code = TEST_FAIL_S;
			printk(KERN_ERR "[_NFC_] [%s:%d] DAIG RETURN VALUE=<%s>\n", __func__, __LINE__, "TEST_FAIL");
			goto nfc_test_err;
		}else{
			pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
			printk(KERN_ERR "[_NFC_] [%s:%d] DAIG RETURN VALUE=<%s>\n", __func__, __LINE__, "NOT_SUPPORT");
			goto nfc_test_err;
		}
			
		if( pReq->nfc == 5 || pReq->nfc == 7 || pReq->nfc == 9 || pReq->nfc == 10 )
		{
			if(szExtraData == NULL ){
				printk(KERN_ERR "[_NFC_] [%s:%d] response Data is NULL \n", __func__, __LINE__);
				pRsp->ret_stat_code = TEST_FAIL_S;
				goto nfc_test_err;
			}

			printk("[_NFC_] Start Copy From szExtraData : [%s] -> test_mode_rsp buf\n",szExtraData);


			//save data to response to Diag
			strcpy(pRsp->test_mode_rsp.read_nfc_data,(byte*)szExtraData);
	
			printk(KERN_INFO "%s\n", pRsp->test_mode_rsp.read_nfc_data);
			pRsp->ret_stat_code = TEST_OK_S;
		}
	}
	else
	{
		printk(KERN_ERR "[_NFC_] [%s:%d] SubCmd=<%d> ERROR\n", __func__, __LINE__, pReq->nfc);
		pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
	}
	return pRsp;

	nfc_test_err:

		return pRsp;		
}
#endif
//addy.kim@lge.com,  [END]

#if defined(CONFIG_MACH_LGE_120_BOARD) || defined(CONFIG_MACH_LGE_I_BOARD_VZW)
//#define QFUSETEST
void* LGF_TestOTPBlowCommand(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    int scm_result = 0;
    int i;
    int secure_boot_region_value_lsb = 0;
    int secure_boot_region_value_msb = 0;
    int oem_config_region_value_lsb = 0;
    int oem_config_region_value_msb = 0;
    int read_permission_region_value_lsb = 0;
    int read_permission_region_value_msb = 0;
    int write_permission_region_value_lsb = 0;
    int write_permission_region_value_msb = 0;
    int secondary_key_region_value_lsb = 0;
    int secondary_key_region_value_msb = 0;
#ifdef QFUSETEST
    int spare_region_value_lsb = 0;
    int spare_region_value_msb = 0;
    unsigned int randI=0;
#endif
    #define SEC_HW_LSB_MASK                0xC1FF83FF
    #define SEC_HW_MSB_MASK                0x007FE0FF

    typedef struct
    {
        unsigned int Row_Addr;
        unsigned int Row_LSB_Val;
        unsigned int Row_MSB_Val;
    }blow_data_type;

    blow_data_type blow_data_List[] =
    {
#ifdef QFUSETEST
        {0x700400,        0x12345678 /* auth enable */,        0x87654321},  // Spare region Address: 0x700400 ~ 0x700444
#else
        /* ADDRESS        LSB                            MSB*/
        {0x700310,        0x20 /* auth enable */,        0x0},         /* auth enable , QC table, QC public key entry 0 */
        {0x700220,        0x31 /* OEM_ID */ ,            0x23},        /* RPM DEBUG DISABLE/SC_SPIDEN_DISABLE/SC_DBGEN_DISABLE */
        {0x7000A8,        0x03000000,                    0x0},         /* Read permission for 2nd HW key disabled ,blow after readback check */
        {0x7000B0,        0x51100000,                    0x0}          /* write permission for 2nd HW key/OEM_PK_HASH/OEM_CONFIG/OEM_SEC_BOOT. */
#endif
    };

    struct {
        unsigned int address;        /* physical address */
        unsigned int LSB;            /* blow LSB value */
        unsigned int MSB;            /* blow MSB value */
        unsigned int bus_clk_khz;    /* clock */
    } blow_cmd_buf;

#ifdef QFUSETEST
    struct {
        unsigned char *data;        /* physical address */
        unsigned int size;            /* blow LSB value */
    } rand_cmd_buf;
#endif

    if ( pReq->otp_command == PLAYREADY_KEY_VERIFY )
    {
        LGF_TestPRProvisioningCommand(pReq, pRsp);
        return pRsp;
    }

    secure_boot_region_value_lsb= secure_readl(MSM_QFPROM_BASE+0x310);
    secure_boot_region_value_msb = secure_readl(MSM_QFPROM_BASE+0x314);
    printk(KERN_ERR "secure boot region : Address[0x%X] LSB[0x%X] : MSB[0x%X]  \n", (unsigned int)MSM_QFPROM_BASE+0x310, secure_boot_region_value_lsb, secure_boot_region_value_msb);

    oem_config_region_value_lsb = secure_readl(MSM_QFPROM_BASE+0x220);
    oem_config_region_value_msb = secure_readl(MSM_QFPROM_BASE+0x224);
    printk(KERN_ERR "oem config region : Address[0x%X] LSB[0x%X] MSB[0x%X]  \n", (unsigned int)MSM_QFPROM_BASE+0x220, oem_config_region_value_lsb, oem_config_region_value_msb);

    read_permission_region_value_lsb = secure_readl(MSM_QFPROM_BASE+0x0A8);
    read_permission_region_value_msb = secure_readl(MSM_QFPROM_BASE+0x0AC);
    printk(KERN_ERR "read permission region : Address[0x%X] LSB[0x%X] MSB[0x%X]  \n", (unsigned int)MSM_QFPROM_BASE+0x0A8, read_permission_region_value_lsb, read_permission_region_value_msb);

    write_permission_region_value_lsb = secure_readl(MSM_QFPROM_BASE+0x0B0);
    write_permission_region_value_msb = secure_readl(MSM_QFPROM_BASE+0x0B4);
    printk(KERN_ERR "write permission region : Address[0x%X] LSB[0x%X] MSB[0x%X]  \n", (unsigned int)MSM_QFPROM_BASE+0x0B0, write_permission_region_value_lsb, write_permission_region_value_msb);

#ifdef QFUSETEST
    spare_region_value_lsb = secure_readl(MSM_QFPROM_BASE+0x400);
    spare_region_value_msb = secure_readl(MSM_QFPROM_BASE+0x404);
    printk(KERN_ERR "spare region : Address[0x%X] LSB[0x%X] MSB[0x%X]  \n", (unsigned int)MSM_QFPROM_BASE+0x400, spare_region_value_lsb, spare_region_value_msb);
#endif

    pRsp->ret_stat_code = TEST_FAIL_S;

    for(i=0;i<7;i++)
    {
        secondary_key_region_value_lsb = secure_readl(MSM_QFPROM_BASE+0x268+(8 * i));
        secondary_key_region_value_msb = secure_readl(MSM_QFPROM_BASE+0x26C+(8 * i));
        //printk(KERN_ERR "secondary H/W key region : Address[0x%X] LSB[0x%X] MSB[0x%X]  \n", (unsigned int)MSM_QFPROM_BASE+0x268+(8 * i), secondary_key_region_value_lsb, secondary_key_region_value_msb);
    }

    switch(pReq->otp_command)
    {
        case OTP_WRITE:
        {
            printk(KERN_ERR "Write Command \n");

#ifdef QFUSETEST
			randI =0 ;
			rand_cmd_buf.data = (unsigned char *)virt_to_phys(&randI);
			rand_cmd_buf.size= 4;
			scm_result = scm_call(254, 2, &rand_cmd_buf, sizeof(rand_cmd_buf), NULL, 0);
			printk(KERN_ERR "Rand =%X \n", randI);

			/* Spare region blow */
			i=0;
			blow_cmd_buf.address = blow_data_List[0].Row_Addr;
			blow_cmd_buf.LSB  = blow_data_List[0].Row_LSB_Val;
			blow_cmd_buf.MSB  = blow_data_List[0].Row_MSB_Val;
			blow_cmd_buf.bus_clk_khz = 54875;
			scm_result = scm_call(254, 3, &blow_cmd_buf, sizeof(blow_cmd_buf), NULL, 0);
#else
            /* write permission check */
            if ((((write_permission_region_value_lsb & blow_data_List[3].Row_LSB_Val) == blow_data_List[3].Row_LSB_Val) && 
                    ((write_permission_region_value_msb & blow_data_List[3].Row_MSB_Val) == blow_data_List[3].Row_MSB_Val)))
            {
                printk(KERN_ERR " write permission device \n");
                pRsp->ret_stat_code = TEST_FAIL_S;
                break;
            }

            /* Secondary H/W Key blow */
            for(i=0;i<7;i++)
            {
                /* first random generate */
                secondary_key_region_value_lsb = 0;
                secondary_key_region_value_msb = 0;

                get_random_bytes(&secondary_key_region_value_lsb, 4);
                get_random_bytes(&secondary_key_region_value_msb, 4);
                //printk(KERN_ERR "generate secondary H/W key : LSB[0x%X] MSB[0x%X]  \n", secondary_key_region_value_lsb, secondary_key_region_value_msb);

                /* blow command */
                blow_cmd_buf.address = (uint32)MSM_QFPROM_PHYS+0x268+(8 * i);
                blow_cmd_buf.LSB  = secondary_key_region_value_lsb & SEC_HW_LSB_MASK;
                blow_cmd_buf.MSB  = secondary_key_region_value_msb & SEC_HW_MSB_MASK;
                blow_cmd_buf.bus_clk_khz = 54875;

                printk(KERN_ERR "generate secondary H/W key : LSB[0x%X] MSB[0x%X]  \n", blow_cmd_buf.LSB, blow_cmd_buf.MSB);

                scm_result = scm_call(254, 3, &blow_cmd_buf, sizeof(blow_cmd_buf), NULL, 0);
            }

            /* Secure boot region blow */
            i=0;
            blow_cmd_buf.address = blow_data_List[i].Row_Addr;
            blow_cmd_buf.LSB  = blow_data_List[i].Row_LSB_Val;
            blow_cmd_buf.MSB  = blow_data_List[i].Row_MSB_Val;
            blow_cmd_buf.bus_clk_khz = 54875;
            scm_result = scm_call(254, 3, &blow_cmd_buf, sizeof(blow_cmd_buf), NULL, 0);

            /* OEM Config region blow */
            i=1;
            blow_cmd_buf.address = blow_data_List[i].Row_Addr;
            blow_cmd_buf.LSB  = blow_data_List[i].Row_LSB_Val;
            blow_cmd_buf.MSB  = blow_data_List[i].Row_MSB_Val;
            blow_cmd_buf.bus_clk_khz = 54875;
            scm_result = scm_call(254, 3, &blow_cmd_buf, sizeof(blow_cmd_buf), NULL, 0);

            /* Read Permission region blow */
            i=2;
            blow_cmd_buf.address = blow_data_List[i].Row_Addr;
            blow_cmd_buf.LSB  = blow_data_List[i].Row_LSB_Val;
            blow_cmd_buf.MSB  = blow_data_List[i].Row_MSB_Val;
            blow_cmd_buf.bus_clk_khz = 54875;
            //scm_result = scm_call(254, 3, &blow_cmd_buf, sizeof(blow_cmd_buf), NULL, 0);

            /* Write Permission region blow */
            i=3;
            blow_cmd_buf.address = blow_data_List[i].Row_Addr;
            blow_cmd_buf.LSB  = blow_data_List[i].Row_LSB_Val;
            blow_cmd_buf.MSB  = blow_data_List[i].Row_MSB_Val;
            blow_cmd_buf.bus_clk_khz = 54875;
            //scm_result = scm_call(254, 3, &blow_cmd_buf, sizeof(blow_cmd_buf), NULL, 0);

#endif
            pRsp->ret_stat_code = TEST_OK_S;
        }
        break;

        case OTP_READ:
        {
            printk(KERN_ERR "Read Command \n");
#ifdef QFUSETEST
            if (!(((spare_region_value_lsb & blow_data_List[0].Row_LSB_Val) == blow_data_List[0].Row_LSB_Val) && 
                    ((spare_region_value_msb & blow_data_List[0].Row_MSB_Val) == blow_data_List[0].Row_MSB_Val)))
            {
                printk(KERN_ERR " spare region not blow \n");
                break;
            }
#else
            i=0;
            if (!(((secure_boot_region_value_lsb & blow_data_List[i].Row_LSB_Val) == blow_data_List[i].Row_LSB_Val) && 
                    ((secure_boot_region_value_msb & blow_data_List[i].Row_MSB_Val) == blow_data_List[i].Row_MSB_Val)))
            {
                printk(KERN_ERR " secure boot region not blow \n");
                break;
            }

            i=1;
            if (!(((oem_config_region_value_lsb & blow_data_List[i].Row_LSB_Val) == blow_data_List[i].Row_LSB_Val) && 
                    ((oem_config_region_value_msb & blow_data_List[i].Row_MSB_Val) == blow_data_List[i].Row_MSB_Val)))
            {
                printk(KERN_ERR " oem config region not blow \n");
                break;
            }

            i=2;
            if (!(((read_permission_region_value_lsb & blow_data_List[i].Row_LSB_Val) == blow_data_List[i].Row_LSB_Val) && 
                    ((read_permission_region_value_msb & blow_data_List[i].Row_MSB_Val) == blow_data_List[i].Row_MSB_Val)))
            {
                printk(KERN_ERR "read permission region not blow \n");
                break;
            }

            i=3;
            if (!(((write_permission_region_value_lsb & blow_data_List[i].Row_LSB_Val) == blow_data_List[i].Row_LSB_Val) && 
                    ((write_permission_region_value_msb & blow_data_List[i].Row_MSB_Val) == blow_data_List[i].Row_MSB_Val)))
            {
                printk(KERN_ERR " write permission region not blow \n");
                break;
            }

#endif
            pRsp->ret_stat_code = TEST_OK_S;
        }
        break;

        default:
        {
            printk(KERN_ERR "Unknown command \n");
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
        }
        break;
    }

    return pRsp;
}

void* LGF_TestWVProvisioningCommand(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    int result = 0;
    char *command = "UNKNOWN";

    const char *index_filepath = "/drm/widevine/index.txt";
    const char *result_filepath = "/drm/WV_OK";
    const char *envp[] = {
        "HOME=/",
        "TERM=linux",
        NULL,
    };

    printk(KERN_ERR "[WV] : sub2 = %d / type = %d\n", pReq->wv_command.sub2, pReq->wv_command.type);

    // Type of WV command is 21
    if (pReq->wv_command.type != 21)
    {
        printk(KERN_ERR "[WV] : Unsupported type = %d\n", pReq->wv_command.type);
        pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
        return pRsp;
    }
    else
    {
        // Blind command, remove previous result fist
        mm_segment_t old_fs = get_fs();
        set_fs(KERNEL_DS);
        sys_unlink((const char __user *)result_filepath);
        set_fs(old_fs);
    }

    pRsp->ret_stat_code = TEST_OK_S;

    switch(pReq->wv_command.sub2)
    {
        case WV_ERASE:
        {
            char *argv[] = {
                "WVProvision_Test",
                "cmd_erase",
                NULL,
            };

            printk(KERN_ERR "WV_ERASE : start \n");
            result = call_usermodehelper("/system/xbin/WVProvision_Test", argv, (char**)envp, UMH_WAIT_PROC);
            command = "WV_ERASE";
        }
        break;

        case WV_WRITE:
        {
            char szBuf[256] = {0,};
            char *argv[] = {
                "WVProvision_Test",
                "cmd_write",
                szBuf,
                NULL,
            };

            strncpy(szBuf, pReq->wv_command.data, 256);
            printk(KERN_ERR "WV_WRITE : start / data = %s\n", pReq->wv_command.data);
            result = call_usermodehelper("/system/xbin/WVProvision_Test", argv, (char**)envp, UMH_WAIT_PROC);
            command = "WV_WRITE";
        }
        break;

        case WV_CHECK:
        {
            char szBuf[256] = {0,};
            char *argv[] = {
                "WVProvision_Test",
                "cmd_check",
                szBuf,
                NULL,
            };

            strncpy(szBuf, pReq->wv_command.data, 256);
            printk(KERN_ERR "WV_CHECK : start / data = %s\n", pReq->wv_command.data);
            result = call_usermodehelper("/system/xbin/WVProvision_Test", argv, (char**)envp, UMH_WAIT_PROC);
            command = "WV_CHECK";
        }
        break;

        case WV_WINDEX:
        {
            char szBuf[256] = {0,};
            char *argv[] = {
                "WVProvision_Test",
                "cmd_windex",
                szBuf,
                NULL,
            };

            snprintf(szBuf, 256, "%u", ((unsigned int*)pReq->wv_command.data)[0]);\
            printk(KERN_ERR "WV_WINDEX : start / data = %u\n", ((unsigned int*)pReq->wv_command.data)[0]);
            result = call_usermodehelper("/system/xbin/WVProvision_Test", argv, (char**)envp, UMH_WAIT_PROC);
            command = "WV_WINDEX";
        }
        break;

        case WV_RINDEX:
        {
            unsigned int uindex = 0;
            int fd;
            mm_segment_t old_fs = get_fs();

            printk(KERN_ERR "WV_RINDEX : start \n");

            set_fs(KERNEL_DS);
            fd = sys_open((const char __user *)index_filepath, O_RDONLY, 0);
            if (fd < 0)
            {
                set_fs(old_fs);
                // OK with index 0
                memcpy(pRsp->test_mode_rsp.str_buf, &uindex, 4);

                printk(KERN_ERR "WV_RINDEX : Not exist index file\n");
                printk(KERN_ERR "WV_RINDEX : end  0\n");
                return pRsp;
            }

            // Read index
            sys_read(fd, (void*)&uindex, 4);
            sys_close(fd);
            set_fs(old_fs);
            pRsp->test_mode_rsp.wv_index = uindex;
            printk(KERN_ERR "WV_RINDEX : index = %u\n", uindex);
            printk(KERN_ERR "WV_RINDEX : end  0\n");

            return pRsp;
        }
        break;

        default:
        {
            printk(KERN_ERR "Unknown command \n");
            pRsp->ret_stat_code = TEST_NOT_SUPPORTED_S;
            return pRsp;
        }
        break;
    }

    if (result == 0)
    {
        int fd;
        mm_segment_t old_fs = get_fs();

        // Checking the resulf of call_usermodehelper() is insufficient.
        // So supplement the result using dummy ok file.
        set_fs(KERNEL_DS);
        fd = sys_open((const char __user *)result_filepath, O_RDONLY, 0);
        if (fd < 0)
        {
            printk(KERN_ERR "%s : OK file doesn't exist\n", command);
            result = -1;
        }
        else
        {
            sys_close(fd);
            sys_unlink((const char __user *)result_filepath);
        }
        set_fs(old_fs);
    }

    if (result != 0)
    {
        pRsp->ret_stat_code = TEST_FAIL_S;
    }

    printk(KERN_ERR "%s : end  %x \n", command, result);

    return pRsp;
}

void* LGF_TestPRProvisioningCommand(test_mode_req_type * pReq, DIAG_TEST_MODE_F_rsp_type * pRsp)
{
    int result = -1;
    char *command = "UNKNOWN";
    const char *envp[] = {"HOME=/","TERM=linux",NULL,};

    char *argv[] = {
        "prprovision",
        "pr_cmd_verify",
        NULL,
    };

    printk(KERN_ERR "[PR] : command = %d\n", pReq->otp_command);

    printk(KERN_ERR "[PR]PR_CMD_VERIFY : start \n");
    result = call_usermodehelper("/system/bin/prapp", argv, (char**)envp, UMH_WAIT_PROC);
    command = "PLAYREADY_KEY_CHECK";

    if ( result == 0 )
    {
        pRsp->ret_stat_code = TEST_OK_S;
    }
    else
    {
        pRsp->ret_stat_code = TEST_FAIL_S;
    }

    printk(KERN_ERR "%s : end  %x \n", command, result);
    return pRsp;
}

static ssize_t get_qfuse_blow_status ( struct device *dev, struct device_attribute *attr, char *buf)
{
    int i;
    int secure_boot_region_value_lsb = 0;
    int secure_boot_region_value_msb = 0;
    int oem_config_region_value_lsb = 0;
    int oem_config_region_value_msb = 0;
    int read_permission_region_value_lsb = 0;
    int read_permission_region_value_msb = 0;
    int write_permission_region_value_lsb = 0;
    int write_permission_region_value_msb = 0;

    typedef struct
    {
        unsigned int Row_Addr;
        unsigned int Row_LSB_Val;
        unsigned int Row_MSB_Val;
    }blow_data_type;

    blow_data_type blow_data_List[] =
    {
        /* ADDRESS        LSB                            MSB*/
        {0x700310,        0x20 /* auth enable */,        0x0},         /* auth enable , QC table, QC public key entry 0 */
        {0x700220,        0x31 /* OEM_ID */ ,            0x23},        /* RPM DEBUG DISABLE/SC_SPIDEN_DISABLE/SC_DBGEN_DISABLE */
        {0x7000A8,        0x03000000,                    0x0},         /* Read permission for 2nd HW key disabled ,blow after readback check */
        {0x7000B0,        0x51100000,                    0x0}         /* write permission for 2nd HW key/OEM_PK_HASH/OEM_CONFIG/OEM_SEC_BOOT. */
    };

    if ( buf == NULL ) return 0;
    printk(KERN_ERR "get_qfuse_blow_status Read Command \n");

    secure_boot_region_value_lsb= secure_readl(MSM_QFPROM_BASE+0x310);
    secure_boot_region_value_msb = secure_readl(MSM_QFPROM_BASE+0x314);
    printk(KERN_ERR "secure boot region : Address[0x%X] LSB[0x%X] : MSB[0x%X]  \n", (unsigned int)MSM_QFPROM_BASE+0x310, secure_boot_region_value_lsb, secure_boot_region_value_msb);

    oem_config_region_value_lsb = secure_readl(MSM_QFPROM_BASE+0x220);
    oem_config_region_value_msb = secure_readl(MSM_QFPROM_BASE+0x224);
    printk(KERN_ERR "oem config region : Address[0x%X] LSB[0x%X] MSB[0x%X]  \n", (unsigned int)MSM_QFPROM_BASE+0x220, oem_config_region_value_lsb, oem_config_region_value_msb);

    read_permission_region_value_lsb = secure_readl(MSM_QFPROM_BASE+0x0A8);
    read_permission_region_value_msb = secure_readl(MSM_QFPROM_BASE+0x0AC);
    printk(KERN_ERR "read permission region : Address[0x%X] LSB[0x%X] MSB[0x%X]  \n", (unsigned int)MSM_QFPROM_BASE+0x0A8, read_permission_region_value_lsb, read_permission_region_value_msb);

    write_permission_region_value_lsb = secure_readl(MSM_QFPROM_BASE+0x0B0);
    write_permission_region_value_msb = secure_readl(MSM_QFPROM_BASE+0x0B4);
    printk(KERN_ERR "write permission region : Address[0x%X] LSB[0x%X] MSB[0x%X]  \n", (unsigned int)MSM_QFPROM_BASE+0x0B0, write_permission_region_value_lsb, write_permission_region_value_msb);

    i=0;
    if (!(((secure_boot_region_value_lsb & blow_data_List[i].Row_LSB_Val) == blow_data_List[i].Row_LSB_Val) && 
        ((secure_boot_region_value_msb & blow_data_List[i].Row_MSB_Val) == blow_data_List[i].Row_MSB_Val)))
    {
        printk(KERN_ERR " secure boot region not blow \n");
        buf[0] = '0';
        return sizeof(buf);
    }

    i=1;
    if (!(((oem_config_region_value_lsb & blow_data_List[i].Row_LSB_Val) == blow_data_List[i].Row_LSB_Val) && 
        ((oem_config_region_value_msb & blow_data_List[i].Row_MSB_Val) == blow_data_List[i].Row_MSB_Val)))
    {
        printk(KERN_ERR " oem config region not blow \n");
        buf[0] = '0';
        return sizeof(buf);
    }

    i=2;
    if (!(((read_permission_region_value_lsb & blow_data_List[i].Row_LSB_Val) == blow_data_List[i].Row_LSB_Val) && 
        ((read_permission_region_value_msb & blow_data_List[i].Row_MSB_Val) == blow_data_List[i].Row_MSB_Val)))
    {
        printk(KERN_ERR "read permission region not blow \n");
        buf[0] = '0';
        return sizeof(buf);
    }

    i=3;
    if (!(((write_permission_region_value_lsb & blow_data_List[i].Row_LSB_Val) == blow_data_List[i].Row_LSB_Val) && 
        ((write_permission_region_value_msb & blow_data_List[i].Row_MSB_Val) == blow_data_List[i].Row_MSB_Val)))
    {
        printk(KERN_ERR " write permission region not blow \n");
        buf[0] = '0';
        return sizeof(buf);
    }

    printk(KERN_INFO "get_qfuse_blow_status: SUCCESS \n");
    buf[0] = '1';
    return sizeof(buf);
}

static ssize_t set_qfuse_blow_status(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int r = 0;
    return r;
}

DEVICE_ATTR(qfuse_status, 0644, get_qfuse_blow_status, set_qfuse_blow_status );

static int qfuse_status_probe(struct platform_device *pdev)
{
    int err;
    err = device_create_file(&pdev->dev, &dev_attr_qfuse_status);
    if (err < 0)
        printk("%s : Cannot create the sysfs\n", __func__);

    return 0;
}

static struct platform_device qfuse_status_device = {
    .name = "qfuse_status_check",
    .id   = -1,
};

static struct platform_driver qfuse_status_driver = {
    .probe = qfuse_status_probe,
    .driver = {
        .name = "qfuse_status_check",
    },
};

int __init qfuse_status_init(void)
{
    printk("%s\n", __func__);
    platform_device_register(&qfuse_status_device);

    return platform_driver_register(&qfuse_status_driver);
}

module_init(qfuse_status_init);
MODULE_DESCRIPTION("for easy check qfuse status in kernel");
MODULE_AUTHOR("LEEDANIEL(daniel77.lee@lge.com>");
MODULE_LICENSE("GPL");
#endif
