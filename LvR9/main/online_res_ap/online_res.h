#ifndef ONLINE_RES_AP_H
#define ONLINE_RES_AP_H
extern "C" {
#include <stdio.h>
#include "../utils/misc_utils.h"
#include <string.h>
#include <sys/stat.h>
#include "../mainmenu/tlb_main_img.h"
#include <lvgl/lvgl.h>
#include "../zb_base.h"
#include "../ex_widget/app_msg.h"
#include "../include/img_a_tlb.h"
#include "../data_parse/img_res.h"
}
#include <memory>
#include "../online_tool/json_get_online_res.h"
#include "online_res_read.h"
#include <functional>
#include <thread>



#define AICHAT_ESC_BTN_LOC_Y  10
#define TOP_BTN_BORDER 7
#define TOP_BTN_TOP_SPACE_H 8

#define AP_BG_COLOR_HEX  0xffffff
#define ONLINEAP_HEADER_HEIGHT 56

#define		ONLINE_PALY_MAIN			0
#define 	ONLINE_PALY_CONTENT			1
#define 	ONLINE_PALY_DIS			    2
#define     ONLINE_PALY_DOWN            3
#define     ONLINE_PALY_SETTEING        4

class ApBase{
public:
    ApBase():mwin_BgWin(nullptr),mwin_CurWin(0),mwin_IsInSubItem(false){};
    ~ApBase(){};
    virtual int AP_MainWin_init() = 0;
    virtual int AP_MainWin_handler(lv_event_code_t msg, int param) = 0;
    virtual void Ap_MainWin_clean_res() = 0;
    virtual int AP_MainWin_deinit() = 0;
    virtual void Ap_MainWin_del() = 0;
    virtual void Ap_createEscBtn(lv_obj_t * parent, lv_event_cb_t clickCb, int extra){
	    const lv_img_dsc_t * pImgDsc = g_XXX_img_dsc_tbl + MMN_icn_esc - 1;
	    lv_obj_t * context = lv_obj_create(parent);
	    lv_obj_set_size(context, pImgDsc->header.w + TOP_BTN_BORDER * 2 + 20, pImgDsc->header.h + TOP_BTN_BORDER * 2 + 20);
	    lv_obj_set_pos(context, 5, 0);
	    lv_obj_set_style_border_width(context, 0, 0);

	    lv_obj_t * img = lv_img_create(context);
	    lv_img_set_src(img, pImgDsc);
	    lv_obj_set_size(img, pImgDsc->header.w + TOP_BTN_BORDER * 2, pImgDsc->header.h + TOP_BTN_BORDER * 2);
	    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
	    lv_obj_set_style_pad_all(img, TOP_BTN_BORDER, 0);
	    lv_obj_set_style_radius(img, 4, 0);
	    lv_obj_add_flag(context, LV_OBJ_FLAG_CLICKABLE);
        lv_color_t colorBg;
        if(extra == 1){
            lv_obj_set_style_bg_color(context, lv_color_hex(0xF0F0F0), 0);
            colorBg = lv_color_hex(0xF0F0F0);
        }else if(extra == 2){
            lv_obj_set_style_bg_color(context, lv_color_hex(0xF5F7FA), 0);
            colorBg = lv_color_hex(0xF5F7FA);
        }else{
            colorBg = lv_color_hex(0xc0c0c0);
        }
	    
	    lv_obj_set_style_bg_color(img, colorBg, 0);
	    lv_obj_set_style_bg_opa(img, LV_OPA_0, 0);
	    
	    lv_color_t colorBorderFcs = lv_color_white();
	    lv_obj_set_style_border_color(img, colorBorderFcs, 0);
        lv_obj_set_style_border_width(img, 0, 0);
	    lv_obj_add_event_cb(context, clickCb, LV_EVENT_CLICKED, NULL);
    }
    uint8_t get_cur_win(){
        return mwin_CurWin;
    }

protected:
    lv_obj_t* mwin_BgWin = nullptr;
    uint8_t mwin_CurWin = 0;
    bool mwin_IsInSubItem = false;
};

class OnlinePlayAp : public ApBase{
public:
    OnlinePlayAp(uint8_t ap_sel):
        ApBase(),mwin_ItemSel(ap_sel){};
    ~OnlinePlayAp(){
        AP_MainWin_deinit();
    };
    int AP_MainWin_init() override;
    int AP_ContentWin_init();
    int AP_DisWin_init();
    int AP_DownWin_init();
    int AP_SettingWin_init();

    int AP_MainWin_handler(lv_event_code_t msg, int param) override;
    void Ap_MainWin_clean_res() override;
    int AP_MainWin_deinit() override;
    void Ap_MainWin_del() override;
    void Ap_back_content();
    void set_content_win(lv_img_dsc_t* img,std::string name,OnlineResNode res_node,int index){
        m_img_Src = img;
        m_name = name;
        m_sel_res_node = res_node;
        m_sel_res_index = index;
    }
    uint8_t mwin_ItemSel = 0;
    int m_sel_res_index = 0;
private:
    lv_img_dsc_t* m_img_Src = nullptr;
    std::string m_name = "";
    OnlineResNode m_sel_res_node;
};

extern const lv_img_dsc_t download_img;

#endif