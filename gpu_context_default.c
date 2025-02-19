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

#ifndef GPU_TEST_CONTEXT_DEFAULT_DISABLE

#include "gpu_assert.h"
#include "gpu_context.h"
#include "gpu_fb.h"
#include "gpu_log.h"
#include <stddef.h>

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

bool gpu_test_context_setup(struct gpu_test_context_s* ctx)
{
    GPU_ASSERT_NULL(ctx);

    GPU_LOG_INFO("Initializing GPU");
    extern void gpu_init(void);
    gpu_init();

    if (ctx->param.fbdev_path) {
        ctx->fb = gpu_fb_create(ctx->param.fbdev_path);

        if (!ctx->fb) {
            return false;
        }

        gpu_fb_get_buffer(ctx->fb, &ctx->target_buffer);
    }

    return true;
}

void gpu_test_context_teardown(struct gpu_test_context_s* ctx)
{
    GPU_ASSERT_NULL(ctx);

    if (ctx->fb) {
        gpu_fb_destroy(ctx->fb);
        ctx->fb = NULL;
    }

    extern void gpu_deinit(void);
    GPU_LOG_INFO("Deinitializing GPU");
    gpu_deinit();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /* GPU_TEST_CONTEXT_DEFAULT_DISABLE */
