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

#include "gpu_context.h"
#include "gpu_log.h"
#include "gpu_test.h"
#include "gpu_utils.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/

#ifndef GPU_OUTPUT_DIR_DEFAULT
#define GPU_OUTPUT_DIR_DEFAULT "./gpu"
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void parse_commandline(int argc, char** argv, struct gpu_test_param_s* param);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, char* argv[])
{
    struct gpu_test_context_s ctx = { 0 };
    parse_commandline(argc, argv, &ctx.param);

    if (gpu_dir_create(ctx.param.output_dir) != 0) {
        return EXIT_FAILURE;
    }

    if (!gpu_test_context_setup(&ctx)) {
        GPU_LOG_ERROR("Failed to setup test context");
        return EXIT_FAILURE;
    }

    int retval = gpu_test_run(&ctx);
    gpu_test_context_teardown(&ctx);

    if (retval == 0) {
        GPU_LOG_WARN("GPU Test PASSED");
    } else {
        GPU_LOG_ERROR("GPU Test FAILED: %d", retval);
    }

    return retval;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief Show usage of the program
 * @param progname The name of the program
 * @param exitcode The exit code of the program
 */
static void show_usage(const char* progname, int exitcode)
{
    printf("\nUsage: %s"
           " -m <string> -o <string> -t <string> -s\n"
           " --target <string> --loop-count <int> --cpu-freq <int> --fbdev <string> --tolerance <int>\n",
        progname);

    printf("\nWhere:\n");
    printf("  -m <string> Test mode: default; stress.\n");
    printf("  -o <string> GPU report file output path, default is " GPU_OUTPUT_DIR_DEFAULT "\n");
    printf("  -t <string> Testcase name.\n");
    printf("  -s Enable screenshot.\n");

    printf("  --target <string> Target render image size(px), default is 480x480. Example: "
           "<decimal-value width>x<decimal-value height>\n");
    printf("  --loop-count <int> Stress mode loop count, default is 10000.\n");
    printf("  --cpu-freq <int> CPU frequency in MHz, default is 0 (auto).\n");
    printf("  --fbdev <string> Framebuffer device path.\n");
    printf("  --tolerance <int> Color deviation tolerance, default is 1.\n");

    exit(exitcode);
}

/**
 * @brief Convert string to test mode
 * @param str The string to convert
 * @return The test mode
 */
static enum gpu_test_mode_e gpu_test_string_to_mode(const char* str)
{
#define GPU_TEST_MODE_NAME_MATCH(name, mode) \
    do {                                     \
        if (strcmp(str, name) == 0) {        \
            return mode;                     \
        }                                    \
    } while (0)

    GPU_TEST_MODE_NAME_MATCH("default", GPU_TEST_MODE_DEFAULT);
    GPU_TEST_MODE_NAME_MATCH("stress", GPU_TEST_MODE_STRESS);

#undef GPU_TEST_MODE_NAME_MATCH

    GPU_LOG_WARN("Unknown mode: %s, use default mode", str);
    return GPU_TEST_MODE_DEFAULT;
}

/**
 * @brief Parse long command line arguments
 * @param argc The number of arguments
 * @param argv The arguments
 * @param ch The option character
 * @param longopts The long options
 * @param longindex The index of the long option
 * @param param The test parameters
 */
static void parse_long_commandline(
    int argc, char** argv,
    int ch,
    const struct option* longopts,
    int longindex,
    struct gpu_test_param_s* param)
{
    switch (longindex) {

    case 0: {
        int width = 0;
        int height = 0;
        int converted = sscanf(optarg, "%dx%d", &width, &height);
        if (converted == 2 && width >= 0 && height >= 0) {
            param->target_width = width;
            param->target_height = height;
        } else {
            GPU_LOG_ERROR("Error target image size: %s", optarg);
            show_usage(argv[0], EXIT_FAILURE);
        }
    } break;

    case 1:
        param->run_loop_count = atoi(optarg);
        break;

    case 2:
        param->cpu_freq = atoi(optarg);
        break;

    case 3:
        param->fbdev_path = optarg;
        break;

    case 4:
        param->color_tolerance = atoi(optarg);
        if (param->color_tolerance < 0) {
            GPU_LOG_ERROR("Color deviation tolerance error: %d", param->color_tolerance);
            show_usage(argv[0], EXIT_FAILURE);
        }
        break;

    default:
        GPU_LOG_WARN("Unknown longindex: %d", longindex);
        show_usage(argv[0], EXIT_FAILURE);
        break;
    }
}

/**
 * @brief Parse command line arguments
 * @param argc The number of arguments
 * @param argv The arguments
 * @param param The test parameters
 */
static void parse_commandline(int argc, char** argv, struct gpu_test_param_s* param)
{
    /* set default param */
    memset(param, 0, sizeof(struct gpu_test_param_s));
    param->argc = argc;
    param->argv = argv;
    param->mode = GPU_TEST_MODE_DEFAULT;
    param->output_dir = GPU_OUTPUT_DIR_DEFAULT;
    param->target_width = GPU_TEST_DESIGN_WIDTH;
    param->target_height = GPU_TEST_DESIGN_WIDTH;
    param->run_loop_count = 10000;
    param->color_tolerance = 1;

    int ch;
    int longindex = 0;
    const char* optstring = "m:o:t:sh";
    const struct option longopts[] = {
        { "target", required_argument, NULL, 0 },
        { "loop-count", required_argument, NULL, 0 },
        { "cpu-freq", required_argument, NULL, 0 },
        { "fbdev", required_argument, NULL, 0 },
        { "tolerance", required_argument, NULL, 0 },
        { 0, 0, NULL, 0 }
    };

    while ((ch = getopt_long(argc, argv, optstring, longopts, &longindex)) != -1) {
        switch (ch) {
        case 0:
            parse_long_commandline(argc, argv, ch, longopts, longindex, param);
            break;

        case 'm':
            param->mode = gpu_test_string_to_mode(optarg);
            break;

        case 'o':
            param->output_dir = optarg;
            break;

        case 't':
            param->testcase_name = optarg;
            break;

        case 's':
            param->screenshot_en = true;
            break;

        case 'h':
            show_usage(argv[0], EXIT_FAILURE);
            break;

        case '?':
            GPU_LOG_ERROR("Unknown option: %c", optopt);
            show_usage(argv[0], EXIT_FAILURE);
            break;

        default:
            break;
        }
    }

    if (param->run_loop_count <= 0) {
        GPU_LOG_ERROR("Loop count should be greater than 0");
        show_usage(argv[0], EXIT_FAILURE);
    }

    GPU_LOG_INFO("Test mode: %d", param->mode);
    GPU_LOG_INFO("Output DIR: %s", param->output_dir);
    GPU_LOG_INFO("Target render image size: %dx%d", param->target_width, param->target_height);
    GPU_LOG_INFO("Testcase name: %s", param->testcase_name);
    GPU_LOG_INFO("Screenshot: %s", param->screenshot_en ? "enable" : "disable");
    GPU_LOG_INFO("Loop count: %d", param->run_loop_count);
    GPU_LOG_INFO("CPU frequency: %d MHz (0 means auto)", param->cpu_freq);
    GPU_LOG_INFO("Framebuffer device: %s", param->fbdev_path);
    GPU_LOG_INFO("Color deviation tolerance: %d", param->color_tolerance);
}
