#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lvgl/lvgl.h>
#include"lvglGaussian.h"


int is_circle(int x, int y, int circle_x, int circle_y, int width, int height, int r) {
    if (r <= 0) return 1;

    int dx = x - circle_x;
    int dy = y - circle_y;

    // 左上角
    if (dx < r && dy < r) {
        return (dx * dx + dy * dy) <= (r * r);
    }
    // 右上角
    else if (dx > width - r && dy < r) {
        dx = width - dx;
        return (dx * dx + dy * dy) <= (r * r);
    }
    // 左下角
    else if (dx < r && dy > height - r) {
        dy = height - dy;
        return (dx * dx + dy * dy) <= (r * r);
    }
    // 右下角
    else if (dx > width - r && dy > height - r) {
        dx = width - dx;
        dy = height - dy;
        return (dx * dx + dy * dy) <= (r * r);
    }
    return 1;
}

int is_square(int x, int y, int circle_x, int circle_y, int width, int height, int border) {
    return (x >= circle_x + border && x < circle_x + width - border &&
        y >= circle_y + border && y < circle_y + height - border);
}

float* create_gaussian_kernel(int radius) {
    float sigma = radius / 2.0f;
    float weights_sum = 0;
    int kernel_size = radius * 2 + 1;
    float* weights = (float*)malloc(kernel_size * kernel_size * sizeof(float));

    for (int u = -radius; u <= radius; u++) {
        for (int v = -radius; v <= radius; v++) {
            int idx = (v + radius) * kernel_size + (u + radius);
            weights[idx] = 1.0f / (6.28f * sigma * sigma) * expf((-u * u - v * v) / (2 * sigma * sigma));
            weights_sum += weights[idx];
        }
    }

    // 归一化
    for (int i = 0; i < kernel_size * kernel_size; i++) {
        weights[i] /= weights_sum;
    }

    return weights;
}

void average_blur(lvgl_gaussian_t* g, int radius) {
    if (radius <= 0 || radius >= g->image_height) {
        return;
    }

    uint8_t* temp_buf = (uint8_t*)malloc(g->image_size);
    if (!temp_buf) return;

    int pixel_size = 3; // RGB
    int y_stride = (2 * radius + 1) * g->image_stride;

    // 垂直方向模糊
    for (int x = 0; x < g->image_width; x++) {
        uint8_t* src_col = &g->content[x * pixel_size];
        uint8_t* dst_col = &temp_buf[x * pixel_size];

        int b_sum = 0, g_sum = 0, r_sum = 0;
        int cnt = 0;

        // 初始化垂直窗口
        for (int y = 0; y <= radius; y++) {
            b_sum += src_col[0];
            g_sum += src_col[1];
            r_sum += src_col[2];
            src_col += g->image_stride;
            cnt++;
        }

        dst_col[0] = b_sum / cnt;
        dst_col[1] = g_sum / cnt;
        dst_col[2] = r_sum / cnt;
        dst_col += g->image_stride;

        // 滑动窗口
        for (int y = 1; y < g->image_height; y++) {
            if (y + radius < g->image_height) {
                b_sum += src_col[0];
                g_sum += src_col[1];
                r_sum += src_col[2];
                src_col += g->image_stride;
                cnt++;
            }

            if (y - radius > 0) {
                b_sum -= src_col[-y_stride];
                g_sum -= src_col[-y_stride + 1];
                r_sum -= src_col[-y_stride + 2];
                cnt--;
            }

            dst_col[0] = b_sum / cnt;
            dst_col[1] = g_sum / cnt;
            dst_col[2] = r_sum / cnt;
            dst_col += g->image_stride;
        }
    }

    // 将结果复制回原缓冲区
    memcpy(g->content, temp_buf, g->image_size);
    free(temp_buf);
}

void lv_draw_gaussian_blur(lv_draw_gaussian_blur_dsc_t* blur_dsc) {
    if (LV_COLOR_DEPTH != 16) {
        printf("LV_COLOR_DEPTH ERROR %d\n", LV_COLOR_DEPTH);
        return;
    }

    lvgl_gaussian_t g = { 0 };
    int color_bit = (LV_COLOR_DEPTH / 8) - 1;
    g.image_size = blur_dsc->width * blur_dsc->height * color_bit;
    g.image_width = blur_dsc->width;
    g.image_height = blur_dsc->height;
    g.image_stride = blur_dsc->width * color_bit;
    g.canvas_ = blur_dsc->canvas;

    g.x = blur_dsc->x;
    g.y = blur_dsc->y;
    g.r = blur_dsc->r;
    g.border_width = blur_dsc->border_width;
    g.border_color = blur_dsc->border_color;
    g.border_radius = blur_dsc->border_radius;

    g.content = (uint8_t*)malloc(g.image_size);
    if (!g.content) {
        printf("Memory allocation failed\n");
        return;
    }

    // 读取像素数据
    int f = 0;
    for (int y = g.y; y < g.y + g.image_height; y++) {
        for (int x = g.x; x < g.x + g.image_width; x++) {
            lv_color_t px = lv_canvas_get_px(g.canvas_, x, y);
            g.content[f] = px.ch.blue;
            g.content[f + 1] = px.ch.green;
            g.content[f + 2] = px.ch.red;
            f += color_bit;
        }
    }

    // 应用模糊算法
    switch (blur_dsc->blur_type) {
    case AVERAGEBLUR:
        average_blur(&g, g.r);
        break;
    case AVERAGEBLUR1:
      //  average_blur1(&g, g.r);
        break;
    case AVERAGEBLUR2:
      //  average_blur2(&g, g.r);
        break;
    case GAUSSIANBLUR: {
     //   float* kernel = create_gaussian_kernel(g.r);
   //     gaussian_blur(&g, kernel, g.r);
    //    free(kernel);
        break;
    }
    }

    // 写回像素数据
    f = 0;
    for (int y = g.y; y < g.y + g.image_height; y++) {
        for (int x = g.x; x < g.x + g.image_width; x++) {
            if (is_circle(x, y, g.x, g.y, g.image_width, g.image_height, g.border_radius)) {
                if (is_circle(x, y, g.x + g.border_width, g.y + g.border_width,
                    g.image_width - 2 * g.border_width, g.image_height - 2 * g.border_width, g.border_radius) &&
                    is_square(x, y, g.x, g.y, g.image_width, g.image_height, g.border_width)) {
                    lv_color_t px;
                    px.ch.blue = g.content[f];
                    px.ch.green = g.content[f + 1];
                    px.ch.red = g.content[f + 2];
                    lv_canvas_set_px_color(g.canvas_, x, y, px);
                }
                else {
                    lv_canvas_set_px_color(g.canvas_, x, y, g.border_color);
                }
            }
            f += color_bit;
        }
    }

    free(g.content);
}
