#ifndef  _LVGL_MAIN_H_
#define  _LVGL_MAIN_H_

#include <stdint.h>
#include <lvgl/src/hal/lv_hal_indev.h>


#ifndef QUEUE_H
typedef void * QueueHandle_t;
#endif


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	NTFY_AUDIO_STREAM_PLAY_STATUS = 0 ,
	NTFY_AUDIO_PLAYER_PLAY_STATUS ,
	NTFY_AUDIO_RECORDER_STATUS ,
	NTFY_WIFI_STATUS,
	NTFY_EVAL_STATUS,
	NTFY_ASR_STATUS,
}lvgl_queue_type_t;


typedef struct {

	int  status;
	lvgl_queue_type_t type ;
	unsigned char isDirectMsg: 1;
} lvgl_queue_command_t;


enum zb_power_off_cause {
	pwr_off_cause_auto_off = 1,
	pwr_off_cause_key_power,
	pwr_off_cause_low_battery,
	pwr_off_cause_restart,
};


#if 0
void lvgl_main_loop(QueueHandle_t queue);


void power_on_logo_show();


extern QueueHandle_t SYS_getMainQueue(void);

extern bool sys_is_screenOn(void);



/**  清除自动关屏计数
 */
void zb_clear_autoScreenOffCnt(void);

/**  清除自动关机计数
 */
void zb_clear_autoPwrOffCnt(void);



/** 向主线程(的当前主窗体)发送通知消息. 
 * @param msgCode - 通知消息，可以为 lv_event_code_t 或 lvgl_queue_type_t;
 *     为了区分，传送 lvgl_queue_type_t 型消息时 msgCode 值的 bit15 需置 1 (即 |0x8000 ).
 * Note: 一般供子程里调用 
 */
int zb_postMsgToMainTask(uint16_t msgCode, int parm);


#endif

//增
lv_indev_t* lv_indev_get_by_type(lv_indev_type_t type);


void zb_request_power_off(enum zb_power_off_cause cause);


extern void zb_msgbox_sel_no_save(int8_t selNo);  // (用变量)保存消息框选项号 

extern int8_t zb_msgbox_sel_no_get(void);  // 读取(之前记下的)消息框选项号 


/** 写入全局字串(一般用于 AP 之间传递) 
 * @return >= 0 means success
 */
extern int zb_global_str_write(const char * pcText); 

/** 读取全局字串 
 * @return >= 0 means success
 */
extern int zb_global_str_read(char * pcBuf, int bufLen); 


#ifdef __cplusplus
}
#endif

#endif//_LVGL_MAIN_H_

