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

#include <stdbool.h>
#include <vg_lite.h>

/*********************
 *      DEFINES
 *********************/

#define gcFEATURE_BIT_VG_NONE -1

#define VG_LITE_TEST_CASE_ITEM_DEF(NAME, FEATURE, INSTRUCTIONS)  \
    struct vg_lite_test_item_s vg_lite_test_case_item_##NAME = { \
        .name = #NAME,                                           \
        .instructions = INSTRUCTIONS,                            \
        .feature = gcFEATURE_BIT_VG_##FEATURE,                   \
        .on_setup = on_setup,                                    \
        .on_draw = on_draw,                                      \
        .on_teardown = on_teardown,                              \
    }

/**********************
 *      TYPEDEFS
 **********************/

struct gpu_test_context_s;
struct vg_lite_test_path_s;
struct vg_lite_test_context_s;

typedef vg_lite_error_t (*vg_lite_test_func_t)(struct vg_lite_test_context_s* ctx);

struct vg_lite_test_item_s {
    const char* name;
    const char* instructions;
    int feature;
    vg_lite_test_func_t on_setup;
    vg_lite_test_func_t on_draw;
    vg_lite_test_func_t on_teardown;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Create a new test context
 * @param gpu_ctx The GPU test context to use
 * @return The new test context
 */
struct vg_lite_test_context_s* vg_lite_test_context_create(struct gpu_test_context_s* gpu_ctx);

/**
 * @brief Destroy a test context
 * @param ctx The test context to destroy
 */
void vg_lite_test_context_destroy(struct vg_lite_test_context_s* ctx);

/**
 * @brief Run a test case item
 * @param ctx The test context to use
 * @param item The test case item to run
 * @return True if the test case passed, false if it failed
 */
bool vg_lite_test_context_run_item(struct vg_lite_test_context_s* ctx, const struct vg_lite_test_item_s* item);

/**
 * @brief Get the target buffer for the test case
 * @param ctx The test context to use
 * @return The target buffer for the test case
 */
vg_lite_buffer_t* vg_lite_test_context_get_target_buffer(struct vg_lite_test_context_s* ctx);

/**
 * @brief Get the source buffer for the test case
 * @param ctx The test context to use
 * @return The source buffer for the test case
 */
vg_lite_buffer_t* vg_lite_test_context_get_src_buffer(struct vg_lite_test_context_s* ctx);

/**
 * @brief Set the transform for the test case
 * @param ctx The test context to use
 * @param matrix The transform matrix to set
 */
void vg_lite_test_context_set_transform(struct vg_lite_test_context_s* ctx, const vg_lite_matrix_t* matrix);

/**
 * @brief Get the transform for the test case
 * @param ctx The test context to use
 * @param matrix The transform matrix to get
 */
void vg_lite_test_context_get_transform(struct vg_lite_test_context_s* ctx, vg_lite_matrix_t* matrix);

/**
 * @brief Get the test path for the given format
 * @param format The format of the test path
 * @return The test path for the given format
 */
struct vg_lite_test_path_s* vg_lite_test_context_init_path(struct vg_lite_test_context_s* ctx, vg_lite_format_t format);

/**
 * @brief Get the test path for the given format
 * @param ctx The test context to use
 * @return The test path for the given format
 */
struct vg_lite_test_path_s* vg_lite_test_context_get_path(struct vg_lite_test_context_s* ctx);

/**
 * @brief Set the user data for the test context
 * @param ctx The test context to use
 * @param user_data The user data to set
 */
void vg_lite_test_context_set_user_data(struct vg_lite_test_context_s* ctx, void* user_data);

/**
 * @brief Get the user data for the test context
 * @param ctx The test context to use
 * @return The user data for the test context
 */
void* vg_lite_test_context_get_user_data(struct vg_lite_test_context_s* ctx);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*VG_LITE_TEST_CONTEXT_H*/
