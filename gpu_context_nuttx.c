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

#ifdef GPU_TEST_CONTEXT_NUTTX_ENABLE

/*********************
 *      INCLUDES
 *********************/

#include "gpu_context.h"
#include "gpu_log.h"
#include "gpu_tick.h"
#include <inttypes.h>
#include <nuttx/arch.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/

#define TICK_TO_USEC(tick) ((tick) / g_cpu_freq)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static uint32_t tick_get_cb(void);

/**********************
 *  STATIC VARIABLES
 **********************/

static uint32_t g_cpu_freq = 200; /* default CPU frequency 200 MHz */

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void gpu_test_context_setup(struct gpu_test_context_s* ctx)
{
    static bool initialized = false;
    if (!initialized) {
        return;
    }

    initialized = true;

    GPU_LOG_INFO("Initializing GPU");
    extern void gpu_init(void);
    gpu_init();

    if (ctx->param.cpu_freq <= 0) {
        GPU_LOG_ERROR("Invalid CPU frequency: %d", ctx->param.cpu_freq);
        return;
    }

    uint32_t cpu_freq = ctx->param.cpu_freq * 1000 * 1000;
    up_perf_init((void*)(uintptr_t)cpu_freq);
    GPU_LOG_INFO("perf enabled, cpu_freq = %" PRIu32, cpu_freq);

    g_cpu_freq = ctx->param.cpu_freq;

    uint32_t start_tick = up_perf_gettime();
    usleep(1000);
    uint32_t elapsed_tick = up_perf_gettime() - start_tick;
    uint32_t elapsed_time = TICK_TO_USEC(elapsed_tick);
    GPU_LOG_INFO("perf test elapsed_tick(%" PRIu32 "): %" PRIu32 " us", elapsed_tick, elapsed_time);

    if (elapsed_time < 1000 || elapsed_time - 1000 > 300) {
        GPU_LOG_WARN("CPU frequency: %d may wrong", g_cpu_freq);
    }

    gpu_tick_set_cb(tick_get_cb);
}

void gpu_test_context_teardown(struct gpu_test_context_s* ctx)
{
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static uint32_t tick_get_cb(void)
{
    static uint32_t prev_tick = 0;
    static uint32_t cur_tick_us = 0;
    uint32_t act_time = up_perf_gettime();
    uint32_t elaps;

    /*If there is no overflow in sys_time simple subtract*/
    if (act_time >= prev_tick) {
        elaps = act_time - prev_tick;
    } else {
        elaps = UINT32_MAX - prev_tick + 1;
        elaps += act_time;
    }

    cur_tick_us += TICK_TO_USEC(elaps);
    prev_tick = act_time;
    return cur_tick_us;
}

#endif /* GPU_TEST_CONTEXT_NUTTX_ENABLE */
