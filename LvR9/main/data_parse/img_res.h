
#ifndef _IMG_RES_H_
#define _IMG_RES_H_

#include "stdio.h"

#include "../include/img_a_tlb.h"

#include <lvgl/src/draw/lv_img_buf.h>

typedef struct {
    uint8_t  m_encParm;
    int16_t  m_imgTotal;
    int16_t  m_idxItemW;
    int      m_idxTblOffset;
    uint32_t  * m_pHashIdxTbl;
    FILE * m_fp;
} zb_img_file_res_t;

enum img_free_type_enum {
    IMG_FREE_TYPE_AUTO,   // 自动管理(按 cache)
    IMG_FREE_TYPE_MANUAL, // 调用者来显式地释放 
}; 

typedef struct {
    uint32_t crc;
    lv_img_dsc_t* img_dsc;
} zb_img_cover_res_t;

typedef enum img_free_type_enum img_free_type_e;


extern const lv_img_dsc_t  g_XXX_img_dsc_tbl[];

//------------------------------------------------


/** 打开 zb 图片资源文件.
 */
zb_img_file_res_t * zb_img_res_open(const char * pcPath);

/**  关闭 zb 图片资源文件 
 */
void zb_img_res_close(zb_img_file_res_t * img_res);


/** 读取(默认的)图片资源里的图片数 
 */
int16_t zb_img_res_get_img_total(void);


/**
 * (从 zb 图片资源文件, 按 hash)获取一张图片的描述. 
 * 说明: 尺寸过大(比如 长x宽 > 40KB)的图片，建议 img_free_type_e 使用 IMG_FREE_TYPE_MANUAL), 而小图片使用 IMG_FREE_TYPE_AUTO;
 */
lv_img_dsc_t * zb_img_res_load_one(zb_img_file_res_t * img_res, uint32_t imgHash, 
		img_free_type_e img_free_type, int * pErrNo);


/**
 * 释放单张图片资源 
 * Note: 只有 IMG_FREE_TYPE_MANUAL 类型的图片 才需要(也才能)调用该函数. 
 */
void zb_img_res_free_one(lv_img_dsc_t * pImgDsc);


//------------------------------------------------------



/**
* 從 album.mnu 中獲取封面圖片資源
*/
extern lv_img_dsc_t * zb_img_cover_load_one(const char * mnu_path, int* pErrNo);


/**
* 釋放所有的封面資源
*/
extern void zb_img_cover_unload();

/*
* 从单独的bin数据读取图片数据，
* 注意需要自己去释放资源
*/
extern lv_img_dsc_t* zb_load_image_from_file(char* pcPath);
#endif

