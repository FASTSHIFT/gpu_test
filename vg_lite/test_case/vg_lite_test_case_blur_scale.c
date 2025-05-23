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

#include "../resource/image_cogwheel_index8.h"
#include "../vg_lite_test_context.h"
#include "../vg_lite_test_utils.h"

/*********************
 *      DEFINES
 *********************/

#define BLUR_SCALE 0.3f

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
    vg_lite_test_context_load_src_image(
        ctx,
        imgae_cogwheel_index8_map,
        IMAGE_COGWHEEL_INDEX8_WIDTH,
        IMAGE_COGWHEEL_INDEX8_HEIGHT,
        IMAGE_COGWHEEL_INDEX8_FORMAT,
        IMAGE_COGWHEEL_INDEX8_STRIDE);

    vg_lite_buffer_t temp_buffer;
    struct gpu_buffer_s* temp_gpu_buf = vg_lite_test_buffer_alloc(
        &temp_buffer,
        IMAGE_COGWHEEL_INDEX8_WIDTH * BLUR_SCALE,
        IMAGE_COGWHEEL_INDEX8_HEIGHT * BLUR_SCALE,
        VG_LITE_BGRA8888,
        VG_LITE_TEST_STRIDE_AUTO);

    vg_lite_test_context_set_user_data(ctx, temp_gpu_buf);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_draw(struct vg_lite_test_context_s* ctx)
{
    vg_lite_buffer_t* image = vg_lite_test_context_get_src_buffer(ctx);

    struct gpu_buffer_s* temp_gpu_buf = vg_lite_test_context_get_user_data(ctx);
    vg_lite_buffer_t temp_buffer;
    vg_lite_test_gpu_buffer_to_vg_buffer(&temp_buffer, temp_gpu_buf);

    vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);
    vg_lite_scale(BLUR_SCALE, BLUR_SCALE, &matrix);

    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_set_CLUT(
        sizeof(imgae_cogwheel_index8_color_table) / sizeof(vg_lite_uint32_t),
        (vg_lite_uint32_t*)imgae_cogwheel_index8_color_table));

    /* Blit to temp buffer */
    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_blit(
            &temp_buffer,
            image,
            &matrix,
            VG_LITE_BLEND_SRC_OVER,
            0,
            VG_LITE_FILTER_BI_LINEAR));
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_finish());

    vg_lite_identity(&matrix);
    vg_lite_scale(1.0f / BLUR_SCALE, 1.0f / BLUR_SCALE, &matrix);

    /* Blit the temp buffer to target buffer */
    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_blit(
            vg_lite_test_context_get_target_buffer(ctx),
            &temp_buffer,
            &matrix,
            VG_LITE_BLEND_SRC_OVER,
            0,
            VG_LITE_FILTER_BI_LINEAR));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    struct gpu_buffer_s* temp_gpu_buf = vg_lite_test_context_get_user_data(ctx);
    if (temp_gpu_buf) {
        gpu_buffer_free(temp_gpu_buf);
    }
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(blur_scale, NONE, "Use scale to simulate blur effect");
