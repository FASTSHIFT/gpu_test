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
#include <stdlib.h>
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

struct vg_lite_test_iter_s {
    enum gpu_test_mode_e mode;
    const struct vg_lite_test_item_s* item;
    const struct vg_lite_test_item_s** group;
    int group_size;
    int name_to_index;
    int current_index;
    int current_loop_count;
    int total_loop_count;
};

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

static bool vg_lite_test_run_item(struct vg_lite_test_context_s* ctx, const struct vg_lite_test_item_s* item)
{
    if (item->feature != gcFEATURE_BIT_VG_NONE && !vg_lite_query_feature(item->feature)) {
        GPU_LOG_WARN("Skipping test case: %s (feature %s not supported)", item->name, vg_lite_test_feature_string(item->feature));
        vg_lite_test_context_record(ctx, item, VG_LITE_NOT_SUPPORT);
        return true;
    }

    GPU_LOG_INFO("Running test case: %s", item->name);

    uint32_t start_tick = gpu_tick_get();
    vg_lite_error_t error = item->on_setup(ctx);
    ctx->prepare_tick = gpu_tick_elaps(start_tick);

    if (error == VG_LITE_SUCCESS) {
        start_tick = gpu_tick_get();
        error = vg_lite_finish();
        ctx->finish_tick = gpu_tick_elaps(start_tick);
    }

    if (item->on_teardown) {
        item->on_teardown(ctx);
    }

    if (error == VG_LITE_SUCCESS) {
        GPU_LOG_INFO("Test case '%s' PASS", item->name);
    } else {
        GPU_LOG_ERROR("Test case '%s' FAILED: %d (%s)", item->name, error, vg_lite_test_error_string(error));
    }

    vg_lite_test_context_record(ctx, item, error);

    if (ctx->gpu_ctx->param.screenshot_en) {
        struct gpu_buffer_s screenshot_buffer;
        vg_lite_test_vg_buffer_to_gpu_buffer(&screenshot_buffer, &ctx->target_buffer);
        gpu_screenshot(ctx->gpu_ctx->param.output_dir, item->name, &screenshot_buffer);
    }

    vg_lite_test_context_cleanup(ctx);

    return error == VG_LITE_SUCCESS;
}

static int vg_lite_test_name_to_index(const struct vg_lite_test_item_s** group, int group_size, const char* name)
{
    if (!name) {
        return -1;
    }

    for (int i = 0; i < group_size; i++) {
        if (strcmp(group[i]->name, name) == 0) {
            return i;
        }
    }

    return -1;
}

static bool vg_lite_test_iter_next(struct vg_lite_test_iter_s* iter)
{
    iter->current_loop_count++;

    switch (iter->mode) {
    case GPU_TEST_MODE_DEFAULT: {
        /* Check if there is a specific test case to run */
        if (iter->name_to_index >= 0) {
            if (iter->current_loop_count == 1) {
                iter->item = iter->group[iter->name_to_index];
                return true;
            }

            return false;
        }

        if (iter->current_index >= iter->group_size) {
            return false;
        }

        iter->item = iter->group[iter->current_index++];
        return true;
    }

    case GPU_TEST_MODE_STRESS: {
        GPU_LOG_INFO("Test loop count: %d/%d", iter->current_loop_count, iter->total_loop_count);
        if (iter->current_loop_count >= iter->total_loop_count) {
            GPU_LOG_INFO("Test loop count reached, exit");
            return false;
        }

        iter->current_index = iter->name_to_index >= 0 ? iter->name_to_index : (rand() % iter->group_size);
        iter->item = iter->group[iter->current_index];
        return true;
    }

    default:
        GPU_LOG_ERROR("Unsupported test mode: %d", iter->mode);
        break;
    }

    return false;
}

static void vg_lite_test_run_group(struct gpu_test_context_s* ctx)
{
    struct vg_lite_test_context_s vg_lite_ctx = { 0 };
    vg_lite_ctx.gpu_ctx = ctx;
    vg_lite_test_context_setup(&vg_lite_ctx);

    /* Import testcase entry */

#define ITEM_DEF(NAME) extern struct vg_lite_test_item_s vg_lite_test_case_item_##NAME;
#include "vg_lite_test_case.inc"
#undef ITEM_DEF

#define ITEM_DEF(NAME) &vg_lite_test_case_item_##NAME,
    static const struct vg_lite_test_item_s* vg_lite_test_group[] = {
#include "vg_lite_test_case.inc"
    };
#undef ITEM_DEF

    struct vg_lite_test_iter_s iter = { 0 };
    iter.mode = ctx->param.mode;
    iter.group = vg_lite_test_group;
    iter.group_size = sizeof(vg_lite_test_group) / sizeof(vg_lite_test_group[0]);
    iter.name_to_index = vg_lite_test_name_to_index(iter.group, iter.group_size, ctx->param.testcase_name);
    iter.total_loop_count = ctx->param.run_loop_count;

    while (vg_lite_test_iter_next(&iter)) {
        if (!vg_lite_test_run_item(&vg_lite_ctx, iter.item)) {
            break;
        }
    }

    vg_lite_test_context_teardown(&vg_lite_ctx);
}
