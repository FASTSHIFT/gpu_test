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

static vg_lite_error_t clear_buffer(vg_lite_buffer_t* image, int x, vg_lite_color_t color)
{
    vg_lite_rectangle_t rect = {
        .x = x,
        .y = 0,
        .width = image->width / 4,
        .height = image->height,
    };

    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_clear(image, &rect, color));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_setup(struct vg_lite_test_context_s* ctx)
{
    vg_lite_buffer_t* target_buffer = vg_lite_test_context_get_target_buffer(ctx);
    vg_lite_buffer_t* image = vg_lite_test_context_alloc_src_buffer(
        ctx,
        target_buffer->width,
        target_buffer->height,
        target_buffer->format,
        target_buffer->stride);

    /* Draw 4 rectangles on the image */
    VG_LITE_TEST_CHECK_ERROR_RETURN(clear_buffer(image, 0, 0xFFFFFFFF));
    VG_LITE_TEST_CHECK_ERROR_RETURN(clear_buffer(image, image->width * 0.25f, 0xFFF00000));
    VG_LITE_TEST_CHECK_ERROR_RETURN(clear_buffer(image, image->width * 0.50f, 0xFF00FF00));
    VG_LITE_TEST_CHECK_ERROR_RETURN(clear_buffer(image, image->width * 0.75f, 0xFF0000FF));
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_finish());

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_draw(struct vg_lite_test_context_s* ctx)
{
    vg_lite_buffer_t* target_buffer = vg_lite_test_context_get_target_buffer(ctx);
    vg_lite_buffer_t* image = vg_lite_test_context_get_src_buffer(ctx);

    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);

    /* Set the image as tiled */
    image->tiled = VG_LITE_TILED;

    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_blit(target_buffer, image, &matrix, VG_LITE_BLEND_SRC_OVER, 0, VG_LITE_FILTER_BI_LINEAR));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(image_full_screen_tiled, NONE, "Draw full screen tiled image");
