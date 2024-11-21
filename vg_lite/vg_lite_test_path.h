/*
 * MIT License
 * Copyright (c) 2023 - 2024 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef VG_LITE_TEST_PATH_H
#define VG_LITE_TEST_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "vg_lite_test_utils.h"
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/

typedef struct vg_lite_test_path_s vg_lite_test_path_t;

typedef void (*vg_lite_test_path_iter_cb_t)(void* user_data, uint8_t op_code, const float* data, uint32_t len);

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

vg_lite_test_path_t* vg_lite_test_path_create(vg_lite_format_t data_format);

void vg_lite_test_path_destroy(vg_lite_test_path_t* path);

void vg_lite_test_path_reset(vg_lite_test_path_t* path, vg_lite_format_t data_format);

void vg_lite_test_path_set_bounding_box(vg_lite_test_path_t* path,
    float min_x, float min_y,
    float max_x, float max_y);

void vg_lite_test_path_get_bounding_box(vg_lite_test_path_t* path,
    float* min_x, float* min_y,
    float* max_x, float* max_y);

bool vg_lite_test_path_update_bounding_box(vg_lite_test_path_t* path);

void vg_lite_test_path_set_transform(vg_lite_test_path_t* path, const vg_lite_matrix_t* matrix);

void vg_lite_test_path_set_quality(vg_lite_test_path_t* path, vg_lite_quality_t quality);

vg_lite_path_t* vg_lite_test_path_get_path(vg_lite_test_path_t* path);

void vg_lite_test_path_move_to(vg_lite_test_path_t* path,
    float x, float y);

void vg_lite_test_path_line_to(vg_lite_test_path_t* path,
    float x, float y);

void vg_lite_test_path_quad_to(vg_lite_test_path_t* path,
    float cx, float cy,
    float x, float y);

void vg_lite_test_path_cubic_to(vg_lite_test_path_t* path,
    float cx1, float cy1,
    float cx2, float cy2,
    float x, float y);

void vg_lite_test_path_close(vg_lite_test_path_t* path);

void vg_lite_test_path_end(vg_lite_test_path_t* path);

void vg_lite_test_path_append_rect(vg_lite_test_path_t* path,
    float x, float y,
    float w, float h,
    float r);

void vg_lite_test_path_append_circle(vg_lite_test_path_t* path,
    float cx, float cy,
    float rx, float ry);

void vg_lite_test_path_append_arc_right_angle(vg_lite_test_path_t* path,
    float start_x, float start_y,
    float center_x, float center_y,
    float end_x, float end_y);

void vg_lite_test_path_append_arc(vg_lite_test_path_t* path,
    float cx, float cy,
    float radius,
    float start_angle,
    float sweep,
    bool pie);

void vg_lite_test_path_append_path(vg_lite_test_path_t* dest, const vg_lite_test_path_t* src);

uint8_t vg_lite_test_vlc_op_arg_len(uint8_t vlc_op);

uint8_t vg_lite_test_path_format_len(vg_lite_format_t format);

void vg_lite_test_path_for_each_data(const vg_lite_path_t* path, vg_lite_test_path_iter_cb_t cb, void* user_data);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*VG_LITE_TEST_PATH_H*/
