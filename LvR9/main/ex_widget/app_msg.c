//===============================================[]
// messsege box
//===============================================[]
#include <lvgl/lvgl.h>
#include "app_msg.h"
#include "../utils/misc_utils.h"

#define TAG  "appMsg:"


#if 1

#define	MSG_CON_W	180
#define	MSG_CON_H	100

#define	MSG_CON_X	0
#define	MSG_CON_Y	0


#define	MSG_BTN_W	60
#define	MSG_BTN_H	30

#define	MSG_TEXT_W	160
#define	MSG_TEXT_H	60

#define	MSG_MAX_W	(SCREEN_WIDTH -10)
#define	MSG_MAX_H	(SCREEN_HEIGHT-10)


#define MSG_TIMEOUT	3000

static lv_obj_t* msg_cont = NULL;
static lv_obj_t * msg_label = NULL;

static lv_obj_t* mbox1 = NULL;
static lv_obj_t* mbox_t = NULL;

static lv_obj_t* focus_bk = NULL;

static lv_indev_t * indev_key;
//static lv_indev_t * indev_tp;

static msgbox_exit_cb_t  msg_exit_cb;
static msgbox_exit_param_t * msg_exit_param;

static int32_t  g_mbox_time_st;  // unit: ms 

//----------------------------------;


// 按钮结构体
#define		MSG_ENTER	0
#define		MSG_ESC		1
#define		MSG_CLOSE	2

typedef struct _app_msg_btn_t {
    int num; 			//按钮序号
    const char* str;	//按钮名称字符串

} app_msg_btn_t;


static void msg_timeout_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_current_target(e);

	LV_LOG("%s : %d  btn: %p\n",__FUNCTION__,__LINE__, btn);

	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * mbox = NULL;

	if (code == LV_EVENT_KEY)
	{
		mbox = lv_obj_get_parent(btn);
		const char* bnt_str = lv_msgbox_get_active_btn_text(btn);
		if( bnt_str != NULL)
		{
			LV_LOG("%s : %d  btn: %s\n",__FUNCTION__,__LINE__, bnt_str);
		}
		

		char key = *((char*)lv_event_get_param(e));

//		LV_LOG("%s : %d key:%d\n",__FUNCTION__,__LINE__, key);
		if ((key == LV_KEY_ENTER)||(key == LV_KEY_ESC))
		{
			int32_t tmNowMs = zb_get_time_ms_since_boot();
			if (tmNowMs > g_mbox_time_st + 1300) { 
				lv_msgbox_close(mbox);
				mbox_t = NULL;
				LV_LOG("%s : %d lv_msgbox_close(mbox1).\n",__FUNCTION__,__LINE__);	
				lv_indev_keypad_wait_release();
			}
		}
	}
	else if (code == LV_EVENT_DELETE)  // sugaoyi 20240726 add: msgbox_timeout 销毁时,把相应的 tiemr 也销毁
	{
		mbox = lv_event_get_target(e);
		lv_timer_t * timer = mbox->user_data;
		if (timer)
		{
			lv_timer_del(timer);  mbox->user_data = NULL;
		}
		mbox_t = NULL; // sugaoyi 20241128 for: 确保在 msgbox 销毁时 mbox_t 变量置为空,以避出现再次创建 app_msgbox_timeout 消息框总是失败 的问题;
	}
	
}

static void msg_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_current_target(e);
    lv_obj_t * mbox = lv_obj_get_parent(btn);
	uint32_t bnt_num = lv_msgbox_get_active_btn(btn);
	
	lv_event_code_t code = lv_event_get_code(e);
	LV_LOG_USER("mbox: %p, Button %d clicked", mbox, (int)bnt_num);
	
	if(mbox == NULL)	return;
	
	if (code == LV_EVENT_CLICKED)
	{
//		if(strcmp("确定",bnt_str)== 0)
		if((bnt_num == MSG_ENTER) || (bnt_num == MSG_ESC))
		{
			
			lv_msgbox_close(mbox);
			mbox1 = NULL;
//			if(focus_bk!= NULL)	lv_group_focus_obj(focus_bk);
			LV_LOG("%s : %d lv_msgbox_close(mbox1).\n",__FUNCTION__,__LINE__);
		}
		

	}
	else if (code == LV_EVENT_KEY)
	{
		char key = *((char*)lv_event_get_param(e));

		LV_LOG("%s : %d key:%d\n",__FUNCTION__,__LINE__, key);
	 	if((key == LV_KEY_ENTER)||(key == LV_KEY_ESC))
		{
			lv_msgbox_close(mbox);
			mbox1 = NULL;

		}
		lv_indev_wait_release(lv_indev_get_by_type(LV_INDEV_TYPE_KEYPAD));
	}
	
}

static void app_msg_timer_cb(lv_timer_t * timer)
{
	LV_LOG("%s :[ %d]\n",__FUNCTION__,__LINE__);	
	if((mbox_t != NULL) &&(timer->user_data != NULL ))
	{
	
		LV_LOG("%s : %d timer->user_data:%p\n",__FUNCTION__,__LINE__, timer->user_data);	

		lv_msgbox_close(timer->user_data); // Note, 该 close 会触发调用 lv_timer_del();
		ESP_LOGI(TAG, "%s, aft close", __FUNCTION__);

		mbox_t = NULL;
	}
	else {
		lv_timer_del(timer);
	}
	ESP_LOGI(TAG, "msgTmrCb G");

	if (msg_exit_cb != NULL) {
		ESP_LOGI(TAG, "%s Y", __FUNCTION__);
		msg_exit_cb(msg_exit_param);
		ESP_LOGI(TAG, "%s Z", __FUNCTION__);
	}
}

// ---------------------  消息框 -----------------------------
// 不带确定按钮，返回消息对象，由父窗口销毁
lv_obj_t* app_msgbox_obj(lv_obj_t * parent,        const char* str_t, const char* str_c )
{

	LV_LOG("msg title:%s : contant:%s \n", str_t, str_c);	
    lv_obj_t*  mbox = lv_msgbox_create(parent, str_t,str_c, NULL, false);
    lv_obj_center(mbox);
	return(mbox);
}
// ---------------------  消息框 -----------------------------
// 自带确定按钮，需按键取消
void app_msgbox(const char* str_t, const char* str_c )
{

    static const char * btns[] = {"ENTER", "CLOSE", ""};
	if(mbox1 !=NULL)
	{
		LV_LOG("%s : %d mbox is running.....  %p\n",__FUNCTION__,__LINE__, mbox_t);	
		return;  
	}
	LV_LOG("msg title:%s : contant:%s \n", str_t, str_c);	
	focus_bk = lv_group_get_focused(lv_group_get_default());
    mbox1 = lv_msgbox_create(NULL, str_t,str_c, btns, false);
    lv_obj_add_event_cb(mbox1, msg_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(mbox1, msg_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_center(mbox1);
//	lv_group_focus_next(lv_group_get_default());

}
// ---------------------  延时消息框 -----------------------------
//str_t --标题
//str_t --提示内容
//timeout -- 延时时间，单位1毫秒
//timeout_exit_fun -- 延时结束回调，不需要可以为空
//exit_param -- 延时结束回调参数。
// @return 0 表示成功

int app_msgbox_timeout(char* str_t, char* str_c , unsigned int timeout, msgbox_exit_cb_t timeout_exit_fun, msgbox_exit_param_t* exit_param) 
{
	static const char * no_btn_txtsA[] = { "" };

	ESP_LOGI(TAG, "msgTmout A");
	if(mbox_t !=NULL)
	{
		LV_LOG("%s : %d mbox is running.....  %p\n",__FUNCTION__,__LINE__, mbox_t);	
		return -1;  
	}
	if (str_t == NULL) {
		str_t = "";
	}
	bool doesTakeAwayFocus = true;
	if (timeout & MSGBOX_TOAST_FLAG) {
		timeout &= ~(MSGBOX_TOAST_FLAG);
		doesTakeAwayFocus = false;
	}
	LV_LOG("msg title:%s : contant:%s \n", str_t, str_c);	
	focus_bk = lv_group_get_focused(lv_group_get_default());
	const char * btn_txts = (doesTakeAwayFocus ? no_btn_txtsA : NULL);
	mbox_t = lv_msgbox_create(NULL, str_t,str_c, btn_txts, false);
	lv_obj_add_event_cb(mbox_t, msg_timeout_event_cb, LV_EVENT_KEY, NULL);
	lv_obj_add_event_cb(mbox_t, msg_timeout_event_cb, LV_EVENT_DELETE, NULL);
	lv_obj_center(mbox_t);

	LV_LOG("%s : %d exit_param :%p\n",__FUNCTION__,__LINE__, exit_param);	
	msg_exit_cb = timeout_exit_fun;
	msg_exit_param = exit_param;
	g_mbox_time_st = zb_get_time_ms_since_boot();

	lv_timer_t * msg_timer = lv_timer_create(app_msg_timer_cb, timeout, mbox_t);

	lv_obj_set_user_data(mbox_t, msg_timer);
	return 0;
}


/** 检查 app_msgbox 提示框是否在显示 
 * @param type - 0 按键确认提示框, 1 定时消失提示框 
 */
bool is_app_msgbox_shown(int16_t type)
{
	if (0 == type) {
		return (mbox1 != NULL);
	}
	else {
		return (mbox_t != NULL);
	}
}

#endif


