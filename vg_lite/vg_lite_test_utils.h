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

#ifndef VG_LITE_TEST_UTILS_H
#define VG_LITE_TEST_UTILS_H

/*********************
 *      INCLUDES
 *********************/

#include "../gpu_log.h"
#include <vg_lite.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/

#define VG_LITE_TEST_CHECK_ERROR(func)                                  \
    do {                                                                \
        vg_lite_error_t error = func;                                   \
        if (error != VG_LITE_SUCCESS) {                                 \
            GPU_LOG_ERROR("Execute '" #func "' error: %d", (int)error); \
            vg_lite_test_error_dump_info(error);                        \
            goto error_handler;                                         \
        }                                                               \
    } while (0)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Dump the information of the VG Lite library.
 */
void vg_lite_test_dump_info(void);

/**
 * @brief Get the error string.
 * @param error The error code.
 * @return The error string.
 */
const char* vg_lite_test_error_string(vg_lite_error_t error);

/**
 * @brief Dump error information.
 * @param error The error code.
 */
void vg_lite_test_error_dump_info(vg_lite_error_t error);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*VG_LITE_TEST_UTILS_H*/
