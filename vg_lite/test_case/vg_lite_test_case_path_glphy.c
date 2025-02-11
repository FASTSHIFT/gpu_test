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

/* '龍' */
static const int16_t glphy_u9f8d_path_data[] = {
    VLC_OP_MOVE, 492.00, -6169.00, 
    VLC_OP_LINE, 3973.00, -6169.00, 
    VLC_OP_LINE, 3973.00, -5636.00, 
    VLC_OP_LINE, 492.00, -5636.00, 
    VLC_OP_LINE, 492.00, -6169.00, 
    VLC_OP_MOVE, 344.00, -4506.00, 
    VLC_OP_LINE, 4121.00, -4506.00, 
    VLC_OP_LINE, 4121.00, -3973.00, 
    VLC_OP_LINE, 344.00, -3973.00, 
    VLC_OP_LINE, 344.00, -4506.00, 
    VLC_OP_MOVE, 1040.00, -2441.00, 
    VLC_OP_LINE, 3441.00, -2441.00, 
    VLC_OP_LINE, 3441.00, -2007.00, 
    VLC_OP_LINE, 1040.00, -2007.00, 
    VLC_OP_LINE, 1040.00, -2441.00, 
    VLC_OP_MOVE, 1040.00, -1409.00, 
    VLC_OP_LINE, 3441.00, -1409.00, 
    VLC_OP_LINE, 3441.00, -975.00, 
    VLC_OP_LINE, 1040.00, -975.00, 
    VLC_OP_LINE, 1040.00, -1409.00, 
    VLC_OP_MOVE, 1032.00, -5448.00, 
    VLC_OP_LINE, 1532.00, -5562.00, 
    VLC_OP_QUAD, 1630.00, -5333.00, 1724.00, -5046.00, 
    VLC_OP_QUAD, 1819.00, -4760.00, 1851.00, -4555.00, 
    VLC_OP_LINE, 1327.00, -4415.00, 
    VLC_OP_QUAD, 1303.00, -4628.00, 1216.00, -4915.00, 
    VLC_OP_QUAD, 1130.00, -5202.00, 1032.00, -5448.00, 
    VLC_OP_MOVE, 2966.00, -5562.00, 
    VLC_OP_LINE, 3523.00, -5431.00, 
    VLC_OP_QUAD, 3383.00, -5112.00, 3240.00, -4792.00, 
    VLC_OP_QUAD, 3097.00, -4473.00, 2966.00, -4235.00, 
    VLC_OP_LINE, 2499.00, -4375.00, 
    VLC_OP_QUAD, 2580.00, -4547.00, 2670.00, -4755.00, 
    VLC_OP_QUAD, 2761.00, -4964.00, 2838.00, -5177.00, 
    VLC_OP_QUAD, 2916.00, -5390.00, 2966.00, -5562.00, 
    VLC_OP_MOVE, 795.00, -3490.00, 
    VLC_OP_LINE, 3473.00, -3490.00, 
    VLC_OP_LINE, 3473.00, -3023.00, 
    VLC_OP_LINE, 1376.00, -3023.00, 
    VLC_OP_LINE, 1376.00, 655.00, 
    VLC_OP_LINE, 795.00, 655.00, 
    VLC_OP_LINE, 795.00, -3490.00, 
    VLC_OP_MOVE, 3138.00, -3490.00, 
    VLC_OP_LINE, 3744.00, -3490.00, 
    VLC_OP_LINE, 3744.00, -66.00, 
    VLC_OP_QUAD, 3744.00, 156.00, 3686.00, 287.00, 
    VLC_OP_QUAD, 3629.00, 418.00, 3473.00, 492.00, 
    VLC_OP_QUAD, 3310.00, 565.00, 3060.00, 581.00, 
    VLC_OP_QUAD, 2810.00, 598.00, 2466.00, 598.00, 
    VLC_OP_QUAD, 2441.00, 467.00, 2379.00, 311.00, 
    VLC_OP_QUAD, 2318.00, 156.00, 2261.00, 41.00, 
    VLC_OP_QUAD, 2523.00, 49.00, 2728.00, 49.00, 
    VLC_OP_QUAD, 2933.00, 49.00, 3006.00, 41.00, 
    VLC_OP_QUAD, 3080.00, 33.00, 3109.00, 8.00, 
    VLC_OP_QUAD, 3138.00, -16.00, 3138.00, -82.00, 
    VLC_OP_LINE, 3138.00, -3490.00, 
    VLC_OP_MOVE, 4776.00, -6234.00, 
    VLC_OP_LINE, 7766.00, -6234.00, 
    VLC_OP_LINE, 7766.00, -5718.00, 
    VLC_OP_LINE, 4776.00, -5718.00, 
    VLC_OP_LINE, 4776.00, -6234.00, 
    VLC_OP_MOVE, 4776.00, -2834.00, 
    VLC_OP_LINE, 7438.00, -2834.00, 
    VLC_OP_LINE, 7438.00, -2425.00, 
    VLC_OP_LINE, 4776.00, -2425.00, 
    VLC_OP_LINE, 4776.00, -2834.00, 
    VLC_OP_MOVE, 4801.00, -1925.00, 
    VLC_OP_LINE, 7397.00, -1925.00, 
    VLC_OP_LINE, 7397.00, -1516.00, 
    VLC_OP_LINE, 4801.00, -1516.00, 
    VLC_OP_LINE, 4801.00, -1925.00, 
    VLC_OP_MOVE, 4751.00, -991.00, 
    VLC_OP_LINE, 7479.00, -991.00, 
    VLC_OP_LINE, 7479.00, -590.00, 
    VLC_OP_LINE, 4751.00, -590.00, 
    VLC_OP_LINE, 4751.00, -991.00, 
    VLC_OP_MOVE, 4530.00, -6881.00, 
    VLC_OP_LINE, 5128.00, -6881.00, 
    VLC_OP_LINE, 5128.00, -4751.00, 
    VLC_OP_LINE, 4530.00, -4751.00, 
    VLC_OP_LINE, 4530.00, -6881.00, 
    VLC_OP_MOVE, 4530.00, -5054.00, 
    VLC_OP_LINE, 7447.00, -5054.00, 
    VLC_OP_LINE, 7447.00, -3342.00, 
    VLC_OP_LINE, 4530.00, -3342.00, 
    VLC_OP_LINE, 4530.00, -3826.00, 
    VLC_OP_LINE, 6849.00, -3826.00, 
    VLC_OP_LINE, 6849.00, -4563.00, 
    VLC_OP_LINE, 4530.00, -4563.00, 
    VLC_OP_LINE, 4530.00, -5054.00, 
    VLC_OP_MOVE, 4530.00, -3555.00, 
    VLC_OP_LINE, 5128.00, -3555.00, 
    VLC_OP_LINE, 5128.00, -238.00, 
    VLC_OP_QUAD, 5128.00, -33.00, 5210.00, 28.00, 
    VLC_OP_QUAD, 5292.00, 90.00, 5595.00, 90.00, 
    VLC_OP_QUAD, 5661.00, 90.00, 5853.00, 90.00, 
    VLC_OP_QUAD, 6046.00, 90.00, 6275.00, 90.00, 
    VLC_OP_QUAD, 6504.00, 90.00, 6709.00, 90.00, 
    VLC_OP_QUAD, 6914.00, 90.00, 7004.00, 90.00, 
    VLC_OP_QUAD, 7152.00, 90.00, 7225.00, 37.00, 
    VLC_OP_QUAD, 7299.00, -16.00, 7332.00, -159.00, 
    VLC_OP_QUAD, 7365.00, -303.00, 7381.00, -582.00, 
    VLC_OP_QUAD, 7479.00, -516.00, 7639.00, -454.00, 
    VLC_OP_QUAD, 7799.00, -393.00, 7922.00, -360.00, 
    VLC_OP_QUAD, 7889.00, 8.00, 7803.00, 221.00, 
    VLC_OP_QUAD, 7717.00, 434.00, 7541.00, 520.00, 
    VLC_OP_QUAD, 7365.00, 606.00, 7045.00, 606.00, 
    VLC_OP_QUAD, 6996.00, 606.00, 6840.00, 606.00, 
    VLC_OP_QUAD, 6685.00, 606.00, 6488.00, 606.00, 
    VLC_OP_QUAD, 6291.00, 606.00, 6090.00, 606.00, 
    VLC_OP_QUAD, 5890.00, 606.00, 5738.00, 606.00, 
    VLC_OP_QUAD, 5587.00, 606.00, 5530.00, 606.00, 
    VLC_OP_QUAD, 5145.00, 606.00, 4927.00, 536.00, 
    VLC_OP_QUAD, 4710.00, 467.00, 4620.00, 286.00, 
    VLC_OP_QUAD, 4530.00, 106.00, 4530.00, -238.00, 
    VLC_OP_LINE, 4530.00, -3555.00, 
    VLC_OP_MOVE, 1810.00, -6783.00, 
    VLC_OP_LINE, 2335.00, -6922.00, 
    VLC_OP_QUAD, 2441.00, -6701.00, 2535.00, -6439.00, 
    VLC_OP_QUAD, 2630.00, -6177.00, 2662.00, -5997.00, 
    VLC_OP_LINE, 2105.00, -5841.00, 
    VLC_OP_QUAD, 2081.00, -6029.00, 1995.00, -6299.00, 
    VLC_OP_QUAD, 1909.00, -6570.00, 1810.00, -6783.00, 
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
        sizeof(glphy_u9f8d_path_data),
        (void*)glphy_u9f8d_path_data, -10000, -10000, 10000, 10000));

    vg_lite_matrix_t matrix;
    vg_lite_test_context_get_transform(ctx, &matrix);
    vg_lite_translate(0, 50, &matrix);
    vg_lite_scale(0.005, 0.005, &matrix);

    vg_lite_buffer_t* target_buffer = vg_lite_test_context_get_target_buffer(ctx);

    for (int i = 0; i < 5; i++) {
        vg_lite_translate(10000, 0, &matrix);
        VG_LITE_TEST_CHECK_ERROR_RETURN(
            vg_lite_draw(
                target_buffer,
                &path,
                VG_LITE_FILL_NON_ZERO,
                &matrix,
                VG_LITE_BLEND_SRC_OVER,
                0xFF0000FF));
    }

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(path_glphy, NONE, "Draw 5x '龍' (size 40x40) glphy paths");
