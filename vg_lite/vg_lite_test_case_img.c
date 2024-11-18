/****************************************************************************
 * apps/testing/gpu/vg_lite/vg_lite_test_case_img.c
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
#include "resource/image_a8.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define IMG_WIDTH  (ctx->param.img_width)
#define IMG_HEIGHT (ctx->param.img_height)
#define IMG_SET_CLUT(colors, len) \
  VG_LITE_CHECK_ERROR(vg_lite_set_CLUT(len, colors));

/****************************************************************************
 * Private Types
 ****************************************************************************/

enum img_transform_mode_e
{
  IMG_TRANSFORM_NONE = 0,
  IMG_TRANSFORM_OFFSET,
  IMG_TRANSFORM_ROTATE,
  IMG_TRANSFORM_SCALE,
  IMG_TRANSFORM_MAX,
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: vg_lite_test_image_transform
 ****************************************************************************/

static vg_lite_error_t vg_lite_test_image_transform(
  struct gpu_test_context_s *ctx,
  vg_lite_blend_t blend,
  vg_lite_filter_t filter,
  enum img_transform_mode_e tf_mode,
  int rotation)
{
  char tf_str[64];
  char instructions_str[256];

  vg_lite_error_t error = VG_LITE_SUCCESS;

  vg_lite_matrix_t matrix;
  vg_lite_identity(&matrix);
  switch (tf_mode)
  {
    case IMG_TRANSFORM_NONE:
      strncpy(tf_str, "None", sizeof(tf_str) - 1);
      break;
    case IMG_TRANSFORM_OFFSET:
          vg_lite_translate(64, 64, &matrix);
          strncpy(tf_str, "Offset (X64 Y64)", sizeof(tf_str) - 1);
      break;
    case IMG_TRANSFORM_ROTATE:
          matrix = vg_lite_init_matrix(0, 0, rotation, 1.0,
                                       IMG_WIDTH / 2, IMG_HEIGHT / 2);
          snprintf(tf_str, sizeof(tf_str), "Rotate (%d')", rotation);
      break;
    case IMG_TRANSFORM_SCALE:
          vg_lite_scale(1.7, 1.7, &matrix);
          strncpy(tf_str, "Scale (X1.7 Y1.7)", sizeof(tf_str) - 1);
      break;
    default:
      break;
  }

  GPU_LOG_INFO("Transform: %s", tf_str);
  GPU_LOG_INFO("Blend: 0x%x (%s)", blend, vg_lite_get_blend_string(blend));
  GPU_LOG_INFO("Filter: 0x%x (%s)", filter,
               vg_lite_get_filter_string(filter));

  GPU_PERF_PREPARE_START();
  VG_LITE_CHECK_ERROR(vg_lite_blit(
    VG_LITE_DEST_BUF,
    VG_LITE_SRC_BUF,
    &matrix,
    blend,
    0xffff0000,
    filter));
  GPU_PERF_PREPARE_STOP();

  GPU_PERF_RENDER_START();
  VG_LITE_CHECK_ERROR(vg_lite_finish());
  GPU_PERF_RENDER_STOP();

error_handler:

  snprintf(instructions_str, sizeof(instructions_str), "Transform: %s %s %s",
            tf_str,
            vg_lite_get_blend_string(blend),
            vg_lite_get_filter_string(filter));

  vg_lite_test_report_write(
    ctx,
    VG_LITE_SRC_BUF->tiled ? "image-tiled" : "image-linear",
    instructions_str,
    VG_LITE_IS_ERROR(error) ? vg_lite_get_error_type_string(error) : NULL,
    error == VG_LITE_SUCCESS);
  VG_LITE_FB_UPDATE();
  return error;
}

/****************************************************************************
 * Name: vg_lite_test_image_transform
 ****************************************************************************/

static vg_lite_error_t vg_lite_test_image_combination(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  static const vg_lite_blend_t blend_arr[] =
  {
    VG_LITE_BLEND_NONE,
    VG_LITE_BLEND_SRC_OVER,
  };

  static const vg_lite_filter_t filter_arr[] =
  {
    VG_LITE_FILTER_POINT,
    VG_LITE_FILTER_LINEAR,
    VG_LITE_FILTER_BI_LINEAR,
  };

  static const int rotation[] =
    {
      0,
      45,
      89,
      90,
      180,
    };

  for (int b = 0; b < GPU_ARRAY_SIZE(blend_arr); b++)
    {
      for (int f = 0; f < GPU_ARRAY_SIZE(filter_arr); f++)
        {
          for (int tf = IMG_TRANSFORM_NONE; tf < IMG_TRANSFORM_MAX; tf++)
            {
              if (tf == IMG_TRANSFORM_ROTATE)
                {
                  for (int r = 0; r < GPU_ARRAY_SIZE(rotation); r++)
                    {
                      VG_LITE_CHECK_ERROR(vg_lite_test_image_transform(
                        ctx,
                        blend_arr[b], filter_arr[f], tf, rotation[r]));
                    }
                }
              else
                {
                  VG_LITE_CHECK_ERROR(vg_lite_test_image_transform(
                      ctx,
                      blend_arr[b], filter_arr[f], tf, 0));
                }
            }
        }
    }

error_handler:
  return error;
}

/****************************************************************************
 * Name: create_index1
 ****************************************************************************/

static void create_index1(vg_lite_buffer_t *buffer)
{
  uint32_t i;
  uint32_t block = 16;
  uint8_t *p = (uint8_t *)buffer->memory;
  const uint8_t values[2] = {
    0xff, 0x00
  };

  for (i = 0; i < buffer->height; i++)
    {
      memset(p, values[(i / block) % 2], buffer->stride);
      p += buffer->stride;
    }
}

/****************************************************************************
 * Name: create_index2
 ****************************************************************************/

static void create_index2(vg_lite_buffer_t *buffer)
{
  uint32_t i;
  uint32_t block = 16;
  uint8_t *p = (uint8_t *)buffer->memory;
  const uint8_t values[] = {
    0x00, 0x55, 0xaa, 0xff
  };

  for (i = 0; i < buffer->height; i++)
    {
      memset(p, values[(i / block) % 4], buffer->stride);
      p += buffer->stride;
    }
}

/****************************************************************************
 * Name: create_index4
 ****************************************************************************/

static void create_index4(vg_lite_buffer_t *buffer)
{
  uint32_t i;
  uint32_t block = 16;
  uint8_t *p = (uint8_t *)buffer->memory;
  const uint8_t values[] = {
    0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb,
    0xcc, 0xdd, 0xee, 0xff
  };

  for (i = 0; i < buffer->height; i++)
    {
      memset(p, values[(i / block) % 16], buffer->stride);
      p += buffer->stride;
    }
}

/****************************************************************************
 * Name: create_index8
 ****************************************************************************/

static void create_index8(vg_lite_buffer_t *buffer)
{
  uint32_t i;
  uint32_t block = 1;
  uint8_t *p = (uint8_t *)buffer->memory;

  for (i = 0; i < buffer->height; i++)
    {
      memset(p, (i / block) % 256, buffer->stride);
      p += buffer->stride;
    }
}

/****************************************************************************
 * Name: create_index8
 ****************************************************************************/

static void create_index_image(vg_lite_buffer_t *buffer)
{
  switch (buffer->format)
    {
    case VG_LITE_INDEX_8:
        create_index8(buffer);
        break;

    case VG_LITE_INDEX_4:
        create_index4(buffer);
        break;

    case VG_LITE_INDEX_2:
        create_index2(buffer);
        break;

    case VG_LITE_INDEX_1:
        create_index1(buffer);
        break;

    default:
        break;
    }
}

/****************************************************************************
 * Name: create_index_table
 ****************************************************************************/

static void create_index_table(uint32_t *colors)
{
  int32_t i = 0;

  colors[0] = 0xff000000;
  colors[1] = 0xffffffff;
  colors[2] = 0xffff0000;
  colors[3] = 0xff00ff00;
  colors[4] = 0xff0000ff;
  colors[5] = 0xffffff00;
  colors[6] = 0xffff00ff;
  colors[7] = 0xff00ffff;
  colors[15] = 0xff000000;
  colors[14] = 0xffffffff;
  colors[13] = 0xffff0000;
  colors[12] = 0xff00ff00;
  colors[11] = 0xff0000ff;
  colors[10] = 0xffffff00;
  colors[9] = 0xffff00ff;
  colors[8] = 0xff00ffff;

  for (i = 16; i < 256; i++)
    {
      colors[i] = colors[i % 16];
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: vg_lite_test_image_bgra8888
 ****************************************************************************/

vg_lite_error_t vg_lite_test_image_bgra8888(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;
  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image, IMG_WIDTH, IMG_HEIGHT, VG_LITE_BGRA8888, false));

  /* Draw image */

  for (int y = 0; y < image->height; y++)
    {
      uint8_t alpha = y * 0xff / image->height;
      gpu_color32_t *dst = image->memory + y * image->stride;

      for (int x = 0; x < image->width; x++)
        {
          dst->ch.alpha = alpha;
          dst->ch.red = 0xff;
          dst->ch.red = dst->ch.red * alpha / 0xff;
          dst->ch.green = dst->ch.green * alpha / 0xff;
          dst->ch.blue = dst->ch.blue * alpha / 0xff;
          dst++;
        }
    }

  VG_LITE_CHECK_ERROR(
    vg_lite_test_image_combination(ctx));

error_handler:
  vg_lite_delete_image(image);

  return error;
}

/****************************************************************************
 * Name: vg_lite_test_image_indexed1
 ****************************************************************************/

vg_lite_error_t vg_lite_test_image_indexed1(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;
  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image, IMG_WIDTH, IMG_HEIGHT, VG_LITE_INDEX_1, false));

  uint32_t colors[256];
  create_index_table(colors);
  IMG_SET_CLUT(colors, 2);

  create_index_image(image);

  VG_LITE_CHECK_ERROR(
    vg_lite_test_image_combination(ctx));

error_handler:
  vg_lite_delete_image(image);

  return error;
}

/****************************************************************************
 * Name: vg_lite_test_image_indexed2
 ****************************************************************************/

vg_lite_error_t vg_lite_test_image_indexed2(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;
  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image, IMG_WIDTH, IMG_HEIGHT, VG_LITE_INDEX_2, false));

  uint32_t colors[256];
  create_index_table(colors);
  IMG_SET_CLUT(colors, 4);

  create_index_image(image);

  VG_LITE_CHECK_ERROR(
    vg_lite_test_image_combination(ctx));

error_handler:
  vg_lite_delete_image(image);

  return error;
}

/****************************************************************************
 * Name: vg_lite_test_image_indexed4
 ****************************************************************************/

vg_lite_error_t vg_lite_test_image_indexed4(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;
  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image, IMG_WIDTH, IMG_HEIGHT, VG_LITE_INDEX_4, false));

  uint32_t colors[256];
  create_index_table(colors);
  IMG_SET_CLUT(colors, 16);

  create_index_image(image);

  VG_LITE_CHECK_ERROR(
    vg_lite_test_image_combination(ctx));

error_handler:
  vg_lite_delete_image(image);

  return error;
}

/****************************************************************************
 * Name: vg_lite_test_image_indexed8
 ****************************************************************************/

vg_lite_error_t vg_lite_test_image_indexed8(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;
  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image, IMG_WIDTH, IMG_HEIGHT, VG_LITE_INDEX_8, false));

  uint32_t colors[256];
  create_index_table(colors);
  IMG_SET_CLUT(colors, 256);

  create_index_image(image);

  VG_LITE_CHECK_ERROR(
    vg_lite_test_image_combination(ctx));

error_handler:
  vg_lite_delete_image(image);

  return error;
}

/****************************************************************************
 * Name: vg_lite_test_image_alpha8
 ****************************************************************************/

vg_lite_error_t vg_lite_test_image_alpha8(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;

  int src_stride = 100;

  /* align to 16px */

  int dst_width = 112;

  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image, dst_width, 100, VG_LITE_A8, false));

  image->image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;

  const uint8_t *src = (uint8_t *)image_a8_100x100;
  uint8_t *dst = image->memory;

  for (int y = 0; y < image->height; y++)
    {
      memcpy(dst, src, src_stride);
      src += src_stride;
      dst += image->stride;
    }

  vg_lite_matrix_t matrix;
  vg_lite_identity(&matrix);
  vg_lite_translate(100, 100, &matrix);

  VG_LITE_CHECK_ERROR(
    vg_lite_test_image_combination(ctx));

error_handler:
  vg_lite_delete_image(image);

  return error;
}

/****************************************************************************
 * Name: vg_lite_test_image_bgra5658
 ****************************************************************************/

vg_lite_error_t vg_lite_test_image_bgra5658(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;

  /* Prepare image */

  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;
  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image, IMG_WIDTH, IMG_HEIGHT, VG_LITE_BGRA5658, false));

  /* Generate image */

  for (int y = 0; y < image->height; y++)
    {
      uint8_t alpha = y * 0xff / image->height;
      gpu_color16_alpha_t *dst = image->memory + y * image->stride;

      for (int x = 0; x < image->width; x++)
        {
          dst->alpha = alpha;
          dst->color.ch.red = 31;
          dst->color.ch.red = dst->color.ch.red * alpha / 0xff;
          dst->color.ch.green = dst->color.ch.green * alpha / 0xff;
          dst->color.ch.blue = dst->color.ch.blue * alpha / 0xff;
          dst++;
        }
    }

  VG_LITE_CHECK_ERROR(
    vg_lite_test_image_combination(ctx));

error_handler:
  vg_lite_delete_image(image);

  return error;
}

/****************************************************************************
 * Name: vg_lite_test_image_bgra8888_tiled
 ****************************************************************************/

vg_lite_error_t vg_lite_test_image_bgra8888_tiled(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;
  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image, IMG_WIDTH, IMG_HEIGHT, VG_LITE_BGRA8888, true));

  /* Draw image */

  for (int y = 0; y < image->height; y++)
    {
      uint8_t alpha = y * 0xff / image->height;
      gpu_color32_t *dst = image->memory + y * image->stride;

      for (int x = 0; x < image->width; x++)
        {
          dst->ch.alpha = alpha;
          dst->ch.red = 0xff;
          dst->ch.red = dst->ch.red * alpha / 0xff;
          dst->ch.green = dst->ch.green * alpha / 0xff;
          dst->ch.blue = dst->ch.blue * alpha / 0xff;
          dst++;
        }
    }

  VG_LITE_CHECK_ERROR(
    vg_lite_test_image_combination(ctx));

error_handler:
  vg_lite_delete_image(image);
  return error;
}

/****************************************************************************
 * Name: vg_lite_test_image_indexed8_tiled
 ****************************************************************************/

vg_lite_error_t vg_lite_test_image_indexed8_tiled(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;
  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image, IMG_WIDTH, IMG_HEIGHT, VG_LITE_INDEX_8, true));

  image->tiled = VG_LITE_TILED;

  VG_LITE_CHECK_ERROR(
    vg_lite_test_image_combination(ctx));

error_handler:
  vg_lite_delete_image(image);
  return error;
}

/****************************************************************************
 * Name: vg_lite_test_image_bgra5658_tiled
 ****************************************************************************/

vg_lite_error_t vg_lite_test_image_bgra5658_tiled(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;

  /* Prepare image */

  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;
  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image, IMG_WIDTH, IMG_HEIGHT, VG_LITE_BGRA5658, true));

  image->tiled = VG_LITE_TILED;

  /* Generate image */

  for (int y = 0; y < image->height; y++)
    {
      uint8_t alpha = y * 0xff / image->height;
      gpu_color16_alpha_t *dst = image->memory + y * image->stride;

      for (int x = 0; x < image->width; x++)
        {
          dst->alpha = alpha;
          dst->color.ch.red = 31;
          dst->color.ch.red = dst->color.ch.red * alpha / 0xff;
          dst->color.ch.green = dst->color.ch.green * alpha / 0xff;
          dst->color.ch.blue = dst->color.ch.blue * alpha / 0xff;
          dst++;
        }
    }

  VG_LITE_CHECK_ERROR(
    vg_lite_test_image_combination(ctx));

error_handler:
  vg_lite_delete_image(image);
  return error;
}

/****************************************************************************
 * Name: vg_lite_test_image_draw_pattern
 ****************************************************************************/

vg_lite_error_t vg_lite_test_image_draw_pattern(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;
  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image, ctx->xres, 40, VG_LITE_INDEX_8, false));

  /* Prepare clip path */

  int32_t path_data[VG_LITE_RECT_PATH_LEN_MAX];
  struct vg_lite_area_s area =
    {
      0, 0, ctx->xres - 1, ctx->yres - 1
    };

  vg_lite_path_t path;

  int path_len = vg_lite_fill_round_rect_path(path_data, &area, 0);
  vg_lite_init_path(
    &path,
    VG_LITE_S32,
    VG_LITE_HIGH,
    path_len * sizeof(int32_t),
    path_data,
    0, 0,
    VG_LITE_FB_WIDTH, VG_LITE_FB_HEIGHT);

  vg_lite_matrix_t src_matrix;
  vg_lite_identity(&src_matrix);

  vg_lite_matrix_t matrix;
  vg_lite_identity(&matrix);
  vg_lite_translate(ctx->xres / 2, ctx->yres / 2, &matrix);

  for (int i = 0; i < 360 * 2; i++)
    {
      vg_lite_rotate(1, &matrix);

      /* clear fb */

      VG_LITE_CHECK_ERROR(vg_lite_clear(VG_LITE_DEST_BUF, NULL, 0xffffffff));

      /* Draw iamge */

      VG_LITE_CHECK_ERROR(
        vg_lite_draw_pattern(
        VG_LITE_DEST_BUF,
        &path,
        VG_LITE_FILL_EVEN_ODD,
        &src_matrix,
        image,
        &matrix,
        VG_LITE_BLEND_SRC_OVER,
        VG_LITE_PATTERN_COLOR,
        0,
        0,
        VG_LITE_FILTER_BI_LINEAR));

      VG_LITE_CHECK_ERROR(vg_lite_finish());
      gpu_delay(10);
    }

error_handler:
  vg_lite_delete_image(image);
  return error;
}
