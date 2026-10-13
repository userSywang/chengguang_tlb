#ifndef UI_COLLECT_H
#define UI_COLLECT_H

#ifdef __cplusplus
extern "C" {
#endif

    /*********************
     *      INCLUDES
     *********************/
#include <lvgl/lvgl.h>
#include "../data_support/db_manager.h"
#include "../mainmenu/tlb_main_img.h"
     /*********************
      *      DEFINES
      *********************/
    enum collect_screen_e {
        COLLECT_LIST_CONTENT,
        REMOEVE_COLLECT_CONTENT,
        COLLECT_DOWNLOAD_CONTENT,

    };

    typedef struct {
        bool checked;           // 当前选中状态
        lv_obj_t* img_obj;      // 关联的图片对象
    } btn_check_state_t;

    // 复选框结构体
    typedef struct {
        lv_obj_t* obj;          // 容器对象
        lv_obj_t* box;          // 复选框框体
        lv_obj_t* checkmark;    // 选中标记
        lv_obj_t* label;        // 文本标签
        bool checked;           // 当前状态
    } custom_checkbox_t;


//#define MAX_MUSICS 61  // 最大音乐数量
    typedef struct {
        lv_obj_t* ui_collect;
        lv_obj_t* ui_title;
        lv_obj_t* ui_Panel1;
        lv_obj_t* ui_Label9;
        lv_obj_t* ui_choosebtn;
        lv_obj_t* ui_Label2;
        lv_obj_t* collect_Panel;
        lv_obj_t* ui_line1;
        lv_obj_t* ui_Label1;
        lv_obj_t* ui_line2;
        lv_obj_t* ui_Label3;
        lv_obj_t* ui_line3;
        lv_obj_t* ui_Label4;
        lv_obj_t* ui_Label5;

        lv_obj_t* mbox;
        lv_obj_t* mobx_title;
        lv_obj_t* msgbtn_affirm;
        lv_obj_t* affirm_text;
        lv_obj_t* msgbtn_cancel;
        lv_obj_t* cancel_text;
        lv_obj_t* mask;
        lv_obj_t* NObtn_label;


        lv_obj_t* g_lines[62]; //焦点控制line= { 0 }
        lv_obj_t* g_check[62]; //复选框      = { 0 }
        lv_obj_t* g_groups[2];  //music and collection

        char* textName;

       lv_img_dsc_t* chooseImg;
       lv_obj_t* ui_ImgButton1;
       lv_obj_t* ui_ImgButton2;

       lv_obj_t* btn_audio;
       lv_obj_t* btn_book;

       lv_obj_t* player_obj;

       lv_obj_t* created_component;

       lv_img_dsc_t* g_img_checked;
       lv_img_dsc_t* g_img_unchecked;

       lv_obj_t* remCol_btn;
       lv_obj_t* remCol_label;
       lv_obj_t* check_all_btn; // 全选按钮对象

       int total_count;        // 总收藏项数量
       bool all_selected;     // 全选状态标志
       int selected_count;     // 添加选中计数器
       bool not_tip;         //不再提示
       enum collect_screen_e current_screen;

       CollectionItem* cached_items;   // 缓存的数据
       //uint32_t cached_count;          // 缓存的数量
       //CollectionType cached_type;     // 缓存的数据类型

    } lv_PageResource_Collection;

#define  REMOVE_LABLE_COLOR_DEFAULT (0x18191B)
#define  REMOVE_LABLE_COLOR_SELECT (0x008CFF)
#define  REMOVE_LABLE_COLOR_BG (0xF5F7FA)
#define  REMOVE_LABLE_COLOR_TIME (0x787F87)


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif 
