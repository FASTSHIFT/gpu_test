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

#include "gpu_screenshot.h"
#include "gpu_assert.h"
#include "gpu_buffer.h"
#include "gpu_log.h"
#include <png.h>
#include <stdio.h>
#include <string.h>

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

int gpu_screenshot_save(const char* path, const struct gpu_buffer_s* buffer)
{
    GPU_ASSERT_NULL(path);
    GPU_ASSERT_NULL(buffer);

    GPU_LOG_INFO("Taking screenshot of '%s' ...", path);

    /* Construct the PNG image structure. */
    png_image image;
    memset(&image, 0, sizeof(image));

    image.version = PNG_IMAGE_VERSION;
    image.width = buffer->width;
    image.height = buffer->height;

    switch (buffer->format) {
    case GPU_COLOR_FORMAT_BGR888:
        image.format = PNG_FORMAT_BGR;
        break;

    case GPU_COLOR_FORMAT_BGRA8888:
    case GPU_COLOR_FORMAT_BGRX8888:
        image.format = PNG_FORMAT_BGRA;
        break;

    default:
        GPU_LOG_ERROR("Unsupported color format: %d", buffer->format);
        return -1;
    }

    /* Write the PNG image. */
    if (!png_image_write_to_file(&image, path, 0, buffer->data, buffer->stride, NULL)) {
        GPU_LOG_ERROR("Failed");
        return -1;
    }

    GPU_LOG_INFO("Successed");
    return 0;
}

struct gpu_buffer_s* gpu_screenshot_load(const char* path)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&image, path)) {
        GPU_LOG_WARN("Failed to read PNG image from %s", path);
        return NULL;
    }

    struct gpu_buffer_s* buffer = gpu_buffer_alloc(image.width, image.height, GPU_COLOR_FORMAT_BGRA8888, image.width * sizeof(uint32_t), 8);

    image.format = PNG_FORMAT_BGRA;

    if (!png_image_finish_read(&image, NULL, buffer->data, buffer->stride, NULL)) {
        GPU_LOG_WARN("Failed to finish reading PNG image from %s", path);
        gpu_buffer_free(buffer);
        return NULL;
    }

    png_image_free(&image);
    return buffer;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
