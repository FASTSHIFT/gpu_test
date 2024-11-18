/****************************************************************************
 * apps/testing/gpu/vg_lite/vg_lite_test.c
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
#include "vg_lite_test_case.h"
#include <stdlib.h>

#define GC(X) gc##X
#define gcFEATURE_BIT_VG_NONE -1

typedef vg_lite_error_t (*vg_lite_test_func_t)(
  struct gpu_test_context_s *ctx);

struct vg_lite_test_item_s
  {
    const char *name;
    vg_lite_test_func_t func;
    int feature;
  };

/* Import testcase entry function */

#define ITEM_DEF(NAME, FEATURE) \
  vg_lite_error_t vg_lite_test_##NAME(struct gpu_test_context_s *ctx);
#include "vg_lite_test_case.inc"
#undef ITEM_DEF

/* Generate testcase group */

#define ITEM_DEF(NAME, FEATURE) \
  {#NAME, vg_lite_test_##NAME, gcFEATURE_BIT_VG_##FEATURE},
static const struct vg_lite_test_item_s g_vg_lite_test_group[] =
{
#include "vg_lite_test_case.inc"
};
#undef ITEM_DEF

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

enum test_case_e
  {
    TEST_CASE_DEFAULT      = 0,
    TEST_CASE_ROTATION     = 1,
    TEST_CASE_ROTATION_RGB = 2,
    TEST_CASE_ALIGN_BUF_NOTALIGN_STRIDE = 3,
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
 * Name: vg_lite_test_run_item
 ****************************************************************************/

static bool vg_lite_test_run_item(struct gpu_test_context_s *ctx,
                                  const struct vg_lite_test_item_s *item)
{
  vg_lite_error_t error;
  bool            is_passed = false;

  if (item->feature != GC(FEATURE_BIT_VG_NONE) &&
     !vg_lite_query_feature(item->feature))
    {
      GPU_LOG_INFO("Testcase [%s] -> SKIP: Require feature: %d (%s)",
                   item->name,
                   item->feature,
                   vg_lite_get_feature_string(item->feature));
      return true;
    }

  GPU_PERF_RESET();
  VG_LITE_CHECK_ERROR(vg_lite_clear(VG_LITE_DEST_BUF, NULL, 0));
  VG_LITE_CHECK_ERROR(vg_lite_finish());

  vg_lite_buffer_init(VG_LITE_SRC_BUF);

  GPU_LOG_WARN("Testcase [%s] Running...", item->name);

  error     = item->func(ctx);
  is_passed = (error == VG_LITE_SUCCESS);

  GPU_LOG_WARN("Testcase [%s] -> %s",
               item->name,
               is_passed ? "PASS" : "FAIL");

error_handler:
  gpu_screenshot(ctx, item->name);

  vg_lite_test_report_write(
    ctx,
    item->name,
    NULL,
    is_passed ? NULL : vg_lite_get_error_type_string(error),
    is_passed
  );

  VG_LITE_FB_UPDATE();

  return is_passed;
}

/****************************************************************************
 * Name: stress_test_get_next
 ****************************************************************************/

static int stress_test_get_next(struct gpu_test_context_s *ctx,
                                int current)
{
  if (ctx->param.mode == GPU_TEST_MODE_STRESS_RANDOM)
    {
      current = rand();
    }
  else
    {
      current++;
    }

  return current % GPU_ARRAY_SIZE(g_vg_lite_test_group);
}

/****************************************************************************
 * Name: vg_lite_test_run_stress
 ****************************************************************************/

static void vg_lite_test_run_stress(struct gpu_test_context_s *ctx)
{
  int loop_cnt = 0;
  int index    = 0;

  while (1)
    {
      index = stress_test_get_next(ctx, index);
      const struct vg_lite_test_item_s *item = &g_vg_lite_test_group[index];
      if (!vg_lite_test_run_item(ctx, item))
        {
          GPU_LOG_ERROR("Stress Test Loop %d FAILED", loop_cnt);
          return;
        }

      loop_cnt++;
    }
}

/****************************************************************************
 * Name: vg_lite_test_run
 ****************************************************************************/

static void vg_lite_test_run(struct gpu_test_context_s *ctx)
{
  int index;

  if (ctx->param.mode == GPU_TEST_MODE_STRESS ||
      ctx->param.mode == GPU_TEST_MODE_STRESS_RANDOM)
    {
      srand(time(NULL));
      vg_lite_test_run_stress(ctx);
      return;
    }

  vg_lite_test_report_start(ctx);

  for (index = 0; index < GPU_ARRAY_SIZE(g_vg_lite_test_group); index++)
    {
      const struct vg_lite_test_item_s *item = &g_vg_lite_test_group[index];
      vg_lite_test_run_item(ctx, item);
    }

  vg_lite_test_report_finish(ctx);

  GPU_LOG_WARN("Test finish");
}

/****************************************************************************
 * Name: vg_lite_dump_info
 ****************************************************************************/

static void vg_lite_dump_info(void)
{
  char     name[64];
  uint32_t chip_id;
  uint32_t chip_rev;
  uint32_t cid;

  vg_lite_get_product_info(name, &chip_id, &chip_rev);
  vg_lite_get_register(0x30, &cid);
  GPU_LOG_INFO("Product Info: %s"
               " | Chip ID: 0x%" PRIx32
               " | Revision: 0x%" PRIx32
               " | CID: 0x%" PRIx32,
               name, chip_id, chip_rev, cid);

  vg_lite_info_t info;
  vg_lite_get_info(&info);
  GPU_LOG_INFO("VGLite API version: 0x%" PRIx32, info.api_version);
  GPU_LOG_INFO("VGLite API header version: 0x%" PRIx32,
               info.header_version);
  GPU_LOG_INFO("VGLite release version: 0x%" PRIx32, info.release_version);

  for (int feature = 0; feature < GC(FEATURE_COUNT); feature++)
    {
      const char *feature_string  = vg_lite_get_feature_string(feature);
      const char *feature_support = vg_lite_query_feature(
        (vg_lite_feature_t)feature
      ) ? "YES" : "NO";

      if (feature_string)
        {
          GPU_LOG_INFO("Feature: %s - %s", feature_string, feature_support);
        }
      else
        {
          GPU_LOG_INFO("Feature: %d - %s", feature, feature_support);
        }
    }

  uint32_t mem_avail;
  vg_lite_mem_avail(&mem_avail);
  GPU_LOG_INFO("Memory Avaliable: %" PRId32 " Bytes", mem_avail);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: vg_lite_test
 ****************************************************************************/

int vg_lite_test(struct gpu_test_context_s *ctx)
{
  vg_lite_error_t error = VG_LITE_SUCCESS;

  GPU_LOG_INFO("GPU init...");

#ifdef CONFIG_TESTING_GPU_VG_LITE_CUSTOM_INIT
  extern void gpu_init(void);

  static bool is_init = false;

  if (!is_init)
    {
      gpu_init();
      is_init = true;
    }

#endif

  vg_lite_dump_info();

  struct vg_lite_test_ctx_s vg_ctx;

  memset(&vg_ctx, 0, sizeof(vg_ctx));
  ctx->user_data = &vg_ctx;

  vg_lite_buffer_t *buffer = &vg_ctx.dest_buffer;

#if VG_LITE_USE_DIRECT_MODE
  VG_LITE_CHECK_ERROR(vg_lite_init_custom_buffer(
    buffer,
    ctx->fbmem,
    ctx->xres,
    ctx->yres,
    vg_lite_bpp_to_format(ctx->bpp)
  ));
#else
  vg_lite_buffer_init(buffer);
  buffer->width = ctx->xres;
  buffer->height = ctx->yres;
  buffer->format = VG_LITE_BGRX8888;
  VG_LITE_CHECK_ERROR(vg_lite_allocate(buffer));
#endif

  vg_lite_dump_buffer_info(__func__, buffer);

  if (vg_lite_query_feature(GC(FEATURE_BIT_VG_SCISSOR)))
    {
      vg_lite_enable_scissor();
      vg_lite_set_scissor(0, 0, buffer->width, buffer->height);
      GPU_LOG_INFO("Scissor enabled");
    }

  switch (ctx->param.test_case)
    {
      case TEST_CASE_DEFAULT:
        vg_lite_test_run(ctx);
        break;
      case TEST_CASE_ROTATION:
        vg_lite_test_rotation(ctx);
        break;
      case TEST_CASE_ROTATION_RGB:
        vg_lite_test_rotation_rgb(ctx);
        break;
      case TEST_CASE_ALIGN_BUF_NOTALIGN_STRIDE:
        vg_lite_test_align_buf_notalign_stride(ctx);
        break;
      default:
        GPU_LOG_ERROR("Invalid test case: %d", ctx->param.test_case);
        break;
    }

error_handler:

  if (buffer->handle)
    {
      vg_lite_free(buffer);
    }

  ctx->user_data = NULL;
  GPU_LOG_INFO("GPU test finish");

  return error == VG_LITE_SUCCESS ? 0 : -1;
}
