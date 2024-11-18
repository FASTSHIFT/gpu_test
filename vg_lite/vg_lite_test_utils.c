/****************************************************************************
 * apps/testing/gpu/vg_lite/vg_lite_test_utils.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "vg_lite_test_utils.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define VG_ENUM_TO_STRING_CASE_DEF(e) \
  case (VG_LITE_##e): return #e

#define FEATURE_ENUM_TO_STRING(e) \
    case (gcFEATURE_BIT_VG_##e):  \
    return #e

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: vg_lite_get_error_type_string
 ****************************************************************************/

#ifndef CONFIG_LV_USE_GPU_MANAGER
const char *vg_lite_get_error_type_string(vg_lite_error_t error)
{
  switch (error)
    {
      VG_ENUM_TO_STRING_CASE_DEF(SUCCESS);
      VG_ENUM_TO_STRING_CASE_DEF(INVALID_ARGUMENT);
      VG_ENUM_TO_STRING_CASE_DEF(OUT_OF_MEMORY);
      VG_ENUM_TO_STRING_CASE_DEF(NO_CONTEXT);
      VG_ENUM_TO_STRING_CASE_DEF(TIMEOUT);
      VG_ENUM_TO_STRING_CASE_DEF(OUT_OF_RESOURCES);
      VG_ENUM_TO_STRING_CASE_DEF(GENERIC_IO);
      VG_ENUM_TO_STRING_CASE_DEF(NOT_SUPPORT);
      VG_ENUM_TO_STRING_CASE_DEF(ALREADY_EXISTS);
      VG_ENUM_TO_STRING_CASE_DEF(NOT_ALIGNED);
      VG_ENUM_TO_STRING_CASE_DEF(FLEXA_TIME_OUT);
      VG_ENUM_TO_STRING_CASE_DEF(FLEXA_HANDSHAKE_FAIL);
      default:
        break;
    }

  return "UNKNOW_ERROR";
}

/****************************************************************************
 * Name: vg_lite_get_buffer_format_string
 ****************************************************************************/

const char *vg_lite_get_buffer_format_string(
  vg_lite_buffer_format_t format)
{
  switch (format)
    {
      VG_ENUM_TO_STRING_CASE_DEF(RGBA8888);
      VG_ENUM_TO_STRING_CASE_DEF(BGRA8888);
      VG_ENUM_TO_STRING_CASE_DEF(RGBX8888);
      VG_ENUM_TO_STRING_CASE_DEF(BGRX8888);
      VG_ENUM_TO_STRING_CASE_DEF(RGB565);
      VG_ENUM_TO_STRING_CASE_DEF(BGR565);
      VG_ENUM_TO_STRING_CASE_DEF(RGBA4444);
      VG_ENUM_TO_STRING_CASE_DEF(BGRA4444);
      VG_ENUM_TO_STRING_CASE_DEF(BGRA5551);
      VG_ENUM_TO_STRING_CASE_DEF(A4);
      VG_ENUM_TO_STRING_CASE_DEF(A8);
      VG_ENUM_TO_STRING_CASE_DEF(L8);
      VG_ENUM_TO_STRING_CASE_DEF(YUYV);

      VG_ENUM_TO_STRING_CASE_DEF(YUY2);
      VG_ENUM_TO_STRING_CASE_DEF(NV12);
      VG_ENUM_TO_STRING_CASE_DEF(ANV12);
      VG_ENUM_TO_STRING_CASE_DEF(AYUY2);

      VG_ENUM_TO_STRING_CASE_DEF(YV12);
      VG_ENUM_TO_STRING_CASE_DEF(YV24);
      VG_ENUM_TO_STRING_CASE_DEF(YV16);
      VG_ENUM_TO_STRING_CASE_DEF(NV16);

      VG_ENUM_TO_STRING_CASE_DEF(YUY2_TILED);
      VG_ENUM_TO_STRING_CASE_DEF(NV12_TILED);
      VG_ENUM_TO_STRING_CASE_DEF(ANV12_TILED);
      VG_ENUM_TO_STRING_CASE_DEF(AYUY2_TILED);

      VG_ENUM_TO_STRING_CASE_DEF(INDEX_1);
      VG_ENUM_TO_STRING_CASE_DEF(INDEX_2);
      VG_ENUM_TO_STRING_CASE_DEF(INDEX_4);
      VG_ENUM_TO_STRING_CASE_DEF(INDEX_8);

      VG_ENUM_TO_STRING_CASE_DEF(RGBA2222);
      VG_ENUM_TO_STRING_CASE_DEF(BGRA2222);
      VG_ENUM_TO_STRING_CASE_DEF(ABGR2222);
      VG_ENUM_TO_STRING_CASE_DEF(ARGB2222);
      VG_ENUM_TO_STRING_CASE_DEF(ABGR4444);
      VG_ENUM_TO_STRING_CASE_DEF(ARGB4444);
      VG_ENUM_TO_STRING_CASE_DEF(ABGR8888);
      VG_ENUM_TO_STRING_CASE_DEF(ARGB8888);
      VG_ENUM_TO_STRING_CASE_DEF(ABGR1555);
      VG_ENUM_TO_STRING_CASE_DEF(RGBA5551);
      VG_ENUM_TO_STRING_CASE_DEF(ARGB1555);
      VG_ENUM_TO_STRING_CASE_DEF(XBGR8888);
      VG_ENUM_TO_STRING_CASE_DEF(XRGB8888);
      VG_ENUM_TO_STRING_CASE_DEF(RGBA8888_ETC2_EAC);
      VG_ENUM_TO_STRING_CASE_DEF(RGB888);
      VG_ENUM_TO_STRING_CASE_DEF(BGR888);
      VG_ENUM_TO_STRING_CASE_DEF(ABGR8565);
      VG_ENUM_TO_STRING_CASE_DEF(BGRA5658);
      VG_ENUM_TO_STRING_CASE_DEF(ARGB8565);
      VG_ENUM_TO_STRING_CASE_DEF(RGBA5658);
    default:
      break;
    }

  return "UNKNOW_BUFFER_FORMAT";
}

/****************************************************************************
 * Name: vg_lite_get_filter_string
 ****************************************************************************/

const char *vg_lite_get_filter_string(vg_lite_filter_t filter)
{
  switch (filter)
    {
      VG_ENUM_TO_STRING_CASE_DEF(FILTER_POINT);
      VG_ENUM_TO_STRING_CASE_DEF(FILTER_LINEAR);
      VG_ENUM_TO_STRING_CASE_DEF(FILTER_BI_LINEAR);
    default:
      break;
    }

  return "UNKNOW_FILTER";
}

/****************************************************************************
 * Name: vg_lite_get_blend_string
 ****************************************************************************/

const char *vg_lite_get_blend_string(vg_lite_blend_t blend)
{
  switch (blend)
    {
      VG_ENUM_TO_STRING_CASE_DEF(BLEND_NONE);
      VG_ENUM_TO_STRING_CASE_DEF(BLEND_SRC_OVER);
      VG_ENUM_TO_STRING_CASE_DEF(BLEND_DST_OVER);
      VG_ENUM_TO_STRING_CASE_DEF(BLEND_SRC_IN);
      VG_ENUM_TO_STRING_CASE_DEF(BLEND_DST_IN);
      VG_ENUM_TO_STRING_CASE_DEF(BLEND_SCREEN);
      VG_ENUM_TO_STRING_CASE_DEF(BLEND_MULTIPLY);
      VG_ENUM_TO_STRING_CASE_DEF(BLEND_ADDITIVE);
      VG_ENUM_TO_STRING_CASE_DEF(BLEND_SUBTRACT);
    default:
      break;
    }

  return "UNKNOW_BLEND";
}

/****************************************************************************
 * Name: vg_lite_get_global_alpha_string
 ****************************************************************************/

const char *vg_lite_get_global_alpha_string(
  vg_lite_global_alpha_t global_alpha)
{
    switch (global_alpha)
    {
      VG_ENUM_TO_STRING_CASE_DEF(NORMAL);
      VG_ENUM_TO_STRING_CASE_DEF(GLOBAL);
      VG_ENUM_TO_STRING_CASE_DEF(SCALED);
    default:
      break;
    }

  return "UNKNOW_GLOBAL_ALPHA";
}

#endif

/****************************************************************************
 * Name: vg_lite_get_feature_string
 ****************************************************************************/

const char *vg_lite_get_feature_string(vg_lite_feature_t feature)
{
  switch (feature)
    {
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
      FEATURE_ENUM_TO_STRING(SRC_PREMULTIPLIED);
      FEATURE_ENUM_TO_STRING(HW_PREMULTIPLY);
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
    default:
      break;
    }

  return NULL;
}

/****************************************************************************
 * Name: vg_lite_dump_buffer_info
 ****************************************************************************/

void vg_lite_dump_buffer_info(
  const char *name,
  const vg_lite_buffer_t *buffer)
{
  GPU_LOG_INFO("name: %s", name);
  GPU_LOG_INFO("memory: %p", buffer->memory);
  GPU_LOG_INFO("size: %d x %d", buffer->width, buffer->height);
  GPU_LOG_INFO("stride: %d", buffer->stride);
  GPU_LOG_INFO("format: %d (%s) (%s)",
                buffer->format,
                vg_lite_get_buffer_format_string(buffer->format),
                buffer->tiled ? "tiled" : "linear");
}

/****************************************************************************
 * Name: vg_lite_draw_fb
 ****************************************************************************/

void vg_lite_draw_fb(struct gpu_test_context_s *ctx,
                     vg_lite_buffer_t *buffer,
                     int x_pos, int y_pos)
{
  int x;
  int y;
  int w = buffer->width;
  int h = buffer->height;
  gpu_color32_t *src = buffer->memory;
  gpu_color32_t *dest;
  dest = ctx->fbmem + y_pos * ctx->stride + x_pos * sizeof(gpu_color32_t);

  for (y = 0; y < h; y++)
    {
      for (x = 0; x < w; x++)
        {
          *dest = gpu_sw_color32_mix(*src, *dest, src->ch.alpha);
          dest++;
          src++;
        }

      dest += (ctx->xres - w);
    }
}

/****************************************************************************
 * Name: vg_lite_draw_fb_center
 ****************************************************************************/

void vg_lite_draw_fb_center(struct gpu_test_context_s *ctx)
{
  vg_lite_buffer_t *buffer = VG_LITE_DEST_BUF;
  vg_lite_draw_fb(
    ctx,
    buffer,
    (ctx->xres - buffer->width) / 2,
    (ctx->yres - buffer->height) / 2);
}

#ifndef CONFIG_LV_USE_GPU_MANAGER
/****************************************************************************
 * Name: vg_lite_buffer_init
 ****************************************************************************/

void vg_lite_buffer_init(vg_lite_buffer_t *buffer)
{
  memset(buffer, 0, sizeof(vg_lite_buffer_t));
}
#endif

/****************************************************************************
 * Name: vg_lite_create_image
 ****************************************************************************/

vg_lite_error_t vg_lite_create_image(vg_lite_buffer_t *image,
                                     int width,
                                     int height,
                                     vg_lite_buffer_format_t fmt,
                                     bool tiled)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  vg_lite_buffer_init(image);
  image->width = width;
  image->height = height;
  image->format = fmt;
  image->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
  image->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
  image->tiled = tiled ? VG_LITE_TILED : VG_LITE_LINEAR;
  VG_LITE_CHECK_ERROR(vg_lite_allocate(image));
  memset(image->memory, 0x5a, image->height * image->stride);
  vg_lite_dump_buffer_info(__func__, image);

error_handler:
  return error;
}

/****************************************************************************
 * Name: vg_lite_delete_image
 ****************************************************************************/

void vg_lite_delete_image(vg_lite_buffer_t *image)
{
  GPU_ASSERT_NULL(image);
  if (!image->handle)
    {
      GPU_LOG_WARN("image->handle is NULL");
      return;
    }

  GPU_LOG_INFO("free image: %p", image->handle);
  vg_lite_free(image);
}

#ifndef CONFIG_LV_USE_GPU_MANAGER

/****************************************************************************
 * Name: vg_lite_bpp_to_format
 ****************************************************************************/

vg_lite_buffer_format_t vg_lite_bpp_to_format(int bpp)
{
  switch (bpp)
    {
    case 16:
      return VG_LITE_BGR565;

    case 24:
      return VG_LITE_BGR888;

    case 32:
      return VG_LITE_BGRX8888;

    default:
      GPU_LOG_ERROR("Invalid bpp: %d", bpp);
      break;
    }

  return VG_LITE_RGBA8888;
}

/****************************************************************************
 * Name: vg_lite_get_format_bytes
 ****************************************************************************/

void vg_lite_get_format_bytes(vg_lite_buffer_format_t format,
                              uint32_t *mul,
                              uint32_t *div,
                              uint32_t *bytes_align)
{
  /* Get the bpp information of a color format. */

  *mul = *div = 1;
  *bytes_align = 4;
  switch (format)
    {
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
          *mul = 3;
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
          break;
    }
}
#endif

/****************************************************************************
 * Name: vg_lite_init_custon_buffer
 ****************************************************************************/

vg_lite_error_t vg_lite_init_custom_buffer(
  vg_lite_buffer_t *buffer,
  void *ptr,
  int32_t width,
  int32_t height,
  vg_lite_buffer_format_t format)
{
  uint32_t mul;
  uint32_t div;
  uint32_t align;
  memset(buffer, 0, sizeof(vg_lite_buffer_t));
  buffer->format = format;
  buffer->tiled = VG_LITE_LINEAR;
  buffer->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
  buffer->transparency_mode = VG_LITE_IMAGE_OPAQUE;

  buffer->width = width;
  buffer->height = height;

  vg_lite_get_format_bytes(buffer->format, &mul, &div, &align);
  buffer->stride = VG_LITE_ALIGN((buffer->width * mul / div), align);

  buffer->memory = ptr;
  buffer->address = (uint32_t)(uintptr_t)ptr;

  return VG_LITE_SUCCESS;
}

/****************************************************************************
 * Name: vg_lite_init_matrix
 ****************************************************************************/

vg_lite_matrix_t vg_lite_init_matrix(float x, float y,
                                     float angle, float scale,
                                     int pivot_x, int pivot_y)
{
  vg_lite_matrix_t matrix;
  vg_lite_identity(&matrix);
  vg_lite_translate(x, y, &matrix);
  vg_lite_translate(pivot_x, pivot_y, &matrix);
  vg_lite_scale(scale, scale, &matrix);
  vg_lite_rotate(angle, &matrix);
  vg_lite_translate(-pivot_x, -pivot_y, &matrix);
  return matrix;
}

/****************************************************************************
 * Name: vg_lite_get_coord_format_size
 ****************************************************************************/

size_t vg_lite_get_coord_format_size(vg_lite_format_t format)
{
  switch (format)
    {
    case VG_LITE_S8:
      return sizeof(int8_t);

    case VG_LITE_S16:
      return sizeof(int16_t);

    case VG_LITE_S32:
      return sizeof(int32_t);

    case VG_LITE_FP32:
      return sizeof(vg_lite_float_t);

    default:
      GPU_LOG_WARN("Invalid format: %d", format);
      return 0;
    }
}

/****************************************************************************
 * Name: vg_lite_fill_round_rect_path
 ****************************************************************************/

int vg_lite_fill_round_rect_path(
  int32_t *path,
  const struct vg_lite_area_s *rect,
  uint16_t radius)
{
  /* 3(MOVE) + 3(LINE) * 3 + 1(CLOSE/END) */

  static const int rect_path_len = 13;

  /* 3(MOVE) + 3(LINE) * 3 + 7(CUBIC) * 4 + 1(CLOSE/END) */

  static const int point_path_len = 41;

  /* Magic number from https://spencermortensen.com/articles/bezier-circle/ */

  static const float arc_magic = 0.55191502449351f;

  if (!radius)
    {
      *(uint32_t *)path = VLC_OP_MOVE;
      *(uint32_t *)(path + 3)
      = *(uint32_t *)(path + 6)
      = *(uint32_t *)(path + 9)
      = VLC_OP_LINE;
      *(uint32_t *)(path + 12) = VLC_OP_END;
      path[1] = path[4] = rect->x1;
      path[7] = path[10] = rect->x2 + 1;
      path[2] = path[11] = rect->y1;
      path[5] = path[8] = rect->y2 + 1;
      return rect_path_len;
    }

  float r = radius;
  float c = arc_magic * r;
  float cx0 = rect->x1 + r;
  float cx1 = rect->x2 - r;
  float cy0 = rect->y1 + r;
  float cy1 = rect->y2 - r;
  *(uint32_t *)path++ = VLC_OP_MOVE;
  *path++ = cx0 - r;
  *path++ = cy0;
  *(uint32_t *)path++ = VLC_OP_CUBIC;
  *path++ = cx0 - r;
  *path++ = cy0 - c;
  *path++ = cx0 - c;
  *path++ = cy0 - r;
  *path++ = cx0;
  *path++ = cy0 - r;
  *(uint32_t *)path++ = VLC_OP_LINE;
  *path++ = cx1;
  *path++ = cy0 - r;
  *(uint32_t *)path++ = VLC_OP_CUBIC;
  *path++ = cx1 + c;
  *path++ = cy0 - r;
  *path++ = cx1 + r;
  *path++ = cy0 - c;
  *path++ = cx1 + r;
  *path++ = cy0;
  *(uint32_t *)path++ = VLC_OP_LINE;
  *path++ = cx1 + r;
  *path++ = cy1;
  *(uint32_t *)path++ = VLC_OP_CUBIC;
  *path++ = cx1 + r;
  *path++ = cy1 + c;
  *path++ = cx1 + c;
  *path++ = cy1 + r;
  *path++ = cx1;
  *path++ = cy1 + r;
  *(uint32_t *)path++ = VLC_OP_LINE;
  *path++ = cx0;
  *path++ = cy1 + r;
  *(uint32_t *)path++ = VLC_OP_CUBIC;
  *path++ = cx0 - c;
  *path++ = cy1 + r;
  *path++ = cx0 - r;
  *path++ = cy1 + c;
  *path++ = cx0 - r;
  *path++ = cy1;
  *(uint32_t *)path++ = VLC_OP_END;
  return point_path_len;
}

/****************************************************************************
 * Name: vg_lite_test_report_start
 ****************************************************************************/

void vg_lite_test_report_start(struct gpu_test_context_s *ctx)
{
  GPU_ASSERT_NULL(ctx);
  GPU_ASSERT(ctx->recorder == NULL);
  ctx->recorder = gpu_recorder_create(ctx, "vg_lite");

  if (!ctx->recorder)
    {
      GPU_LOG_WARN("Failed to create recorder");
      return;
    }

  gpu_recorder_write_string(ctx->recorder,
    "Testcase,"
    "Instructions,"
    "Src Format,Dest Format,"
    "Src Address,Dest Address,"
    "Src Area,Dest Area,"
    "Prepare Time(ms),"
    "Render Time(ms),"
    "Result,"
    "Remark"
    "\n");
}

/****************************************************************************
 * Name: vg_lite_test_report_finish
 ****************************************************************************/

void vg_lite_test_report_finish(struct gpu_test_context_s *ctx)
{
  GPU_ASSERT_NULL(ctx);
  if (!ctx->recorder)
    {
      return;
    }

  gpu_recorder_delete(ctx->recorder);
  ctx->recorder = NULL;
}

/****************************************************************************
 * Name: vg_lite_test_report_write
 ****************************************************************************/

void vg_lite_test_report_write(struct gpu_test_context_s *ctx,
                               const char *testcase_str,
                               const char *instructions_str,
                               const char *remark_str,
                               bool is_passed)
{
  GPU_ASSERT_NULL(ctx);

  if (!ctx->recorder)
    {
      return;
    }

  if (!is_passed)
    {
      GPU_PERF_RESET();
    }

  struct vg_lite_test_ctx_s *vg_lite_ctx = VG_LITE_CTX;

  char result[256];
  snprintf(result, sizeof(result),
    "%s,"
    "%s,"
    "%s,%s,"
    "%p,%p,"
    "%dx%d,%dx%d,"
    "%0.3f,"
    "%0.3f,"
    "%s,"
    "%s\n",
    testcase_str,
    instructions_str ? instructions_str : "",
    vg_lite_get_buffer_format_string(vg_lite_ctx->src_buffer.format),
    vg_lite_get_buffer_format_string(vg_lite_ctx->dest_buffer.format),
    vg_lite_ctx->src_buffer.memory,
    vg_lite_ctx->dest_buffer.memory,
    (int)vg_lite_ctx->src_buffer.width,
    (int)vg_lite_ctx->src_buffer.height,
    (int)vg_lite_ctx->dest_buffer.width,
    (int)vg_lite_ctx->dest_buffer.height,
    ctx->perf.prepare / 1000.0f,
    ctx->perf.render / 1000.0f,
    is_passed ? "PASS" : "FAIL",
    remark_str ? remark_str : "");

  gpu_recorder_write_string(ctx->recorder, result);
}
