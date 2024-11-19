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

#include "vg_lite_test_context.h"
#include "../gpu_context.h"
#include "../gpu_recorder.h"
#include "vg_lite_test_utils.h"
#include <stdio.h>

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

void vg_lite_test_context_setup(struct vg_lite_test_context_s* ctx)
{
    vg_lite_test_buffer_alloc(
        &ctx->target_buffer,
        ctx->gpu_ctx->param.img_width,
        ctx->gpu_ctx->param.img_height,
        VG_LITE_BGRA8888,
        VG_LITE_TEST_STRIDE_AUTO);

    if (ctx->gpu_ctx->recorder) {
        gpu_recorder_write_string(ctx->gpu_ctx->recorder,
            "Testcase,"
            "Target Format,Src Format,"
            "Target Address,Src Address,"
            "Target Area,Src Area,"
            "Prepare Time(ms),Render Time(ms),"
            "Result,"
            "Remark"
            "\n");
    }
}

void vg_lite_test_context_teardown(struct vg_lite_test_context_s* ctx)
{
    vg_lite_test_buffer_free(&ctx->target_buffer);

    if (ctx->src_buffer.memory) {
        vg_lite_test_buffer_free(&ctx->src_buffer);
    }
}

void vg_lite_test_context_reset(struct vg_lite_test_context_s* ctx)
{
    /* Clear the target buffer */
    VG_LITE_TEST_CHECK_ERROR(vg_lite_clear(&ctx->target_buffer, NULL, 0));
    VG_LITE_TEST_CHECK_ERROR(vg_lite_finish());
    ctx->remark_text[0] = '\0';
    ctx->prepare_tick = 0;
    ctx->render_tick = 0;

    if (ctx->src_buffer.memory) {
        vg_lite_test_buffer_free(&ctx->src_buffer);
    }
}

void vg_lite_test_context_record(struct vg_lite_test_context_s* ctx, const char* testcase_str, vg_lite_error_t error)
{
    if (!ctx->gpu_ctx->recorder) {
        return;
    }

    char result[256];
    snprintf(result, sizeof(result),
        "%s,"
        "%s,%s,"
        "%p,%p,"
        "%dx%d,%dx%d,"
        "%0.3f,"
        "%0.3f,"
        "%s,"
        "%s\n",
        testcase_str,
        vg_lite_test_buffer_format_string(ctx->target_buffer.format),
        vg_lite_test_buffer_format_string(ctx->src_buffer.format),
        ctx->target_buffer.memory,
        ctx->src_buffer.memory,
        (int)ctx->target_buffer.width,
        (int)ctx->target_buffer.height,
        (int)ctx->src_buffer.width,
        (int)ctx->src_buffer.height,
        ctx->prepare_tick / 1000.0f,
        ctx->render_tick / 1000.0f,
        vg_lite_test_error_string(error),
        ctx->remark_text);

    gpu_recorder_write_string(ctx->gpu_ctx->recorder, result);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
