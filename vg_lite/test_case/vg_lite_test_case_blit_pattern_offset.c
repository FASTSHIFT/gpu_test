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

static vg_lite_error_t on_setup(struct vg_lite_test_context_s* ctx)
{
    vg_lite_test_context_alloc_src_buffer(ctx, 256, 50, VG_LITE_BGRA8888, VG_LITE_TEST_STRIDE_AUTO);
    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_draw(struct vg_lite_test_context_s* ctx)
{
    vg_lite_matrix_t matrix;
    vg_lite_test_context_get_transform(ctx, &matrix);
    vg_lite_translate(50, 0, &matrix);
    vg_lite_rotate(45, &matrix);

    struct vg_lite_test_path_s* path = vg_lite_test_context_init_path(ctx, VG_LITE_FP32);
    vg_lite_test_path_set_bounding_box(path, 0, 0, 256, 50);
    vg_lite_test_path_append_rect(path, 0, 0, 256, 50, 0);
    vg_lite_test_path_end(path);

    vg_lite_buffer_t* image = vg_lite_test_context_get_src_buffer(ctx);
    vg_lite_buffer_t* target_buffer = vg_lite_test_context_get_target_buffer(ctx);

    /* Test vg_lite_draw_pattern */
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_clear(image, NULL, 0xFF0000FF));
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_finish());

    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_draw_pattern(
            target_buffer,
            vg_lite_test_path_get_path(path),
            VG_LITE_FILL_EVEN_ODD,
            &matrix,
            image,
            &matrix,
            VG_LITE_BLEND_SRC_OVER,
            VG_LITE_PATTERN_COLOR,
            0,
            0,
            VG_LITE_FILTER_BI_LINEAR));
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_finish());

    /* Test vg_lite_blit */
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_clear(image, NULL, 0xFF00FF00));
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_finish());

    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_blit(
            target_buffer,
            image,
            &matrix,
            VG_LITE_BLEND_SRC_OVER,
            0,
            VG_LITE_FILTER_BI_LINEAR));
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_finish());

    /* Test vg_lite_blit_rect */
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_clear(image, NULL, 0xFFFF0000));
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_finish());

    vg_lite_rectangle_t rect = {
        .x = 0,
        .y = 0,
        .width = image->width,
        .height = image->height,
    };

    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_blit_rect(
            target_buffer,
            image,
            &rect,
            &matrix,
            VG_LITE_BLEND_SRC_OVER,
            0,
            VG_LITE_FILTER_BI_LINEAR));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(blit_pattern_offset, NONE, "Test whether blit and draw_pattern completely overlap");
