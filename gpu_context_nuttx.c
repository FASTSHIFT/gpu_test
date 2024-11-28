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

#include "gpu_assert.h"
#include "gpu_context.h"
#include "gpu_fb.h"
#include "gpu_log.h"
#include "gpu_tick.h"
#include <inttypes.h>
#include <nuttx/arch.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/

#define TICK_TO_USEC(tick) ((tick) / g_cpu_freq_mhz)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static uint32_t tick_get_cb(void);
static uint32_t calc_avg_cpu_freq(void);

/**********************
 *  STATIC VARIABLES
 **********************/

static uint32_t g_cpu_freq_mhz = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void gpu_test_context_setup(struct gpu_test_context_s* ctx)
{
    GPU_ASSERT_NULL(ctx);

    if (ctx->param.fbdev_path) {
        ctx->fb = gpu_fb_create(ctx->param.fbdev_path);

        if (ctx->fb) {
            gpu_fb_get_buffer(ctx->fb, &ctx->target_buffer);
        }
    }

    static bool initialized = false;

#ifdef CONFIG_ARCH_SIM
    /* In simulation mode, we need to initialize the GPU all the time */
    initialized = false;
#endif

    if (initialized) {
        GPU_LOG_INFO("GPU already initialized");
        return;
    }

    initialized = true;

#ifdef CONFIG_TESTING_GPU_TEST_CUSTOM_INIT
    GPU_LOG_INFO("Initializing GPU");
    extern void gpu_init(void);
    gpu_init();
#endif

    /* Default CPU frequency 200 MHz */
    uint32_t cpu_freq_hz = 200 * 1000 * 1000;

    if (ctx->param.cpu_freq > 0) {
        g_cpu_freq_mhz = ctx->param.cpu_freq;
    } else {
        /* Enable performance counter */
        up_perf_init((void*)(uintptr_t)cpu_freq);

        /* Calculate average CPU frequency */
        g_cpu_freq_mhz = calc_avg_cpu_freq() / (1000 * 1000);
    }

    GPU_LOG_INFO("CPU frequency: %" PRIu32 " MHz", g_cpu_freq_mhz);

    if (g_cpu_freq_mhz == 0) {
        GPU_LOG_ERROR("Failed to calculate CPU frequency");
        return;
    }

    cpu_freq_hz = g_cpu_freq_mhz * 1000 * 1000;

    up_perf_init((void*)(uintptr_t)cpu_freq_hz);

    gpu_tick_set_cb(tick_get_cb);
}

void gpu_test_context_teardown(struct gpu_test_context_s* ctx)
{
    GPU_ASSERT_NULL(ctx);

    if (ctx->fb) {
        gpu_fb_destroy(ctx->fb);
        ctx->fb = NULL;
    }

#ifdef CONFIG_ARCH_SIM
    extern void gpu_deinit(void);
    GPU_LOG_INFO("Deinitializing GPU");
    gpu_deinit();
#endif
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

static uint32_t calc_avg_cpu_freq(void)
{
    uint32_t start_tick = up_perf_gettime();

    /* Wait 1 second*/
    usleep(1000 * 1000);

    uint32_t elapsed_tick = up_perf_gettime() - start_tick;

    GPU_LOG_INFO("perf elapsed_tick: %" PRIu32, elapsed_tick);

    return elapsed_tick;
}

#endif /* GPU_TEST_CONTEXT_NUTTX_ENABLE */
