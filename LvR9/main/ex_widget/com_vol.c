
#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"
#include "../utils/global_parm.h"
#include "../data_parse/img_res.h"
#ifndef C_PC_SIM
#include "../../trd/tz_audio_player/include/tz_audio_player.h"
#endif


#if 1

#define TAG  "comVol"


#define VOL_DEBUG_FREE_AP_RES  1  // 为1 表示调试一键退出 AP 


#ifndef C_PC_SIM
#define C_VOLUME_DLG_SHOW_IDLE_TIME  4000
#else
#define C_VOLUME_DLG_SHOW_IDLE_TIME  5000
#endif

#define VOL_CONT_WIDTH (VOL_SLIDER_WIDTH+26)
#define VOL_CONT_HEIGH (VOL_SLIDER_HEIGH+45)

#define VOL_SLIDER_WIDTH 48
#define VOL_SLIDER_HEIGH 300

static void vod_event_cb(lv_event_t * e);
static int sys_vol = -1;
static lv_group_t * sys_group;
static lv_obj_t * slider_bg;
static lv_obj_t * slider_cont;
static lv_obj_t * slider;


static lv_obj_t *  slider_label;

lv_timer_t * s_timer = NULL;
int timer_data = 1; 
#if VOL_DEBUG_FREE_AP_RES != 0
bool g_doesReqQuitAp;
#endif


void vol_box_del(void)
{
	LV_LOG_USER("del A");

	if (slider_bg) 
	{
		lv_obj_del(slider_bg);
		slider_bg = NULL;
	}
	LV_LOG_USER("vol_box_delete Z\n");
}

static void vol_timer_cb(lv_timer_t * timer)
{
	LV_LOG_USER("----------timer->user_data : %d\n", *(int*)(timer->user_data));
	//---- sugy 20240827 modify,  layer_stack 里的各层,必须是从外层起 逐层地退. 
	if (sys_group && zb_is_groupOnTopLayer(sys_group)) {
		vol_box_del();
	}
}

static void update_vol_slider(int vol)
{
	LV_LOG("FUNCTION:%s,[%d] vol:%d", __FUNCTION__,__LINE__, vol);
	//char buf[10];	
	if (vol < 0 || vol > MAX_VOL) {
		ESP_LOGE(TAG, "error vol,  correct it.");
		vol = 0;
	}
	lv_slider_set_value(slider, vol, LV_ANIM_OFF);		
}

static void vod_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	/* if (code == LV_EVENT_CLICKED)
	{
		sys_vol = lv_slider_get_value(slider);
		
		update_vol_slider(sys_vol);
		if(s_timer ==NULL)
		{
			timer_data = 1;
			s_timer = lv_timer_create(vol_timer_cb, C_VOLUME_DLG_SHOW_IDLE_TIME, &timer_data);
		}
		else
		{
			lv_timer_reset(s_timer);
			
			LV_LOG("%s : %d reset timer.\n",__FUNCTION__,__LINE__);
		}	
	}
	else */ if(code == LV_EVENT_DEFOCUSED)
	{
		LV_LOG("%s : %d DEFOCUSED ....\n",__FUNCTION__,__LINE__);

	}
	else if (code == LV_EVENT_DELETE) {
		slider = NULL;
		ESP_LOGI(TAG, "_delete Z");
	}
}

static void vod_val_chg_evt_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (LV_EVENT_VALUE_CHANGED == code)
	{
		sys_vol = lv_slider_get_value(slider);
		int16_t volume = PARM_VOLUME_GLB_TO_DRV(sys_vol);
		ESP_LOGI(TAG, "set vol: %d", volume);
	#ifndef C_PC_SIM
		tz_play_set_volume(volume);
	#endif

		update_vol_slider(sys_vol);
		if (s_timer != NULL) {
			lv_timer_reset(s_timer);
		}
	}
}

static void vod_bg_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code) {
	case LV_EVENT_KEY:
	case LV_EVENT_KEY_STAGE:
    {
		int key = *((int*)lv_event_get_param(e));

		if (code == LV_EVENT_KEY_STAGE) {
		#if VOL_DEBUG_FREE_AP_RES != 0
			if ((LV_KEY_EX_FUNC1|LV_EX_FLAG_KEY_LONG_UP) == key)
			{
			#if VOL_DEBUG_FREE_AP_RES != 0
				g_doesReqQuitAp = true;
			#endif
				vol_box_del();
				lv_indev_keypad_wait_release();
				return;
			}
		#endif

			if ((LV_EX_FLAG_KEY_MASK & key) != LV_EX_FLAG_KEY_REPEAT) return;
			key &= ~LV_EX_FLAG_KEY_MASK;
		}
//		sys_vol = lv_slider_get_value(slider);
		LV_LOG("%s : %d key:%d\n",__FUNCTION__,__LINE__, key);
		if ((key == LV_KEY_DOWN) || (key == LV_KEY_UP)
			|| (key == LV_KEY_EX_VOLUME_DEC) || (key == LV_KEY_EX_VOLUME_INC))
        {
			if ((key == LV_KEY_DOWN) || (key == LV_KEY_EX_VOLUME_DEC))
			{
				if(sys_vol > 0) {
					sys_vol--;
				#ifndef C_PC_SIM
					int16_t volume = PARM_VOLUME_GLB_TO_DRV(sys_vol);
					tz_play_set_volume(volume);
				#endif
				}
			}
			else if ((key == LV_KEY_UP) || (key == LV_KEY_EX_VOLUME_INC))
			{
				if (sys_vol < MAX_VOL) {
					sys_vol++;
				#ifndef C_PC_SIM
					int16_t volume = PARM_VOLUME_GLB_TO_DRV(sys_vol);
					tz_play_set_volume(volume);
				#endif
				}
			}
			
			update_vol_slider(sys_vol);

			if(s_timer == NULL)
			{
				timer_data = 1;
				s_timer = lv_timer_create(vol_timer_cb, C_VOLUME_DLG_SHOW_IDLE_TIME, &timer_data);
			}
			else
			{
				lv_timer_reset(s_timer);
				
				LV_LOG("%s : %d reset timer.\n",__FUNCTION__,__LINE__);
			}	
		}
		else if((key == LV_KEY_ENTER)||(key == LV_KEY_ESC))
		{
			vol_box_del();
			lv_indev_keypad_wait_release();
		}
		break;
	}
	case LV_EVENT_SHORT_CLICKED: {
		lv_indev_t * indevTp = lv_indev_get_act();
		if (!indevTp)
			break;
		int x = indevTp->proc.types.pointer.last_point.x;
		if (x < SCREEN_WIDTH - 50) {
			lv_obj_del(slider_bg);
			lv_indev_wait_release(indevTp);
			lv_event_stop_processing(e);
		}
		break;
	}
	case LV_EVENT_DEFOCUSED:
	{
		LV_LOG("%s : %d DEFOCUSED ....\n",__FUNCTION__,__LINE__);
		break;
	}
	case LV_EVENT_DELETE: {
		setPrjGlobalParm(ZB_PARM_VOLUME, 2, &sys_vol);

		if (sys_group != NULL) 
		{
			lv_group_del(sys_group);  sys_group = NULL;
		}
		
		if (s_timer) 
		{
			lv_timer_del(s_timer);  s_timer = NULL;
		}
		
		LV_LOG_USER("_delete B\n");
		zb_group_pop(false);
	#if VOL_DEBUG_FREE_AP_RES != 0
		if (g_doesReqQuitAp) {	
			g_doesReqQuitAp = false;
			lv_group_t * groupDflt = lv_group_get_default();
			if (groupDflt && zb_is_groupOnTopLayer(groupDflt)) {
				LV_LOG_USER("reqSwt to mmnu");
				zb_requestSwitchAp(C_ZB_AP_MAINMENU);
			}
		}
	#endif
		slider_bg = NULL;  slider_cont = NULL;
		LV_LOG_USER("_delete Z\n");
		break;
	}
	default:
		break;
	}
}


/**
 *Volume adjustment box
 */
void lv_com_vol(int vol)
{
	ESP_LOGI(TAG, "comVol A");
	if(slider_cont !=NULL ) return;

	g_doesReqQuitAp = false;

	sys_group = lv_group_create();
	ESP_LOGI(TAG, "comVol A3");
	zb_group_push(sys_group, 0);

	/*Create a slider in the center of the display*/
	slider_bg = lv_obj_create(lv_layer_top());
	slider_bg->flags |= LV_OBJ_FLAG_OUT_OF_DEF_GROUP;
	lv_obj_set_size(slider_bg, SCREEN_WIDTH, SCREEN_HEIGHT);
	lv_obj_set_style_border_width(slider_bg, 0, 0);
	lv_obj_set_style_radius(slider_bg, 0, 0);
	lv_obj_add_flag(slider_bg, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_style_bg_opa(slider_bg, 0, 0);
	//-----------------
	slider_cont = lv_obj_create(slider_bg);
	slider_cont->flags |= LV_OBJ_FLAG_OUT_OF_DEF_GROUP;
	lv_obj_set_style_border_width(slider_cont, 0, 0);
	lv_obj_set_style_bg_opa(slider_cont, 0, 0);



	if (sys_vol < 0)
	{
		sys_vol = getPrjGlobalParm(ZB_PARM_VOLUME, 0, NULL);
	}

	if (vol >= 0)
	{
		sys_vol = vol;
	} 
	else
	{ // volume use old value.
	#ifndef C_PC_SIM
		int curVol = 0;
		curVol = getPrjGlobalParm(ZB_PARM_VOLUME, 0, NULL);
		if (curVol < 0 || curVol > MAX_VOL) {
			ESP_LOGE(TAG, "error, Ln[%d], got_vol:%d", __LINE__, curVol);
			curVol = 0;
		}
		vol = curVol;
		sys_vol = curVol;
	#else
		vol = sys_vol;
	#endif
	}


	lv_obj_set_size(slider_cont, VOL_CONT_WIDTH, VOL_CONT_HEIGH); //150,46



	lv_obj_set_style_pad_all(slider_cont, 0, 0);
	
	lv_obj_align(slider_cont, LV_ALIGN_RIGHT_MID, -15, 0);


	slider = lv_slider_create(slider_cont);
	lv_obj_set_size(slider, VOL_SLIDER_WIDTH, VOL_SLIDER_HEIGH);
	
	lv_obj_align(slider,LV_ALIGN_TOP_MID, 0, (40));
	

	lv_slider_set_range(slider, 0, MAX_VOL);
	
	lv_slider_set_mode(slider, LV_SLIDER_MODE_NORMAL);
	lv_obj_set_style_opa(slider, LV_OPA_0, LV_PART_KNOB);
	lv_obj_set_style_outline_width(slider, 0,0);
	lv_obj_set_style_radius(slider, 0, LV_PART_INDICATOR);
	lv_color_t colorIndicator = lv_color_hex(0xb3ddff);
	lv_color_t  colorbg = lv_color_hex(0xFFFFFF);
	lv_obj_set_style_bg_color(slider, colorbg, 0);
	lv_obj_set_style_bg_color(slider, colorIndicator, LV_PART_INDICATOR);
	lv_obj_set_style_radius(slider, 12, 0);
	lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_width(slider, 0, 0);
	//lv_obj_set_style_outline_color(slider, colorBorder, 0); // lv_palette_main(LV_PALETTE_BLUE)
	lv_obj_set_style_outline_opa(slider, 0,0);
	lv_obj_set_style_outline_pad(slider, 0, 0);
	lv_obj_add_event_cb(slider, vod_val_chg_evt_cb, LV_EVENT_VALUE_CHANGED, NULL); 

	lv_obj_t* img = lv_img_create(slider_cont);
	lv_img_dsc_t* pImgDsc_img = zb_img_res_load_one(NULL, SYS_ICON_IMG_VOICE_ICON, IMG_FREE_TYPE_AUTO, NULL);
	lv_img_set_src(img, pImgDsc_img);
	lv_obj_align_to(img, slider_cont, LV_ALIGN_BOTTOM_MID, 0, -10);

	update_vol_slider(vol);

	ESP_LOGI(TAG, "comVol F");

	lv_group_add_obj(sys_group, slider_cont);
	lv_group_add_obj(sys_group, slider);
	
	//lv_group_focus_obj(slider); 
	lv_obj_add_event_cb(slider, vod_event_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(slider_bg, vod_bg_event_cb, LV_EVENT_ALL, NULL);
	sys_setCurMainWinAndCb(slider_bg, vod_bg_event_cb);

	if(s_timer ==NULL)
	{
		timer_data = 1;
		s_timer = lv_timer_create(vol_timer_cb, C_VOLUME_DLG_SHOW_IDLE_TIME, &timer_data);
	}

}

bool is_com_vol_shown() {
	return slider_cont != NULL;
}


int volume_incOrDec(int diff, int extra)
{
	ESP_LOGI(TAG, "volInc or dec, diff %d", diff);
	if (0 == extra) {
		vol_box_del();
	}

	if (0 == diff)
		return 0;

	bool hasChange = false;
	int vol = getPrjGlobalParm(ZB_PARM_VOLUME, 0, NULL);
	if (diff < 0) {
		if (vol > 0) {
			vol--;
		#ifndef C_PC_SIM
			int16_t volume = PARM_VOLUME_GLB_TO_DRV(vol);
			tz_play_set_volume(volume);
		#endif
			hasChange = true;
		}
	}
	else {
		if (vol < MAX_VOL) {
			vol++;
		#ifndef C_PC_SIM
			int16_t volume = PARM_VOLUME_GLB_TO_DRV(vol);
			tz_play_set_volume(volume);
		#endif
			hasChange = true;
		}
	}

	if (hasChange) {
		setPrjGlobalParm(ZB_PARM_VOLUME, 2, &vol);
	}
	return 1;
}

#endif

