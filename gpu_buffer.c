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

#include "gpu_buffer.h"
#include "gpu_assert.h"
#include "gpu_log.h"
#include "gpu_utils.h"
#include <stdlib.h>
#include <string.h>

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

struct gpu_buffer_s* gpu_buffer_alloc(enum gpu_color_format_e format, uint32_t width, uint32_t height, uint32_t stride, uint32_t align)
{
    GPU_ASSERT(width > 0);
    GPU_ASSERT(height > 0);
    GPU_ASSERT(stride > 0);

    struct gpu_buffer_s* buffer = calloc(1, sizeof(struct gpu_buffer_s));
    GPU_ASSERT_NULL(buffer);

    buffer->format = format;
    buffer->width = width;
    buffer->height = height;
    buffer->stride = stride;

    buffer->data_unaligned = calloc(1, stride * height + align);
    GPU_ASSERT_NULL(buffer->data_unaligned);
    buffer->data = (void*)GPU_ALIGN_UP(buffer->data_unaligned, align);

    GPU_LOG_INFO("Allocated buffer %p, format %d, size W%dxH%d, stride %d, data %p",
        buffer, format, width, height, stride, buffer->data);

    return buffer;
}

void gpu_buffer_free(struct gpu_buffer_s* buffer)
{
    GPU_ASSERT_NULL(buffer);
    GPU_ASSERT_NULL(buffer->data);
    GPU_ASSERT_NULL(buffer->data_unaligned);

    GPU_LOG_INFO("Freed buffer %p, format %d, size W%dxH%d, stride %d, data %p",
        buffer, buffer->format, buffer->width, buffer->height, buffer->stride, buffer->data);
    free(buffer->data_unaligned);

    memset(buffer, 0, sizeof(struct gpu_buffer_s));
    free(buffer);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
