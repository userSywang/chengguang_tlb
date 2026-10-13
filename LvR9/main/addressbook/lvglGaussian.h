#ifndef __LVGL_GAUSSIAN_H__
#define __LVGL_GAUSSIAN_H__

#include <stdint.h>
#include <lvgl/lvgl.h>


typedef enum {
    AVERAGEBLUR = 0,  // 均值模糊，累加优化
    AVERAGEBLUR1 = 1, // 均值模糊，无优化
    AVERAGEBLUR2 = 2, // 均值模糊，行模糊+列模糊
    GAUSSIANBLUR = 3  // 高斯模糊
} lv_blur_type_t;

typedef struct {
    lv_obj_t* canvas;
    lv_coord_t x;
    lv_coord_t y;
    int width;
    int height;
    int r; // 模糊半径，越大越模糊
    int border_width;
    lv_color_t border_color; // 边框颜色
    int border_radius; // 边框半径
    lv_blur_type_t blur_type;
} lv_draw_gaussian_blur_dsc_t;

typedef struct {
    uint8_t* content;
    int image_size;
    int image_stride;
    int image_width;
    int image_height;
    int x;
    int y;
    int r;
    int border_width;
    lv_color_t border_color;
    int border_radius;
    lv_obj_t* canvas_;
} lvgl_gaussian_t;

// 辅助函数
int is_circle(int x, int y, int circle_x, int circle_y, int width, int height, int r);
int is_square(int x, int y, int circle_x, int circle_y, int width, int height, int border);

// 核心功能函数
void lv_draw_gaussian_blur(lv_draw_gaussian_blur_dsc_t* blur_dsc);

// 模糊算法实现
void average_blur(lvgl_gaussian_t* g, int radius);
void average_blur1(lvgl_gaussian_t* g, int radius);
void average_blur2(lvgl_gaussian_t* g, int radius);
void gaussian_blur(lvgl_gaussian_t* g, float* weights, int radius);
float* create_gaussian_kernel(int radius);

#endif // __LVGL_GAUSSIAN_H__
