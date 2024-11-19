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
#include "vg_lite_test_context.h"
#include "vg_lite_test_utils.h"
#include <string.h>

/*********************
 *      DEFINES
 *********************/

#define gcFEATURE_BIT_VG_NONE -1

/**********************
 *      TYPEDEFS
 **********************/
typedef vg_lite_error_t (*vg_lite_test_func_t)(
    struct gpu_test_context_s* ctx);

struct vg_lite_test_item_s {
    const char* name;
    vg_lite_test_func_t func;
    int feature;
};

/* Import testcase entry function */

#define ITEM_DEF(NAME, FEATURE) \
    vg_lite_error_t vg_lite_test_##NAME(struct gpu_test_context_s* ctx);
#include "vg_lite_test_case.inc"
#undef ITEM_DEF

/* Generate testcase group */

#define ITEM_DEF(NAME, FEATURE) \
    { #NAME, vg_lite_test_##NAME, gcFEATURE_BIT_VG_##FEATURE },
static const struct vg_lite_test_item_s g_vg_lite_test_group[] = {
#include "vg_lite_test_case.inc"
};
#undef ITEM_DEF

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

int vg_lite_test(struct gpu_test_context_s* ctx)
{
#ifdef VG_LITE_TEST_CUSTOM_INIT
    GPU_LOG_INFO("Initializing GPU");
    extern void gpu_init(void);
    static bool is_init = false;
    if (!is_init) {
        gpu_init();
        is_init = true;
    }
#endif

    vg_lite_test_dump_info();

    struct vg_lite_test_ctx_s vg_ctx = { 0 };
    ctx->user_data = &vg_ctx;

#ifdef VG_LITE_TEST_CUSTOM_DEINIT
    /* Deinitialize the GPU */
    extern void gpu_deinit(void);
    GPU_LOG_INFO("Deinitializing GPU");
    gpu_deinit();
#endif

    ctx->user_data = NULL;
    GPU_LOG_INFO("GPU test finish");
    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
