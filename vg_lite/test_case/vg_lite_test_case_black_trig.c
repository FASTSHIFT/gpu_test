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
#include "../../gpu_cache.h"
#include "../vg_lite_test_context.h"
#include "../vg_lite_test_utils.h"
#include "../vg_lite_test_path.h"

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
    vg_lite_translate(109, 311, &matrix);

    vg_lite_buffer_t* target_buffer = vg_lite_test_context_get_target_buffer(ctx);
    vg_lite_path_t* vg_path = vg_lite_test_path_get_path(vg_lite_test_context_get_path(ctx));

    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_draw_pattern(
            target_buffer,
            vg_path,
            VG_LITE_FILL_NON_ZERO,
            &matrix,
            image,
            &matrix,
            VG_LITE_BLEND_SRC_OVER,
            VG_LITE_PATTERN_COLOR,
            0,
            0,
            VG_LITE_FILTER_BI_LINEAR));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_setup(struct vg_lite_test_context_s* ctx)
{
    vg_lite_buffer_t* image = vg_lite_test_context_alloc_src_buffer(ctx, 90, 92, VG_LITE_BGRA8888, VG_LITE_TEST_STRIDE_AUTO);
    gpu_cache_flush(image->memory, image->stride * image->height);
    vg_lite_rectangle_t rec = {0, 2, 90, 90};
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_clear(image, &rec, 0xFF0000FF));
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_finish());

    vg_lite_test_path_t* path = vg_lite_test_context_init_path(ctx, VG_LITE_FP32);
    vg_lite_test_path_set_bounding_box(path, -1, -1, 90, 90);
    vg_lite_test_path_move_to(path, 45.00, 0.00);

    vg_lite_test_path_cubic_to(path, 52.90, 0.00, 60.66, 2.08, 67.50, 6.03);
    vg_lite_test_path_line_to(path, 63.50, 12.96);
    vg_lite_test_path_cubic_to(path, 57.88, 9.71, 51.49, 8.00, 45.00, 8.00);
    vg_lite_test_path_line_to(path, 45.00, 0.00);

    vg_lite_test_path_close(path);
    vg_lite_test_path_end(path);


    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_draw(struct vg_lite_test_context_s* ctx)
{
    vg_lite_buffer_t* image = vg_lite_test_context_get_src_buffer(ctx);

    draw_image(ctx,  image, 0, 0, 0);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(black_trig, NONE, "Test FILTER_BI_LINEAR edge antialiasing.");
