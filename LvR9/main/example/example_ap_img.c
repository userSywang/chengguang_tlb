
#include <lvgl/lvgl.h>

#include "../utils/misc_utils.h"
#include "../data_parse/img_res.h"

#include "../include/img_a_tlb.h"

//------------------------------


#define TAG  "exmpImg"

//------------------------------
const uint32_t g_aImgIdTbl[] = {
	LISTEN_AI_IMG_EXCELLANT, LISTEN_AI_IMG_PLAY, LISTEN_CHAT_IMG_AI_CHAT_BTN,  LISTEN_CHAT_IMG_MEMBER_BTN,
};


//------------------------------
static lv_obj_t * g_mainWin;

static lv_img_dsc_t * g_pstImgDsc = NULL;

static int16_t g_imgTotal = 0;
static int16_t g_imgNo = 0;

static lv_obj_t * g_labelInf = NULL;
static lv_obj_t * g_imgA = NULL;


//------------------------------

int zb_example_ap_img_cb(lv_event_code_t msg, int param) {
#if 0
    switch (msg) {
    case EVENT_FREE_AP_RES:
        lv_obj_del(g_mainWin);  g_mainWin = NULL;
        break;
    default:
        break;
    }
#endif
    return 0;
}

static void _upd_img(void) { 
	if ((g_imgTotal < 1) || !g_imgA) return;

	if (g_imgNo < 0 || g_imgNo >= g_imgTotal) {
		return;
	}

	uint32_t  imgId = g_aImgIdTbl[g_imgNo];
	int ret = 0;
	g_pstImgDsc = zb_img_res_load_one(NULL, imgId, IMG_FREE_TYPE_AUTO, &ret);
	if (ret < 0) {
		ESP_LOGE(TAG, "fail to load imgNo %hd, id 0x%x", g_imgNo, (int)imgId);
		return;
	}
	
	lv_img_set_src(g_imgA, g_pstImgDsc);
	lv_label_set_text_fmt(g_labelInf, "No%d", (int)g_imgNo);
}


static void _myEvent_cb(lv_event_t * evt) {

#if 0
    lv_event_code_t evtCode = evt->code;
    uint16_t keyVal = 0;
    if (LV_EVENT_KEY_STAGE == evt->code) {
        keyVal = lv_event_get_key(evt);
        if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            evtCode = LV_EVENT_KEY;
        }
    }
    if (LV_EVENT_KEY == evtCode) { 
        ESP_LOGI(TAG, "key prs in exampleImgB");
        if (!keyVal) {
            keyVal = lv_event_get_key(evt);
        }

        if (LV_KEY_RIGHT == keyVal) {
            if (g_imgTotal <= 0) return;
            g_imgNo = (g_imgNo + 1) % g_imgTotal;
            _upd_img();
        }
        else if (LV_KEY_LEFT == keyVal) {
            if (g_imgTotal <= 0) return;
            g_imgNo = (g_imgNo + g_imgTotal - 1) % g_imgTotal;
            _upd_img();
        }
        else if (LV_KEY_ESC == keyVal) {
            zb_requestSwitchAp(C_ZB_AP_MAINMENU);
            lv_event_stop_processing(evt);
        }
    }
#endif

}

static void _BtnEvent_cb(lv_event_t * evt) {
    if (evt->code == LV_EVENT_CLICKED) {
        zb_requestSwitchAp(C_ZB_AP_MAINMENU);
    }
}


/**
 */
int zb_example_ap_img_init(void)
{
#define C_AREA_A_W  SCREEN_WIDTH
#define C_AREA_A_H  SCREEN_HEIGHT
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_border_width(&style, 1);
    lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_ORANGE));
    lv_style_set_pad_all(&style, 2);

    lv_obj_t * winA = lv_obj_create(lv_scr_act());
    lv_obj_set_size(winA, C_AREA_A_W, C_AREA_A_H);
    lv_obj_set_style_pad_all(winA, 0, 0);
    lv_obj_set_flex_flow(winA, LV_FLEX_FLOW_COLUMN);


    lv_group_add_obj(lv_group_get_default(), winA);
    lv_obj_add_flag(winA, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(winA, _myEvent_cb, LV_EVENT_ALL, NULL);
    //--------------

    /* lv_obj_t * btnA = lv_btn_create(winA);
    lv_obj_t * labelBtnA = lv_label_create(btnA);
    lv_label_set_text(labelBtnA, "BtnA"); 
    */

    lv_obj_t * objHeader = lv_obj_create(winA);
    lv_obj_set_size(objHeader, LV_PCT(100), LV_SIZE_CONTENT);

    lv_obj_t * btnB = lv_btn_create(objHeader);
    lv_obj_t * labelBtnB = lv_label_create(btnB);
    lv_label_set_text(labelBtnB, "Quit");
    lv_obj_add_event_cb(btnB, _BtnEvent_cb, LV_EVENT_CLICKED, NULL);

	lv_obj_t * labelInf = lv_label_create(objHeader);
	lv_label_set_text(labelInf, "No.");
	lv_obj_align(labelInf, LV_ALIGN_RIGHT_MID, 0, 0);
	//----------------- 
	lv_obj_t * imgA = lv_img_create(winA);
	g_imgTotal = M_ARRAY_LEN(g_aImgIdTbl);
	if (g_imgTotal < 0) {
		ESP_LOGE(TAG, "There is no img");
	} else {
		g_pstImgDsc = NULL;
	}

	g_imgNo = 0;
	g_labelInf = labelInf;
	g_imgA = imgA;
	_upd_img();

	//------------ 

    lv_group_focus_obj(winA);
    g_mainWin = winA;
    return 0;
}


//==============================================================================
#define C_IMG_TEST_N  3


static short g_curImgNo = 0;

lv_obj_t * g_imgMain = NULL;

static const char * g_imgFileNameTbl[] = { "testA.jpg", "testB.jpg", "testC.png" };

//------------------------------------
static void _updateImg(void) {

	int idx = g_curImgNo % C_IMG_TEST_N;
	const char * pName = g_imgFileNameTbl[idx];
	char path[250] = { 0 };
	sprintf(path, "/sdcard/img/%s", pName);
	ESP_LOGI(TAG, "imgPath:%s", path);
	lv_img_set_src(g_imgMain, path);
	return;
}


static void _ap_img_dev_evt_cb(lv_event_t * evt) {

#if 0
    lv_event_code_t evtCode = evt->code;
    uint16_t keyVal = 0;
    if (LV_EVENT_KEY_STAGE == evt->code) {
        keyVal = lv_event_get_key(evt);
        if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_SHORT) {
            switch (keyVal & 0x0fff) {
                case LV_KEY_ESC: {
                    zb_requestSwitchAp(C_ZB_AP_MAINMENU);
                }
                break;
                case LV_KEY_DOWN:
                case LV_KEY_RIGHT: {
                    g_curImgNo = (g_curImgNo + 1) % C_IMG_TEST_N;;
                    _updateImg();
                }
                break;
                default:
                break;
            }
        }
    }
#endif
}

int zb_example_ap_img_dec_init(void)
{
#if 0
	lv_obj_t * mainWin = NULL;
	lv_obj_t * img = NULL;

	mainWin = lv_img_create(lv_scr_act());

	g_curImgNo = 0;
	img = lv_img_create(mainWin);
	g_imgMain = img;
	_updateImg();

	lv_group_add_obj(lv_group_get_default(), mainWin);
	lv_obj_add_flag(mainWin, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE);
	lv_obj_add_event_cb(mainWin, _ap_img_dev_evt_cb, LV_EVENT_KEY_STAGE, NULL);
	g_mainWin = mainWin;
#endif
	return 0;
 }


int zb_example_ap_img_dec_cb(lv_event_code_t msg, int param) {
#if 0
    switch (msg) {
    case EVENT_FREE_AP_RES:
        lv_obj_del(g_mainWin);  g_mainWin = NULL;
        g_imgA = NULL;
        break;
    default:
        break;
    }
#endif
    return 0;
}



