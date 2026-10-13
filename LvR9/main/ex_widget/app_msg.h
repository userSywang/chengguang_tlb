//======================================================[]
//部分供外部调用的函数
//======================================================[]
#ifndef _APP_MSG_H_
#define _APP_MSG_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include <lvgl/lvgl.h>

#define MSGBOX_TOAST_FLAG  0x10000

typedef  struct _msgbox_exit_param_t{

	void * param; 

}msgbox_exit_param_t;

//typedef  void * msgbox_exit_param_t

typedef void (*msgbox_exit_cb_t)(struct _msgbox_exit_param_t *);


// --------------------------------------------------------------
extern void lv_msgbox_ensure_be_del(void);


// --------------------------------------------------------------
//功能：普通消息框,不带确定按钮，返回消息对象，由父窗口销毁
//---------------------------------------------------------------
lv_obj_t* app_msgbox_obj(lv_obj_t * parent,        const char* str_t, const char* str_c );


// ----------------------------------------------------------
//功能：普通消息框， 自带确定按钮，需按键取消
//
//str_t --标题
//str_t --提示内容
//------------------------------------------------------------
void app_msgbox(const char* str_t, const char* str_c );

// ---------------------  ---------------------------------------------------------------
//功能：延时消息框，可在退出执行传进来的回调及参数，没有可以传空
//str_t --标题
//str_t --提示内容
//timeout -- 延时时间，单位1毫秒; 若带有 MSGBOX_TOAST_FLAG(0x10000)位标志 则表示只显示msg 不会夺走界面原来的焦点;
//timeout_exit_fun -- 延时结束回调，不需要可以为空
//exit_param -- 延时结束回调参数。
//----------------------------------------------------------------------------------------
int app_msgbox_timeout(char* str_t, char* str_c , unsigned int timeout, msgbox_exit_cb_t timeout_exit_fun, msgbox_exit_param_t* pdata);


/** 检查 app_msgbox 提示框是否在显示 
 * @param type - 0 按键确认提示框, 1 定时消失提示框 
 */
bool is_app_msgbox_shown(int16_t type);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_COM_VOL_H_
//======================================================[]
//end.
//======================================================[]
