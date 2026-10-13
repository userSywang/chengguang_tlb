/**
 * @file lv_file_explorer.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>
#include "lv_file_explorer.h"
#include "../mainmenu/tlb_main_img.h"
#include "../utils/misc_utils.h"

#if LV_USE_FILE_EXPLORER != 0

#include <string.h>
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_file_explorer_class


#define	USE_TBLE_COL_EXPAND		0  //

#define	USE_SYMBOLS		0  //文件列表头是否加上图标


#define	PATH_LABLE_X	0
#define	PATH_LABLE_Y	0
#define	PATH_LABLE_W	SCREEN_WIDTH
#define	PATH_LABLE_H	42

#define	FILE_LIST_W		(SCREEN_WIDTH) 					//list 右侧滚动条不到边，所以+10个点
#define	FILE_LIST_H		(SCREEN_HEIGHT - PATH_LABLE_H)
#define	FILE_LIST_X		0
#define	FILE_LIST_Y		PATH_LABLE_H
#define	FILE_LIST_ROW_W		FILE_LIST_W
//#define	FILE_LIST_COL_H		32

#define	FILE_ICON_W		20  //文件类型图标
#define	FILE_ICON_H		20


#define DIRECTORY_TYPE	0
#define IMAGE_TYPE	1
#define AUDIO_TYPE	2
#define VIDEO_TYPE	3
#define FILE_TYPE	4

#define FILE_PATH_BG_COLOR	0xF1EFFF //0x9E97FF  //路径背景色

/**********************
 *      TYPEDEFS
 **********************/

extern lv_img_dsc_t  g_XXX_img_dsc_tbl[];
/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_file_explorer_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_file_explorer_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);

static void brower_file_event_handler(lv_event_t * e);


static void init_style(lv_obj_t * obj);
static void show_dir(lv_obj_t * obj, char * path);
static void strip_ext(char * dir);
static void sort_table_items(lv_obj_t * tb, int16_t lo, int16_t hi);
static void exch_table_item(lv_obj_t * tb, int16_t i, int16_t j);
static bool is_begin_with(const char * str1, const char * str2);
static bool is_end_with(const char * str1, const char * str2);
static void file_icon_draw_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_file_explorer_class = {
    .constructor_cb = lv_file_explorer_constructor,
    .destructor_cb  = lv_file_explorer_destructor,
    .width_def      = LV_SIZE_CONTENT,
    .height_def     = LV_SIZE_CONTENT,
    .instance_size  = sizeof(lv_file_explorer_t),
    .base_class     = &lv_obj_class
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   extra  FUNCTIONS
 **********************/

typedef int funcConvertStr(unsigned short *);

int GbkToUtf8(char* pStrSrc, char* pcStrDst, int iDstBuffSiz, wchar_t* pwAid,
    int iMidBuffSiz, funcConvertStr funConvert);

int UTF8ToGBK(char* pcStrSrc, char* pcStrDst, int iDstBuffSize, int param);


/**********************
 *   GLOBAL FUNCTIONS
 **********************/



lv_obj_t * lv_file_explorer_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}


/*=====================
 * Setter functions
 *====================*/


void lv_file_explorer_set_sort(lv_obj_t * obj, lv_file_explorer_sort_t sort)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;

    uint16_t sum = lv_table_get_row_cnt(explorer->file_list);

    switch (sort)
    {
        case LV_EXPLORER_SORT_KIND:
            sort_table_items(explorer, 0, sum - 1);
            break;
        case LV_EXPLORER_SORT_NAME:
            /* TODO */
            break;
        default:
            break;
    }
}


/*=====================
 * Getter functions
 *====================*/
char * lv_file_explorer_get_sel_fn(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;

    return explorer->sel_fp;
}

char * lv_file_explorer_get_cur_path(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;

    return explorer->cur_path;
}

lv_obj_t * lv_file_explorer_get_file_list(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;

    return explorer->file_list;
}

//lv_obj_t * lv_file_explorer_get_head(lv_obj_t * obj)
//{
//    LV_ASSERT_OBJ(obj, MY_CLASS);

 //   lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;

 //   return explorer->head_area;
//}

lv_obj_t * lv_file_explorer_get_path_obj(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;

    return explorer->path_label;
}



/*=====================
 * Other functions
 *====================*/
void lv_file_explorer_open_dir(lv_obj_t * obj, char * dir)
{
    lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;

    show_dir(obj, dir);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

//============================================[]
static void file_icon_draw_cb(lv_event_t * e) 
{
	lv_obj_t * obj = lv_event_get_user_data(e);
	lv_obj_draw_part_dsc_t * draw_dec = lv_event_get_draw_part_dsc(e);

	lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;
	lv_area_t * p_area = draw_dec->draw_area;
	
//   LV_LOG("%s [%d] \n", __FUNCTION__,__LINE__);
#if 1

   //-----------------
    lv_img_dsc_t * pImgDst ;
	uint8_t file_type = explorer->file_type[draw_dec->id];
	
//	LV_LOG("part: %05x,type:%d, id: %d: %d,%d, %d,%d\r\n", draw_dec->part, draw_dec->type, draw_dec->id, p_area->x1, p_area->y1, p_area->x2, p_area->y2);
	if(draw_dec->part == LV_PART_ITEMS)
	{
		switch(file_type)
	    {
	    
			case DIRECTORY_TYPE:
				
//				pImgDst = &g_FILE_ICON_img_dsc_tbl[FILE_ICON_directory_file-1];
				pImgDst = &g_XXX_img_dsc_tbl[MMN_directory_file-1];
					
				break;
			case IMAGE_TYPE:
				pImgDst = &g_XXX_img_dsc_tbl[MMN_directory_file-1];
				break;
			case AUDIO_TYPE:
				pImgDst = &g_XXX_img_dsc_tbl[MMN_directory_file-1];
				break;
			case VIDEO_TYPE:
				pImgDst = &g_XXX_img_dsc_tbl[MMN_video_file-1];
				break;
			case FILE_TYPE:
				pImgDst = &g_XXX_img_dsc_tbl[MMN_nor_file-1];
				break;

			default:
				pImgDst = &g_XXX_img_dsc_tbl[MMN_nor_file-1];
			
			break;
		}

	    int16_t iconW = pImgDst->header.w;
	    lv_area_t icon_coords;
	    icon_coords.x1 = p_area->x1+8;
	    icon_coords.y1 = p_area->y1+8;
	    icon_coords.x2 = icon_coords.x1 +FILE_ICON_W;
	    icon_coords.y2 = icon_coords.y1 +FILE_ICON_H;
		
	 
	    lv_draw_img_dsc_t dsc;
	    lv_draw_img_dsc_init(&dsc);
	    dsc.opa = LV_OPA_100;
	    dsc.blend_mode = LV_BLEND_MODE_MULTIPLY;
	    lv_draw_img(draw_dec->draw_ctx, &dsc, &icon_coords,  pImgDst);  // 
	    
	}    
#endif


}


static void lv_file_explorer_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");
	lv_area_t coords;

    lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;
	
    lv_memset_00(explorer->cur_path, sizeof(explorer->cur_path));

	lv_obj_set_size(obj, SCREEN_WIDTH, SCREEN_HEIGHT);
	


    explorer->cont = lv_obj_create(obj);
	lv_obj_set_size(explorer->cont, LV_PCT(100), LV_PCT(100));	
    lv_obj_set_style_pad_all(explorer->cont, 0, 0);
    lv_obj_set_style_border_width(explorer->cont, 0, 0);		
	lv_obj_set_style_radius(explorer->cont, 0, 0);
    lv_obj_set_style_bg_color(explorer->cont, lv_color_hex(FILE_PATH_BG_COLOR), 0);
	
	lv_obj_clear_flag(explorer->cont, LV_OBJ_FLAG_SCROLLABLE);
	
    lv_obj_align(explorer->cont, LV_ALIGN_TOP_LEFT, 0, 0);
	
    // 展示当前路径
    explorer->path_label = lv_label_create(explorer->cont);
    lv_label_set_text(explorer->path_label, " File Browser"); 		//LV_SYMBOL_EYE_OPEN
    lv_obj_set_size(explorer->path_label, PATH_LABLE_W, PATH_LABLE_H);

    lv_obj_set_style_pad_all(explorer->path_label,0, 0);
    lv_obj_set_style_pad_top(explorer->path_label,0, 0);  //字符的显示位置往下偏一点    
    lv_obj_set_style_border_width(explorer->path_label, 0, 0);
	

    lv_obj_align(explorer->path_label, LV_ALIGN_TOP_LEFT, 0, 10);
	
	lv_label_set_long_mode(explorer->path_label, LV_LABEL_LONG_SCROLL_CIRCULAR );  //LV_LABEL_LONG_SCROLL
    lv_obj_set_scroll_dir(explorer->path_label, LV_DIR_HOR);	
	
   //增 lv_obj_set_style_text_font(explorer->path_label, &lv_font_simsun_16_cjk, 0);
   
//	lv_obj_set_style_text_align(explorer->path_label, LV_TEXT_ALIGN_CENTER, 0);

    // 目录内容展示列表

    explorer->file_list = lv_table_create(explorer->cont);	
    lv_obj_set_size(explorer->file_list, FILE_LIST_W, FILE_LIST_H); // 
	
	lv_obj_set_style_radius(explorer->file_list, 0, 0);	
    lv_obj_set_style_pad_all(explorer->file_list, 0, 0);
    lv_obj_set_style_border_width(explorer->file_list, 0, 0);		
	lv_obj_set_style_outline_width(explorer->file_list, 0, 0);  //选中时不显示边框状态
	lv_obj_set_style_outline_width(explorer->file_list, 0, LV_STATE_FOCUS_KEY);  //选中时不显示边框状态
	lv_obj_set_style_outline_width(explorer->file_list, 0, LV_STATE_FOCUSED);  //选中时不显示边框状态

	lv_obj_set_style_pad_all(explorer->file_list, 10, LV_PART_ITEMS); //单元格的 pad

	lv_obj_align(explorer->file_list, LV_ALIGN_TOP_LEFT, FILE_LIST_X, FILE_LIST_Y);
	

    lv_table_set_col_width(explorer->file_list, 0, FILE_LIST_ROW_W); //
    lv_table_set_col_cnt(explorer->file_list, 1);
//    lv_table_set_row_cnt(explorer->file_list, 1);
	
	
//	lv_obj_clear_flag(explorer->file_list, LV_OBJ_FLAG_SCROLLABLE);
	
 //   lv_obj_add_event_cb(explorer->path_label, path_label_draw_cb, LV_EVENT_DRAW_PART_BEGIN, obj);


	lv_group_add_obj(lv_group_get_default(), explorer->file_list);  //xzz

    lv_obj_add_event_cb(explorer->file_list, brower_file_event_handler, LV_EVENT_SIZE_CHANGED, obj);
    lv_obj_add_event_cb(explorer->file_list, brower_file_event_handler, LV_EVENT_KEY, obj);
    lv_obj_add_event_cb(explorer->file_list, brower_file_event_handler, LV_EVENT_VALUE_CHANGED, obj);
	
    lv_obj_add_event_cb(explorer->file_list, file_icon_draw_cb, LV_EVENT_DRAW_PART_END, obj);

    // 初始化样式
//    init_style(obj);


    lv_obj_add_flag(explorer->file_list, LV_OBJ_FLAG_CLICK_FOCUSABLE); //xzz 240320
	lv_obj_add_state(explorer->file_list, LV_STATE_FOCUS_KEY);  //加此状态后才会有选中框效果. xzz 240322
	lv_group_focus_obj(explorer->file_list);  //xzz

	
    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_file_explorer_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");
} 
#if 0
static void init_style(lv_obj_t * obj)
{
    lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;

    // cont区域风格样式
    static lv_style_t cont_style;
    lv_style_init(&cont_style);
    lv_style_set_radius(&cont_style, 0);
    lv_style_set_bg_opa(&cont_style, LV_OPA_0);
    lv_style_set_border_width(&cont_style, 0);
    lv_style_set_outline_width(&cont_style, 0);
    lv_style_set_pad_column(&cont_style, 0);
    lv_style_set_pad_row(&cont_style, 0);
    lv_style_set_pad_all(&cont_style, 0);
	

    // 右侧文件浏览区域的样式风格
    static lv_style_t file_list_style;
    lv_style_init(&file_list_style);
    lv_style_set_bg_color(&file_list_style, lv_color_hex(0xffffff));
    lv_style_set_pad_all(&file_list_style, 0);
    lv_style_set_radius(&file_list_style, 0);
    lv_style_set_border_width(&file_list_style, 0);
    lv_style_set_outline_width(&file_list_style, 0);

    // 设置 file_explorer 的样式
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf2f1f6), 0);


    lv_obj_add_style(explorer->cont, &cont_style, 0);	

    lv_obj_add_style(explorer->file_list, &file_list_style, 0);

}

#endif


static void brower_file_event_handler(lv_event_t * e)
{
#if 0
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_user_data(e);
    lv_obj_t * btn = lv_event_get_target(e);

    lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;
#if LV_USE_FS_WIN32
    char temp_fn[LV_FILE_EXPLORER_PATH_MAX_LEN];
#endif
//	LV_LOG("%s code: %d\r\n", __FUNCTION__, code);
    if(code == LV_EVENT_VALUE_CHANGED) 
	{
        //struct stat stat_buf;
        char file_name[LV_FILE_EXPLORER_PATH_MAX_LEN];
        char * str_fn = NULL;
        uint16_t row;
        uint16_t col;

        memset(file_name, 0, sizeof(file_name));
        lv_table_get_selected_cell(explorer->file_list, &row, &col);
        str_fn = lv_table_get_cell_value(explorer->file_list, row, col);

#if USE_SYMBOLS
		str_fn = str_fn+5;
#else
		str_fn = str_fn+3;  //字符串前加了3个空格
#endif		

        LV_LOG("fpath: %s\r\n",  str_fn);
        if((strcmp(str_fn, ".") == 0))  return;
        
        if((strcmp(str_fn, "..") == 0) && (strlen(explorer->cur_path) > 3))
        {
            strip_ext(explorer->cur_path);
            strip_ext(explorer->cur_path); // 去掉最后的 '/' 路径
//            lv_snprintf(file_name, sizeof(file_name), "%s", explorer->cur_path);
			sprintf(file_name, "%s", explorer->cur_path);
        }
        else
        {
            if(strcmp(str_fn, "..") != 0){
//                lv_snprintf(file_name, sizeof(file_name), "%s%s", explorer->cur_path, str_fn);
				sprintf(file_name, "%s%s", explorer->cur_path,str_fn);
            }
        }
#if LV_USE_FS_WIN32			
		memset(temp_fn, 0, sizeof(temp_fn));
		UTF8ToGBK(file_name, temp_fn, LV_FILE_EXPLORER_PATH_MAX_LEN, 0);
		strcpy(file_name, temp_fn); // file name -> gbk
#else

#endif

        lv_fs_dir_t dir;
        if(lv_fs_dir_open(&dir, file_name) == LV_FS_RES_OK) {
            lv_fs_dir_close(&dir);
            show_dir(obj, file_name);
        }
        else {
            if(strcmp(str_fn, "..") != 0) {
                explorer->sel_fp = str_fn;
                lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
            }
        }
		
    }
    else if(code == LV_EVENT_SIZE_CHANGED) 
	{
        lv_table_set_col_width(explorer->file_list, 0, lv_obj_get_width(explorer->file_list));
    }
   else if (code == LV_EVENT_KEY) //|| code == LV_EVENT_KEY_STAGE)
    {
		int key = *((int*)lv_event_get_param(e));

		if (key == LV_KEY_ESC)
		{			
			lv_event_send(obj, code, lv_event_get_param(e));
		}
		
	}
#endif
}

static void show_dir(lv_obj_t * obj, char * path)
{
    lv_file_explorer_t * explorer = (lv_file_explorer_t *)obj;

    char fn[LV_FILE_EXPLORER_PATH_MAX_LEN];
    uint16_t index = 0;
    lv_fs_dir_t dir;
    lv_fs_res_t res;
#if LV_USE_FS_WIN32
    char temp_fn[LV_FILE_EXPLORER_PATH_MAX_LEN];
#endif
//	GbkToUtf8(path, temp_fn, LV_FILE_EXPLORER_PATH_MAX_LEN, 0, 0,0);
//    res = lv_fs_dir_open(&dir, temp_fn);

	res = lv_fs_dir_open(&dir, path);	


    if(res != LV_FS_RES_OK) {
        LV_LOG_USER("Open dir error %d!\r\n", res);
        return;
    }

	explorer->file_type[index] = 0;  //返回上一级目录
#if USE_SYMBOLS
	lv_table_set_cell_value_fmt(explorer->file_list, index++, 0, LV_SYMBOL_DIRECTORY "  %s", "..");
#else
    lv_table_set_cell_value_fmt(explorer->file_list, index++, 0, "   %s", "..");
#endif	

	
    while(1)
	{
		memset(fn, 0, sizeof(fn));
#if LV_USE_FS_WIN32
		memset(temp_fn, 0, sizeof(temp_fn));
        res = lv_fs_dir_read(&dir, temp_fn);  //read next file or direct. xzz
		GbkToUtf8(temp_fn, fn, LV_FILE_EXPLORER_PATH_MAX_LEN, 0, 0,0);	

#else
        res = lv_fs_dir_read(&dir, fn);  //read next file or direct. xzz

#endif
        if(res != LV_FS_RES_OK) 
		{
//            LV_LOG_USER("Driver, file or directory is not exists %d!", res);
            LV_LOG("All file or directory have been read out. %d\r\n", res); //xzz
            break;
        }

        /*fn is empty, if not more files to read*/
        if(strlen(fn) == 0) {
            LV_LOG_USER("Not more files to read!\r\n");
            break;
        }

        // 识别并展示文件
        if ((is_end_with(fn, ".png") == true)  || (is_end_with(fn, ".PNG") == true)  ||\
            (is_end_with(fn , ".jpg") == true) || (is_end_with(fn , ".JPG") == true) ||\
            (is_end_with(fn , ".bmp") == true) || (is_end_with(fn , ".BMP") == true) ||\
            (is_end_with(fn , ".gif") == true) || (is_end_with(fn , ".GIF") == true)) {
#if USE_SYMBOLS
            lv_table_set_cell_value_fmt(explorer->file_list, index, 0, LV_SYMBOL_IMAGE "  %s", fn);
#else
            lv_table_set_cell_value_fmt(explorer->file_list, index, 0, "   %s", fn);
#endif

			explorer->file_type[index] = 1;  //图片类型
	
        }
        else if ((is_end_with(fn , ".mp3") == true) || (is_end_with(fn , ".MP3") == true)) {
#if USE_SYMBOLS
            lv_table_set_cell_value_fmt(explorer->file_list, index, 0, LV_SYMBOL_AUDIO "  %s", fn);
#else
            lv_table_set_cell_value_fmt(explorer->file_list, index, 0, "   %s", fn);
#endif 
			explorer->file_type[index] = 2;  //音乐类型
        }
        else if ((is_end_with(fn , ".mp4") == true) || (is_end_with(fn , ".MP4") == true)) {
#if USE_SYMBOLS
			lv_table_set_cell_value_fmt(explorer->file_list, index, 0, LV_SYMBOL_VIDEO "  %s", fn);
#else
			lv_table_set_cell_value_fmt(explorer->file_list, index, 0,"   %s", fn);
#endif 
			explorer->file_type[index] = 3;  //视频类

        }
        else if((is_end_with(fn , ".") == true) || (is_end_with(fn , "..") == true)) {
            /*is dir*/
            //lv_table_set_cell_value_fmt(explorer->file_list, index, 0, LV_SYMBOL_DIRECTORY "  %s", fn);
            continue;
        }
        else if(fn[0] == '/') {/*is dir*/
#if USE_SYMBOLS
			lv_table_set_cell_value_fmt(explorer->file_list, index, 0, LV_SYMBOL_DIRECTORY "  %s", fn+1);
#else
			lv_table_set_cell_value_fmt(explorer->file_list, index, 0, "   %s", fn+1);
#endif 
			explorer->file_type[index] = 0;  //目录

        }
        else {
#if USE_SYMBOLS
		lv_table_set_cell_value_fmt(explorer->file_list, index, 0, LV_SYMBOL_FILE "  %s", fn);
#else
		lv_table_set_cell_value_fmt(explorer->file_list, index, 0, "   %s", fn);
#endif	
		explorer->file_type[index] = 4;  //普通文件
        }

        index++; 
		if(index > LV_FILE_EXPLORER_MAX_NUM)   //xzz 
        {
        	LV_LOG("The number of files exceeds the maximum limit %d \r\n", index);
		}
    }

    lv_fs_dir_close(&dir);
	lv_table_t * table = (lv_table_t *)explorer->file_list;
	if(table->row_act >= index-1)  //进入新目录时原选择项超过现有子目录时，保证有选中项
	{
		table->row_act = index-1; 
		table->col_act = 0; 
	}
		
    lv_table_set_row_cnt(explorer->file_list, index);
    lv_file_explorer_set_sort(obj, LV_EXPLORER_SORT_KIND);

    // 让table移动到最顶部
    lv_obj_scroll_to_y(explorer->file_list, 0, LV_ANIM_OFF);
	lv_group_focus_obj(explorer->file_list);  //xzz
//	lv_obj_add_state(explorer->file_list, LV_STATE_FOCUS_KEY);  //加此状态后才会有选中框效果. xzz 240322

    lv_memset_00(explorer->cur_path, sizeof(explorer->cur_path));
#if LV_USE_FS_WIN32
	GbkToUtf8(path, explorer->cur_path, sizeof(explorer->cur_path), 0, 0,0);
#else			
    strcpy(explorer->cur_path, path);
#endif
	if(strlen(explorer->cur_path ) == 0)   //到最后根目录时加上'/'
			*explorer->cur_path = '/';

    lv_label_set_text_fmt(explorer->path_label, " %s", explorer->cur_path); //LV_SYMBOL_EYE_OPEN
    


    size_t cur_path_len = strlen(explorer->cur_path);
    if((*((explorer->cur_path) + cur_path_len) != '/') && (cur_path_len < LV_FILE_EXPLORER_PATH_MAX_LEN) && (cur_path_len > 1)) {
        *((explorer->cur_path) + cur_path_len) = '/';
    }  
}


// 去掉最后的''/''
static void strip_ext(char *dir)
{
    char *end = dir + strlen(dir);

    while (end >= dir && *end != '/') {
        --end;
    }

    if (end > dir) {
        *end = '\0';
    }
    else if (end == dir) {
        *(end+1) = '\0';
    }

}



static void exch_table_item(lv_obj_t * tb, int16_t i,int16_t j )
{

	if(i == j)  return;
#if USE_TBLE_COL_EXPAND
    char * tmp;
	lv_file_explorer_t * explorer = (lv_file_explorer_t *)tb;
	tmp = lv_table_get_cell_value(explorer->file_list, i, 0);
	
    lv_table_set_cell_value(explorer->file_list, 0, 2, tmp);
	
    lv_table_set_cell_value(explorer->file_list, i, 0, lv_table_get_cell_value(explorer->file_list, j, 0));
    lv_table_set_cell_value(explorer->file_list, j, 0, lv_table_get_cell_value(explorer->file_list, 0, 2));

    tmp = lv_table_get_cell_value(explorer->file_list, i, 1);
    lv_table_set_cell_value(explorer->file_list, 0, 2, tmp);
    lv_table_set_cell_value(explorer->file_list, i, 1, lv_table_get_cell_value(explorer->file_list, j, 1));
    lv_table_set_cell_value(explorer->file_list, j, 1, lv_table_get_cell_value(explorer->file_list, 0, 2));

#else 
    char * tmp;
	char cell_text[256];
	lv_file_explorer_t * explorer = (lv_file_explorer_t *)tb;
	tmp = lv_table_get_cell_value(explorer->file_list, i, 0);
	strcpy(cell_text,tmp);
	lv_table_set_cell_value(explorer->file_list, i, 0, lv_table_get_cell_value(explorer->file_list, j, 0));
	lv_table_set_cell_value(explorer->file_list, j, 0, cell_text);

	uint8_t t_data = explorer->file_type[i];
	explorer->file_type[i] = explorer->file_type[j];
	explorer->file_type[j] = t_data;

	
#endif		
    
}


// Quick sort 3 way
// 三切分快速排序
static void sort_table_items(lv_obj_t * tb, int16_t lo, int16_t hi )
{

	lv_file_explorer_t * explorer = (lv_file_explorer_t *)tb;
	
    if( lo >= hi ) return;  //单个元素或者没有元素的情况

    int16_t lt = lo;
    int16_t i = lo + 1;  //第一个元素是切分元素，所以指针i可以从lo+1开始
    int16_t gt = hi;
#if USE_TBLE_COL_EXPAND //原来用字符串比较
    const char * v = lv_table_get_cell_value(explorer->file_list, lo, 1);
    while( i <= gt )
    {
        if(strcmp(lv_table_get_cell_value(explorer->file_list, i, 1), v) < 0)  	//小于切分元素的放在lt左边，因此指针lt和指针i整体右移
            exch_table_item(explorer, lt++, i++);
        else if(strcmp(lv_table_get_cell_value(explorer->file_list, i, 1), v) > 0)  //大于切分元素的放在gt右边，因此指针gt需要左移
            exch_table_item(explorer, i, gt--);
        else
            i++;
    }
#else
    uint8_t v = explorer->file_type[lo];
    while( i <= gt )
    {
	    uint8_t i_data = explorer->file_type[i];
		
		if(i_data < v) //小于切分元素的放在lt左边，因此指针lt和指针i整体右移
		{
			exch_table_item(explorer, lt++, i++);
		}
		else if(i_data > v)  //大于切分元素的放在gt右边，因此指针gt需要左移
		{
			exch_table_item(explorer, i, gt--);
		}
		else 
			i++;


    }
#endif
    //lt-gt的元素已经排定，只需对lt左边和gt右边的元素进行递归求解
    sort_table_items(tb, lo, lt-1);
    sort_table_items(tb, gt+1, hi);
}


static bool is_begin_with(const char * str1, const char * str2)
{
    if(str1 == NULL || str2 == NULL)
        return false;

    uint16_t len1 = strlen(str1);
    uint16_t len2 = strlen(str2);
    if((len1 < len2) || (len1 == 0 || len2 == 0))
        return false;

    uint16_t i = 0;
    char * p = str2;
    while(*p != '\0')
    {
        if(*p != str1[i])
            return false;

        p++;
        i++;
    }

    return true;
}



static bool is_end_with(const char * str1, const char * str2)
{
    if(str1 == NULL || str2 == NULL)
        return false;
    
    uint16_t len1 = strlen(str1);
    uint16_t len2 = strlen(str2);
    if((len1 < len2) || (len1 == 0 || len2 == 0))
        return false;
    
    while(len2 >= 1)
    {
        if(str2[len2 - 1] != str1[len1 - 1])
            return false;

        len2--;
        len1--;
    }

    return true;
}


#endif  /*LV_USE_FILE_EXPLORER*/
