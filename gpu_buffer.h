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

#ifndef GPU_BUFFER_H
#define GPU_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "gpu_color.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct gpu_buffer_s {
    enum gpu_color_format_e format;
    int width;
    int height;
    int stride;
    void* data;
    void* data_unaligned;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Allocate a new GPU buffer with the given format, width, height, and stride.
 * The buffer will be initialized with zeros.
 * @param format The color format of the buffer.
 * @param width The width of the buffer in pixels.
 * @param height The height of the buffer in pixels.
 * @param stride The stride of the buffer in bytes.
 * @return A pointer to the new GPU buffer, or NULL if there was an error.
 */
struct gpu_buffer_s* gpu_buffer_alloc(enum gpu_color_format_e format, int width, int height, int stride);

/**
 * Free a GPU buffer.
 * @param buffer The GPU buffer to free.
 */
void gpu_buffer_free(struct gpu_buffer_s* buffer);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GPU_TEMPL_H*/
