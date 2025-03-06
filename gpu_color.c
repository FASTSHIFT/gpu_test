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

#include "gpu_color.h"
#include "gpu_log.h"
#include <stdlib.h>

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

uint32_t gpu_color_format_get_bpp(gpu_color_format_t format)
{
    switch (format) {
    case GPU_COLOR_FORMAT_BGR565:
        return sizeof(gpu_color_bgr565_t) * 8;

    case GPU_COLOR_FORMAT_BGR888:
        return sizeof(gpu_color_bgr888_t) * 8;

    case GPU_COLOR_FORMAT_BGRA8888:
    case GPU_COLOR_FORMAT_BGRX8888:
        return sizeof(gpu_color_bgra8888_t) * 8;

    case GPU_COLOR_FORMAT_BGRA5658:
        return sizeof(gpu_color_bgra5658_t) * 8;

    case GPU_COLOR_FORMAT_INDEX_8:
        return sizeof(uint8_t) * 8;

    case GPU_COLOR_FORMAT_A4:
        return 4;

    case GPU_COLOR_FORMAT_A8:
        return 8;

    default:
        GPU_LOG_ERROR("Unsupported color format: %d", format);
        break;
    }

    return 0;
}

bool gpu_color_bgra8888_compare(gpu_color_bgra8888_t color1, gpu_color_bgra8888_t color2, int tolerance)
{
    if (abs(color1.ch.red - color2.ch.red) > tolerance) {
        return false;
    }

    if (abs(color1.ch.green - color2.ch.green) > tolerance) {
        return false;
    }

    if (abs(color1.ch.blue - color2.ch.blue) > tolerance) {
        return false;
    }

    /* Skip checking alpha channel */

    return true;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
