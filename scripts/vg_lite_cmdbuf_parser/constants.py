#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite 常量定义
===============
包含寄存器映射、命令类型、格式定义等常量
"""

from enum import IntEnum


class CommandType(IntEnum):
    """VGLite命令类型"""

    END = 0x00000000
    SEMAPHORE = 0x10000000
    STALL = 0x20000000
    STATE = 0x30000000
    DATA = 0x40000000
    CALL = 0x60000000
    RETURN = 0x70000000
    NOP = 0x80000000


# GPU 硬件寄存器定义 (基于 Vivante/VGLite 架构)
GPU_REGISTER_INFO = {
    "idle": ("GPU Idle状态", "0x7fffffff 表示所有模块都空闲"),
    "AQHiClockControl": ("时钟控制寄存器", "控制GPU各模块时钟"),
    "0x0c": ("AQIntrAcknowledge", "中断确认寄存器"),
    "0x10": ("AQIntrEnbl", "中断使能寄存器"),
    "0x14": ("AQIdent", "芯片标识寄存器"),
    "0x18": ("AQFeatures", "GPU特性寄存器"),
    "0x1c": ("AQChipId", "芯片ID"),
    "0x20": ("AQChipRev", "芯片版本号"),
    "0x24": ("AQChipDate", "芯片日期"),
    "0x28": ("AQChipTime", "芯片时间"),
    "0x2c": ("AQChipMinorRev", "芯片次版本号"),
    "0x30": ("AQProductId", "产品ID"),
    "0x34": ("AQChipMask", "芯片掩码"),
    "0x40": ("AQCmdBufferAddr", "命令缓冲区当前读取地址"),
    "0x44": ("AQCmdBufferCtrl", "命令缓冲区控制/结束地址"),
    "0x48": ("AQLinkReturn", "CALL命令返回地址"),
    "0x4c": ("AQCmdBufferStart", "命令缓冲区起始地址"),
    "0x50": ("AQFetchAddr", "当前获取地址"),
    "0x54": ("AQProgramCounter", "程序计数器"),
    "0x58": ("AQDmaAddr", "DMA地址"),
    "0x5c": ("AQDmaConfig", "DMA配置"),
    "0x60": ("AQDmaStatus", "DMA状态"),
    "0x98": ("AQMemoryFeatures", "内存特性"),
    "0xa4": ("AQMemoryConfig", "内存配置"),
    "0xa8": ("AQMemoryDebug", "内存调试"),
    "0xe8": ("AQCmdState", "命令状态"),
    "0x100": ("AQDebugControl", "调试控制"),
    "0x104": ("AQDebugData", "调试数据"),
    "0x108": ("AQDebugAddress", "调试地址"),
    "0x438": ("AQDebugSignals0", "调试信号组0 - GPU内部状态"),
    "0x43c": ("AQDebugSignals1", "调试信号组1"),
    "0x440": ("AQDebugSignals2", "调试信号组2"),
    "0x444": ("AQDebugSignals3", "调试信号组3"),
    "0x448": ("AQModuleDebug", "模块调试寄存器组"),
    "0x450": ("AQPipeDebug", "管线调试寄存器组"),
    "0x454": ("AQFetchDebug", "获取调试寄存器组"),
    "0x45c": ("AQRenderDebug", "渲染调试寄存器组"),
    "0x468": ("AQTessDebug", "细分调试寄存器组"),
    "0x46c": ("AQPathDebug", "路径调试寄存器组"),
    "0x500": ("AQMMUConfig", "MMU配置"),
    "0x504": ("AQMMUStatus", "MMU状态"),
    "0x508": ("AQMMUException", "MMU异常地址"),
}


# VGLite 路径操作码 (VLC_OP_*)
VLC_OP_CODES = {
    0x00: ("END", 0),  # 结束路径
    0x01: ("CLOSE", 0),  # 闭合路径
    0x02: ("MOVE", 2),  # 移动到 (x, y)
    0x03: ("MOVE_REL", 2),  # 相对移动 (dx, dy)
    0x04: ("LINE", 2),  # 直线到 (x, y)
    0x05: ("LINE_REL", 2),  # 相对直线 (dx, dy)
    0x06: ("QUAD", 4),  # 二次贝塞尔 (cx, cy, x, y)
    0x07: ("QUAD_REL", 4),  # 相对二次贝塞尔
    0x08: ("CUBIC", 6),  # 三次贝塞尔 (cx1, cy1, cx2, cy2, x, y)
    0x09: ("CUBIC_REL", 6),  # 相对三次贝塞尔
    0x0A: ("BREAK", 0),  # 断开路径
    0x0B: ("HLINE", 1),  # 水平线 (x)
    0x0C: ("HLINE_REL", 1),  # 相对水平线 (dx)
    0x0D: ("VLINE", 1),  # 垂直线 (y)
    0x0E: ("VLINE_REL", 1),  # 相对垂直线 (dy)
    0x0F: ("SQUAD", 2),  # 平滑二次贝塞尔 (x, y)
    0x10: ("SQUAD_REL", 2),  # 相对平滑二次贝塞尔
    0x11: ("SCUBIC", 4),  # 平滑三次贝塞尔 (cx2, cy2, x, y)
    0x12: ("SCUBIC_REL", 4),  # 相对平滑三次贝塞尔
    0x13: ("SCCWARC", 5),  # 小逆时针弧 (rx, ry, rot, x, y)
    0x14: ("SCCWARC_REL", 5),  # 相对小逆时针弧
    0x15: ("SCWARC", 5),  # 小顺时针弧
    0x16: ("SCWARC_REL", 5),  # 相对小顺时针弧
    0x17: ("LCCWARC", 5),  # 大逆时针弧
    0x18: ("LCCWARC_REL", 5),  # 相对大逆时针弧
    0x19: ("LCWARC", 5),  # 大顺时针弧
    0x1A: ("LCWARC_REL", 5),  # 相对大顺时针弧
}


# VGLite GPU 寄存器地址映射表
REGISTER_MAP = {
    # 渲染控制寄存器
    0x0A00: "VgControl",  # 渲染控制
    0x0A01: "VgTargetAddress",  # 目标缓冲区地址
    0x0A02: "VgColor",  # 颜色
    0x0A03: "VgClearColor",  # 清除颜色
    0x0A04: "VgImageAddress",  # 图像源地址
    0x0A05: "VgImageConfig",  # 图像配置
    0x0A06: "VgImageStride",  # 图像步长
    0x0A07: "VgImageUAddress",  # U平面地址
    0x0A08: "VgImageVAddress",  # V平面地址
    0x0A09: "VgImageUVStride",  # UV步长
    0x0A0A: "VgImageSize",  # 图像尺寸
    0x0A0B: "VgPaintColor",  # 画笔颜色
    0x0A0C: "VgPatternAddress",  # 图案地址
    0x0A0D: "VgPatternConfig",  # 图案配置
    0x0A0E: "VgPatternStride",  # 图案步长
    0x0A0F: "VgPatternSize",  # 图案尺寸
    # 目标缓冲区相关
    0x0A10: "VgTargetStride",  # 目标步长
    0x0A11: "VgTargetWidth",  # 目标宽度
    0x0A12: "VgTargetHeight",  # 目标高度
    0x0A13: "VgTargetConfig",  # 目标配置
    0x0A14: "VgColorKey",  # 色键
    0x0A15: "VgScissorLeft",  # 裁剪左边界
    0x0A16: "VgScissorTop",  # 裁剪上边界
    0x0A17: "VgScissorRight",  # 裁剪右边界
    0x0A18: "VgScissorBottom",  # 裁剪下边界
    0x0A19: "VgGlobalAlpha",  # 全局Alpha
    0x0A1A: "VgMaskAddress",  # 遮罩地址
    0x0A1B: "VgTessControl",  # 曲面细分控制
    0x0A1C: "VgTessCount",  # 曲面细分计数
    0x0A1D: "VgTessAddress",  # 曲面细分缓冲区地址
    0x0A1E: "VgBorderColor",  # 边框颜色
    0x0A1F: "VgDstAlphaFactor",  # 目标Alpha因子
    # Blit 变换步进参数 (c_step, x_step, y_step)
    0x0A18: "VgBlitCStepX",  # Blit c_step[0]
    0x0A19: "VgBlitCStepY",  # Blit c_step[1]
    0x0A1A: "VgBlitCStepZ",  # Blit c_step[2]
    0x0A1C: "VgBlitXStepX",  # Blit x_step[0]
    0x0A1D: "VgBlitXStepY",  # Blit x_step[1]
    0x0A1E: "VgBlitXStepZ",  # Blit x_step[2]
    0x0A20: "VgBlitYStepX",  # Blit y_step[0]
    0x0A21: "VgBlitYStepY",  # Blit y_step[1]
    0x0A22: "VgBlitYStepZ",  # Blit y_step[2]
    # 源图像配置 (用于 blit/draw_pattern)
    0x0A25: "VgSourceConfig",  # 源格式配置 (filter | format | compression)
    0x0A27: "VgSourceClip",  # 源裁剪 (通常为0)
    0x0A29: "VgSourceAddress",  # 源图像地址
    0x0A2B: "VgSourceStride",  # 源步长 | tiled标志
    0x0A2D: "VgSourceOrigin",  # 源区域起点 (rect_x | rect_y << 16)
    0x0A2F: "VgSourceSize",  # 源尺寸 (width | height << 16)
    # 图像变换矩阵
    0x0A30: "VgImageMatrix0",  # 图像矩阵0 (m00)
    0x0A31: "VgImageMatrix1",  # 图像矩阵1 (m01)
    0x0A32: "VgImageMatrix2",  # 图像矩阵2 (m02)
    0x0A33: "VgImageMatrix3",  # 图像矩阵3 (m10)
    0x0A34: "VgPathControl",  # 路径控制
    0x0A35: "VgImageMatrix4",  # 图像矩阵4 (m11)
    0x0A36: "VgImageMatrix5",  # 图像矩阵5 (m12)
    0x0A37: "VgImageMatrix6",  # 图像矩阵6 (m20)
    0x0A38: "VgImageMatrix7",  # 图像矩阵7 (m21)
    0x0A39: "VgTessWindow",  # 曲面细分窗口 (x | y << 16)
    0x0A3A: "VgTessWindowSize",  # 曲面细分窗口大小 (w | h << 16)
    0x0A3B: "VgPathScale",  # 路径缩放
    0x0A3C: "VgPathBias",  # 路径偏移
    0x0A3D: "VgTessSize",  # 曲面细分大小
    # 路径变换矩阵
    0x0A40: "VgPathMatrix0",  # 路径矩阵0 (m00)
    0x0A41: "VgPathMatrix1",  # 路径矩阵1 (m01)
    0x0A42: "VgPathMatrix2",  # 路径矩阵2 (m02)
    0x0A43: "VgPathMatrix3",  # 路径矩阵3 (m10)
    0x0A44: "VgPathMatrix4",  # 路径矩阵4 (m11)
    0x0A45: "VgPathMatrix5",  # 路径矩阵5 (m12)
    # CLUT (Color Look Up Table)
    0x0A46: "VgClutAddress",  # CLUT地址
    0x0A47: "VgClutConfig",  # CLUT配置
    # 颜色变换
    0x0A50: "VgColorTransformLow",  # 颜色变换低位
    0x0A51: "VgColorTransformHigh",  # 颜色变换高位
    0x0A52: "VgColorTransformScale",  # 颜色变换缩放
    0x0A53: "VgColorTransformBias",  # 颜色变换偏移
    0x0A5C: "VgImageAlphaAddress",  # Alpha平面地址
    0x0A5D: "VgImageAlphaStride",  # Alpha平面步长
    # 颜色查找表
    0x0A90: "VgClutData0",  # CLUT数据0
    0x0A91: "VgClutData1",  # CLUT数据1
    0x0A92: "VgClutData2",  # CLUT数据2
    0x0A93: "VgClutData3",  # CLUT数据3
    0x0A94: "VgClutData4",  # CLUT数据4
    0x0A95: "VgClutData5",  # CLUT数据5
    0x0A96: "VgClutData6",  # CLUT数据6
    0x0A97: "VgClutData7",  # CLUT数据7
    # 更多寄存器
    0x0AC8: "VgConfig",  # VG配置
    0x0ACB: "VgMaskStride",  # 遮罩步长
    0x0ACC: "VgMaskConfig",  # 遮罩配置
    0x0ACD: "VgPathTransX",  # 路径平移X
    0x0ACE: "VgPathTransY",  # 路径平移Y
}

# 混合模式映射
BLEND_MODES = {
    0x00000000: "NONE",
    0x00000100: "SRC_OVER",
    0x00000200: "DST_OVER",
    0x00000300: "SRC_IN",
    0x00000400: "DST_IN",
    0x00000500: "MULTIPLY",
    0x00000600: "SCREEN",
    0x00000700: "DARKEN",
    0x00000800: "LIGHTEN",
    0x00000900: "ADDITIVE",
    0x00000A00: "SUBTRACT",
    0x00000C00: "SUBTRACT_LVGL",
}

# 图像格式映射
IMAGE_FORMATS = {
    0x00: "L8",
    0x01: "A4",
    0x02: "A8",
    0x03: "BGRA4444",
    0x04: "BGRA5551",
    0x05: "BGR565",
    0x06: "BGRX8888",
    0x07: "BGRA8888",
    0x08: "YUYV/YUY2",
    0x09: "YV12",
    0x0A: "NV16",
    0x0B: "NV12",
    0x0C: "YV16",
    0x0D: "YV24",
    0x0E: "ANV12",
    0x0F: "AYUY2",
    0x13: "ABGR4444",
    0x14: "ABGR1555",
    0x15: "BGRA5551",
    0x16: "XBGR8888",
    0x17: "ABGR8888",
    0x21: "BGR565",
    0x23: "RGBA4444",
    0x24: "RGBA5551",
    0x25: "RGB565",
    0x26: "RGBX8888",
    0x27: "RGBA8888",
    0x33: "ARGB4444",
    0x34: "ARGB1555",
    0x35: "ARGB5551",
    0x36: "XRGB8888",
    0x37: "ARGB8888",
}

# 路径格式
PATH_FORMATS = {
    0x00000000: "S8",
    0x10000000: "S16",
    0x20000000: "S32",
    0x30000000: "FP32",
}

# 路径质量
PATH_QUALITY = {
    0x00000000: "LOW",
    0x01000000: "MEDIUM",
    0x02000000: "HIGH",
    0x03000000: "BETTER",
}

# 真正的地址寄存器（用于检测空指针）
# 注意：0x0A1D, 0x0A1A, 0x0A21 等在 Blit 模式下是浮点变换参数，不是地址
ADDRESS_REGISTERS = [
    0x0A04,  # VgImageAddress - 图像源地址
    0x0A07,  # VgImageUAddress - U平面地址
    0x0A08,  # VgImageVAddress - V平面地址
    0x0A0C,  # VgPatternAddress - 图案地址
    0x0A11,  # VgTargetWidth - 实际是目标缓冲区地址
    0x0A29,  # VgSourceAddress - 源图像地址 (blit)
    0x0A46,  # VgPaintAddress - 画笔地址
    0x0ACB,  # VgMaskStride - 遮罩缓冲区地址
]

# 有效寄存器地址范围
VALID_REG_RANGES = [
    (0x0A00, 0x0AFF),  # VG寄存器
]

# 可疑的地址值（可能是空指针或未初始化）
SUSPICIOUS_ADDRESSES = [0x00000000, 0xDEADBEEF, 0xCAFEBABE, 0xFFFFFFFF]
