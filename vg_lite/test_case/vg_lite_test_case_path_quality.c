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

#include "../resource/glphy_paths.h"
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
    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_draw(struct vg_lite_test_context_s* ctx)
{
    vg_lite_path_t path;
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_init_path(
        &path,
        VG_LITE_S16,
        VG_LITE_HIGH,
        sizeof(glphy_u0030_path_data),
        (void*)glphy_u0030_path_data, -10000, -10000, 10000, 10000));

    vg_lite_matrix_t matrix;
    vg_lite_test_context_get_transform(ctx, &matrix);
    vg_lite_translate(0, 50, &matrix);
    vg_lite_scale(0.005, 0.005, &matrix);

    vg_lite_buffer_t* target_buffer = vg_lite_test_context_get_target_buffer(ctx);

    const vg_lite_quality_t quality_settings[] = {
        VG_LITE_LOW,
        VG_LITE_MEDIUM,
        VG_LITE_HIGH,
    };

    for (int i = 0; i < sizeof(quality_settings) / sizeof(vg_lite_quality_t); i++) {

        /* Set the path quality */
        path.quality = quality_settings[i];

        vg_lite_translate(10000, 0, &matrix);

        VG_LITE_TEST_CHECK_ERROR_RETURN(
            vg_lite_draw(
                target_buffer,
                &path,
                VG_LITE_FILL_NON_ZERO,
                &matrix,
                VG_LITE_BLEND_SRC_OVER,
                0xFFFFFFFF));
    }

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(path_quality, NONE, "Draw '0' glyph with LOW/MEDIUM/HIGH path quality settings");
