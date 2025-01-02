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
    vg_lite_test_path_t* path = vg_lite_test_context_init_path(ctx, VG_LITE_FP32);
    vg_lite_test_path_set_bounding_box(path, -240, -240, 240, 240);
    vg_lite_test_path_append_circle(path, 0, 0, 50, 50);
    vg_lite_test_path_end(path);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_draw(struct vg_lite_test_context_s* ctx)
{
    const vg_lite_blend_t blend_modes[] = {
        VG_LITE_BLEND_NONE,
        VG_LITE_BLEND_SRC_OVER,
        VG_LITE_BLEND_DST_OVER,
        VG_LITE_BLEND_SRC_IN,
        VG_LITE_BLEND_DST_IN,
        VG_LITE_BLEND_MULTIPLY,
        VG_LITE_BLEND_SCREEN,
        VG_LITE_BLEND_ADDITIVE,
        VG_LITE_BLEND_SUBTRACT,
    };

    vg_lite_buffer_t* target_buffer = vg_lite_test_context_get_target_buffer(ctx);
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_clear(target_buffer, NULL, 0xAAAAAAAA));

    const vg_lite_color_t colors[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFF00 };
    const int color_len = sizeof(colors) / sizeof(vg_lite_color_t);

    vg_lite_path_t* vg_path = vg_lite_test_path_get_path(vg_lite_test_context_get_path(ctx));

    for (int i = 0; i < sizeof(blend_modes) / sizeof(vg_lite_blend_t); i++) {

        const vg_lite_blend_t blend_mode = blend_modes[i];
        vg_lite_matrix_t matrix;
        vg_lite_test_context_get_transform(ctx, &matrix);
        vg_lite_translate(100 + 80 * (i % color_len), 100 + 80 * (i / color_len), &matrix);

        VG_LITE_TEST_CHECK_ERROR_RETURN(
            vg_lite_draw(
                target_buffer,
                vg_path,
                VG_LITE_FILL_NON_ZERO,
                &matrix,
                blend_mode,
                colors[i % color_len]));
    }

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(blend_mode_base, NONE, "Test blend mode base.");
