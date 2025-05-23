############################################################################
# apps/testing/gpu_test/Makefile
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

ifneq ($(CONFIG_TESTING_GPU_TEST_VG_LITE_INCLUDE), "")
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/../$(CONFIG_TESTING_GPU_TEST_VG_LITE_INCLUDE)
else
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/graphics/vg_lite_tvg
endif

# Basic definitions
CFLAGS += -DGPU_TEST_CONTEXT_DEFAULT_DISABLE=1
CFLAGS += -DGPU_TEST_CONTEXT_LINUX_DISABLE=1
CFLAGS += -DGPU_TEST_CONTEXT_NUTTX_ENABLE=1
CFLAGS += -DGPU_OUTPUT_DIR_DEFAULT=\"/data/gpu\"

# NuttX cache definitions
CFLAGS += -DGPU_CACHE_INCLUDE_H=\"nuttx/cache.h\"
CFLAGS += -DGPU_CACHE_INVALIDATE_FUNC=up_invalidate_dcache
CFLAGS += -DGPU_CACHE_CLEAN_FUNC=up_clean_dcache
CFLAGS += -DGPU_CACHE_FLUSH_FUNC=up_flush_dcache

CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/../external/libpng
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/../external/libpng/libpng

PROGNAME = gpu_test
PRIORITY = $(CONFIG_TESTING_GPU_TEST_PRIORITY)
STACKSIZE = $(CONFIG_TESTING_GPU_TEST_STACKSIZE)
MODULE = $(CONFIG_TESTING_GPU_TEST)

MAINSRC = gpu_main.c

CSRCS += $(wildcard vg_lite/*.c)
CSRCS += $(wildcard vg_lite/*/*.c)
CSRCS += $(filter-out gpu_main.c, $(wildcard *.c))

include $(APPDIR)/Application.mk
