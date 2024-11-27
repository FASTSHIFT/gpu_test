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
#include <string.h>

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
    vg_lite_color_ramp_t color_ramp[] = {
        { .stop = 0.25f, .red = 1, .green = 0, .blue = 0, .alpha = 1 },
        { .stop = 0.50f, .red = 0, .green = 1, .blue = 0, .alpha = 1 },
        { .stop = 0.75f, .red = 0, .green = 0, .blue = 1, .alpha = 1 }
    };

    const vg_lite_radial_gradient_parameter_t grad_param = {
        .cx = 50,
        .cy = 50,
        .r = 50,
        .fx = 50,
        .fy = 50,
    };

    static vg_lite_radial_gradient_t radial_grad;
    memset(&radial_grad, 0, sizeof(vg_lite_radial_gradient_t));

    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_set_radial_grad(
            &radial_grad,
            sizeof(color_ramp) / sizeof(vg_lite_color_ramp_t),
            color_ramp,
            grad_param,
            VG_LITE_GRADIENT_SPREAD_PAD,
            1));

    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_update_radial_grad(&radial_grad));

    vg_lite_test_context_set_user_data(ctx, &radial_grad);

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_draw(struct vg_lite_test_context_s* ctx)
{
    vg_lite_radial_gradient_t* radial_grad = vg_lite_test_context_get_user_data(ctx);

    vg_lite_matrix_t matrix;
    vg_lite_test_context_get_transform(ctx, &matrix);

    vg_lite_matrix_t* grad_mat_p = vg_lite_get_radial_grad_matrix(radial_grad);
    vg_lite_identity(grad_mat_p);

    struct vg_lite_test_path_s* path = vg_lite_test_context_init_path(ctx, VG_LITE_FP32);
    vg_lite_test_path_set_bounding_box(path, 0, 0, 200, 200);
    vg_lite_test_path_append_rect(path, 0, 0, 200, 100, 10);
    vg_lite_test_path_end(path);

    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_draw_radial_grad(
            vg_lite_test_context_get_target_buffer(ctx),
            vg_lite_test_path_get_path(path),
            VG_LITE_FILL_EVEN_ODD,
            &matrix,
            radial_grad,
            0,
            VG_LITE_BLEND_SRC_OVER,
            VG_LITE_FILTER_BI_LINEAR));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    vg_lite_radial_gradient_t* radial_grad = vg_lite_test_context_get_user_data(ctx);
    if (radial_grad) {
        VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_clear_radial_grad(radial_grad));
    }
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(radial_gradient, RADIAL_GRADIENT, "Draw a RGB radial gradient");