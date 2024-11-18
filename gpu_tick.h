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

#ifndef GPU_TICK_H
#define GPU_TICK_H

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

typedef uint32_t (*gpu_tick_get_cb_t)(void);

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Set the callback function to get the GPU tick
 * @param cb The callback function to get the GPU tick
 */
void gpu_tick_set_cb(gpu_tick_get_cb_t cb);

/**
 * @brief Get the GPU tick
 * @return The GPU tick
 */
uint32_t gpu_tick_get(void);

/**
 * @brief Get the elapsed time between two GPU ticks
 * @param prev_tick The previous GPU tick
 * @return The elapsed time in GPU ticks
 */
uint32_t gpu_tick_elaps(uint32_t prev_tick);

/**
 * @brief Delay for a specified number of milliseconds
 * @param ms The number of milliseconds to delay
 */
void gpu_delay(uint32_t ms);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GPU_TICK_H*/
