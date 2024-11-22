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

#ifdef GPU_TEST_CONTEXT_NUTTX_ENABLE

#include "gpu_assert.h"
#include "gpu_buffer.h"
#include "gpu_fb.h"
#include "gpu_log.h"
#include <errno.h>
#include <fcntl.h>
#include <nuttx/video/fb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct gpu_fb_s {
    int fd;
    struct fb_videoinfo_s vinfo;
    struct fb_planeinfo_s pinfo;
    void* memory;
};

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

struct gpu_fb_s* gpu_fb_create(const char* path)
{
    struct gpu_fb_s* fb = malloc(sizeof(struct gpu_fb_s));
    GPU_ASSERT_NULL(fb);
    memset(fb, 0, sizeof(struct gpu_fb_s));

    fb->fd = open(path, O_RDWR | O_CLOEXEC);
    if (fb->fd < 0) {
        GPU_LOG_ERROR("Failed to open framebuffer device: %s, error: %d", path, errno);
        goto failed;
    }

    /* Get fixed screen information*/
    if (ioctl(fb->fd, FBIOGET_VIDEOINFO, &fb->vinfo) < 0) {
        GPU_LOG_ERROR("ioctl FBIOGET_VIDEOINFO failed: %d", errno);
        goto failed;
    }

    /* Get variable screen information*/
    if (ioctl(fb->fd, FBIOGET_PLANEINFO, &fb->pinfo) < 0) {
        GPU_LOG_ERROR("ioctl FBIOGET_PLANEINFO failed: %d", errno);
        goto failed;
    }

    /* Map the device to memory*/
    fb->memory = mmap(NULL, fb->pinfo.fblen, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_FILE, fb->fd, 0);
    if ((intptr_t)fb->memory == -1) {
        GPU_LOG_ERROR("mmap failed: %d", errno);
        goto failed;
    }

    GPU_LOG_INFO("Framebuffer device opened: %s, size: %ux%u, format: %u",
        path, fb->vinfo.xres, fb->vinfo.yres, fb->vinfo.fmt);

    return fb;

failed:
    gpu_fb_destroy(fb);
    return NULL;
}

void gpu_fb_destroy(struct gpu_fb_s* fb)
{
    GPU_ASSERT_NULL(fb);
    if (fb->memory) {
        GPU_LOG_INFO("munmap memory: %p, size: %u", fb->memory, fb->pinfo.fblen);
        munmap(fb->memory, fb->pinfo.fblen);
    }

    if (fb->fd >= 0) {
        GPU_LOG_INFO("close fd: %d", fb->fd);
        close(fb->fd);
    }

    GPU_LOG_INFO("free fb: %p", fb);
    memset(fb, 0, sizeof(struct gpu_fb_s));
    free(fb);
}

void gpu_fb_get_buffer(const struct gpu_fb_s* fb, struct gpu_buffer_s* buffer)
{
    GPU_ASSERT_NULL(fb);
    GPU_ASSERT_NULL(buffer);

    buffer->data = fb->memory;
    buffer->width = fb->vinfo.xres;
    buffer->height = fb->vinfo.yres;
    buffer->stride = fb->pinfo.stride;

    switch (fb->vinfo.fmt) {
    case FB_FMT_RGB16_565:
        buffer->format = GPU_COLOR_FORMAT_BGR565;
        break;

    case FB_FMT_RGB24:
        buffer->format = GPU_COLOR_FORMAT_BGR888;
        break;

    case FB_FMT_RGB32:
        buffer->format = GPU_COLOR_FORMAT_BGRX8888;
        break;

    case FB_FMT_RGBA32:
        buffer->format = GPU_COLOR_FORMAT_BGRA8888;
        break;

    default:
        buffer->format = GPU_COLOR_FORMAT_UNKNOWN;
        GPU_LOG_ERROR("Unsupported color format: %d", fb->vinfo.fmt);
        break;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /* GPU_TEST_CONTEXT_LINUX_DISABLE */
