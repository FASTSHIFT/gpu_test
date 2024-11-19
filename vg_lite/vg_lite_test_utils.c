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

#include "vg_lite_test_utils.h"
#include "../gpu_assert.h"
#include "../gpu_utils.h"
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/

#define ENUM_TO_STRING(e) \
    case (e):             \
        return #e

#define VG_LITE_ENUM_TO_STRING(e) \
    case (VG_LITE_##e):           \
        return #e

#define FEATURE_ENUM_TO_STRING(e) \
    case (gcFEATURE_BIT_VG_##e):  \
        return #e

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static const char* vg_lite_test_feature_string(vg_lite_feature_t feature);

static void vg_lite_test_buffer_format_bytes(
    vg_lite_buffer_format_t format,
    uint32_t* mul,
    uint32_t* div,
    uint32_t* bytes_align);

static enum gpu_color_format_e vg_lite_test_vg_format_to_gpu_format(vg_lite_buffer_format_t format);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void vg_lite_test_dump_info(void)
{
    char name[64];
    vg_lite_uint32_t chip_id;
    vg_lite_uint32_t chip_rev;
    vg_lite_uint32_t cid;
    vg_lite_get_product_info(name, &chip_id, &chip_rev);
    vg_lite_get_register(0x30, &cid);
    GPU_LOG_INFO("Product Info: %s"
                 " | Chip ID: 0x%" PRIx32
                 " | Revision: 0x%" PRIx32
                 " | CID: 0x%" PRIx32,
        name, (uint32_t)chip_id, (uint32_t)chip_rev, (uint32_t)cid);

    vg_lite_info_t info;
    vg_lite_get_info(&info);
    GPU_LOG_INFO("VGLite API version: 0x%" PRIx32, (uint32_t)info.api_version);
    GPU_LOG_INFO("VGLite API header version: 0x%" PRIx32, (uint32_t)info.header_version);
    GPU_LOG_INFO("VGLite release version: 0x%" PRIx32, (uint32_t)info.release_version);

    for (int feature = 0; feature < gcFEATURE_COUNT; feature++) {
        vg_lite_uint32_t ret = vg_lite_query_feature((vg_lite_feature_t)feature);
        GPU_LOG_INFO("Feature-%d: %s\t - %s",
            feature, vg_lite_test_feature_string((vg_lite_feature_t)feature),
            ret ? "YES" : "NO");
    }

    vg_lite_uint32_t mem_size = 0;
    vg_lite_get_mem_size(&mem_size);
    GPU_LOG_INFO("Memory size: %" PRId32 " Bytes", (uint32_t)mem_size);
}

void vg_lite_test_error_dump_info(vg_lite_error_t error)
{
    GPU_LOG_INFO("Error code: %d(%s)", (int)error, vg_lite_test_error_string(error));
    switch (error) {
    case VG_LITE_SUCCESS:
        GPU_LOG_INFO("No error");
        break;

    case VG_LITE_OUT_OF_MEMORY:
    case VG_LITE_OUT_OF_RESOURCES: {
        vg_lite_uint32_t mem_size = 0;
        vg_lite_error_t ret = vg_lite_get_mem_size(&mem_size);
        if (ret != VG_LITE_SUCCESS) {
            GPU_LOG_ERROR("vg_lite_get_mem_size error: %d(%s)",
                (int)ret, vg_lite_test_error_string(ret));
            return;
        }

        GPU_LOG_INFO("Memory size: %" PRId32 " Bytes", (uint32_t)mem_size);
    } break;

    case VG_LITE_TIMEOUT:
    case VG_LITE_FLEXA_TIME_OUT: {
        vg_lite_error_t ret = vg_lite_dump_command_buffer();
        if (ret != VG_LITE_SUCCESS) {
            GPU_LOG_ERROR("vg_lite_dump_command_buffer error: %d(%s)",
                (int)ret, vg_lite_test_error_string(ret));
            return;
        }

        GPU_LOG_INFO("Command buffer finished");
    } break;

    default:
        vg_lite_test_dump_info();
        break;
    }
}

const char* vg_lite_test_error_string(vg_lite_error_t error)
{
    switch (error) {
        VG_LITE_ENUM_TO_STRING(SUCCESS);
        VG_LITE_ENUM_TO_STRING(INVALID_ARGUMENT);
        VG_LITE_ENUM_TO_STRING(OUT_OF_MEMORY);
        VG_LITE_ENUM_TO_STRING(NO_CONTEXT);
        VG_LITE_ENUM_TO_STRING(TIMEOUT);
        VG_LITE_ENUM_TO_STRING(OUT_OF_RESOURCES);
        VG_LITE_ENUM_TO_STRING(GENERIC_IO);
        VG_LITE_ENUM_TO_STRING(NOT_SUPPORT);
        VG_LITE_ENUM_TO_STRING(ALREADY_EXISTS);
        VG_LITE_ENUM_TO_STRING(NOT_ALIGNED);
        VG_LITE_ENUM_TO_STRING(FLEXA_TIME_OUT);
        VG_LITE_ENUM_TO_STRING(FLEXA_HANDSHAKE_FAIL);
    default:
        break;
    }
    return "UNKNOW_ERROR";
}

void vg_lite_test_buffer_alloc(vg_lite_buffer_t* buffer, uint32_t width, uint32_t height, vg_lite_buffer_format_t format, uint32_t stride)
{
    GPU_ASSERT_NULL(buffer);
    if (vg_lite_query_feature(gcFEATURE_BIT_VG_16PIXELS_ALIGN)) {
        width = GPU_ALIGN_UP(width, 16);
    }

    if (stride == VG_LITE_TEST_STRIDE_AUTO) {
        uint32_t mul, div, align;
        vg_lite_test_buffer_format_bytes(format, &mul, &div, &align);

        stride = GPU_ALIGN_UP(((width * mul + div - 1) / div), align);
    }

    struct gpu_buffer_s* gpu_buffer = gpu_buffer_alloc(
        width, height, stride, 64, vg_lite_test_vg_format_to_gpu_format(format));

    memset(buffer, 0, sizeof(vg_lite_buffer_t));
    buffer->memory = gpu_buffer->data;
    buffer->width = width;
    buffer->height = height;
    buffer->format = format;
    buffer->stride = stride;
    buffer->handle = gpu_buffer;
}

void vg_lite_test_buffer_free(vg_lite_buffer_t* buffer)
{
    GPU_ASSERT_NULL(buffer);
    GPU_ASSERT_NULL(buffer->handle);

    struct gpu_buffer_s* gpu_buffer = buffer->handle;
    gpu_buffer_free(gpu_buffer);
    memset(buffer, 0, sizeof(vg_lite_buffer_t));
}

void vg_lite_test_vg_buffer_to_gpu_buffer(struct gpu_buffer_s* gpu_buffer, const vg_lite_buffer_t* vg_buffer)
{
    GPU_ASSERT_NULL(gpu_buffer);
    GPU_ASSERT_NULL(vg_buffer);

    memset(gpu_buffer, 0, sizeof(struct gpu_buffer_s));
    gpu_buffer->data = vg_buffer->memory;
    gpu_buffer->width = vg_buffer->width;
    gpu_buffer->height = vg_buffer->height;
    gpu_buffer->stride = vg_buffer->stride;
    gpu_buffer->format = vg_lite_test_vg_format_to_gpu_format(vg_buffer->format);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static const char* vg_lite_test_feature_string(vg_lite_feature_t feature)
{
    switch (feature) {
        FEATURE_ENUM_TO_STRING(IM_INDEX_FORMAT);
        FEATURE_ENUM_TO_STRING(SCISSOR);
        FEATURE_ENUM_TO_STRING(BORDER_CULLING);
        FEATURE_ENUM_TO_STRING(RGBA2_FORMAT);
        FEATURE_ENUM_TO_STRING(QUALITY_8X);
        FEATURE_ENUM_TO_STRING(IM_FASTCLAER);
        FEATURE_ENUM_TO_STRING(RADIAL_GRADIENT);
        FEATURE_ENUM_TO_STRING(GLOBAL_ALPHA);
        FEATURE_ENUM_TO_STRING(RGBA8_ETC2_EAC);
        FEATURE_ENUM_TO_STRING(COLOR_KEY);
        FEATURE_ENUM_TO_STRING(DOUBLE_IMAGE);
        FEATURE_ENUM_TO_STRING(YUV_OUTPUT);
        FEATURE_ENUM_TO_STRING(FLEXA);
        FEATURE_ENUM_TO_STRING(24BIT);
        FEATURE_ENUM_TO_STRING(DITHER);
        FEATURE_ENUM_TO_STRING(USE_DST);
        FEATURE_ENUM_TO_STRING(PE_CLEAR);
        FEATURE_ENUM_TO_STRING(IM_INPUT);
        FEATURE_ENUM_TO_STRING(DEC_COMPRESS);
        FEATURE_ENUM_TO_STRING(LINEAR_GRADIENT_EXT);
        FEATURE_ENUM_TO_STRING(MASK);
        FEATURE_ENUM_TO_STRING(MIRROR);
        FEATURE_ENUM_TO_STRING(GAMMA);
        FEATURE_ENUM_TO_STRING(NEW_BLEND_MODE);
        FEATURE_ENUM_TO_STRING(STENCIL);
        FEATURE_ENUM_TO_STRING(SRC_PREMULTIPLIED); /*! Valid only if FEATURE_ENUM_TO_STRING(HW_PREMULTIPLY is 0   */
        FEATURE_ENUM_TO_STRING(HW_PREMULTIPLY); /*! HW multiplier can accept either premultiplied or not */
        FEATURE_ENUM_TO_STRING(COLOR_TRANSFORMATION);
        FEATURE_ENUM_TO_STRING(LVGL_SUPPORT);
        FEATURE_ENUM_TO_STRING(INDEX_ENDIAN);
        FEATURE_ENUM_TO_STRING(24BIT_PLANAR);
        FEATURE_ENUM_TO_STRING(PIXEL_MATRIX);
        FEATURE_ENUM_TO_STRING(NEW_IMAGE_INDEX);
        FEATURE_ENUM_TO_STRING(PARALLEL_PATHS);
        FEATURE_ENUM_TO_STRING(STRIPE_MODE);
        FEATURE_ENUM_TO_STRING(IM_DEC_INPUT);
        FEATURE_ENUM_TO_STRING(GAUSSIAN_BLUR);
        FEATURE_ENUM_TO_STRING(RECTANGLE_TILED_OUT);
        FEATURE_ENUM_TO_STRING(TESSELLATION_TILED_OUT);
        FEATURE_ENUM_TO_STRING(IM_REPEAT_REFLECT);
        FEATURE_ENUM_TO_STRING(YUY2_INPUT);
        FEATURE_ENUM_TO_STRING(YUV_INPUT);
        FEATURE_ENUM_TO_STRING(YUV_TILED_INPUT);
        FEATURE_ENUM_TO_STRING(AYUV_INPUT);
        FEATURE_ENUM_TO_STRING(16PIXELS_ALIGN);
        FEATURE_ENUM_TO_STRING(DEC_COMPRESS_2_0);
    default:
        break;
    }
    return "UNKNOW_FEATURE";
}

static void vg_lite_test_buffer_format_bytes(
    vg_lite_buffer_format_t format,
    uint32_t* mul,
    uint32_t* div,
    uint32_t* bytes_align)
{
    /* Get the bpp information of a color format. */
    *mul = *div = 1;
    *bytes_align = 4;
    switch (format) {
    case VG_LITE_L8:
    case VG_LITE_A8:
    case VG_LITE_RGBA8888_ETC2_EAC:
        break;
    case VG_LITE_A4:
        *div = 2;
        break;
    case VG_LITE_ABGR1555:
    case VG_LITE_ARGB1555:
    case VG_LITE_BGRA5551:
    case VG_LITE_RGBA5551:
    case VG_LITE_RGBA4444:
    case VG_LITE_BGRA4444:
    case VG_LITE_ABGR4444:
    case VG_LITE_ARGB4444:
    case VG_LITE_RGB565:
    case VG_LITE_BGR565:
    case VG_LITE_YUYV:
    case VG_LITE_YUY2:
    case VG_LITE_YUY2_TILED:
    /* AYUY2 buffer memory = YUY2 + alpha. */
    case VG_LITE_AYUY2:
    case VG_LITE_AYUY2_TILED:
        *mul = 2;
        break;
    case VG_LITE_RGBA8888:
    case VG_LITE_BGRA8888:
    case VG_LITE_ABGR8888:
    case VG_LITE_ARGB8888:
    case VG_LITE_RGBX8888:
    case VG_LITE_BGRX8888:
    case VG_LITE_XBGR8888:
    case VG_LITE_XRGB8888:
        *mul = 4;
        break;
    case VG_LITE_NV12:
    case VG_LITE_NV12_TILED:
        *mul = 1;
        break;
    case VG_LITE_ANV12:
    case VG_LITE_ANV12_TILED:
        *mul = 4;
        break;
    case VG_LITE_INDEX_1:
        *div = 8;
        *bytes_align = 8;
        break;
    case VG_LITE_INDEX_2:
        *div = 4;
        *bytes_align = 8;
        break;
    case VG_LITE_INDEX_4:
        *div = 2;
        *bytes_align = 8;
        break;
    case VG_LITE_INDEX_8:
        *bytes_align = 1;
        break;
    case VG_LITE_RGBA2222:
    case VG_LITE_BGRA2222:
    case VG_LITE_ABGR2222:
    case VG_LITE_ARGB2222:
        *mul = 1;
        break;
    case VG_LITE_RGB888:
    case VG_LITE_BGR888:
    case VG_LITE_ABGR8565:
    case VG_LITE_BGRA5658:
    case VG_LITE_ARGB8565:
    case VG_LITE_RGBA5658:
        *mul = 3;
        break;
    default:
        GPU_LOG_ERROR("unsupport color format: 0x%" PRIx32, (uint32_t)format);
        GPU_ASSERT(false);
        break;
    }
}

static enum gpu_color_format_e vg_lite_test_vg_format_to_gpu_format(vg_lite_buffer_format_t format)
{
#define COLOR_FORMAT_MATCH(FMT) \
    case VG_LITE_##FMT:         \
        return GPU_COLOR_FORMAT_##FMT;

    switch (format) {
        COLOR_FORMAT_MATCH(BGR565);
        COLOR_FORMAT_MATCH(BGR888);
        COLOR_FORMAT_MATCH(BGRA8888);
        COLOR_FORMAT_MATCH(BGRX8888);
        COLOR_FORMAT_MATCH(BGRA5658);

    default:
        break;
    }

#undef COLOR_FORMAT_MATCH

    return GPU_COLOR_FORMAT_UNKNOWN;
}
