#include "../../lv_examples.h"
#if LV_USE_MSGBOX && LV_BUILD_EXAMPLES

static void event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_current_target(e);
    uint16_t btnNo = lv_msgbox_get_active_btn(obj);
    LV_LOG_USER("Button (No%d) %s clicked", (int)btnNo, lv_msgbox_get_active_btn_text(obj));
}

void lv_example_msgbox_1(void)
{
#if 1
#if 1
    static const char * btns[] = {"Apply确认", "Close取消", ""};

    lv_obj_t * mbox1 = lv_msgbox_create(NULL, "Hello", "This is a message box with two buttons.", btns, true);
    lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_center(mbox1);
#endif

	//app_msgbox_timeout("提示", "已经学到最末一个单词", 5000);


#else
    static const char * btns[] = {""};

    lv_obj_t * mbox1 = lv_msgbox_create(NULL, "", "This is a message box with two buttons. 广东省深圳市", btns, false);
    lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_center(mbox1);
#endif
}

#endif
