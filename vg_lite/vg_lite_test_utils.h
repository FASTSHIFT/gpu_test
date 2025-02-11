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

#include "../gpu_buffer.h"
#include "../gpu_log.h"
#include <vg_lite.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/

#define VG_LITE_TEST_CHECK_ERROR(func)                                \
    do {                                                              \
        vg_lite_error_t err = func;                                   \
        if (err != VG_LITE_SUCCESS) {                                 \
            GPU_LOG_ERROR("Execute '" #func "' error: %d", (int)err); \
            vg_lite_test_error_dump_info(err);                        \
        }                                                             \
    } while (0)

#define VG_LITE_TEST_CHECK_ERROR_RETURN(func)                         \
    do {                                                              \
        vg_lite_error_t err = func;                                   \
        if (err != VG_LITE_SUCCESS) {                                 \
            GPU_LOG_ERROR("Execute '" #func "' error: %d", (int)err); \
            vg_lite_test_error_dump_info(err);                        \
            return err;                                               \
        }                                                             \
    } while (0)

#define VG_LITE_TEST_STRIDE_AUTO 0

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
 * @brief Get the feature string.
 * @param feature The feature code.
 * @return The feature string.
 */
const char* vg_lite_test_feature_string(vg_lite_feature_t feature);

/**
 * @brief Dump error information.
 * @param error The error code.
 */
void vg_lite_test_error_dump_info(vg_lite_error_t error);

/**
 * @brief Allocate a GPU buffer for VG Lite.
 * @param buffer The VG Lite buffer to be allocated.
 * @param width The width of the buffer.
 * @param height The height of the buffer.
 * @param format The format of the buffer.
 * @param stride The stride of the buffer. If it is VG_LITE_TEST_STRIDE_AUTO, the stride will be calculated automatically.
 * @return The allocated GPU buffer.
 */
struct gpu_buffer_s* vg_lite_test_buffer_alloc(vg_lite_buffer_t* buffer, uint32_t width, uint32_t height, vg_lite_buffer_format_t format, uint32_t stride);

/**
 * @brief Convert a VG Lite buffer to a GPU buffer.
 * @param gpu_buffer The GPU buffer to be copied.
 * @param vg_buffer The VG Lite buffer to be copied.
 */
void vg_lite_test_vg_buffer_to_gpu_buffer(struct gpu_buffer_s* gpu_buffer, const vg_lite_buffer_t* vg_buffer);

/**
 * @brief Convert a GPU buffer to a VG Lite buffer.
 * @param vg_buffer The VG Lite buffer to be copied.
 * @param gpu_buffer The GPU buffer to be copied.
 */
void vg_lite_test_gpu_buffer_to_vg_buffer(vg_lite_buffer_t* vg_buffer, const struct gpu_buffer_s* gpu_buffer);

/**
 * @breif Convert a VG Lite buffer format to a string.
 * @param format The VG Lite buffer format.
 * @return The string of the VG Lite buffer format.
 */
const char* vg_lite_test_buffer_format_string(vg_lite_buffer_format_t format);

/**
 * @breif Flush the GPU command queue if GPU is idle.
 * @return VG_LITE_SUCCESS if the flush is successful, otherwise the error code.
 */
vg_lite_error_t vg_lite_test_idle_flush(void);

/**
 * @breif Transform a point by a matrix.
 * @param x The x coordinate of the point.
 * @param y The y coordinate of the point.
 * @param matrix The matrix to transform the point.
 * @note The transformed point will be stored in the x and y parameters.
 */
void vg_lite_test_transform_point(float* x, float* y, const vg_lite_matrix_t* matrix);

/**
 * @breif Transform a rectangle by a matrix.
 * @param rect The rectangle to be transformed.
 * @param matrix The matrix to transform the rectangle.
 * @note The transformed rectangle will be stored in the rect parameter.
 */
void vg_lite_test_transform_retangle(vg_lite_rectangle_t* rect, const vg_lite_matrix_t* matrix);

/**
 * @breif Fill a buffer with a gray gradient.
 * @param buffer The buffer to be filled.
 */
void vg_lite_test_fill_gray_gradient(vg_lite_buffer_t* buffer);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*VG_LITE_TEST_UTILS_H*/
