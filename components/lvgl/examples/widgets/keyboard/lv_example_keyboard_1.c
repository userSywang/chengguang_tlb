#include "../../lv_examples.h"
#if LV_USE_KEYBOARD && LV_BUILD_EXAMPLES

extern const lv_font_t lv_font_montserrat_16;


typedef struct _lv_user_t {    
    struct lv_keyboard_t* kb;
    struct _lv_obj_t * ta1;
    struct _lv_obj_t * ta2;
	
}lv_uset_data;


//----------------------------------------- 
lv_keyboard_t * my_keyboard;


static lv_uset_data data; 


static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
	LV_LOG("%s :[ %d] code: %d\n",__FUNCTION__,__LINE__, code);	
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
//        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
//        lv_keyboard_set_textarea(kb, NULL);
//        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
	else if(code == LV_EVENT_CANCEL){ //取消键，输入结束处理

		LV_LOG("%s :[ %d] LV_EVENT_CANCEL ta: %x\n",__FUNCTION__,__LINE__, ta);	
		LV_LOG("chars: %s \n",lv_textarea_get_text(ta));
		
	}
	else if(code == LV_EVENT_READY){//确认键，输入结束处理
		LV_LOG("%s :[ %d] LV_EVENT_READY ta: %x \n",__FUNCTION__,__LINE__, ta);	
		LV_LOG("chars: %s \n",lv_textarea_get_text(ta));
		
	}
}

static void keyboard_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
 
	LV_LOG("%s :[ %d] code: %d\n",__FUNCTION__,__LINE__, code);	
 	if(code == LV_EVENT_FOCUSED) {
        lv_obj_add_state(kb, LV_STATE_FOCUS_KEY);
    }
	LV_LOG("%s [%d], my_keyboard->ta: %x\n",__FUNCTION__,__LINE__, ((lv_keyboard_t *)kb)->ta);	
		
		

}
static void app_timer_cb(lv_timer_t * timer)
{
//	LV_LOG("%s :[ %d]\n",__FUNCTION__,__LINE__);	
	

	 lv_obj_t * f_obj = lv_group_get_focused(lv_group_get_default());

	LV_LOG("%s [%d], ta1: %x\n",__FUNCTION__,__LINE__, ((lv_uset_data *)timer->user_data)->ta1);
	LV_LOG("%s [%d], ta2: %x\n",__FUNCTION__,__LINE__, ((lv_uset_data *)timer->user_data)->ta2);
	LV_LOG("%s [%d], kb: %x\n",__FUNCTION__,__LINE__, ((lv_uset_data *)timer->user_data)->kb);

	LV_LOG("%s [%d], focus: %x\n",__FUNCTION__,__LINE__, f_obj);	
		
//	int btn_no = lv_btnmatrix_get_selected_btn(((lv_uset_data *)timer->user_data)->kb);

//	LV_LOG("%s [%d], btn_no: %d\n",__FUNCTION__,__LINE__, btn_no);	
	
	my_keyboard = (lv_keyboard_t *)((lv_uset_data *)timer->user_data)->kb;
//	LV_LOG("%s [%d], my_keyboard->ta: %x\n",__FUNCTION__,__LINE__, my_keyboard->ta);

}

void lv_example_keyboard_1(void)
{
    /*Create a keyboard to use it with an of the text areas*/
    lv_obj_t * kb = lv_keyboard_create(lv_scr_act());
    lv_obj_set_style_text_font(kb, &lv_font_montserrat_16, 0);
	lv_obj_add_flag(kb, LV_OBJ_FLAG_CLICK_FOCUSABLE);
	
    /*Create a text area. The keyboard will write here*/
    lv_obj_t * ta;
    ta = lv_textarea_create(lv_scr_act());
    lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 10, 10);
	
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
	
    lv_textarea_set_placeholder_text(ta, "password");
	
    lv_obj_set_size(ta, 150, 50);
	data.ta1 = ta;
	
    ta = lv_textarea_create(lv_scr_act());
	
    lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 10, 100);
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
    lv_obj_set_size(ta, 100, 50);
	
    lv_obj_add_event_cb(kb, keyboard_event_cb, LV_EVENT_CLICKED, kb);
    lv_obj_add_event_cb(kb, keyboard_event_cb, LV_EVENT_FOCUSED, kb);
    lv_obj_add_event_cb(kb, keyboard_event_cb, LV_EVENT_KEY, kb);

    lv_keyboard_set_textarea(kb, ta);

	data.ta2 = ta;
	data.kb = (lv_keyboard_t *)kb;
	
//	lv_timer_t * msg_timer = lv_timer_create(app_timer_cb, 3000, (void*)&data);
	
	
}
#endif
