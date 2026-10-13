#ifndef _ZB_BASE_H_
#define _ZB_BASE_H_

#include <stdint.h>


#define EVENT_FREE_AP_RES  (45+1)	/** free AP resources, when AP quit; AP only receives this evt and should not send it */
#define EVENT_EX_CMD       (45+2)	/** for AP's different screen pass msg, or from widget to AP */
#define EVENT_EX_AP_PAUSE  (45+3)	/** 响闹之前 AP	   会收到该消息 */
#define EVENT_EX_AP_RESUME (45+4)	/** 响闹结束后 AP	   会收到该消息 */
#define EVENT_EX_AUD_PLAYER (45+5)
#define EVENT_EX_AUD_STREAM (45+6)
#define EVENT_EX_TTS        (45+7)
#define EVENT_EX_RECORD     (45+8)
#define EVENT_EX_NETWORK    (45+9) 
#define EVENT_EX_EVAL       (45+10)   /** 语音评测消息 */
#define EVENT_EX_ASR        (45+11)   /** 语音识别消息 */
#define EVENT_DOES_BLOCK_VOLUME_WIN  (45+12)



/* Note: 下面这些消息的值跟相应的不带 C_ 前缀的相应消息(名称上只差 C_ 前缀)能对应上的，其数值必须相等(比如 (int)C_EVENT_EX_CMD == (int)EVENT_EX_CMD) 
 *   加这一组常量的原因是: 在 lv_event.h 里増加消息时, 会导致几乎所有 lvgl 的源文件重新编译, 这一编译过程太耗时. 
 */
#define  C_EVENT_KEY_STAGE      45
#define  EVENT_FREE_AP_RES     (45+1)	/** free AP resources, when AP quit; AP only receives this evt and should not send it */
#define  C_EVENT_FREE_AP_RES   (45+1)	/** free AP resources, when AP quit; AP only receives this evt and should not send it */
#define  EVENT_EX_CMD          (45+2)	/** for AP's different screen pass msg, or from widget to AP */
#define  C_EVENT_EX_CMD        (45+2)	/** for AP's different screen pass msg, or from widget to AP */

#define  C_EVENT_EX_AP_PAUSE   (45+3)	/** 响闹之前 AP	   会收到该消息 */
#define  C_EVENT_EX_AP_RESUME  (45+4)	/** 响闹结束后 AP	   会收到该消息 */
#define  C_EVENT_EX_AUD_PLAYER (45+5)
#define  C_EVENT_EX_AUD_STREAM (45+6)
#define  C_EVENT_EX_TTS        (45+7)
#define  C_EVENT_EX_RECORD     (45+8)
#define  C_EVENT_EX_NETWORK    (45+9)
#define  C_EVENT_EX_EVAL       (45+10)	   /** 语音评测消息 */
#define  C_EVENT_EX_ASR        (45+11)	   /** 语音识别消息 */
#define  C_EVENT_DOES_BLOCK_VOLUME_WIN  (45+12)  /** 查询AP当前是否屏蔽音量框 */

#define  C_EVENT_EX_DOWNLOAD_STOP     (45+15) /** 下载线程结束(完成或失败) */
#define  C_EVENT_DOES_BLOCK_ALARM     (45+16) /** 查询AP当前是否屏蔽响闹 */
#define  C_EVENT_EX_AP_ENTER_SLEEP    (45+17) /** AP 进入睡眠, No 62  */
#define  C_EVENT_EX_AP_EXIT_SLEEP     (45+18) /** AP 退出睡眠, No 63  */
#define  C_EVENT_DOES_BLOCK_SYS_POWER_KEY (45+19) /** 查询AP当前是否屏蔽 POWER键, parm 1短键 2长按 */


#define GLB_PARAM_PATH	"/userdata/_data"

//----------------------------------------------------------------------

extern int zb_postMsgToMainTask(uint16_t msgCode, int parm);



#endif

