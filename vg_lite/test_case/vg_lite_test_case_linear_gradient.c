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

#include "../../gpu_math.h"
#include "../vg_lite_test_context.h"
#include "../vg_lite_test_utils.h"
#include "../vg_lite_test_path.h"
#include <string.h>

/*********************
 *      DEFINES
 *********************/

#ifdef SQUARE
#undef SQUARE
#endif
#define SQUARE(x) ((x) * (x))

#ifndef M_PI
#define M_PI 3.1415926f
#endif

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

static void grad_point_to_matrix(vg_lite_matrix_t* grad_matrix, float x1, float y1, float x2, float y2)
{
    vg_lite_translate(x1, y1, grad_matrix);

    float angle = atan2f(y2 - y1, x2 - x1) * 180.0f / (float)M_PI;
    vg_lite_rotate(angle, grad_matrix);
    float length = MATH_SQRTF(SQUARE(x2 - x1) + SQUARE(y2 - y1));
    vg_lite_scale(length / 256.0f, 1, grad_matrix);
}

static vg_lite_error_t on_setup(struct vg_lite_test_context_s* ctx)
{
    static vg_lite_linear_gradient_t linear_grad;
    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_init_grad(&linear_grad));

    vg_lite_test_context_set_user_data(ctx, &linear_grad);

    vg_lite_uint32_t colors[] = {
        0xFFFF0000,
        0xFF00FF00,
        0xFF0000FF,
    };

    vg_lite_uint32_t stops[] = {
        64,
        128,
        192,
    };

    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_set_grad(&linear_grad, 3, colors, stops));

    VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_update_grad(&linear_grad));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_draw(struct vg_lite_test_context_s* ctx)
{
    vg_lite_linear_gradient_t* linear_grad = vg_lite_test_context_get_user_data(ctx);

    vg_lite_matrix_t* grad_mat_p = vg_lite_get_grad_matrix(linear_grad);
    vg_lite_identity(grad_mat_p);
    grad_point_to_matrix(grad_mat_p, 0, 0, 100, 100);

    struct vg_lite_test_path_s* path = vg_lite_test_context_init_path(ctx, VG_LITE_FP32);
    vg_lite_test_path_set_bounding_box(path, 0, 0, 100, 100);
    vg_lite_test_path_append_rect(path, 0, 0, 100, 100, 10);
    vg_lite_test_path_end(path);

    vg_lite_matrix_t matrix;
    vg_lite_test_context_get_transform(ctx, &matrix);

    VG_LITE_TEST_CHECK_ERROR_RETURN(
        vg_lite_draw_grad(
            vg_lite_test_context_get_target_buffer(ctx),
            vg_lite_test_path_get_path(path),
            VG_LITE_FILL_EVEN_ODD,
            &matrix,
            linear_grad,
            VG_LITE_BLEND_SRC_OVER));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t on_teardown(struct vg_lite_test_context_s* ctx)
{
    vg_lite_linear_gradient_t* linear_grad = vg_lite_test_context_get_user_data(ctx);
    if (linear_grad) {
        VG_LITE_TEST_CHECK_ERROR_RETURN(vg_lite_clear_grad(linear_grad));
    }
    return VG_LITE_SUCCESS;
}

VG_LITE_TEST_CASE_ITEM_DEF(linear_gradient, NONE, "Draw a RGB linear gradient");
