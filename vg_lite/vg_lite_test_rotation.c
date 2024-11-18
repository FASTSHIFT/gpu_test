/****************************************************************************
 * apps/testing/gpu/vg_lite/vg_lite_test_rotation.c
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
#include "resource/image_yuyv_tiled.h"
#include "resource/image_rgb.h"

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
 * Name: vg_lite_test_image_transform
 ****************************************************************************/

static vg_lite_error_t vg_lite_test_image_transform(
  struct gpu_test_context_s *ctx,
  vg_lite_blend_t blend,
  vg_lite_filter_t filter,
  int rotation, int pivot_x, int pivot_y)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;

  vg_lite_matrix_t matrix;
  matrix =
    vg_lite_init_matrix(240 - 24, 0, rotation, 1.0, pivot_x, pivot_y);

  VG_LITE_CHECK_ERROR(vg_lite_blit(
    VG_LITE_DEST_BUF,
    VG_LITE_SRC_BUF,
    &matrix,
    blend,
    0,
    filter
  ));
  GPU_PERF_PREPARE_STOP();

  GPU_PERF_RENDER_START();
  VG_LITE_CHECK_ERROR(vg_lite_finish());
  GPU_PERF_RENDER_STOP();

error_handler:
  VG_LITE_FB_UPDATE();
  return error;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

vg_lite_error_t vg_lite_test_rotation(struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;
  vg_lite_buffer_t *image = VG_LITE_SRC_BUF;
  error = vg_lite_create_image(image, 48, 480, VG_LITE_YUY2_TILED, true);

  if (error != VG_LITE_SUCCESS)
    {
      GPU_LOG_ERROR("Execute vg_lite_create_image error(%d): %s",
                    (int)error,
                    vg_lite_get_error_type_string(error));
      return error;
    }

  /* copy image content */

  memcpy(image->memory, image_yuyv_tiled_48x480, 48 * 480 * 2);

  int i     = 10 * 1000;
  int angle = 0;
  while (i--)
    {
      vg_lite_test_image_transform(
        ctx, VG_LITE_BLEND_SRC_OVER,
        VG_LITE_FILTER_LINEAR, angle, 24, 240
      );
      angle++;
      if (angle > 360)
        {
          angle = 0;
        }

      usleep(1000);
    }

  vg_lite_delete_image(image);
  return error;
}

vg_lite_error_t vg_lite_test_rotation_rgb(struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error    = VG_LITE_SUCCESS;
  vg_lite_buffer_t * image = VG_LITE_SRC_BUF;
  error = vg_lite_create_image(image, 48, 480, VG_LITE_BGRA8888, false);

  if (error != VG_LITE_SUCCESS)
    {
      GPU_LOG_ERROR("Execute vg_lite_create_image error(%d): %s",
                    (int)error,
                    vg_lite_get_error_type_string(error));
      return error;
    }

  /* copy image content */

  memcpy(image->memory, image_rgb_48x480, 48 * 480 * 4);

  int i     = 10 * 1000;
  int angle = 0;
  while (i--)
    {
      vg_lite_test_image_transform(
        ctx, VG_LITE_BLEND_SRC_OVER,
        VG_LITE_FILTER_LINEAR, angle, 24, 240
      );
      angle++;
      if (angle > 360)
        {
          angle = 0;
        }

      usleep(1000);
    }

  vg_lite_delete_image(image);
  return error;
}
