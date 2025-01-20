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

#ifndef GPU_COLOR_H
#define GPU_COLOR_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef enum gpu_color_format_e {
    GPU_COLOR_FORMAT_UNKNOWN,
    GPU_COLOR_FORMAT_BGR565,
    GPU_COLOR_FORMAT_BGR888,
    GPU_COLOR_FORMAT_BGRA8888,
    GPU_COLOR_FORMAT_BGRX8888,
    GPU_COLOR_FORMAT_BGRA5658,
    GPU_COLOR_FORMAT_INDEX_8,
} gpu_color_format_t;

#pragma pack(1)

typedef union gpu_color_bgra8888_u {
    struct
    {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    } ch;
    uint32_t full;
} gpu_color32_t, gpu_color_bgra8888_t;

typedef union gpu_color_bgr888_u {
    struct
    {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
    } ch;
    uint8_t full[3];
} gpu_color24_t, gpu_color_bgr888_t;

typedef union gpu_color_bgr565_u {
    struct
    {
        uint16_t blue : 5;
        uint16_t green : 6;
        uint16_t red : 5;
    } ch;
    uint16_t full;
} gpu_color16_t, gpu_color_bgr565_t;

typedef struct gpu_color_bgra5658_u {
    struct
    {
        uint16_t blue : 5;
        uint16_t green : 6;
        uint16_t red : 5;
        uint16_t alpha : 8;
    } ch;
    uint8_t full[3];
} gpu_color16_alpha_t, gpu_color_bgra5658_t;

#pragma pack()

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Get the number of bits per pixel for a given color format
 * @param format The color format to get the bits per pixel for
 * @return The number of bits per pixel for the given color format
 */
uint32_t gpu_color_format_get_bpp(gpu_color_format_t format);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GPU_COLOR_H*/
