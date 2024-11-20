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

#ifndef VG_LITE_TEST_MATH_H
#define VG_LITE_TEST_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <math.h>
#include <stdbool.h>
#include <float.h>

/*********************
 *      DEFINES
 *********************/

#define MATH_PI  3.14159265358979323846f
#define MATH_HALF_PI 1.57079632679489661923f
#define MATH_TWO_PI 6.28318530717958647692f
#define DEG_TO_RAD 0.017453292519943295769236907684886f
#define RAD_TO_DEG 57.295779513082320876798154814105f

#define MATH_TANF(x) tanf(x)
#define MATH_SINF(x) sinf(x)
#define MATH_COSF(x) cosf(x)
#define MATH_ASINF(x) asinf(x)
#define MATH_ACOSF(x) acosf(x)
#define MATH_FABSF(x) fabsf(x)
#define MATH_SQRTF(x) sqrtf(x)

#define MATH_RADIANS(deg) ((deg) * DEG_TO_RAD)
#define MATH_DEGREES(rad) ((rad) * RAD_TO_DEG)

#define MATH_MIN(a, b) ((a) < (b)? (a) : (b))
#define MATH_MAX(a, b) ((a) > (b)? (a) : (b))

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

static inline bool math_zero(float a)
{
    return (MATH_FABSF(a) < FLT_EPSILON);
}

static inline bool math_equal(float a, float b)
{
    return math_zero(a - b);
}

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*VG_LITE_TEST_MATH_H*/
