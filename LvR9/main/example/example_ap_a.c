#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"


LV_IMG_DECLARE(imgbtn_lionA)
LV_IMG_DECLARE(img_lv_demo_music_btn_loop)
LV_IMG_DECLARE(img_lv_demo_music_icon_2)
LV_IMG_DECLARE(ext_icon_markA)
LV_IMG_DECLARE(ext_icon_markB)
LV_IMG_DECLARE(ext_icon_lineHor);

#define TAG "exmpApA"


#define C_SCROLL_STEP 12

static void _my_draw_ext_cb(lv_event_t * e) {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);

    lv_area_t widget_coords;
    lv_obj_get_coords(obj, &widget_coords);

    lv_draw_line_dsc_t dscLine;
    lv_draw_line_dsc_init(&dscLine);
    dscLine.width = 2;
    dscLine.color = lv_palette_main(LV_PALETTE_BLUE);
    lv_point_t point1 = { widget_coords.x1, widget_coords.y1 };
    lv_point_t point2 = { widget_coords.x2, point1.y };
    lv_draw_line(draw_ctx, &dscLine, &point1, &point2); 

    //-----------------
    lv_img_dsc_t* pImgDst = NULL;// &ext_icon_markA;
    int16_t iconW = pImgDst->header.w;
    lv_area_t txt_coords = widget_coords;
    txt_coords.x1 = (widget_coords.x2 - pImgDst->header.h);
    txt_coords.x2 = txt_coords.x1 + iconW - 1;
    txt_coords.y2 = txt_coords.y1 + pImgDst->header.h - 1;
    lv_draw_img_dsc_t dsc;
    lv_draw_img_dsc_init(&dsc);
    dsc.opa = LV_OPA_100;
    dsc.blend_mode = LV_BLEND_MODE_MULTIPLY;
    lv_draw_img(draw_ctx, &dsc, &txt_coords,  pImgDst);  // 

}


static lv_obj_t * g_labelA = NULL;
static lv_obj_t * g_mainContentObj = NULL;
static short g_isPause = 0;
static short g_curBtn = -1;
static lv_obj_t * g_btns[4] = { 0 };
	
#define C_BTN_N_IN_LINE  3

//-------------------------
lv_obj_t * g_mainWin;


static void updateStateView(void) {
    if (!g_labelA) return;
	lv_label_set_text(g_labelA,  (g_isPause ? "pause" : "playing"));
}

int zb_example_ap_a_cb(lv_event_code_t msg, int param) {
    switch (msg) {
    /*case EVENT_FREE_AP_RES: {
        lv_obj_del(g_mainWin);  g_mainWin = NULL;
        int openedN = zb_getOpenedFileNum();
        ESP_LOGI(TAG, "%s exitAp, openedN %d", g_p____bar, openedN);
        break;
    }
    case EVENT_EX_CMD:
        ESP_LOGI(TAG, "evtCmd");
        break;*/
    default:
        break;
    }
    return 0;
}

static void getChildsCoord(lv_obj_t * mainView) { 
	int childNum = lv_obj_get_child_cnt(mainView);
	for (int idx = 0; idx < childNum; idx++) { 
		lv_obj_t  * pChild = lv_obj_get_child(mainView, idx);
		ESP_LOGI(TAG, "child%2d: (%d,%d) (%d,%d)", idx, (int)pChild->coords.x1,(int)pChild->coords.y1, 
				(int)pChild->coords.x2,(int)pChild->coords.y2);
	}
}


static void _myMainWinCb(lv_event_t * evt) {

#if 0
    int keyVal = 0;
    lv_event_code_t evtCode = evt->code;
    if (LV_EVENT_KEY_STAGE == evt->code) {
        keyVal = lv_event_get_key(evt);
        if ((LV_EX_FLAG_KEY_MASK & keyVal) == LV_EX_FLAG_KEY_REPEAT) {
            keyVal &= ~LV_EX_FLAG_KEY_MASK;
            evtCode = LV_EVENT_KEY;
        }
    }
    if (LV_EVENT_KEY == evtCode) {
        if (!keyVal) {
            keyVal = lv_event_get_key(evt);
        }
        if (LV_KEY_LEFT == keyVal || LV_KEY_RIGHT == keyVal) {
            short btnNoPrev = g_curBtn;
            if (btnNoPrev < 0 && LV_KEY_LEFT == keyVal) {
                btnNoPrev = C_BTN_N_IN_LINE;
            }
            short diff = (LV_KEY_LEFT == keyVal ? -1 : 1);
            short btnNo = btnNoPrev + diff;
            bool bNeedUnfocus = false;
            if (btnNo < 0 || btnNo >= C_BTN_N_IN_LINE) {
                g_curBtn = -1;
                bNeedUnfocus = true;
            } else {
                g_curBtn = btnNo;
                lv_group_focus_obj(g_btns[btnNo]);
            }
            if (bNeedUnfocus) {
                lv_obj_clear_state(g_btns[btnNoPrev], LV_STATE_FOCUSED|LV_STATE_FOCUS_KEY);
            }
            lv_event_stop_processing(evt);
        } 
        else if (LV_KEY_UP == keyVal || LV_KEY_DOWN == keyVal) {
            getChildsCoord(g_mainContentObj);
            lv_coord_t contentH = 800; // non-std, lv_obj_get_height(objSpans);
            lv_coord_t maxScroll = contentH - lv_obj_get_height(g_mainContentObj);
            if (maxScroll < 0) {
                maxScroll = 0;
            }
            
            lv_coord_t posY = lv_obj_get_scroll_y(g_mainContentObj);
            lv_coord_t scrollYNew = -1;
            bool bChg = false;
            
            ESP_LOGI(TAG, "key prs in exampleApA");
            if (LV_KEY_DOWN == keyVal) {
                if (posY < maxScroll) {
                    scrollYNew = posY + C_SCROLL_STEP;
                    bChg = true;
                }
            }
            else if (LV_KEY_UP == keyVal) {
                if (posY > 0) {
                    scrollYNew = posY - C_SCROLL_STEP;
                    if (scrollYNew < 0) {
                        scrollYNew = 0;
                    }
                    bChg = true;
                }
            }
            if (bChg) {
                lv_obj_scroll_to_y(g_mainContentObj, scrollYNew, LV_ANIM_OFF);
            }
            lv_event_stop_processing(evt);
        }
        else if (LV_KEY_ENTER == keyVal) {
            if (g_curBtn < 0) {
                g_isPause = !g_isPause;
                updateStateView();
                lv_event_stop_processing(evt);
                lv_indev_keypad_wait_release();
                // lv_timer_create(zb_ex_cmd_timer_cb, 0, 0x138a);
            }
        }
    }
#endif

}

static void _my_btn_click_cb(lv_event_t * evt) {

    lv_obj_t * targetObj = lv_event_get_target(evt);
    int idx = 0;
    for (idx = 0; idx < M_ARRAY_LEN(g_btns) - 1; idx++) {
        if (targetObj == g_btns[idx]) break;
    }
    if ((idx < M_ARRAY_LEN(g_btns) - 1) && (idx != g_curBtn)) {
        g_curBtn = idx;
        lv_group_focus_obj(g_btns[idx]);
        lv_obj_add_state(g_btns[idx], LV_STATE_FOCUS_KEY|LV_STATE_FOCUSED);
    }
    if (1 == g_curBtn) { // switch to another AP 
        zb_requestSwitchAp(C_ZB_AP_EXAMPLE_B);
    }
    else if (2 == g_curBtn) {
        zb_requestSwitchAp(C_ZB_AP_MAINMENU);
    }
}


#define C_MAIN_WIN_W   SCREEN_WIDTH
#define C_FORM_PADDING_W  1
#define C_MAIN_CONTENT_W  (SCREEN_WIDTH - C_FORM_PADDING_W * 2)

/**
 * Show mixed LTR, RTL and Chinese label
 */
int zb_example_ap_a_init(void)
{
	int openedN = zb_getOpenedFileNum();
	ESP_LOGI(TAG, "%s initAp, openedN %d", g_p____bar, openedN);

    static const lv_coord_t col_dsc[] = // 
        {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),  LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t row_dsc[] = { LV_GRID_FR(1),  32, 32, LV_GRID_TEMPLATE_LAST};

    lv_obj_t * mainWin = lv_obj_create(lv_scr_act());
    lv_obj_set_style_border_width(mainWin, 0, 0);
    lv_obj_set_style_pad_all(mainWin, C_FORM_PADDING_W, 0);
    lv_obj_set_size(mainWin, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_grid_dsc_array(mainWin, col_dsc, row_dsc);

    lv_obj_t * mainContentView = lv_obj_create(mainWin);
    lv_obj_set_style_border_width(mainContentView, 0, 0);
    lv_obj_set_style_pad_all(mainContentView, 0, 0);
    lv_obj_set_grid_cell(mainContentView, LV_GRID_ALIGN_STRETCH, 0, 3,
                             LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_flex_flow(mainContentView, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(mainContentView, LV_DIR_VER);
    lv_obj_set_size(mainContentView, LV_PCT(100), LV_PCT(100));

    lv_obj_t * ltr_label = lv_label_create(mainContentView);
    lv_obj_set_width(ltr_label, C_MAIN_CONTENT_W);
    lv_label_set_text(ltr_label, "In modern terminology, a microcontroller is similar to a system on a chip (SoC).");
    lv_obj_set_width(ltr_label, C_MAIN_WIN_W);
    lv_obj_align(ltr_label, LV_ALIGN_TOP_LEFT, 5, 5);


    lv_obj_t * label_b = lv_label_create(mainContentView);
    lv_obj_set_width(label_b, C_MAIN_CONTENT_W);
    lv_label_set_text(label_b, "[全体中国解放的量时野理想]紛繁蔚然砥砺簸箕鼎盛骥巍然缠绕盛事豪迈犒劳墙壁阑珊慵懒藏着煤耗铿锵，晋太元中，武陵人捕鱼为业。缘溪行，忘路之远近。忽逢桃花林，夹岸数百步，中无杂树，芳草鲜美，落英缤纷。渔人甚异之，复前行，欲穷其林。\n" // 
 "林尽水源，便得一山，山有小口，仿佛若有光。便舍船，从口入。初极狭，才通人。复行数十步，豁然开朗。土地平旷，屋舍俨然，有良田、美池、桑竹之属。阡陌交通，鸡犬相闻。其中往来种作，男女衣着，悉如外人。黄发垂髫，并怡然自乐。\n"
 "见渔人，乃大惊，问所从来。具答之。便要还家，设酒杀鸡作食。村中闻有此人，咸来问讯。自云先世避秦时乱，率妻子邑人来此绝境，不复出焉，遂与外人间隔。问今是何世，乃不知有汉，无论魏晋。此人一一为具言所闻，皆叹惋。余人各复延至其家，皆出酒食。停数日，辞去。此中人语云：“不足为外人道也。”\n"
 "既出，得其船，便扶向路，处处志之。及郡下，诣太守，说如此。太守即遣人随其往，寻向所志，遂迷，不复得路。\n"
 "南阳刘子骥，高尚士也，闻之，欣然规往。未果，寻病终，后遂无问津者。\n"
		"ˈwelkəm,  kənˈstrʌkʃn;  ˈbjuːtɪf(ə)l; Kang Yingfeng, deputy general manager of China Railway International Multimodal Transportation, said there had been a significant increase in inquiries about the China-Europe freight train service since the beginning of this year.]");
    lv_obj_set_style_border_width(label_b, 4, 0);
    //lv_obj_set_style_border_color(label_b, lv_palette_main(LV_PALETTE_NONE), 0);
    lv_obj_set_style_border_opa(label_b, 0, 0);
    lv_obj_set_style_border_side(label_b, LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_add_event_cb(label_b, _my_draw_ext_cb, LV_EVENT_DRAW_MAIN_END, NULL);


    lv_obj_t * label_c = lv_label_create(mainContentView);
    lv_obj_set_width(label_c, C_MAIN_CONTENT_W);
    lv_label_set_text(label_c, "as a key meeting set the tone for the overall deepening of reforms this year.");


    //----------
    lv_obj_t * cz_label = lv_label_create(mainContentView);
    lv_obj_set_width(cz_label, C_MAIN_CONTENT_W);
    lv_label_set_text(cz_label,
                      "嵌入式系统（Embedded System），\n是一种嵌入机械或电气系统内部、具有专一功能和实时计算性能的计算机系统。");

    lv_obj_set_style_border_width(cz_label, 2, 0);
    lv_obj_set_style_border_color(cz_label, lv_palette_main(LV_PALETTE_PURPLE), 0);
    lv_obj_set_style_border_side(cz_label, LV_BORDER_SIDE_BOTTOM, 0);


    lv_obj_t* label_f = lv_label_create(mainContentView);
    lv_obj_set_width(label_f, C_MAIN_CONTENT_W);
    lv_label_set_text(label_f, "It was stressed at the meeting that China will continue to work on reform measures to expand domestic demand, boost confidence.\n");
    //---------------------------------------
    g_labelA = lv_label_create(mainWin);
    updateStateView();
    lv_obj_set_grid_cell(g_labelA, LV_GRID_ALIGN_CENTER, 1, 2,
                             LV_GRID_ALIGN_CENTER, 1, 1);
    //----------------
    lv_obj_t * label_tmp = NULL;
    lv_color_t colorBtnBg = LV_COLOR_MAKE(0xe0, 0, 0);
    lv_obj_t * btnA = lv_btn_create(mainWin);
    label_tmp = lv_label_create(btnA);
    lv_label_set_text(label_tmp, "BtnA");
    lv_obj_center(label_tmp);
    lv_obj_add_event_cb(btnA, _my_btn_click_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_set_grid_cell(btnA, LV_GRID_ALIGN_START, 0, 1,
                             LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_style_radius(btnA, 0, 0);
    lv_obj_set_style_border_color(btnA, colorBtnBg, LV_STATE_FOCUSED);
    g_btns[0] = btnA;
    //----------------
    lv_obj_t * btnB = lv_btn_create(mainWin);
    lv_obj_add_event_cb(btnB, _my_btn_click_cb, LV_EVENT_SHORT_CLICKED, NULL);
    label_tmp = lv_label_create(btnB);
    lv_label_set_text(label_tmp, "NxAp");
    lv_obj_center(label_tmp);
    lv_obj_set_grid_cell(btnB, LV_GRID_ALIGN_START, 1, 1,
                             LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_style_radius(btnB, 0, 0);
    g_btns[1] = btnB;
    //----------------
    lv_obj_t * btnC = lv_btn_create(mainWin);
    lv_obj_add_event_cb(btnC, _my_btn_click_cb, LV_EVENT_SHORT_CLICKED, NULL);
    label_tmp = lv_label_create(btnC);
    lv_label_set_text(label_tmp, "Exit");
    lv_obj_center(label_tmp);
    lv_obj_set_grid_cell(btnC, LV_GRID_ALIGN_START, 2, 1,
                             LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_style_radius(btnC, 0, 0);
    g_btns[2] = btnC;
    //------------
    g_mainContentObj = mainContentView;
    g_curBtn = 0;
    lv_group_add_obj(lv_group_get_default(), mainWin);
    lv_obj_add_event_cb(mainWin, _myMainWinCb, LV_EVENT_ALL, NULL);
    
    //增ly
    //sys_setCurMainWinAndCb(mainWin, );

    g_mainWin = mainWin;
    getChildsCoord(g_mainContentObj); // debug 
    return 0;
}

