/****************************************************************************
 * apps/testing/gpu/vg_lite/vg_lite_test_blit.c
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
#include "resource/image_rgb_565.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

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
 * Name: vg_lite_test_create_rect_path
 ****************************************************************************/

static vg_lite_error_t vg_lite_test_create_rect_path(
  struct gpu_test_context_s *ctx,
  vg_lite_path_t *path,
  int32_t x, int32_t y, int32_t w, int32_t h)
{
  vg_lite_error_t error      = VG_LITE_SUCCESS;
  const int16_t   tmp_data[] =
    {
      VLC_OP_MOVE, x, y,
      VLC_OP_LINE, x, (y + h),
      VLC_OP_LINE, (x + w), (y + h),
      VLC_OP_LINE, (x + w), y,
      VLC_OP_LINE, x, y,
      VLC_OP_END
    };

  int  path_data_size = sizeof(tmp_data);
  void *path_data     = malloc(path_data_size * sizeof(int16_t));

  memcpy(path_data, tmp_data, sizeof(tmp_data));

  VG_LITE_CHECK_ERROR(vg_lite_init_path(
    path, VG_LITE_S16, VG_LITE_HIGH, path_data_size, path_data, 0.0f, 0.0f,
    VG_LITE_FB_WIDTH,
    VG_LITE_FB_HEIGHT
  ));

  error_handler:
  return error;
}

/****************************************************************************
 * Name: vg_lite_test_destroy_rect_path
 ****************************************************************************/

static void vg_lite_test_destroy_rect_path(vg_lite_path_t *path)
{
  free(path->path);
}

/****************************************************************************
 * Name: vg_lite_test_image_pattern_draw
 ****************************************************************************/

static vg_lite_error_t vg_lite_test_image_pattern_draw(
  struct gpu_test_context_s *ctx,
  vg_lite_blend_t blend,
  vg_lite_filter_t filter,
  int rotation, int pivot_x, int pivot_y,
  int src_x, int src_y, int src_w, int src_h,
  int dst_x, int dst_y)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;

  vg_lite_matrix_t matrix;
  matrix = vg_lite_init_matrix(
    dst_x, dst_y,
    rotation, 1.0, pivot_x, pivot_y
  );

  vg_lite_path_t vg_path;
  VG_LITE_CHECK_ERROR(
    vg_lite_test_create_rect_path(
      ctx, &vg_path, src_x, src_y, src_w, src_h
    ));

  GPU_PERF_PREPARE_START();
  VG_LITE_CHECK_ERROR(vg_lite_draw_pattern(
    VG_LITE_DEST_BUF,
    &vg_path,
    VG_LITE_FILL_EVEN_ODD,
    &matrix,
    VG_LITE_SRC_BUF,
    &matrix,
    blend,
    VG_LITE_PATTERN_COLOR,
    0,
    0,
    filter
  ));
  GPU_PERF_PREPARE_STOP();

  GPU_PERF_RENDER_START();
  VG_LITE_CHECK_ERROR(vg_lite_finish());
  GPU_PERF_RENDER_STOP();

  vg_lite_test_destroy_rect_path(&vg_path);

  error_handler:
  VG_LITE_FB_UPDATE();
  return error;
}

/****************************************************************************
 * Name: check_buf_sanity
 ****************************************************************************/

static bool check_buf_sanity(const uint8_t *buf,
                             int32_t buf_w, int32_t buf_h,
                             int32_t x, int32_t y,
                             int32_t cont_w, int32_t cont_h)
{
  bool cmp_res = true;

  for (int row = 0; row < buf_h; ++row)
    {
      for (int col = 0; col < buf_w; ++col)
        {
          uint16_t val = ((uint16_t *)buf)[row * buf_w + col];
          if (x <= col && col < x + cont_w &&
              y <= row && row < y + cont_h)
            {
              if (val == 0x5a5a)
                {
                  GPU_LOG_ERROR("row: %d, col: %d, val: %d",
                                row, col, val);
                  cmp_res = false;
                }

              continue;
            }

          if (val != 0x5a5a)
            {
              GPU_LOG_ERROR("row: %d, col: %d, val: %d",
                            row, col, val);
              cmp_res = false;
            }
        }
    }

  return cmp_res;
}

/****************************************************************************
 * Name: img_expand_stride_inplace
 ****************************************************************************/

static void img_expand_stride_inplace(uint8_t *buf, int32_t img_w,
                                      int32_t img_h,
                                      int32_t stride)
{
  stride /= 2;
  for (int row = img_h - 1; row >= 0; --row)
    {
      for (int col = img_w - 1; col >= 0; --col)
        {
          uint16_t val = ((uint16_t *)buf)[row * img_w + col];
          ((uint16_t *)buf)[row * stride + col] = val;
        }

      memset(
        &((uint16_t *)buf)[row * stride + img_w],
        0x5a, 2 * (stride - img_w)
      );
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: vg_lite_test_align_buf_notalign_stride
 ****************************************************************************/

vg_lite_error_t vg_lite_test_align_buf_notalign_stride(
  struct gpu_test_context_s *ctx)
{
  vg_lite_error_t  error;
  vg_lite_buffer_t ins_buf;

  vg_lite_buffer_t *image      = VG_LITE_SRC_BUF;
  vg_lite_buffer_t ori_src_buf;
  vg_lite_buffer_t ori_dst_buf = *VG_LITE_DEST_BUF;

  const int img_w     = 128;
  const int img_h     = 128;
  const int dst_buf_w = 150;
  const int dst_buf_h = 192;

  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    &ins_buf,
    dst_buf_w, dst_buf_h,
    VG_LITE_RGB565, false
  ));

  int dst_buf_ori_stride = ins_buf.stride;

  VG_LITE_CHECK_ERROR(vg_lite_create_image(
    image,
    img_w, img_h, VG_LITE_RGB565, false
  ));

  /* copy image content */

  memcpy(image->memory, image_rgb_565_128x128, img_w * img_h * 2);
  ori_src_buf = *VG_LITE_SRC_BUF;

  for (int test_w = 1; test_w < img_w; test_w++)
    {
      for (int test_h = 1; test_h < img_h; test_h++)
        {
          /* blit image to dst buf */

          memset(ins_buf.memory, 0x5a, dst_buf_ori_stride * dst_buf_h);
          ins_buf.stride = dst_buf_w * 2;
          *VG_LITE_SRC_BUF  = ori_src_buf;
          *VG_LITE_DEST_BUF = ins_buf;

          int test_dst_x = test_w % 30;
          int test_dst_y = test_h % 30;

          VG_LITE_CHECK_ERROR(vg_lite_test_image_pattern_draw(
            ctx, VG_LITE_BLEND_SRC_OVER, VG_LITE_FILTER_LINEAR,
            0, 0, 0,
            0, 0, test_w, test_h,
            test_dst_x, test_dst_y
          ));

          if (!check_buf_sanity(
            ins_buf.memory, dst_buf_w, dst_buf_h,
            test_dst_x, test_dst_y, test_w, test_h
          ))
            {
              GPU_LOG_ERROR("check blit sanity failed");
            }

          /* visualization */

          img_expand_stride_inplace(
            ins_buf.memory, dst_buf_w, dst_buf_h,
            dst_buf_ori_stride
          );

          /* restore img buf */

          ins_buf.stride = dst_buf_ori_stride;
          *VG_LITE_SRC_BUF  = ins_buf;
          *VG_LITE_DEST_BUF = ori_dst_buf;

          VG_LITE_CHECK_ERROR(vg_lite_test_image_pattern_draw(
            ctx, VG_LITE_BLEND_SRC_OVER, VG_LITE_FILTER_LINEAR,
            0, 0, 0,
            0, 0, dst_buf_w, dst_buf_h,
            0, 0
          ));
        }
    }

error_handler:
  vg_lite_delete_image(image);

  return error;
}
