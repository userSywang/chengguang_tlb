
#ifndef _IMG_STD_RES_H_
#define _IMG_STD_RES_H_

#include "stdio.h"

#include <lvgl/src/draw/lv_img_buf.h>

//------------------------------------------------


/** (从文件里)加载独立 jpg/png 图片资源 
 * @return  >= 0 means success 
 */
extern lv_img_dsc_t * zb_stdImgRes_load(const char * pcImgPath, int * pErrCode);

/** 释放独立的图片资源 
 */
extern void zb_stdImgRes_unload(lv_img_dsc_t * imgDsc);



//------------------------------------------------

int zb_stdImgPool_init(void);

void zb_stdImgPool_deinit(void);

/**
 * (从原图片文件或 cache 里)读取一张图的数据到内存 img_dsc struct, 并自动加到缓存里; 
 * @param pcImgPath 路径文件路径(必须有值)
 * @param piHash  图片的哈希值变量的地址(非必须,有它可减轻 load_one 内部计算的工作量)
 * @return note 0  means success. 
 */
lv_img_dsc_t * zb_stdImgPool_load_one(const char * pcImgPath, uint32_t * piHash, int * pErrNo);



#endif

