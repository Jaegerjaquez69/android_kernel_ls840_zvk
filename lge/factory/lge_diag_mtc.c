/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
#include <linux/module.h>
#include <lg_diagcmd.h>
#include <lge_diag_mtc.h>

#include <linux/unistd.h> /*for open/close*/
#include <linux/fcntl.h> /*for O_RDWR*/

#include <linux/fb.h> /* to handle framebuffer ioctls */
#include <linux/ioctl.h>
#include <linux/uaccess.h>

#include <linux/syscalls.h> //for sys operations

#include <linux/input.h> // for input_event
#include <linux/fs.h> // for file struct
#include <linux/types.h> // for ssize_t
#include <linux/input.h> // for event parameters
#include <linux/jiffies.h>
#include <linux/delay.h>
#if 1 //SPRINT_SLATE_KEYPRESS_TEST
#include <linux/crc-ccitt.h>
#include <linux/delay.h>

#define ESC_CHAR     0x7D
#define CONTROL_CHAR 0x7E
#define ESC_MASK     0x20

#define CRC_16_L_SEED           0xFFFF

#define CRC_16_L_STEP(xx_crc, xx_c) \
	crc_ccitt_byte(xx_crc, xx_c)

void *lg_diag_mtc_req_pkt_ptr;
unsigned short lg_diag_mtc_req_pkt_length;
#endif //SPRINT_SLATE_KEYPRESS_TEST

#ifndef LG_FW_DUAL_TOUCH
#define LG_FW_DUAL_TOUCH
#endif
///////////
typedef struct {
  unsigned int handset_7k_key_code;
  unsigned int Android_key_code;
}keycode_trans_type;

#define HANDSET_7K_KEY_TRANS_MAP_SIZE 60
keycode_trans_type handset_7k_keytrans_table[HANDSET_7K_KEY_TRANS_MAP_SIZE]=
{
   {KEY_BACK, '^'}, {KEY_1,'1'}, {KEY_2,'2'}, {KEY_3,'3'}, {KEY_4,'4'}, {KEY_5,'5'}, {KEY_6,'6'}, 
	{KEY_7,'7'}, {KEY_8,'8'}, {KEY_9,'9'}, {KEY_0,'0'}, {KEY_BACKSPACE,'Y'}, {KEY_HOME,'!'}, {KEY_MENU,'O'},
   {KEY_SEARCH, '+'}, {KEY_Q,'q'}, {KEY_W,'w'}, {KEY_E,'e'}, {KEY_R,'r'}, {KEY_T,'t'}, {KEY_Y,'y'}, 
	{KEY_U,'u'}, {KEY_I,'i'}, {KEY_O,'o'}, {KEY_P,'p'}, {KEY_LEFT,'/'},
	{KEY_LEFTALT,'$' }, {KEY_A,'a'}, {KEY_S,'s'}, {KEY_D,'d'}, {KEY_F,'f'}, {KEY_G,'g'}, {KEY_H,'h'}, 
	{KEY_J,'j'}, {KEY_K,'k'}, {KEY_L,'l'}, {KEY_ENTER,'='}, {KEY_UP,'L'}, {KEY_REPLY,32}, {KEY_DOWN,'R'},
	{KEY_LEFTSHIFT, '~'}, {KEY_Z,'z'}, {KEY_X,'x'}, {KEY_C,'c'}, {KEY_V,'v'}, {KEY_B,'b'}, {KEY_N,'n'}, 
	{KEY_M,'m'}, {KEY_DOT, 32}, {KEY_RIGHT,'V'},
	{KEY_MENU,'O'}, {KEY_HOME, '+'}, {KEY_BACK, '^'}, {KEY_SEARCH, '+'}, 
	{KEY_SEND,'S'}, {KEY_END,'E'},
	{KEY_VOLUMEUP,'U'}, {KEY_VOLUMEDOWN,'D'}, {KEY_VIDEO_PREV,'Z'}, {KEY_CAMERA,'A'}
};
///////////
/*===========================================================================

                      EXTERNAL FUNCTION AND VARIABLE DEFINITIONS

===========================================================================*/
extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);
extern PACK(void *) diagpkt_alloc2 (diagpkt_cmd_code_type code, unsigned int length, unsigned int packet_length);
extern PACK(void *) diagpkt_free (PACK(void *)pkt);
extern void send_to_arm9( void*	pReq, void	*pRsp);

#ifdef CONFIG_LGE_DIAG_ATS_ETA_MTC_KEY_LOGGING
extern unsigned long int ats_mtc_log_mask;
extern void diagpkt_commit (PACK(void *)pkt);
extern int event_log_start(void);
extern int event_log_end(void);
#endif

extern int base64_encode(char *text, int numBytes, char *encodedText);
/*===========================================================================

            LOCAL DEFINITIONS AND DECLARATIONS FOR MODULE

  This section contains local definitions for constants, macros, types,
  variables and other items needed by this module.

===========================================================================*/
#ifdef CONFIG_LGE_DIAG_ATS_ETA_MTC_KEY_LOGGING
#define JIFFIES_TO_MS(t) ((t) * 1000 / HZ)
#endif 

extern mtc_user_table_entry_type mtc_mstr_tbl[MTC_MSTR_TBL_SIZE];

// LGE_CHANGE_S [jaekyung83.lee@lge.com] [2011.10.21] [CAYMAN] slate touch logging [START]
#if CONFIG_LGE_DIAG_ICD
extern uint16_t max_x;
extern uint16_t max_y;
#endif
// LGE_CHANGE_E [jaekyung83.lee@lge.com] [2011.10.21] [CAYMAN] slate touch logging [END]

unsigned char g_diag_mtc_check = 0;
unsigned char g_diag_mtc_capture_rsp_num = 0;

#if 1 //SPRINT_SLATE_KEYPRESS_TEST
char mtc_running = 0;
EXPORT_SYMBOL(mtc_running);
#endif //SPRINT_SLATE_KEYPRESS_TEST
/*===========================================================================

                      INTERNAL FUNCTION DEFINITIONS

===========================================================================*/
PACK (void *)LGF_MTCProcess (
        PACK (void	*)req_pkt_ptr,	/* pointer to request packet  */
        unsigned short		pkt_len )		      /* length of request packet   */
{
	DIAG_MTC_F_req_type *req_ptr = (DIAG_MTC_F_req_type *) req_pkt_ptr;
  	DIAG_MTC_F_rsp_type *rsp_ptr = NULL;
  	mtc_func_type func_ptr= NULL;
  	int nIndex = 0;
  
  	g_diag_mtc_check = 1;
#if 1 //SPRINT_SLATE_KEYPRESS_TEST
  	mtc_running = 1;
#endif //SPRINT_SLATE_KEYPRESS_TEST

#if 1
  	for( nIndex = 0 ; nIndex < MTC_MSTR_TBL_SIZE  ; nIndex++)
  	{
    	if( mtc_mstr_tbl[nIndex].cmd_code == req_ptr->hdr.sub_cmd)
    	{
      		if( mtc_mstr_tbl[nIndex].which_procesor == MTC_ARM11_PROCESSOR)
        		func_ptr = mtc_mstr_tbl[nIndex].func_ptr;
      		break;
    	}
    	else if (mtc_mstr_tbl[nIndex].cmd_code == MTC_MAX_CMD)
      		break;
    	else
      		continue;
  	}
#endif
  	printk(KERN_INFO "[MTC]cmd_code : [0x%X], sub_cmd : [0x%X]\n",req_ptr->hdr.cmd_code, req_ptr->hdr.sub_cmd);

  	if( func_ptr != NULL)
  	{
    	printk(KERN_INFO "[MTC]cmd_code : [0x%X], sub_cmd : [0x%X]\n",req_ptr->hdr.cmd_code, req_ptr->hdr.sub_cmd);
    	rsp_ptr = func_ptr((DIAG_MTC_F_req_type*)req_ptr);
  	}

#if 1 //SPRINT_SLATE_KEYPRESS_TEST
	mtc_running = 0;
#endif //SPRINT_SLATE_KEYPRESS_TEST

	return (rsp_ptr);
}

EXPORT_SYMBOL(LGF_MTCProcess);
DIAG_MTC_F_rsp_type* slate_execute(DIAG_MTC_F_req_type *pReq)
{
  	int ret;
  	char cmdstr[100];
	int fd;

  	unsigned int req_len = 0;
  	unsigned int rsp_len = 0;
  	DIAG_MTC_F_rsp_type *pRsp = (void *)NULL;
  	unsigned char *mtc_cmd_buf_encoded = NULL;
  	int lenb64 = 0;
  
  	char *envp[] = {
  		"HOME=/",
  		"TERM=linux",
  		NULL,
  	};
  
  	char *argv[] = {
		"/system/bin/slate",
  		cmdstr,
  		NULL,
  	};

  	printk(KERN_INFO "[SLATE]slate_execute\n");
  
  	switch(pReq->hdr.sub_cmd)
  	{
    	case MTC_KEY_EVENT_REQ_CMD:
      		req_len = sizeof(mtc_key_req_type);
      		rsp_len = sizeof(mtc_key_req_type);
      		printk(KERN_INFO "[slate] KEY_EVENT_REQ rsp_len :(%d)\n", rsp_len);

	  		pRsp = (DIAG_MTC_F_rsp_type *)diagpkt_alloc(DIAG_MTC_F, rsp_len);
      		if (pRsp == NULL) {
        		printk(KERN_ERR "[slate] diagpkt_alloc failed\n");
        		return pRsp;
      		}
      		pRsp->mtc_rsp.key.hold = pReq->mtc_req.key.hold;
      		pRsp->mtc_rsp.key.key_code = pReq->mtc_req.key.key_code;
      		break;

    	case MTC_TOUCH_REQ_CMD:
      		req_len = sizeof(mtc_touch_req_type);
      		rsp_len = sizeof(mtc_touch_req_type);
      		printk(KERN_INFO "[slate] TOUCH_EVENT_REQ rsp_len :(%d)\n", rsp_len);

	  		pRsp = (DIAG_MTC_F_rsp_type *)diagpkt_alloc(DIAG_MTC_F, rsp_len);
      		if (pRsp == NULL) {
        		printk(KERN_ERR "[slate] diagpkt_alloc failed\n");
        		return pRsp;
      		}
      		pRsp->mtc_rsp.touch.screen_id = pReq->mtc_req.touch.screen_id;
      		pRsp->mtc_rsp.touch.action = pReq->mtc_req.touch.action;
      		pRsp->mtc_rsp.touch.x = pReq->mtc_req.touch.x;
      		pRsp->mtc_rsp.touch.y = pReq->mtc_req.touch.y;
      		break;

    	default:
      		printk(KERN_ERR "[slate] unknown sub_cmd : (%d)\n", pReq->hdr.sub_cmd);
      		break;
  	}

  	pRsp->hdr.cmd_code = pReq->hdr.cmd_code;
  	pRsp->hdr.sub_cmd = pReq->hdr.sub_cmd;

  	mtc_cmd_buf_encoded = kmalloc(sizeof(unsigned char)*50, GFP_KERNEL);
  	memset(cmdstr,0x00, 50);
  	memset(mtc_cmd_buf_encoded,0x00, 50);

  	lenb64 = base64_encode((char *)pReq, req_len, (char *)mtc_cmd_buf_encoded);

  	if ( (fd = sys_open((const char __user *) "/system/bin/slate", O_RDONLY ,0) ) < 0 )
  	{
  		printk("\n [slate]can not open /system/bin/slate - execute /system/bin/mtc\n");
  		sprintf(cmdstr, "/system/bin/mtc ");
  	}
  	else
  	{
		memcpy((void*)cmdstr, (void*)mtc_cmd_buf_encoded, lenb64);
	
		printk("\n [slate]execute /system/bin/slate, %s\n", cmdstr);
  		sys_close(fd);
  	}
  	// END: eternalblue@lge.com.2009-10-23
  
  	printk(KERN_INFO "[slate]execute mtc : data - %s\n\n", cmdstr);
  	if ((ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC)) != 0) {
	  	printk(KERN_ERR "[slate]slate failed to run : %i\n", ret);
  	}
  	else
  		printk(KERN_INFO "[slate]execute ok, ret = %d\n", ret);

  	kfree(mtc_cmd_buf_encoded);
  
  	return pRsp;
}
EXPORT_SYMBOL(slate_execute);
mtc_user_table_entry_type mtc_mstr_tbl[MTC_MSTR_TBL_SIZE] =
{ 
/*	sub_command							fun_ptr								which procesor              */
	{ MTC_KEY_EVENT_REQ_CMD				,slate_execute						, MTC_ARM11_PROCESSOR},
	{ MTC_TOUCH_REQ_CMD					,slate_execute						, MTC_ARM11_PROCESSOR},
};
