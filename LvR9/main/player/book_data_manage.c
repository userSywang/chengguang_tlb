//#include "../components/lvgl/lvgl.h"
//#include "../components/lvgl/examples/lv_examples.h"
//#include "../components/lvgl/src/misc/lv_types.h"
#include "../lvgl_main.h"
#include "player.h"
#include "book_data_manage.h"
#include "player_main_online.h"
//#include "../upd_data_parse/favorites.h"
#include "../data_parse/img_res.h"
#include "../utils/misc_utils.h"
#include "../ex_widget/app_msg.h"

#ifndef C_PC_SIM
extern bool zb_is_wifi_connect();
#endif

//#include "../data_parse/analyze_listening.h"

//————
#define TAG "BOOK_MANAGE"

#define C_MAIN_WIN_W   SCREEN_WIDTH  //窗口宽度
#define C_FORM_PADDING_W  1          //设置窗口或控件的填充宽度
#define POP_MSG_WIDTH   186
#define POP_MSG_HEIGHT  50


#define M_ARRAY_LEN(arr)  (sizeof(arr)/sizeof(arr[0]))
static lv_obj_t* selects_obj;
static lv_obj_t* manage_win;

static bv_node* g_bvHead = NULL;    // 显示列表数据
static char* g_bv_status = NULL;    // 当前列表数据的状态
static char g_bv_op = 0;            // 操作标记：0：AB编辑； 1：AB删除 
static uint16_t g_bv_num = 0;       // 列表的总数    
static lv_style_t bookItemStyle;
static lv_obj_t* deleteMsgbox;      // 弹出删除对话框
static lv_obj_t* g_btn_op;          // 删除操作按钮
static uint16_t  g_cur_book;        // 当前教材活动焦点
static uint8_t   g_exist_del_op;    // 当前是否有删除操作
static void _my_imgbtn_xuanze_cb(lv_event_t* e);//选择按钮回调函数
static void _myMainWinCb(lv_event_t* evt);//键盘处理回调函数
static int get_down_book(char* path, int indent);
static int del_empty_grd(char* path, int indent);
static void pop_delete_msgbox(int num);
static int static_list_sum();
int zb_book_data_manage_free(void);
int del_down_one_book(const char* path, const char* version);

#define MY_TOP_WIN_H    50
#define MY_LST_ITEM_W   LV_PCT(100)   // 195
//static const lv_coord_t col_dsc[] = { LV_GRID_FR(1),  LV_GRID_TEMPLATE_LAST };//列1
//static const lv_coord_t row_dsc[] = { MY_TOP_WIN_H,  LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST }; //第一行高60  第二行剩下全部
static int8_t  g_MYD_scrNo;  // start from 1
static lv_obj_t *g_pop_del_msg = NULL;

extern int player_class_num;
extern const char* class_info_tl[];

extern int read_grd_parse(const char* path, nmbox_book_list_t* nblist);

static void abKey_cb(lv_event_t* e)
{
}

//弹窗提示图片 加载中，请稍后。。。
static int deleting_please_wait(const char *msg) {
    LV_LOG_USER("已进入：弹窗提示图片 加载中，请稍后。。。\n");

    //样式
    static lv_style_t style_load;
    lv_style_init(&style_load);
    lv_style_set_bg_color(&style_load, lv_color_hex(0x000000));
    lv_style_set_bg_opa(&style_load, LV_OPA_80);
    lv_style_set_border_width(&style_load, 0);
    lv_style_set_clip_corner(&style_load, true);
    lv_style_set_radius(&style_load, 20);

    // 创建对话框
    g_pop_del_msg = lv_obj_create(lv_layer_top());
    lv_obj_add_style(g_pop_del_msg, &style_load, 0);
    lv_obj_set_size(g_pop_del_msg, POP_MSG_WIDTH, POP_MSG_HEIGHT);
    lv_obj_set_pos(g_pop_del_msg, 28, 150);

    lv_obj_t* label = lv_label_create(g_pop_del_msg);
    lv_label_set_text(label, msg);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);
    LV_LOG_USER("exit：弹窗提示图片 加载中，请稍候。。。\n");
    return 0;
}
int insert_bv_node(int grade, char *version, char *book_id, char * title) {
    bv_node* new_node = (bv_node*)heap_caps_malloc_dflt(sizeof(bv_node));
    if (new_node != NULL)
    {
        snprintf(new_node->data.book_id, sizeof(new_node->data.book_id), "%s/%s", version, book_id);
        strcpy(new_node->data.book_title, title);
        new_node->data.grade = grade;
        new_node->next = NULL;
    }
    else{
        ESP_LOGI(TAG, "插入节点失败：%s， %s . [%d]", book_id, title, __LINE__);
        return -1;
    }

    ESP_LOGI(TAG, "插入节点：%s， %s . [%d]", book_id, title, __LINE__);

    if (g_bvHead == NULL) {
        g_bvHead = new_node;
    }
    else {
        bv_node* temp = g_bvHead;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
    return 0;
}

static int static_select_book_num() {
    int sum = 0;
    for (int i = 0; i < g_bv_num; i++) {
        if (g_bv_status[i] == 1)
            sum++;
    }
    return sum;
}
static void book_data_manage_cb(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    if (e->code == LV_EVENT_KEY) {
        int keyVal = lv_event_get_key(e);
        switch (keyVal)
        {
        case LV_KEY_UP:
        {
            lv_group_focus_prev(lv_group_get_default());
            lv_obj_t* fobj = lv_group_get_focused(lv_group_get_default());
            g_cur_book = lv_obj_get_index(fobj);
            break;
        }
        case LV_KEY_DOWN:
        {
            lv_group_focus_next(lv_group_get_default());
            lv_obj_t* fobj = lv_group_get_focused(lv_group_get_default());
            g_cur_book = lv_obj_get_index(fobj);
            break;
        }
        //-------------------test start-----------------------
        case LV_KEY_DEL:
        {
            deleting_please_wait("删除中，请稍候...");
            break;
        }
        case LV_KEY_EX_FUNC1: // AB 键 用于删除教材数据
        {
            int child_cnt = static_list_sum();
            if (child_cnt == 0) {
                ;
            }
            else {
                if (g_bv_op == 0) {
                    lv_obj_add_state(g_btn_op, LV_STATE_CHECKED);
                    g_bv_op = 1;
                }
                else {
                    int sum = static_select_book_num();
                    if (sum > 0)
                        pop_delete_msgbox(sum);
                    else {
                        app_msgbox_timeout("", "   用 OK 键操作\n   要删除的教材！", 1500, NULL, NULL);
                        lv_obj_t* obj = lv_obj_get_child(selects_obj, g_cur_book);
                        lv_group_focus_obj(obj);
                    }
                }
            }
            lv_event_stop_processing(e);
            break;
        }
        case LV_KEY_ENTER:
        {
            if (g_bv_op == 1) {
                lv_obj_t* fobj = lv_group_get_focused(lv_group_get_default());
                //uint32_t idx = lv_obj_get_index(fobj);
                uint32_t idx = fobj->user_data;
                g_bv_status[idx] ^= BOOK_SELECT;
                if (g_bv_status[idx] == BOOK_SELECT)
                    lv_obj_add_style(fobj, &bookItemStyle, 0);
                else
                    lv_obj_remove_style(fobj, &bookItemStyle, 0);
            }
            break;
        }
        case LV_KEY_ESC:
        {
            if (g_bv_op == 1) {
                for (int i = 0; i < g_bv_num; i++) {
                    if (g_bv_status[i] == BOOK_SELECT) {
                        lv_obj_t* obj = lv_obj_get_child(selects_obj, i);
                        lv_obj_remove_style(obj, &bookItemStyle, 0);
                        g_bv_status[i] = 0;
                    }
                }
                lv_obj_clear_state(g_btn_op, LV_STATE_CHECKED);
                g_bv_op = 0;
            }
            else {
                ESP_LOGI(TAG, "ESC \n");
                zb_book_data_manage_free();
                //zb_requestSwitchAp(C_ZB_AP_ENGLISH_TEXTBOOK);
                if (g_exist_del_op) {
                    char traverse_path[300] = { 0 };
                    snprintf(traverse_path, sizeof(traverse_path), "%s%s", PLAYER_ONLINE_DATA_PATH, class_info_tl[player_class_num]);
                    del_empty_grd(traverse_path, 0);
                    g_exist_del_op = 0;
                }
#if play_main_online_page_init_has
                play_main_online_page_init(player_class_num, 0);
#endif
                lv_indev_keypad_wait_release();
            }
            break;
        }
        }
    }
}

extern struct lv_img_dsc_t* get_book_cover_img(char* version, char* book_id);

int fill_book_list() 
{
    bv_node* pnode = g_bvHead;
    int i = 0;
    int curBookNum = 0;
    lv_img_dsc_t* book_img = NULL;
    
    if (player_class_num == ENGLISH) //CHINESE_TEXTBOOK
        book_img = zb_img_res_load_one(NULL, PLAYER_IMG_ENGBOOKCOVER, IMG_FREE_TYPE_AUTO, NULL);
    else
        book_img = zb_img_res_load_one(NULL, PLAYER_IMG_CHNBOOKCOVER, IMG_FREE_TYPE_AUTO, NULL);
    
    lv_obj_clean(selects_obj);
    while (pnode)
    {
        if (g_bv_status[i] == 2) {
            pnode = pnode->next;
            i++;
            continue;
        }
        // 创建一个容器来放置图片和文本
        lv_obj_t* container = lv_btn_create(selects_obj);
        lv_obj_set_width(container, lv_disp_get_hor_res(NULL) - 30); // 设置容器宽度为屏幕宽度
        lv_obj_set_height(container, 61 + 4); // 设置容器高度
        lv_obj_align(container, LV_ALIGN_LEFT_MID, 0, 0); // 在屏幕左边中间位置对齐容器
        lv_obj_set_style_bg_color(container, lv_color_white(), 0);
        lv_obj_set_style_border_width(container, 1, 0);
        lv_obj_set_style_border_color(container, lv_color_hex(0x909090), 0);
        lv_obj_set_user_data(container, i);
        //lv_obj_add_event_cb(container, NULL, LV_EVENT_CLICKED, NULL);

        // 在容器中添加图片
        lv_obj_t* img = lv_img_create(container);
        lv_img_dsc_t* pimg = get_book_cover_img(pnode->data.book_id, NULL);

        if (pimg != NULL)
            lv_img_set_src(img, pimg);
        else
            lv_img_set_src(img, book_img);

        lv_obj_set_width(img, 48); // 设置图片宽度
        lv_obj_set_height(img, 61); // 设置图片高度
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0); // 在容器左边中间位置放置图片

        // 在容器中添加文本
        lv_obj_t* label = lv_label_create(container);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(label, 140);
        //lv_obj_set_style_border_width(label, 2, 0);
        //lv_obj_set_style_border_color(label, lv_color_black(), 0);
        lv_obj_set_style_text_color(label, lv_color_black(), LV_STATE_DEFAULT);

        lv_label_set_text(label, pnode->data.book_title); // 设置文本内容
        lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, 8, 0); // 在图片的右边中间位置放置文本
        if (curBookNum == 0)
        {
            lv_obj_scroll_to_view(container, LV_ANIM_OFF);
            lv_group_focus_obj(container);
        }
        i++;
        curBookNum++;
        pnode = pnode->next;
    }

    //int child_cnt = lv_obj_get_child_cnt(selects_obj);
    //if (child_cnt == 0) {
    if (curBookNum == 0) {
        lv_obj_t* container = lv_obj_create(selects_obj);
        
        lv_obj_set_style_bg_color(container, lv_color_make(128, 128, 128), 0);
        lv_obj_set_size(container, POP_MSG_WIDTH, POP_MSG_HEIGHT);
        lv_obj_set_style_radius(container, 18, 0);

        lv_label_t* msg = lv_label_create(container);
        lv_label_set_text(msg, "暂无下载教材");
        lv_obj_set_style_text_color(msg, lv_color_white(), 0);
        lv_obj_set_style_align(msg, LV_ALIGN_CENTER, 0);

        lv_obj_set_style_pad_top(selects_obj, 100, 0);
        lv_obj_set_style_pad_left(selects_obj, 25, 0);
    }

    return i;
}
//我的下载
int zb_book_data_manage_init(const char* language)
{
    g_exist_del_op = 0;
    g_cur_book = 0;
    g_btn_op = 0;
    LV_LOG_USER("已进入：我的下载\n");

    manage_win = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(manage_win);
    lv_obj_set_size(manage_win, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_add_event_cb(manage_win, book_data_manage_cb, LV_EVENT_ALL, NULL);
    player_style = player_normal_style;
    sys_setCurMainWinObj(manage_win);
    lv_img_dsc_t* pImgDsc = zb_img_res_load_one(NULL, player_style.page_bg_img1, IMG_FREE_TYPE_AUTO, NULL);
    lv_obj_set_style_bg_img_src(manage_win, pImgDsc, 0);

    // 标题
    ESP_LOGI(TAG, "%s line[%d]\n", __FUNCTION__, __LINE__);
    lv_obj_t* title_label = lv_label_create(manage_win);
    lv_label_set_text(title_label, "我的下载");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(title_label, &lv_font_simsun_16_cjk, 0);
    lv_obj_set_pos(title_label, 15, 15);

    // AB选择
    g_btn_op = lv_obj_create(manage_win);
    lv_obj_remove_style_all(g_btn_op);
    lv_obj_set_size(g_btn_op, 80, 40);
    lv_obj_set_style_bg_img_src(g_btn_op, zb_img_res_load_one(NULL, MY_DOWNLOAD_IMG_ABBIANJI, IMG_FREE_TYPE_AUTO, NULL), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(g_btn_op, zb_img_res_load_one(NULL, MY_DOWNLOAD_IMG_ABSHANCHU, IMG_FREE_TYPE_AUTO, NULL), LV_STATE_CHECKED);

    lv_obj_align_to(g_btn_op, title_label, LV_ALIGN_OUT_RIGHT_MID, 60, 0);
    //lv_obj_set_style_bg_color(book_manage_btn, lv_color_white(), 0);
    //lv_obj_set_style_border_width(book_manage_btn, 1, 0);
    //lv_obj_set_style_border_color(book_manage_btn, lv_color_white(), 0);
    lv_obj_add_event_cb(g_btn_op, abKey_cb, LV_EVENT_ALL, NULL);
    //lv_obj_t* book_manage_label = lv_label_create(book_manage_btn);
    //lv_label_set_text(book_manage_label, "AB选择");
    //lv_obj_set_style_text_color(book_manage_label, lv_color_black(), LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(title_label, &lv_font_simsun_16_cjk, 0);
    //lv_obj_center(book_manage_label);


    // 项目列表
    ESP_LOGI(TAG, "%s line[%d]\n", __FUNCTION__, __LINE__);
    selects_obj = lv_obj_create(manage_win);
    lv_obj_set_size(selects_obj, SCREEN_WIDTH, 267);
    lv_obj_align_to(selects_obj, manage_win, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(selects_obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_top(selects_obj, 15, 0);
    lv_obj_set_style_pad_left(selects_obj, 15, 0);
    lv_obj_set_style_pad_row(selects_obj, 15, 0);
    ESP_LOGI(TAG, "%s line[%d]\n", __FUNCTION__, __LINE__);

    if (false) {
        deleting_please_wait("准备数据，请稍候...");
#ifndef C_PC_SIM
        play_http_post_create(REQUEST_BOOK_LIST);
#endif
    }
    else {
        char traverse_path[300] = { 0 };
        snprintf(traverse_path, sizeof(traverse_path), "%s%s", PLAYER_ONLINE_DATA_PATH, class_info_tl[player_class_num]);
        get_down_book(traverse_path, 0);
        fill_book_list();
    }
    lv_style_init(&bookItemStyle);
    lv_style_set_opa(&bookItemStyle, LV_OPA_50);
    
    return 0;
}

//删除界面 我的下载  选择界面
int zb_book_data_manage_free(void) {
    if (g_bvHead) {
        heap_caps_free(g_bvHead);
        g_bvHead = NULL;
    }
    if (g_bv_status) {
        heap_caps_free(g_bv_status);
        g_bv_status = NULL;
    }
    if (manage_win) {
        lv_obj_del(manage_win);
        manage_win = NULL;
    }
    if (deleteMsgbox) {
        lv_msgbox_close(deleteMsgbox);
        deleteMsgbox = NULL;
    }
    g_bv_op = 0;
    return 0;
}

static bool ends_with(const char* src, const char* sub) {
    int len1 = strlen(src);
    int len2 = strlen(sub);

    if (len1 < len2) {
        return false;
    }
    // Compare the last n characters
    if (strcasecmp(src + len1 - len2, sub) == 0) {
        return true;
    }
    else {
        return false;
    }

    return false;
}

/// 统计当前下载列表的书本数量
static int static_list_sum()
{
    int cnt = 0;
    for (int i = 0; i < g_bv_num; i++) {
        if (g_bv_status[i] != 2) {
            cnt++;
        }
    }
    return cnt;
}

// 统计当前已经下载的所有书本的数量
static int static_node_sum()
{
    int sum = 0;
    bv_node* pnode = g_bvHead;
    
    if (pnode == NULL)
        return sum;

    while (pnode)
    {
        sum++;
        pnode = pnode->next;
    }
    return sum;
}

int get_down_book(char* path, int indent) {
    char new_path[300]; //新的路径
    ESP_LOGI(TAG, "已进入：打印出指定目录%s,   indent:%d\n", path, indent);
#ifndef C_PC_SIM
    //DIR* dir;
    //struct dirent* entry;
    ////1,打开目录文件
    //dir = opendir(path);
    //if (NULL == dir)
    //{
    //    ESP_LOGE(TAG, "opendir failed");
    //    return -1;
    //}
    //printf("cur: path:  %s, indent = %d\n", path, indent);
    ////2, 循环读取目录下的文件信息 --> 直到读取完毕 
    //int i = 0;
    //while ((entry = readdir(dir)))	//循环读取目录中的信息,如果返回值为NULL,结束循环 
    //{
    //    //目录：递归调用自己
    //    if (entry->d_type == DT_DIR) {
    //        //忽略当前目录和上级目录
    //        if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
    //        {
    //            continue;
    //        }
    //        snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);//构造新路径
    //        ESP_LOGI(TAG, "当前文件夹：%s\n", new_path);
    //        if (indent < 1)
    //            get_down_book(new_path, indent + 1); // 递归调用  
    //    }
    //    //文件：直接打印文件名
    //    else {
    //        printf("%s\n", entry->d_name);// 打印文件 
    //        if (ends_with(entry->d_name, ".grd"))
    //        {
    //            nmbox_book_list_t nblt;
    //            snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);//构造新路径
    //            ESP_LOGE(TAG, "grd path : %s", new_path);
    //            read_grd_parse(new_path, &nblt);
    //            for (int i = 0; i < nblt.book_num; i++)
    //            {
    //                if (nblt.down[i] == 1) {
    //                    insert_bv_node(0, nblt.version, nblt.book_id[i], nblt.term[i]);
    //                }
    //            }
    //        }
    //    }
    //}
    //ESP_LOGI(TAG, "一共检索到%d个文件!\n", i);
    ////3, 关闭目录 
    //closedir(dir);
#else
    nmbox_book_list_t nblt;
    snprintf(new_path, sizeof(new_path), "%s/清华大学版/grade_1.grd", path);//构造新路径
    ESP_LOGE(TAG, "grd path : %s", new_path);
    read_grd_parse(new_path, &nblt);
    for (int i = 0; i < nblt.book_num; i++)
    {
        if (nblt.down[i] == 1) {
            insert_bv_node(0, nblt.version, nblt.book_id[i], nblt.term[i]);
            //insert_bv_node(0, nblt.version, nblt.book_id[i], nblt.term[i]);
            //insert_bv_node(0, nblt.version, nblt.book_id[i], nblt.term[i]);
        }
    }
    snprintf(new_path, sizeof(new_path), "%s/清华大学版/grade_2.grd", path);//构造新路径
    ESP_LOGE(TAG, "grd path : %s", new_path);
    read_grd_parse(new_path, &nblt);
    for (int i = 0; i < nblt.book_num; i++)
    {
        if (nblt.down[i] == 1) {
            insert_bv_node(0, nblt.version, nblt.book_id[i], nblt.term[i]);
        }
    }

#endif
    g_bv_num = static_node_sum();
    if (g_bv_num > 0) {
        g_bv_status = (char*)heap_caps_malloc_dflt(sizeof(char) * g_bv_num);
        memset(g_bv_status, 0, g_bv_num * sizeof(char));
    }
    else {
        g_bv_status = NULL;
    }
    return g_bv_num;
}

/*
* 删除没有使用的 *.grd 文件，当存在新教材的时候给机会删除，重新获取新的版本数据
*/
static int del_empty_grd(char* path, int indent) {
    char new_path[300]; //新的路径
    ESP_LOGI(TAG, "已进入：del_empty_grd:%s,   indent:%d\n", path, indent);
#ifndef C_PC_SIM
    //DIR* dir;
    //struct dirent* entry;
    ////1,打开目录文件
    //dir = opendir(path);
    //if (NULL == dir)
    //{
    //    ESP_LOGE(TAG, "opendir failed");
    //    return -1;
    //}
    //printf("cur: path:  %s, indent = %d\n", path, indent);
    ////2, 循环读取目录下的文件信息 --> 直到读取完毕 
    //int i = 0;
    //while ((entry = readdir(dir)))	//循环读取目录中的信息,如果返回值为NULL,结束循环 
    //{
    //    //目录：递归调用自己
    //    if (entry->d_type == DT_DIR) {
    //        //忽略当前目录和上级目录
    //        if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
    //        {
    //            continue;
    //        }
    //        snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);//构造新路径
    //        ESP_LOGI(TAG, "当前文件夹：%s\n", new_path);
    //        if (indent < 1)
    //            del_empty_grd(new_path, indent + 1); // 递归调用  
    //    }
    //    //文件：直接打印文件名
    //    else {
    //        printf("%s\n", entry->d_name);// 打印文件 
    //        if (ends_with(entry->d_name, ".grd"))
    //        {
    //            nmbox_book_list_t nblt;
    //            uint8_t is_empty = true;
    //            snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);//构造新路径
    //            ESP_LOGE(TAG, "grd path : %s", new_path);
    //            read_grd_parse(new_path, &nblt);
    //            for (int i = 0; i < nblt.book_num; i++)
    //            {
    //                if (nblt.down[i] == 1) {
    //                    is_empty = false;
    //                }
    //            }
    //            if (is_empty) {
    //                ESP_LOGE(TAG, "删除grd 文件 : %s", new_path);
    //                zb_remove(new_path);
    //            }
    //            else {
    //                ESP_LOGE(TAG, "该年级还存在数据 %s", new_path);
    //            }
    //        }
    //    }
    //}
    ////3, 关闭目录 
    //closedir(dir);
#else
    nmbox_book_list_t nblt;
    snprintf(new_path, sizeof(new_path), "%s/清华大学版/grade_1.grd", path);//构造新路径
    uint8_t is_empty = true;
    ESP_LOGE(TAG, "grd path : %s", new_path);
    read_grd_parse(new_path, &nblt);
    for (int i = 0; i < nblt.book_num; i++)
    {
        if (nblt.down[i] == 1) {
            is_empty = false;
        }
    }
    if (is_empty)
        zb_remove(new_path);
#endif
    return 0;
}


int del_down_one_book(const char *big_v, const char *small_v)
{
    char book_path[250] = { 0 }; //新的路径
    char ini_path[360] = { 0 };
    char detail_version[100] = { 0 };

    ESP_LOGE(TAG, "big_v = %s, small_v = %s", big_v, small_v);

    snprintf(ini_path, sizeof(ini_path), "%s%s/%s/%s.ini", PLAYER_ONLINE_DATA_PATH, class_info_tl[player_class_num], big_v,small_v);
    ESP_LOGE(TAG, "ini_path = %s", ini_path);
    read_book_version(ini_path, detail_version, sizeof(detail_version));

    snprintf(book_path, sizeof(book_path), "%s%s/%s/%s", PLAYER_ONLINE_DATA_PATH, class_info_tl[player_class_num], big_v, detail_version);
    ESP_LOGE(TAG, "book_path = %s", book_path);

#ifndef C_PC_SIM
    //// 1、 删除 *.ini
    //zb_remove(ini_path);

    //// 2、 删除 *.bin
    //DIR* dir;
    //struct dirent* entry;
    ////1,打开目录文件
    //dir = opendir(book_path);
    //if (NULL == dir)
    //{
    //    ESP_LOGE(TAG, "opendir : [%s] failed", book_path);
    //    return -1;
    //}else
    //    printf("cur: path:  %s\n", book_path);
    ////2, 循环读取目录下的文件信息 --> 直到读取完毕 
    //int i = 0;
    //while ((entry = readdir(dir)))	//循环读取目录中的信息,如果返回值为NULL,结束循环 
    //{
    //    if (entry->d_type != DT_DIR) {
    //        strcpy(ini_path, book_path);
    //        strcat(ini_path, "/");
    //        strcat(ini_path, entry->d_name);
    //        //sprintf(ini_path, "%s/%s", book_path, entry->d_name);
    //        zb_remove(ini_path);
    //        printf("删除文件：%s\n", ini_path);
    //    }
    //}

    ////3, 关闭目录 
    //closedir(dir);
    //printf("删除目录：%s\n", book_path);
    //zb_rmdir(book_path);
    //return 1;
#else
    return 1;
#endif
}
//弹出框回调函数
static void msgbox_cb(lv_event_t* e)
{
    if (e->code == LV_EVENT_KEY) {
        int keyVal = lv_event_get_key(e);
        ESP_LOGI(TAG, "ENTER:222, key = %d  \n", keyVal);
        if (keyVal == LV_KEY_ESC) {
            lv_msgbox_close(deleteMsgbox);
            deleteMsgbox = NULL;
            lv_group_focus_obj(lv_obj_get_child(selects_obj, g_cur_book));
            return;
        }
    }
    else if (e->code == LV_EVENT_VALUE_CHANGED) {
        uint16_t idx = lv_msgbox_get_active_btn(deleteMsgbox);
        LV_LOG_USER("clicked which btn: %d\n", idx);

        lv_msgbox_close(deleteMsgbox); //删除
        deleteMsgbox = NULL;

        bv_node* pHead = g_bvHead;
        if (idx == 1) { // 确定
            g_exist_del_op = 1;
            deleting_please_wait("删除中，请稍候..."); // 显示：正在删除中...
            lv_refr_now(NULL);      // 立刻刷新屏幕
            for (int i = 0; i < g_bv_num; i++) {
                if (g_bv_status[i] == BOOK_SELECT) {
                    LV_LOG_USER("book_id:%s, len=%d", pHead->data.book_id, strlen(pHead->data.book_id));
                    char* p = strchr(pHead->data.book_id, '/');
                    *p = '\0';
                    del_down_one_book(pHead->data.book_id, p + 1);
                    *p = '/';
                    g_bv_status[i] = BOOK_DELETE;
                }
                pHead = pHead->next;
            }

            if (g_pop_del_msg) {
                lv_obj_del(g_pop_del_msg);
                g_pop_del_msg = NULL;
            }
            fill_book_list();
        }
        else {
            lv_group_focus_obj(lv_obj_get_child(selects_obj, g_cur_book));
            LV_LOG_USER("----- continue ------");
        }
    }
}

static void pop_delete_msgbox(int num)
{
    static const char* btns[] = { "取消", "确定", "" };
    char msg[300] = { 0 };
    sprintf(msg, "\n     确定要删除\n 所选的 %d 本教材吗？\n", num);
    deleteMsgbox = lv_msgbox_create(NULL, "", msg, btns, false);
    lv_obj_set_size(deleteMsgbox, 200, 130);

    lv_obj_add_event_cb(deleteMsgbox, msgbox_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(deleteMsgbox, msgbox_cb, LV_EVENT_KEY, NULL);
    lv_obj_center(deleteMsgbox);
}

