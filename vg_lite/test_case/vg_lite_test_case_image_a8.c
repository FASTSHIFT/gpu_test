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

/*********************
 *      INCLUDES
 *********************/

#include "../vg_lite_test_context.h"
#include "../vg_lite_test_path.h"
#include "../vg_lite_test_utils.h"
#include <string.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static vg_lite_error_t draw_image(struct vg_lite_test_context_s* ctx, vg_lite_buffer_t* image, int x_ofs, int y_ofs, vg_lite_color_t bg_color)
{
    vg_lite_matrix_t matrix;
    vg_lite_test_context_get_transform(ctx, &matrix);
    vg_lite_translate(x_ofs, y_ofs, &matrix);

    vg_lite_rectangle_t rect = {
        .x = 0,
        .y = 0,
        .width = image->width * 3,
        .height = image->height,
    };

    vg_lite_test_transform_retangle(&rect, &matrix);

    vg_lite_rectangle_t rect_image = {
        .x = image->width / 4,
        .y = 0,
        .width = image->width / 2,
        .height = image->height,
    };

    vg_lite_buffer_t* target_buffer = vg_lite_test_context_get_target_buffer(ctx);
    vg_lite_path_t* vg_path = vg_lite_test_path_get_path(vg_lite_test_context_get_path(ctx));

    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_clear(target_buffer, &rect, bg_color));

    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_blit(
            target_buffer,
            image,
            &matrix,
            VG_LITE_BLEND_SRC_OVER,
            0xFF0000FF,
            VG_LITE_FILTER_BI_LINEAR));

    vg_lite_translate(image->width, 0, &matrix);
    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_blit_rect(
            target_buffer,
            image,
            &rect_image,
            &matrix,
            VG_LITE_BLEND_SRC_OVER,
            0xFF0000FF,
            VG_LITE_FILTER_BI_LINEAR));

    vg_lite_translate(image->width, 0, &matrix);
    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_draw_pattern(
            target_buffer,
            vg_path,
            VG_LITE_FILL_EVEN_ODD,
            &matrix,
            image,
            &matrix,
            VG_LITE_BLEND_SRC_OVER,
            VG_LITE_PATTERN_COLOR,
            0,
            0xFF0000FF,
            VG_LITE_FILTER_BI_LINEAR));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_setup(struct vg_lite_test_context_s* ctx)
{
    vg_lite_buffer_t* image = vg_lite_test_context_get_src_buffer(ctx);
    vg_lite_test_buffer_alloc(image, 64, 64, VG_LITE_A8, VG_LITE_TEST_STRIDE_AUTO);

    image->image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    uint8_t* dst = image->memory;

    /* Fill gradient image with alpha values */
    for (int y = 0; y < image->height; y++) {
        uint8_t alpha = y * 0xFF / image->height;
        memset(dst, alpha, image->width);
        dst += image->stride;
    }

    vg_lite_test_path_t* path = vg_lite_test_context_init_path(ctx, VG_LITE_FP32);
    vg_lite_test_path_set_bounding_box(path, 0, 0, 64, 64);
    vg_lite_test_path_append_circle(path, 32, 32, 32, 32);
    vg_lite_test_path_end(path);

    VG_LITE_TEST_CHECK_ERROR_RETURN(draw_image(ctx, image, 0, 0, 0xFF000000));
    VG_LITE_TEST_CHECK_ERROR_RETURN(draw_image(ctx, image, 0, 100, 0xFF1F1F1F));
    VG_LITE_TEST_CHECK_ERROR_RETURN(draw_image(ctx, image, 0, 200, 0xFFFFFFFF));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(image_a8, NONE, "Draw A8 image using different drawing methods");
