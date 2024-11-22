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

#ifndef GPU_CONTEXT_H
#define GPU_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "gpu_buffer.h"
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/

#define GPU_TEST_DESIGN_WIDTH 480
#define GPU_TEST_DESIGN_HEIGHT 480

/**********************
 *      TYPEDEFS
 **********************/

struct gpu_recorder_s;

enum gpu_test_mode_e {
    GPU_TEST_MODE_DEFAULT = 0,
    GPU_TEST_MODE_STRESS,
};

struct gpu_test_param_s {
    enum gpu_test_mode_e mode;
    const char* output_dir;
    const char* testcase_name;
    int img_width;
    int img_height;
    int run_loop_count;
    bool screenshot_en;
    int cpu_freq;
};

struct gpu_test_context_s {
    struct gpu_recorder_s* recorder;
    struct gpu_test_param_s param;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Initialize the GPU test context.
 * @param ctx The GPU test context to initialize.
 */
void gpu_test_context_setup(struct gpu_test_context_s* ctx);

/**
 * @brief Deinitialize the GPU test context.
 * @param ctx The GPU test context to deinitialize.
 */
void gpu_test_context_teardown(struct gpu_test_context_s* ctx);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GPU_CONTEXT_H*/
