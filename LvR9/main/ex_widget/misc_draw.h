#ifndef _MISC_DRAW_H_
#define _MISC_DRAW_H_

#include <stdint.h>


typedef struct { 
    uint32_t m_type : 8;   // 0 text, 1 img
    uint32_t m_locX : 12;
    uint32_t m_locY : 12;
    union {
        const char * m_pText;  // text
        uint32_t   m_imgHash;
    } unn;
} misc_draw_item_t;


typedef struct {
	int16_t  m_num;
	misc_draw_item_t * pstDrawItems;
} misc_draw_mult_items_t;



/** 显示多项文字、图片 
 */
void draw_txt_img_group_cb(lv_event_t * evt, misc_draw_mult_items_t * pDrawItems);



#endif

