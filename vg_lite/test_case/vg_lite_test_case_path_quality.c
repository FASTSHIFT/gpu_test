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

/* clang-format off */

/* '0' */
static const int16_t glphy_u0030_path_data[] = {
    VLC_OP_MOVE, 2662.00, 164.00, 
    VLC_OP_QUAD, 1606.00, 164.00, 1032.00, -651.00, 
    VLC_OP_QUAD, 459.00, -1466.00, 459.00, -3039.00, 
    VLC_OP_QUAD, 459.00, -4620.00, 1032.00, -5439.00, 
    VLC_OP_QUAD, 1606.00, -6259.00, 2662.00, -6259.00, 
    VLC_OP_QUAD, 3727.00, -6259.00, 4300.00, -5439.00, 
    VLC_OP_QUAD, 4874.00, -4620.00, 4874.00, -3039.00, 
    VLC_OP_QUAD, 4874.00, -1466.00, 4300.00, -651.00, 
    VLC_OP_QUAD, 3727.00, 164.00, 2662.00, 164.00, 
    VLC_OP_MOVE, 2662.00, -737.00, 
    VLC_OP_QUAD, 3244.00, -737.00, 3563.00, -1323.00, 
    VLC_OP_QUAD, 3883.00, -1909.00, 3883.00, -3039.00, 
    VLC_OP_QUAD, 3883.00, -4178.00, 3567.00, -4763.00, 
    VLC_OP_QUAD, 3252.00, -5349.00, 2662.00, -5349.00, 
    VLC_OP_QUAD, 2081.00, -5349.00, 1765.00, -4763.00, 
    VLC_OP_QUAD, 1450.00, -4178.00, 1450.00, -3039.00, 
    VLC_OP_QUAD, 1450.00, -1909.00, 1765.00, -1323.00, 
    VLC_OP_QUAD, 2081.00, -737.00, 2662.00, -737.00, 
    VLC_OP_END, 
};

/* clang-format on */

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
