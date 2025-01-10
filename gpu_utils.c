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

#include "gpu_utils.h"
#include "gpu_log.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

const char* gpu_get_localtime_str(void)
{
    static char str_buf[32];
    time_t rawtime;
    time(&rawtime);

    struct tm* timeinfo = localtime(&rawtime);
    if (!timeinfo) {
        GPU_LOG_ERROR("localtime() failed");
        return NULL;
    }

    snprintf(str_buf, sizeof(str_buf), "%04d%02d%02d_%02d%02d%02d",
        1900 + timeinfo->tm_year,
        timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour,
        timeinfo->tm_min, timeinfo->tm_sec);

    return str_buf;
}

int gpu_dir_create(const char* dir_path)
{
    if (access(dir_path, F_OK) == 0) {
        GPU_LOG_INFO("directory: %s already exists", dir_path);
        return 0;
    }

    GPU_LOG_WARN("can't access directory: %s, creating...", dir_path);

    int retval = mkdir(dir_path, 0777);
    if (retval == 0) {
        GPU_LOG_INFO("OK");
    } else {
        GPU_LOG_ERROR("failed: %d", errno);
    }

    return retval;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
