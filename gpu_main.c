/****************************************************************************
 * apps/testing/gpu/gpu_main.c
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

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#include "gpu_test.h"

/****************************************************************************
 * Preprocessor Definitions
 ****************************************************************************/

#define GPU_PREFIX "GPU: "

#define OPTARG_TO_VALUE(value, type, base)                             \
  do                                                                   \
  {                                                                    \
    char *ptr;                                                     \
    value = (type)strtoul(optarg, &ptr, base);                         \
    if (*ptr != '\0')                                                  \
      {                                                                \
        printf(GPU_PREFIX "Parameter error: -%c %s\n", ch, optarg); \
        show_usage(argv[0], EXIT_FAILURE);                             \
      }                                                                \
  } while (0)

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: show_usage
 ****************************************************************************/

static void show_usage(const char *progname, int exitcode)
{
  printf("\nUsage: %s"
         " -o <string> -m <string> -i <string> -s\n",
         progname);
  printf("\nWhere:\n");
  printf("  -o <string> GPU report file output path.\n");
  printf("  -m <string> Test mode: default; random; stress.\n");
  printf("  -i <string> Test image size(px): "
         "<decimal-value width>x<decimal-value height>\n");
  printf("  -s Enable screenshot\n");

  exit(exitcode);
}

/****************************************************************************
 * Name: gpu_test_string_to_mode
 ****************************************************************************/

static enum gpu_test_mode_e gpu_test_string_to_mode(const char *str)
{
  if (strcmp(str, "default") == 0)
    {
      return GPU_TEST_MODE_DEFAULT;
    }
  else if (strcmp(str, "random") == 0)
    {
      return GPU_TEST_MODE_RANDOM;
    }
  else if (strcmp(str, "stress") == 0)
    {
      return GPU_TEST_MODE_STRESS;
    }
  else if (strcmp(str, "stress_random") == 0)
    {
      return GPU_TEST_MODE_STRESS_RANDOM;
    }

  printf(GPU_PREFIX "Unknown mode: %s\n", str);

  return GPU_TEST_MODE_DEFAULT;
}

/****************************************************************************
 * Name: parse_commandline
 ****************************************************************************/

static void parse_commandline(int argc, char **argv,
                              struct gpu_test_param_s *param)
{
  int ch;
  int converted;

  /* set default param */

  memset(param, 0, sizeof(struct gpu_test_param_s));
  param->output_dir = "/data/gpu";
  param->mode = GPU_TEST_MODE_DEFAULT;
  param->img_width = 128;
  param->img_height = 128;

  while ((ch = getopt(argc, argv, "ho:m:i:sc:")) != -1)
    {
      switch (ch)
        {
          case 'o':
            param->output_dir = optarg;
            break;

          case 'm':
            param->mode = gpu_test_string_to_mode(optarg);
            break;

          case 'i':
          {
            int width;
            int height;
            converted = sscanf(optarg, "%dx%d", &width, &height);
            if (converted == 2 && width >= 0 && height >= 0)
              {
                param->img_width = width;
                param->img_height = height;
              }
            else
              {
                printf(GPU_PREFIX ": Error image size: %s\n", optarg);
                show_usage(argv[0], EXIT_FAILURE);
              }
            break;
          }

          case 's':
            param->screenshot_en = true;
            break;

          case 'c':
            param->test_case = atoi(optarg);
            break;

          case '?':
            printf(GPU_PREFIX ": Unknown option: %c\n", optopt);
          case 'h':
            show_usage(argv[0], EXIT_FAILURE);
            break;
        }
    }

  printf(GPU_PREFIX "Output DIR: %s\n", param->output_dir);
  printf(GPU_PREFIX "Test mode: %d\n", param->mode);
  printf(GPU_PREFIX "Image size: %dx%d\n",
         param->img_width,
         param->img_height);
  printf(GPU_PREFIX "Screenshot: %s\n",
         param->screenshot_en ? "enable" : "disable");
}

/****************************************************************************
 * Name: gpu_fb_init
 ****************************************************************************/

static int gpu_fb_init(struct fb_state_s *state, const char *path)
{
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: gpu_main
 ****************************************************************************/

int main(int argc, char *argv[])
{
  struct gpu_test_context_s ctx;
  int ret;

  memset(&ctx, 0, sizeof(ctx));
  parse_commandline(argc, argv, &ctx.param);

  gpu_dir_create(ctx.param.output_dir);

  gpu_test_run(&ctx);

  printf(GPU_PREFIX "Test finished\n");
  return EXIT_SUCCESS;
}
