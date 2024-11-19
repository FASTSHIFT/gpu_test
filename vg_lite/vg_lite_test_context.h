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

#ifndef VG_LITE_TEST_CONTEXT_H
#define VG_LITE_TEST_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <vg_lite.h>

/*********************
 *      DEFINES
 *********************/

#define gcFEATURE_BIT_VG_NONE -1

#define VG_LITE_TEST_CASE_ITEM_DEF(NAME, FEATURE)                \
    struct vg_lite_test_item_s vg_lite_test_case_item_##NAME = { \
        .name = #NAME,                                           \
        .feature = gcFEATURE_BIT_VG_##FEATURE,                   \
        .on_setup = on_setup,                                    \
        .on_teardown = on_teardown,                              \
    }

/**********************
 *      TYPEDEFS
 **********************/

struct vg_lite_test_context_s {
    vg_lite_buffer_t target_buffer;
};

typedef void (*vg_lite_test_func_t)(struct vg_lite_test_context_s* ctx);

struct vg_lite_test_item_s {
    const char* name;
    int feature;
    vg_lite_test_func_t on_setup;
    vg_lite_test_func_t on_teardown;
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*VG_LITE_TEST_CONTEXT_H*/
