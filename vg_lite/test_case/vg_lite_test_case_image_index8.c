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

static vg_lite_error_t draw_image(struct vg_lite_test_context_s* ctx, vg_lite_buffer_t* image, int x_ofs, int y_ofs, vg_lite_color_t mul_color)
{
    vg_lite_matrix_t matrix;
    vg_lite_test_context_get_transform(ctx, &matrix);
    vg_lite_translate(x_ofs, y_ofs, &matrix);

    vg_lite_rectangle_t rect_image = {
        .x = image->width / 4,
        .y = 0,
        .width = image->width / 2,
        .height = image->height,
    };

    vg_lite_buffer_t* target_buffer = vg_lite_test_context_get_target_buffer(ctx);
    vg_lite_path_t* vg_path = vg_lite_test_path_get_path(vg_lite_test_context_get_path(ctx));

    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_blit(
            target_buffer,
            image,
            &matrix,
            VG_LITE_BLEND_SRC_OVER,
            mul_color,
            VG_LITE_FILTER_BI_LINEAR));

    vg_lite_translate(image->width, 0, &matrix);
    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_blit_rect(
            target_buffer,
            image,
            &rect_image,
            &matrix,
            VG_LITE_BLEND_SRC_OVER,
            mul_color,
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
            mul_color,
            VG_LITE_FILTER_BI_LINEAR));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_setup(struct vg_lite_test_context_s* ctx)
{
    vg_lite_test_context_load_src_image(
        ctx,
        imgae_cogwheel_index8_map,
        IMAGE_COGWHEEL_INDEX8_WIDTH,
        IMAGE_COGWHEEL_INDEX8_HEIGHT,
        IMAGE_COGWHEEL_INDEX8_FORMAT,
        IMAGE_COGWHEEL_INDEX8_STRIDE);

    vg_lite_test_path_t* path = vg_lite_test_context_init_path(ctx, VG_LITE_FP32);
    vg_lite_test_path_set_bounding_box(path, 0, 0, 100, 100);
    vg_lite_test_path_append_rect(path, 10, 20, 80, 60, 20);
    vg_lite_test_path_end(path);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_draw(struct vg_lite_test_context_s* ctx)
{
    vg_lite_buffer_t* image = vg_lite_test_context_get_src_buffer(ctx);

    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_set_CLUT(
        sizeof(imgae_cogwheel_index8_color_table) / sizeof(vg_lite_uint32_t),
        (vg_lite_uint32_t*)imgae_cogwheel_index8_color_table));

    VG_LITE_TEST_CHECK_ERROR_RETURN(draw_image(ctx, image, 0, 0, 0));

    image->image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    VG_LITE_TEST_CHECK_ERROR_RETURN(draw_image(ctx, image, 0, 200, 0xFFFF0000));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(image_index8, IM_INDEX_FORMAT, "Draw an indexed image with 8-bit palette and using different drawing methods");
