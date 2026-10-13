#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"



#define TAG "exmpB"

#define C_SCROLL_STEP  12

//-------------------------
static lv_obj_t * g_mainWin;
static lv_obj_t * g_spanGrp = NULL;

int zb_example_ap_b_cb(lv_event_code_t msg, int param) {
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

static void _myEvent_cb(lv_event_t * evt) {

#if 0
    lv_obj_t* objSpans = g_spanGrp;
    if (!objSpans) return;

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
        ESP_LOGI(TAG, "key prs in exampleApB");
        lv_area_t area = { 0 };
        lv_obj_t* winA = lv_obj_get_parent(objSpans);
        
        lv_coord_t contentH = lv_obj_get_height(objSpans);
        lv_coord_t maxScroll = contentH - lv_obj_get_height(winA);
        if (maxScroll < 0) {
            maxScroll = 0;
        }
        
        lv_coord_t posY = lv_obj_get_scroll_y(winA);
        lv_coord_t scrollYNew = -1;
        bool bChg = false;

        if (!keyVal) {
            keyVal = lv_event_get_key(evt);
        }
        switch (keyVal) {
            case LV_KEY_DOWN: {
                if (posY < maxScroll) {
                    scrollYNew = posY + C_SCROLL_STEP;
                    bChg = true;
                }
            }
            break;
            case LV_KEY_UP: {
                if (posY > 0) {
                    scrollYNew = posY - C_SCROLL_STEP;
                    if (scrollYNew < 0) {
                        scrollYNew = 0;
                    }
                    bChg = true;
                }
            }
            break;
            case LV_KEY_ESC: {
                zb_requestSwitchAp(C_ZB_AP_MAINMENU);
                lv_indev_keypad_wait_release();
            }
            break;
            default:
            break;
        }
        if (bChg) {
            lv_obj_scroll_to_y(winA, scrollYNew, LV_ANIM_OFF);
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
 * Create span.
 */
int zb_example_ap_b_init(void)
{
#define C_AREA_A_W  SCREEN_WIDTH
#define C_AREA_A_H  (SCREEN_HEIGHT-40)
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_border_width(&style, 1);
    lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_ORANGE));
    lv_style_set_pad_all(&style, 2);

    lv_obj_t * winA = lv_obj_create(lv_scr_act());
    lv_obj_set_size(winA, C_AREA_A_W, C_AREA_A_H);
    lv_obj_set_style_pad_all(winA, 0, 0);
    lv_obj_set_flex_flow(winA, LV_FLEX_FLOW_COLUMN);
    lv_obj_t * spans = lv_spangroup_create(winA);
    lv_obj_set_width(spans, C_AREA_A_W); // lv_obj_get_width(winA));
    lv_obj_set_style_text_line_space(spans, 3, 0);
    lv_obj_add_style(spans, &style, 0);


    lv_spangroup_set_align(spans, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(spans, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_indent(spans, 20);
    lv_spangroup_set_mode(spans, LV_SPAN_MODE_BREAK);

    lv_span_t * span = lv_spangroup_new_span(spans);
    lv_span_set_text(span, "China is a beautiful country?"); // China is a beautiful country.
    lv_style_set_text_color(&span->style, lv_palette_main(LV_PALETTE_BROWN));
    lv_style_set_text_decor(&span->style, LV_TEXT_DECOR_UNDERLINE);
    lv_style_set_text_opa(&span->style, LV_OPA_50);

    span = lv_spangroup_new_span(spans);
    lv_span_set_text_static(span, "good good study, day day up.");
#if LV_FONT_MONTSERRAT_24
    lv_style_set_text_font(&span->style,  &lv_font_montserrat_24);
#endif
    lv_style_set_text_color(&span->style, lv_palette_main(LV_PALETTE_GREEN));

    span = lv_spangroup_new_span(spans);
    lv_span_set_text_static(span, "LVGL is an open-source graphics library.");
    lv_style_set_text_color(&span->style, lv_palette_main(LV_PALETTE_BLUE));

    span = lv_spangroup_new_span(spans);
    lv_span_set_text_static(span, "the boy no name.");
    lv_style_set_text_color(&span->style, lv_palette_main(LV_PALETTE_GREEN));
#if LV_FONT_MONTSERRAT_20
    lv_style_set_text_font(&span->style, &lv_font_montserrat_20);
#endif
    lv_style_set_text_decor(&span->style, LV_TEXT_DECOR_UNDERLINE);

    span = lv_spangroup_new_span(spans);
    lv_span_set_text(span, "I have a dream that hope to come true.");
    lv_style_set_text_decor(&span->style, LV_TEXT_DECOR_STRIKETHROUGH);
    //-----
    span = lv_spangroup_new_span(spans);
    lv_span_set_text(span, "The competition in China's cloud computing sector is heating up as major Chinese players have announced cuts to prices of multiple core products.  "
		"Experts said this will mean lower cost of access to cloud services for enterprises, which in turn will boost market development.  "
		"Alibaba Cloud, the cloud computing unit of Alibaba Group Holding Ltd, announced on Thursday a price reduction of up to 55 percent on more than 100 core products, marking the biggest price cut in the company's history.  "
		"The new prices, with 20 percent reduction on average, cover over 100 cloud products, such as computing, storage and database."); //
    lv_style_set_text_color(&span->style, lv_palette_main(LV_PALETTE_BROWN));

    lv_spangroup_refr_mode(spans);
    g_spanGrp = spans;

    lv_group_add_obj(lv_group_get_default(), winA);
    lv_obj_add_flag(winA, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(winA, _myEvent_cb, LV_EVENT_ALL, NULL);
    //--------------

    lv_obj_t * btnA = lv_btn_create(winA);
    lv_obj_t * labelBtnA = lv_label_create(btnA);
    lv_label_set_text(labelBtnA, "BtnA");

    lv_obj_t * btnB = lv_btn_create(winA);
    lv_obj_t * labelBtnB = lv_label_create(btnB);
    lv_label_set_text(labelBtnB, "Quit");
    lv_obj_add_event_cb(btnB, _BtnEvent_cb, LV_EVENT_CLICKED, NULL);

    lv_group_focus_obj(winA);
    g_mainWin = winA;
    return 0;
}
