/*
 * MIT License
 * Copyright (c) 2023 - 2025 _VIFEXTech
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

#include "gpu_cache.h"
#include <stdint.h>

#ifdef GPU_CACHE_INCLUDE_H
#include GPU_CACHE_INCLUDE_H
#endif

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

void gpu_cache_invalidate(void* addr, size_t len)
{
#ifdef GPU_CACHE_INVALIDATE_FUNC
    GPU_CACHE_INVALIDATE_FUNC((uintptr_t)addr, (uintptr_t)addr + len);
#endif
}

void gpu_cache_clean(void* addr, size_t len)
{
#ifdef GPU_CACHE_CLEAN_FUNC
    GPU_CACHE_CLEAN_FUNC((uintptr_t)addr, (uintptr_t)addr + len);
#endif
}

void gpu_cache_flush(void* addr, size_t len)
{
#ifdef GPU_CACHE_FLUSH_FUNC
    GPU_CACHE_FLUSH_FUNC((uintptr_t)addr, (uintptr_t)addr + len);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
