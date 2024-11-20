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

#ifndef VG_LITE_TEST_CONTEXT_H
#define VG_LITE_TEST_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <vg_lite.h>

/*********************
 *      DEFINES
 *********************/

#define gcFEATURE_BIT_VG_NONE -1

#define VG_LITE_TEST_CASE_ITEM_DEF(NAME, FEATURE)                \
    struct vg_lite_test_item_s vg_lite_test_case_item_##NAME = { \
        .name = #NAME,                                           \
        .feature = gcFEATURE_BIT_VG_##FEATURE,                   \
        .on_setup = on_setup,                                    \
        .on_teardown = on_teardown,                              \
    }

/**********************
 *      TYPEDEFS
 **********************/

struct gpu_test_context_s;

struct vg_lite_test_context_s {
    struct gpu_test_context_s* gpu_ctx;
    vg_lite_buffer_t target_buffer;
    vg_lite_buffer_t src_buffer;
    uint32_t prepare_tick;
    uint32_t finish_tick;
    char remark_text[128];
};

typedef vg_lite_error_t (*vg_lite_test_func_t)(struct vg_lite_test_context_s* ctx);

struct vg_lite_test_item_s {
    const char* name;
    int feature;
    vg_lite_test_func_t on_setup;
    vg_lite_test_func_t on_teardown;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Initialize the test context
 * @param ctx The test context to initialize
 */
void vg_lite_test_context_setup(struct vg_lite_test_context_s* ctx);

/**
 * @brief Run a test case
 * @param ctx The test context to use
 */
void vg_lite_test_context_teardown(struct vg_lite_test_context_s* ctx);

/**
 * @brief Reset the test context
 * @param ctx The test context to reset
 * @note This function should be called after each test case to ensure a clean state for the next test case.
 */
void vg_lite_test_context_reset(struct vg_lite_test_context_s* ctx);

/**
 * @brief Record the test context
 * @param ctx The test context to record
 * @param testcase_str The name of the test case
 * @param error The error code of the test case
 */
void vg_lite_test_context_record(struct vg_lite_test_context_s* ctx, const char *testcase_str, vg_lite_error_t error);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*VG_LITE_TEST_CONTEXT_H*/
