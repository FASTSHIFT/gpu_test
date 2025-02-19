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

#include "gpu_test.h"
#include "gpu_context.h"
#include "gpu_recorder.h"
#include "gpu_tick.h"
#include "gpu_utils.h"
#include "vg_lite/vg_lite_test.h"
#include <stdio.h>
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

static void gpu_test_write_header(struct gpu_test_context_s* ctx);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int gpu_test_run(struct gpu_test_context_s* ctx)
{
    ctx->recorder = gpu_recorder_create(ctx->param.output_dir, "vg_lite");
    if (!ctx->recorder) {
        return -1;
    }

    gpu_test_write_header(ctx);

    /* Seed the random number generator with the current time */
    srand(gpu_tick_get());

    int ret = vg_lite_test_run(ctx);

    gpu_recorder_delete(ctx->recorder);

    return ret;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void gpu_test_write_header(struct gpu_test_context_s* ctx)
{
    if (!ctx->recorder) {
        return;
    }

    gpu_recorder_write_string(ctx->recorder, "Command Line,");

    for (int i = 0; i < ctx->param.argc; i++) {
        gpu_recorder_write_string(ctx->recorder, ctx->param.argv[i]);
        gpu_recorder_write_string(ctx->recorder, " ");
    }

    gpu_recorder_write_string(ctx->recorder, "\n\n");
}
