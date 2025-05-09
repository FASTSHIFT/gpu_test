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

#ifndef GPU_RECORDER_H
#define GPU_RECORDER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct gpu_recorder_s;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Create a new gpu recorder
 * @param dir_path The directory path to save the record file
 * @param name The name of the record file
 * @return A pointer to the created recorder object on success, NULL on failure
 */
struct gpu_recorder_s* gpu_recorder_create(const char* dir_path, const char* name);

/**
 * @brief Delete a gpu recorder
 * @param recorder The recorder object to delete
 */
void gpu_recorder_delete(struct gpu_recorder_s* recorder);

/**
 * @brief Start recording
 * @param recorder The recorder object to start recording
 * @return 0 on success, -1 on failure
 */
int gpu_recorder_write_string(struct gpu_recorder_s* recorder, const char* str);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GPU_RECORDER_H*/
