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

#include "../gpu_context.h"
#include "../gpu_log.h"
#include "../gpu_recorder.h"
#include "../gpu_screenshot.h"
#include "../gpu_tick.h"
#include "vg_lite_test_context.h"
#include "vg_lite_test_utils.h"
#include <string.h>

/*********************
 *      DEFINES
 *********************/

/* Enable VG-Lite custom external 'gpu_init()' function */
#ifndef VG_LITE_TEST_USE_GPU_INIT
#define VG_LITE_TEST_USE_GPU_INIT 1

#ifndef VG_LITE_TEST_USE_GPU_INIT_ONCE
#define VG_LITE_TEST_USE_GPU_INIT_ONCE 0
#endif

#endif /* VG_LITE_TEST_USE_GPU_INIT */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void vg_lite_test_run_group(struct gpu_test_context_s* ctx);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int vg_lite_test_run(struct gpu_test_context_s* ctx)
{
#if VG_LITE_TEST_USE_GPU_INIT
#if VG_LITE_TEST_USE_GPU_INIT_ONCE
    static bool is_initialized = false;
    if (!is_initialized) {
#endif
        /* Initialize the GPU */
        extern void gpu_init(void);
        GPU_LOG_INFO("Initializing GPU");
        gpu_init();
#if VG_LITE_TEST_USE_GPU_INIT_ONCE
        is_initialized = true;
    }
#endif
#endif

    vg_lite_test_dump_info();
    vg_lite_test_run_group(ctx);

#if VG_LITE_TEST_USE_GPU_INIT && !VG_LITE_TEST_USE_GPU_INIT_ONCE
    /* Deinitialize the GPU */
    extern void gpu_deinit(void);
    GPU_LOG_INFO("Deinitializing GPU");
    gpu_deinit();
#endif

    GPU_LOG_INFO("GPU test finish");
    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void vg_lite_test_run_item(struct vg_lite_test_context_s* ctx, const struct vg_lite_test_item_s* item)
{
    if (item->feature != gcFEATURE_BIT_VG_NONE && !vg_lite_query_feature(item->feature)) {
        return;
    }

    GPU_LOG_INFO("Running test case: %s", item->name);

    vg_lite_test_context_reset(ctx);

    uint32_t start_tick = gpu_tick_get();
    vg_lite_error_t error = item->on_setup(ctx);
    ctx->prepare_tick = gpu_tick_elaps(start_tick);

    if (error == VG_LITE_SUCCESS) {
        start_tick = gpu_tick_get();
        error = vg_lite_finish();
        ctx->render_tick = gpu_tick_elaps(start_tick);
    }

    if (item->on_teardown) {
        item->on_teardown(ctx);
    }

    if (error == VG_LITE_SUCCESS) {
        GPU_LOG_INFO("Test case %s PASS", item->name);
    }

    vg_lite_test_context_record(ctx, item->name, error);

    if (ctx->gpu_ctx->param.screenshot_en) {
        struct gpu_buffer_s screenshot_buffer;
        vg_lite_test_vg_buffer_to_gpu_buffer(&screenshot_buffer, &ctx->target_buffer);
        gpu_screenshot(ctx->gpu_ctx->param.output_dir, item->name, &screenshot_buffer);
    }
}

static void vg_lite_test_run_group(struct gpu_test_context_s* ctx)
{
    /* Import testcase entry */

#define ITEM_DEF(NAME) extern struct vg_lite_test_item_s vg_lite_test_case_item_##NAME;
#include "vg_lite_test_case.inc"
#undef ITEM_DEF

#define ITEM_DEF(NAME) &vg_lite_test_case_item_##NAME,
    static const struct vg_lite_test_item_s* vg_lite_test_group[] = {
#include "vg_lite_test_case.inc"
    };
#undef ITEM_DEF

    struct vg_lite_test_context_s vg_lite_ctx = { 0 };
    vg_lite_ctx.gpu_ctx = ctx;
    vg_lite_test_context_setup(&vg_lite_ctx);

    for (size_t i = 0; i < sizeof(vg_lite_test_group) / sizeof(vg_lite_test_group[0]); i++) {
        vg_lite_test_run_item(&vg_lite_ctx, vg_lite_test_group[i]);
    }

    vg_lite_test_context_teardown(&vg_lite_ctx);
}
