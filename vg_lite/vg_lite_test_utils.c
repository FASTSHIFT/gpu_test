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
#include <inttypes.h>

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
