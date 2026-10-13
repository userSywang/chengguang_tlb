
#include <lvgl/lvgl.h>
#include "../utils/misc_utils.h"
#include "../data_parse/img_res.h"

#include "./misc_draw.h"


#define TAG  "mscDrw"


void draw_txt_img_group_cb(lv_event_t * evt, misc_draw_mult_items_t * pDrawItems)
{
	lv_obj_t * obj = lv_event_get_target(evt);
	lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(evt);

	lv_area_t widget_coords;
	lv_obj_get_coords(obj, &widget_coords);
	lv_draw_label_dsc_t label_draw_dsc;
	lv_draw_label_dsc_init(&label_draw_dsc);
	lv_color_t colorTxt = LV_COLOR_MAKE(0, 0, 0);  // LV_COLOR_MAKE(0, 0xe0, 0);
	label_draw_dsc.color = colorTxt;
	label_draw_dsc.align = LV_TEXT_ALIGN_LEFT;
	LV_ASSERT(pDrawItems != NULL);

	int16_t scroll_y = lv_obj_get_scroll_y(obj);
	int16_t itemNum = pDrawItems->m_num;
	for (int8_t sn = 0; sn < itemNum; sn++) {
		misc_draw_item_t  * drawItem = pDrawItems->pstDrawItems + sn;
		if (0 == drawItem->m_type) { // is text type.
			label_draw_dsc.ofs_x = drawItem->m_locX;
			label_draw_dsc.ofs_y = drawItem->m_locY - scroll_y; // 
			lv_draw_label(draw_ctx, &label_draw_dsc, &widget_coords, drawItem->unn.m_pText, NULL);
		}
		else { // is img type. 
			lv_area_t icn_coords = widget_coords;
			lv_draw_img_dsc_t dsc;
			lv_draw_img_dsc_init(&dsc);

			const lv_img_dsc_t	* pImgDsc = zb_img_res_load_one(NULL, drawItem->unn.m_imgHash, 
					IMG_FREE_TYPE_AUTO, NULL);
			LV_ASSERT(pImgDsc != NULL);
			dsc.opa = LV_OPA_100;
			dsc.blend_mode = LV_BLEND_MODE_NORMAL;	
			icn_coords.x1 = drawItem->m_locX;
			icn_coords.y1 = drawItem->m_locY - scroll_y;
			icn_coords.x2 = icn_coords.x1 + pImgDsc->header.w - 1;
			icn_coords.y2 = icn_coords.y1 + pImgDsc->header.h - 1;
			lv_draw_img(draw_ctx, &dsc, &icn_coords,  pImgDsc); 
		}
	}
	return;
}



