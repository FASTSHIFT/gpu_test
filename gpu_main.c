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
    gpu_dir_create(ctx.param.output_dir);
    gpu_test_context_setup(&ctx);
    int retval = gpu_test_run(&ctx);
    gpu_test_context_teardown(&ctx);
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
           " --target <string> --loop-count <int> --cpu-freq <int>\n",
        progname);

    printf("\nWhere:\n");
    printf("  -m <string> Test mode: default; stress.\n");
    printf("  -o <string> GPU report file output path, default is " GPU_OUTPUT_DIR_DEFAULT "\n");
    printf("  -t <string> Testcase name.\n");
    printf("  -s Enable screenshot.\n");

    printf("  --target <string> Test output image size(px), default is 480x480. Example: "
           "<decimal-value width>x<decimal-value height>\n");
    printf("  --loop-count <int> Stress mode loop count, default is 1000.\n");
    printf("  --cpu-freq <int> CPU frequency in MHz, default is 200.\n");

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
            param->img_width = width;
            param->img_height = height;
        } else {
            GPU_LOG_ERROR("Error image size: %s", optarg);
            show_usage(argv[0], EXIT_FAILURE);
        }
    } break;

    case 1:
        param->run_loop_count = atoi(optarg);
        break;

    case 2:
        param->cpu_freq = atoi(optarg);
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
    param->mode = GPU_TEST_MODE_DEFAULT;
    param->output_dir = GPU_OUTPUT_DIR_DEFAULT;
    param->img_width = GPU_TEST_DESIGN_WIDTH;
    param->img_height = GPU_TEST_DESIGN_WIDTH;
    param->run_loop_count = 1000;
    param->cpu_freq = 200;

    int ch;
    int longindex = 0;
    const char* optstring = "m:o:t:sh";
    const struct option longopts[] = {
        { "target", required_argument, NULL, 0 },
        { "loop-count", required_argument, NULL, 0 },
        { "cpu-freq", required_argument, NULL, 0 },
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
            GPU_LOG_WARN("Unknown option: %c", optopt);
            break;

        default:
            break;
        }
    }

    if (param->mode == GPU_TEST_MODE_STRESS) {
        if (param->screenshot_en) {
            GPU_LOG_WARN("Screenshot is not enabled in stress mode, disable it");
            param->screenshot_en = false;
        }
    }

    if (param->run_loop_count <= 0) {
        GPU_LOG_ERROR("Loop count should be greater than 0");
        show_usage(argv[0], EXIT_FAILURE);
    }

    GPU_LOG_INFO("Test mode: %d", param->mode);
    GPU_LOG_INFO("Output DIR: %s", param->output_dir);
    GPU_LOG_INFO("Target image size: %dx%d", param->img_width, param->img_height);
    GPU_LOG_INFO("Testcase name: %s", param->testcase_name);
    GPU_LOG_INFO("Screenshot: %s", param->screenshot_en ? "enable" : "disable");
    GPU_LOG_INFO("Loop count: %d", param->run_loop_count);
    GPU_LOG_INFO("CPU frequency: %d MHz", param->cpu_freq);
}
