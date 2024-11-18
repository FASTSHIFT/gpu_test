cmake_minimum_required(VERSION 3.10)
project(vg_lite)

# C/C++ version
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

# Platform config
## Common

option(ENABLE_DEBUG "Enable debug build" ON)

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

if(ENABLE_DEBUG)
        message(STATUS "Build type: Debug")
        add_definitions(-DDEBUG)
        set(CMAKE_BUILD_TYPE "Debug")
        set(ENABLE_ASAN 1)
else()
        message(STATUS "Build type: Release")
        set(CMAKE_BUILD_TYPE "Release")
endif()

## Others
if (WIN32)
        message(STATUS "Platform: windows")
        add_definitions(-DUSE_WIN32DRV=1)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vg_lite)
elseif (UNIX)
        message(STATUS "Platform: UNIX")

set(WARNING_FLAGS
        "-Wall -Wextra \
        -Wshadow -Wundef -Wmaybe-uninitialized -Wno-deprecated-declarations \
        -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized \
        -Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default  \
        -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated  \
        -Wshift-negative-value \
        -Wsizeof-pointer-memaccess -Wpointer-arith -Wno-cast-function-type -Wunused-but-set-variable")

set(WARNING_FLAGS "${WARNING_FLAGS} -Wno-shadow -Wno-unused-but-set-variable -Wno-double-promotion -Werror=narrowing -Werror=narrowing -Wunused-function -Wno-empty-body -Wno-type-limits")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")

        # Address Sanitizer
        if (ENABLE_ASAN)
                set(ASAN_FLAGS "-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fsanitize=leak")
                message(STATUS "ASAN_FLAGS: ${ASAN_FLAGS}")
                add_definitions(${ASAN_FLAGS})
                set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${ASAN_FLAGS})
        endif ()
endif ()

# Include
# include_directories(
#         ${PROJECT_SOURCE_DIR}/
# )

# Sources
# file(GLOB EXAMPLE_SOURCES
#         ${PROJECT_SOURCE_DIR}/example/*.c
#         )

## gpu_test
file(GLOB GPU_TEST_SOURCES
        ${PROJECT_SOURCE_DIR}/*.c
        ${PROJECT_SOURCE_DIR}/vg_lite/*.c
        )

## vg-lite
set(VG_LITE_TVG_SOURCES_DIR ${PROJECT_SOURCE_DIR}/external/vg_lite_tvg)

include_directories(
        ${VG_LITE_TVG_SOURCES_DIR}/
)

file(GLOB VG_LITE_SOURCES
        ${VG_LITE_TVG_SOURCES_DIR}/*.c
        ${VG_LITE_TVG_SOURCES_DIR}/*.cpp
        )

## thorvg
include_directories(
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/inc
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/common
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/lib
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/lib/sw_engine
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/jpg
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/lottie
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/png
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/raw
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/svg
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/tvg
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/utils
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/renderer
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/renderer/sw_engine
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/bindings/capi/
)
file(GLOB THORVG_SOURCES
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/bindings/capi/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/common/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/lib/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/lib/sw_engine/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/jpg/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/lottie/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/png/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/raw/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/svg/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/loaders/tvg/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/utils/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/renderer/*.cpp
        ${VG_LITE_TVG_SOURCES_DIR}/thorvg/src/renderer/sw_engine/*.cpp
        )

set(SOURCES
        ${VG_LITE_SOURCES}
        ${THORVG_SOURCES}
        ${GPU_TEST_SOURCES}
        )

# Packages
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

# Executable
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(gpu_test ${SOURCES} ${INCLUDES})

# Link
target_link_libraries(
        gpu_test PRIVATE
        stdc++
        m
        ${PNG_LIBRARIES}
)

set_target_properties(
        gpu_test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build"
)
