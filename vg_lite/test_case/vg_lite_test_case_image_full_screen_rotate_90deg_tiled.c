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

#include "../../gpu_utils.h"
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

static vg_lite_error_t on_setup(struct vg_lite_test_context_s* ctx)
{
    vg_lite_buffer_t* target_buffer = vg_lite_test_context_get_target_buffer(ctx);
    vg_lite_buffer_t* image = vg_lite_test_context_alloc_src_buffer(
        ctx,
        GPU_ALIGN_UP(target_buffer->width, 16),
        GPU_ALIGN_UP(target_buffer->height, 16),
        VG_LITE_BGRA8888,
        VG_LITE_TEST_STRIDE_AUTO);

    vg_lite_test_fill_gray_gradient(image);

    /* Set the image as tiled */
    image->tiled = VG_LITE_TILED;

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_draw(struct vg_lite_test_context_s* ctx)
{
    vg_lite_buffer_t* target_buffer = vg_lite_test_context_get_target_buffer(ctx);
    vg_lite_buffer_t* image = vg_lite_test_context_get_src_buffer(ctx);

    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);

    /* Rotate the image 90 degrees around the center of the image buffer */
    vg_lite_translate(image->width / 2, image->height / 2, &matrix);
    vg_lite_rotate(90, &matrix);
    vg_lite_translate(-image->width / 2, -image->height / 2, &matrix);

    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_blit(target_buffer, image, &matrix, VG_LITE_BLEND_SRC_OVER, 0, VG_LITE_FILTER_BI_LINEAR));
    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(image_full_screen_rotate_90deg_tiled, NONE, "Draw BGRA8888 tiled image on full screen rotated 90 degrees");
