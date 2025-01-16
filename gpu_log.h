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

#ifndef GPU_LOG_H
#define GPU_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

#define GPU_LOG_DEBUG(format, ...)      \
    gpu_log_printf(GPU_LOG_LEVEL_DEBUG, \
        __func__,                       \
        format,                         \
        ##__VA_ARGS__)
#define GPU_LOG_INFO(format, ...)      \
    gpu_log_printf(GPU_LOG_LEVEL_INFO, \
        __func__,                      \
        format,                        \
        ##__VA_ARGS__)
#define GPU_LOG_WARN(format, ...)      \
    gpu_log_printf(GPU_LOG_LEVEL_WARN, \
        __func__,                      \
        format,                        \
        ##__VA_ARGS__)
#define GPU_LOG_ERROR(format, ...)      \
    gpu_log_printf(GPU_LOG_LEVEL_ERROR, \
        __func__,                       \
        format,                         \
        ##__VA_ARGS__)

/**********************
 *      TYPEDEFS
 **********************/

enum gpu_log_level_type_e {
    GPU_LOG_LEVEL_DEBUG,
    GPU_LOG_LEVEL_INFO,
    GPU_LOG_LEVEL_WARN,
    GPU_LOG_LEVEL_ERROR,
    _GPU_LOG_LEVEL_LAST
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Print log message
 * @param level Log level
 * @param func Function name
 * @param format Log message format
 * @param... Variable arguments
 */
void gpu_log_printf(enum gpu_log_level_type_e level, const char* func, const char* format, ...);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* GPU_LOG_H */
