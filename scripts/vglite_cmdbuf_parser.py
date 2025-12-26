#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite Command Buffer Parser
=============================
解析VGLite GPU驱动的命令缓冲区日志

日志格式来源: vg_lite_kernel.c 的 dump_last_frame 函数
日志示例:
  0x30010A00 0x00000100
  0x30010A02 0xFF0000FF
  0x10000007 0x00000000
  0x20000007 0x00000000
  0x00000000 0x00000001

命令格式:
  - STATE_COMMAND:     0x3001XXXX (写寄存器状态)
  - STATES_COMMAND:    0x30CCXXXX (批量写寄存器, CC=count)
  - END_COMMAND:       0x00000000 | interrupt
  - SEMAPHORE_COMMAND: 0x10000000 | id
  - STALL_COMMAND:     0x20000000 | id
  - DATA_COMMAND:      0x40000000 | count
  - CALL_COMMAND:      0x60000000 | count
  - RETURN_COMMAND:    0x70000000
  - NOP_COMMAND:       0x80000000

Author: VGLite Tools
Date: 2024
"""

import re
import sys
import struct
import argparse
from enum import IntEnum
from dataclasses import dataclass
from typing import List, Optional, Tuple

try:
    from rich.console import Console
    from rich.table import Table
    from rich.panel import Panel
except ImportError:
    print("错误: 需要安装 rich 库")
    print("请运行: pip install rich")
    sys.exit(1)


@dataclass
class LogIntegrityIssue:
    """日志完整性问题"""

    line_number: int  # 行号 (1-based)
    original_line: str  # 原始行内容
    issue_type: str  # 问题类型
    description: str  # 问题描述


class LogIntegrityChecker:
    """日志完整性检测器

    检测日志输出时可能发生的并发冲突、缓冲区溢出等问题导致的数据损坏
    """

    def __init__(self):
        self.issues: List[LogIntegrityIssue] = []

    def check_line(self, line_number: int, line: str) -> Optional[LogIntegrityIssue]:
        """检查单行是否有完整性问题"""
        problems = []

        # 1. 检测时间戳重复/混乱 (如 06:22:562/24 或一行多个时间戳)
        timestamp_pattern = r"\[\d{1,2}/\d{1,2}\s+\d{2}:\d{2}:\d{2}\]"
        timestamps = re.findall(timestamp_pattern, line)
        if len(timestamps) > 1:
            problems.append(
                ("DUPLICATE_TIMESTAMP", f"一行中有{len(timestamps)}个时间戳")
            )

        # 检测损坏的时间戳 (秒数超过2位)
        broken_ts = re.search(r"\d{2}:\d{2}:\d{3,}", line)
        if broken_ts:
            problems.append(("CORRUPTED_TIMESTAMP", f"时间戳损坏: {broken_ts.group()}"))

        # 2. 检测行合并 (一行中出现多个 [ap] 标签)
        ap_count = len(re.findall(r"\[ap\]", line, re.IGNORECASE))
        if ap_count > 1:
            problems.append(("LINE_MERGED", f"检测到{ap_count}个[ap]标签，多行被合并"))

        # 3. 检测标签格式异常 (如 [51][ap] 缺少空格)
        if re.search(r"\]\[ap\]", line, re.IGNORECASE):
            problems.append(("TAG_MALFORMED", "[ap]标签格式异常"))

        # 4. 检测数据与标签混合 (如 0x42AC0000[ap] 或 0x42AC0000[12/24)
        # 排除正常的寄存器地址格式 [0x450] 或 0x450]
        hex_with_tag = re.search(r"0x[0-9A-Fa-f]{8}\[(?!0x)", line)
        if hex_with_tag:
            problems.append(
                ("DATA_TAG_MIXED", f"数据与标签混合: {hex_with_tag.group()}")
            )

        # 5. 检测连续十六进制数无空格 (如 0x300100000x30010A02)
        continuous_hex = re.search(r"0x[0-9A-Fa-f]{8}0x[0-9A-Fa-f]", line)
        if continuous_hex:
            problems.append(
                ("DATA_NO_SEPARATOR", f"数据无分隔符: {continuous_hex.group()}")
            )

        # 6. 检测一行中有多对命令 (正常每行最多1对)
        hex_pairs = re.findall(r"0x[0-9A-Fa-f]{8}\s+0x[0-9A-Fa-f]{8}", line)
        if len(hex_pairs) > 1:
            problems.append(("MULTIPLE_COMMANDS", f"一行有{len(hex_pairs)}对命令"))

        # 7. 检测数据截断 (行尾有孤立的短数字)
        if re.search(r"\s+\d{1,3}$", line) and not re.search(r"0x[0-9A-Fa-f]+$", line):
            if not re.search(r"\[\d+\]\s*=", line):  # 排除寄存器索引
                trailing = re.search(r"\s+(\d{1,3})$", line)
                if trailing:
                    problems.append(
                        ("DATA_TRUNCATED", f"数据可能被截断: '{trailing.group(1)}'")
                    )

        if problems:
            issue_types = [p[0] for p in problems]
            descriptions = [p[1] for p in problems]
            issue = LogIntegrityIssue(
                line_number=line_number,
                original_line=line.strip()[:80]
                + ("..." if len(line.strip()) > 80 else ""),
                issue_type=", ".join(issue_types),
                description="; ".join(descriptions),
            )
            self.issues.append(issue)
            return issue

        return None

    def analyze(self, lines: List[str]) -> List[LogIntegrityIssue]:
        """分析整个文件"""
        self.issues = []
        for i, line in enumerate(lines, 1):
            self.check_line(i, line)
        return self.issues

    def print_report(self, console: Console):
        """打印检测报告"""
        if not self.issues:
            console.print("[green]✓ 日志完整性检查通过，未发现问题[/green]")
            return

        console.print()
        console.print(
            Panel(
                f"[bold red]⚠️ 检测到 {len(self.issues)} 处日志完整性问题[/bold red]\n"
                "[dim]这可能是由于日志输出时的并发冲突导致的，不一定是GPU命令本身的问题[/dim]",
                title="日志完整性检查",
                border_style="red",
            )
        )

        table = Table(title="问题详情", show_header=True, header_style="bold red")
        table.add_column("行号", style="cyan", width=8, justify="right")
        table.add_column("问题类型", style="yellow", width=22)
        table.add_column("描述", style="white", width=35)
        table.add_column("原始内容", style="dim", width=45)

        for issue in self.issues[:20]:
            table.add_row(
                str(issue.line_number),
                issue.issue_type.replace(", ", "\n"),
                issue.description.replace("; ", "\n"),
                issue.original_line,
            )

        console.print(table)

        if len(self.issues) > 20:
            console.print(f"[dim]... 还有 {len(self.issues) - 20} 处问题未显示[/dim]")

        # 按类型统计
        type_counts = {}
        for issue in self.issues:
            for t in issue.issue_type.split(", "):
                type_counts[t] = type_counts.get(t, 0) + 1

        console.print()
        console.print("[bold]问题类型统计:[/bold]")
        type_desc_map = {
            "DUPLICATE_TIMESTAMP": "时间戳重复",
            "CORRUPTED_TIMESTAMP": "时间戳损坏",
            "LINE_MERGED": "多行合并",
            "TAG_MALFORMED": "标签格式异常",
            "DATA_TAG_MIXED": "数据标签混合",
            "DATA_NO_SEPARATOR": "数据无分隔符",
            "MULTIPLE_COMMANDS": "一行多命令",
            "DATA_TRUNCATED": "数据截断",
        }
        for issue_type, count in sorted(type_counts.items(), key=lambda x: -x[1]):
            desc = type_desc_map.get(issue_type, issue_type)
            console.print(f"  {desc}: [bold]{count}[/bold]")


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


@dataclass
class ImageDrawInfo:
    """图片绘制信息"""

    # 源图像信息
    src_address: int = 0
    src_format: str = "UNKNOWN"
    src_format_raw: int = 0
    src_width: int = 0
    src_height: int = 0
    src_stride: int = 0

    # 目标信息
    dst_address: int = 0
    dst_format: str = "UNKNOWN"
    dst_format_raw: int = 0
    dst_width: int = 0
    dst_height: int = 0
    dst_stride: int = 0

    # 变换矩阵
    matrix: List[float] = None

    # 混合模式
    blend_mode: str = "SRC_OVER"

    # 绘制区域
    clip_x: int = 0
    clip_y: int = 0
    clip_width: int = 0
    clip_height: int = 0

    # 命令偏移（用于定位）
    offset: int = 0
    section_name: str = ""

    def __post_init__(self):
        if self.matrix is None:
            self.matrix = [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]

    def get_matrix_str(self) -> str:
        """获取矩阵的字符串表示"""
        if not self.matrix:
            return "Identity"
        # 检查是否为单位矩阵
        identity = [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]
        is_identity = all(abs(a - b) < 0.0001 for a, b in zip(self.matrix, identity))
        if is_identity:
            return "Identity"
        # 检查是否为平移矩阵
        if (
            abs(self.matrix[0] - 1.0) < 0.0001
            and abs(self.matrix[4] - 1.0) < 0.0001
            and abs(self.matrix[1]) < 0.0001
            and abs(self.matrix[3]) < 0.0001
        ):
            tx = self.matrix[2]
            ty = self.matrix[5]
            return f"Translate({tx:.1f}, {ty:.1f})"
        # 检查是否为缩放矩阵
        if abs(self.matrix[1]) < 0.0001 and abs(self.matrix[3]) < 0.0001:
            sx = self.matrix[0]
            sy = self.matrix[4]
            tx = self.matrix[2]
            ty = self.matrix[5]
            if abs(tx) < 0.0001 and abs(ty) < 0.0001:
                return f"Scale({sx:.3f}, {sy:.3f})"
            return f"Scale({sx:.3f}, {sy:.3f})+Translate({tx:.1f}, {ty:.1f})"
        # 一般矩阵
        return f"[{self.matrix[0]:.3f}, {self.matrix[1]:.3f}, {self.matrix[2]:.1f}; {self.matrix[3]:.3f}, {self.matrix[4]:.3f}, {self.matrix[5]:.1f}]"

    def calc_memory_size(self) -> int:
        """计算源图像内存大小（字节）"""
        if self.src_stride > 0 and self.src_height > 0:
            return self.src_stride * self.src_height
        return 0


@dataclass
class PathSegment:
    """路径段"""

    opcode: int
    op_name: str
    coords: List[float]

    def __str__(self):
        if self.coords:
            coord_str = ",".join(f"{c:.2f}" for c in self.coords)
            return f"{self.op_name},{coord_str},"
        return f"{self.op_name},"


@dataclass
class ParsedCommand:
    """解析后的命令"""

    offset: int
    cmd_word: int
    data_word: int
    cmd_type: str
    description: str
    details: List[str]
    is_abnormal: bool = False  # 是否异常
    abnormal_reasons: List[str] = None  # 异常原因
    path_data: List[Tuple[int, int]] = None  # DATA命令的路径数据
    path_segments: List[PathSegment] = None  # 解析后的路径段

    def __post_init__(self):
        if self.abnormal_reasons is None:
            self.abnormal_reasons = []
        if self.path_data is None:
            self.path_data = []
        if self.path_segments is None:
            self.path_segments = []


class VGLitePathParser:
    """VGLite路径数据解析器"""

    def __init__(self, path_format: str = "FP32"):
        """
        Args:
            path_format: 路径数据格式 - "S8", "S16", "S32", "FP32"
        """
        self.path_format = path_format

    def parse_path_data(self, raw_data: List[Tuple[int, int]]) -> List[PathSegment]:
        """解析路径数据

        Args:
            raw_data: 原始数据列表，每个元素是 (dword1, dword2) 元组

        Returns:
            解析后的路径段列表
        """
        if not raw_data:
            return []

        # 将所有数据展平成字节流
        bytes_data = bytearray()
        for d1, d2 in raw_data:
            bytes_data.extend(struct.pack("<II", d1, d2))

        return self._parse_bytes(bytes_data)

    def _parse_bytes(self, data: bytearray) -> List[PathSegment]:
        """从字节流解析路径段"""
        segments = []
        offset = 0

        while offset < len(data):
            # 根据格式获取操作码
            opcode = self._read_opcode(data, offset)
            if opcode is None:
                break

            op_info = VLC_OP_CODES.get(opcode & 0xFF)
            if op_info is None:
                # 未知操作码，尝试跳过
                offset += self._get_format_size()
                continue

            op_name, coord_count = op_info

            # 读取坐标数据
            offset += self._get_format_size()  # 跳过操作码
            coords = []
            for _ in range(coord_count):
                if offset >= len(data):
                    break
                coord = self._read_coord(data, offset)
                if coord is not None:
                    coords.append(coord)
                offset += self._get_format_size()

            segments.append(PathSegment(opcode=opcode, op_name=op_name, coords=coords))

            # 遇到 END 或 CLOSE 可以继续解析（可能有多个子路径）
            if opcode == 0x00:  # END
                break

        return segments

    def _get_format_size(self) -> int:
        """获取单个数据元素的字节大小"""
        if self.path_format == "S8":
            return 1
        elif self.path_format == "S16":
            return 2
        elif self.path_format in ("S32", "FP32"):
            return 4
        return 4

    def _read_opcode(self, data: bytearray, offset: int) -> Optional[int]:
        """读取操作码"""
        if offset >= len(data):
            return None

        if self.path_format == "S8":
            return data[offset]
        elif self.path_format == "S16":
            if offset + 2 > len(data):
                return None
            return struct.unpack_from("<H", data, offset)[0]
        else:  # S32 or FP32
            if offset + 4 > len(data):
                return None
            return struct.unpack_from("<I", data, offset)[0]

    def _read_coord(self, data: bytearray, offset: int) -> Optional[float]:
        """读取坐标值"""
        if offset >= len(data):
            return None

        if self.path_format == "S8":
            return float(struct.unpack_from("<b", data, offset)[0])
        elif self.path_format == "S16":
            if offset + 2 > len(data):
                return None
            return float(struct.unpack_from("<h", data, offset)[0])
        elif self.path_format == "S32":
            if offset + 4 > len(data):
                return None
            return float(struct.unpack_from("<i", data, offset)[0])
        else:  # FP32
            if offset + 4 > len(data):
                return None
            return struct.unpack_from("<f", data, offset)[0]


class VGLiteCommandParser:
    """VGLite命令缓冲区解析器"""

    # 已知有效的寄存器地址范围
    VALID_REG_RANGES = [
        (0x0A00, 0x0AFF),  # VG寄存器
    ]

    # 可疑的地址值（可能是空指针或未初始化）
    SUSPICIOUS_ADDRESSES = [0x00000000, 0xDEADBEEF, 0xCAFEBABE, 0xFFFFFFFF]

    def __init__(
        self, verbose: bool = False, parse_path: bool = False, parse_image: bool = False
    ):
        self.verbose = verbose
        self.parse_path = parse_path
        self.parse_image = parse_image
        self.commands: List[ParsedCommand] = []
        self.abnormal_commands: List[ParsedCommand] = []
        self.current_path_format = "FP32"  # 默认路径格式
        # 分段存储命令
        self.command_sections: List[dict] = (
            []
        )  # [{name, address, size, commands, abnormal_commands}]
        self.current_section: dict = None

        # 图片绘制跟踪
        self.image_draws: List[ImageDrawInfo] = []
        self._current_image = ImageDrawInfo()
        self._image_matrix = [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]
        self._current_blend = "SRC_OVER"

    def _finalize_image_draw(self, offset: int):
        """完成一次图片绘制记录"""
        if self._current_image.src_address == 0:
            return

        # 复制当前状态
        img = ImageDrawInfo(
            src_address=self._current_image.src_address,
            src_format=self._current_image.src_format,
            src_format_raw=self._current_image.src_format_raw,
            src_width=self._current_image.src_width,
            src_height=self._current_image.src_height,
            src_stride=self._current_image.src_stride,
            dst_address=self._current_image.dst_address,
            dst_format=self._current_image.dst_format,
            dst_format_raw=self._current_image.dst_format_raw,
            dst_width=self._current_image.dst_width,
            dst_height=self._current_image.dst_height,
            dst_stride=self._current_image.dst_stride,
            matrix=self._image_matrix.copy(),
            blend_mode=self._current_blend,
            offset=offset,
            section_name=self.current_section["name"] if self.current_section else "",
        )
        self.image_draws.append(img)

        # 重置源地址（但保留目标信息，因为可能多次绘制到同一目标）
        self._current_image.src_address = 0

    def _reset_image_state(self):
        """重置图片状态（新的命令段开始时调用）"""
        self._current_image = ImageDrawInfo()
        self._image_matrix = [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]
        self._current_blend = "SRC_OVER"

    @staticmethod
    def clean_log_line(line: str) -> str:
        """清理日志行，移除时间戳、ANSI颜色码等前缀

        支持的日志格式:
        - 2025-12-24 09:38:04.274 [INFO] [Nuttx] ...  0x30010a35 0x3c23ac80
        - 带ANSI颜色码: \x1b[32m INFO \x1b[0m
        - 纯命令行: 0x30010A00 0x00000100
        """
        # 移除ANSI转义序列 (颜色码等)
        # 格式: \x1b[XXm 或 \033[XXm 或 ^[[XXm
        ansi_pattern = r"(\x1b|\033|\^\[)\[[0-9;]*m"
        line = re.sub(ansi_pattern, "", line)

        # 移除常见的日志前缀格式
        # 格式: 2025-12-24 09:38:04.274 [INFO] [Nuttx] [12/24 06:22:55] [51] [ap]
        log_prefix_pattern = r"^.*?\[ap\]\s*"
        line = re.sub(log_prefix_pattern, "", line, flags=re.IGNORECASE)

        # 如果上面的模式没匹配，尝试更通用的方式
        # 移除时间戳前缀: YYYY-MM-DD HH:MM:SS.xxx
        timestamp_pattern = r"^\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}[.\d]*\s*"
        line = re.sub(timestamp_pattern, "", line)

        # 移除方括号标签: [INFO] [Nuttx] 等
        bracket_tags_pattern = r"\[[^\]]*\]\s*"
        # 只移除不是十六进制数据前的方括号
        if not re.match(r"^\s*0x", line):
            line = re.sub(bracket_tags_pattern, "", line)

        return line.strip()

    def parse_line(self, line: str, offset: int = 0) -> Optional[ParsedCommand]:
        """解析单行日志"""
        # 先清理日志行
        cleaned_line = self.clean_log_line(line)

        # 匹配格式: 0xXXXXXXXX 0xXXXXXXXX
        pattern = r"(0x[0-9A-Fa-f]{8})\s+(0x[0-9A-Fa-f]{8})"
        match = re.search(pattern, cleaned_line)
        if not match:
            return None

        cmd_word = int(match.group(1), 16)
        data_word = int(match.group(2), 16)

        return self._decode_command(offset, cmd_word, data_word)

    def _decode_command(
        self, offset: int, cmd_word: int, data_word: int
    ) -> ParsedCommand:
        """解码单个命令"""
        opcode = cmd_word & 0xF0000000
        cmd_type = "UNKNOWN"
        description = ""
        details = []
        is_abnormal = False
        abnormal_reasons = []

        if opcode == CommandType.STATE:
            # 状态命令
            count = (cmd_word >> 16) & 0xFF
            address = cmd_word & 0xFFFF

            if count == 1:
                cmd_type = "STATE"
                reg_name = REGISTER_MAP.get(address, f"REG_0x{address:04X}")
                description = f"写寄存器 {reg_name}"
                details = self._decode_state_data(address, data_word)

                # 检查异常：未知寄存器地址
                if address not in REGISTER_MAP:
                    is_valid_range = any(
                        start <= address <= end for start, end in self.VALID_REG_RANGES
                    )
                    if not is_valid_range:
                        is_abnormal = True
                        abnormal_reasons.append(f"未知寄存器地址: 0x{address:04X}")

                # 检查异常：地址寄存器写入可疑值
                # 注意：0x0A1D, 0x0A1A, 0x0A21 等在 Blit 模式下是浮点变换参数，不是地址
                # 只检查真正的地址寄存器
                address_regs = [
                    0x0A04,  # VgImageAddress - 图像源地址
                    0x0A07,  # VgImageUAddress - U平面地址
                    0x0A08,  # VgImageVAddress - V平面地址
                    0x0A0C,  # VgPatternAddress - 图案地址
                    0x0A11,  # VgTargetWidth - 实际是目标缓冲区地址
                    0x0A29,  # VgSourceAddress - 源图像地址 (blit)
                    0x0A46,  # VgPaintAddress - 画笔地址
                    0x0ACB,  # VgMaskStride - 遮罩缓冲区地址
                ]
                if address in address_regs:
                    if data_word in self.SUSPICIOUS_ADDRESSES:
                        is_abnormal = True
                        abnormal_reasons.append(
                            f"可疑地址值: 0x{data_word:08X} (可能为空指针)"
                        )
                    # 检查地址是否在合理范围（通常GPU可访问的内存范围）
                    if data_word != 0 and (
                        data_word < 0x10000000 or data_word > 0x80000000
                    ):
                        is_abnormal = True
                        abnormal_reasons.append(
                            f"地址值可能超出有效范围: 0x{data_word:08X}"
                        )

            else:
                cmd_type = "STATES"
                reg_name = REGISTER_MAP.get(address, f"REG_0x{address:04X}")
                description = f"批量写 {count} 个寄存器, 起始: {reg_name}"

                # 检查异常：批量写入数量过大
                if count > 64:
                    is_abnormal = True
                    abnormal_reasons.append(f"批量写入数量过大: {count}")

        elif opcode == CommandType.END:
            cmd_type = "END"
            interrupt = cmd_word & 0x00FFFFFF
            description = f"结束命令"
            if interrupt:
                details.append(f"触发中断: {interrupt}")

        elif opcode == CommandType.SEMAPHORE:
            cmd_type = "SEMAPHORE"
            sem_id = cmd_word & 0x0FFFFFFF
            description = f"信号量 ID={sem_id}"

            # 检查异常：信号量ID异常
            if sem_id > 31:
                is_abnormal = True
                abnormal_reasons.append(f"信号量ID超出范围: {sem_id}")

        elif opcode == CommandType.STALL:
            cmd_type = "STALL"
            stall_id = cmd_word & 0x0FFFFFFF
            description = f"停顿等待 ID={stall_id}"
            if stall_id == 7:
                details.append("等待所有路径渲染完成")

            # 检查异常：停顿ID异常
            if stall_id > 31:
                is_abnormal = True
                abnormal_reasons.append(f"停顿ID超出范围: {stall_id}")

        elif opcode == CommandType.DATA:
            cmd_type = "DATA"
            data_count = cmd_word & 0x0FFFFFFF

            # data_count == 1 通常是 push_rectangle 写入的矩形数据（用于 blit）
            if data_count == 1:
                description = f"矩形绘制数据 (Blit)"
                # 收集图片绘制信息（如果有源图像地址）
                if self.parse_image and self._current_image.src_address != 0:
                    self._finalize_image_draw(offset)
            else:
                description = f"路径数据 ({data_count * 8} 字节, {data_count} 条目)"

            # 检查异常：数据量过大 (合理范围内的路径数据一般不会超过64K条目)
            if data_count > 0x10000:  # 超过64K条目
                is_abnormal = True
                abnormal_reasons.append(f"数据量异常大: {data_count * 8} 字节")

            # 注意：DATA命令的data_word是路径数据的第一个DWORD，不是地址
            # 所以不检查data_word是否为可疑地址

        elif opcode == CommandType.CALL:
            cmd_type = "CALL"
            call_count = cmd_word & 0x0FFFFFFF
            call_bytes = call_count * 8  # 每个count是8字节
            description = f"调用上传路径 (Uploaded Path)"
            details.append(f"地址: 0x{data_word:08X}")
            details.append(f"长度: {call_bytes} 字节 ({call_count} 条目)")

            # 检查异常：调用地址为空
            if data_word in self.SUSPICIOUS_ADDRESSES:
                is_abnormal = True
                abnormal_reasons.append(f"调用地址可疑: 0x{data_word:08X}")

        elif opcode == CommandType.RETURN:
            cmd_type = "RETURN"
            description = "返回"

        elif opcode == CommandType.NOP:
            cmd_type = "NOP"
            description = "空操作"

        else:
            cmd_type = "UNKNOWN"
            description = f"未知命令 (opcode: 0x{opcode:08X})"
            is_abnormal = True
            abnormal_reasons.append(f"未知操作码: 0x{opcode:08X}")

        return ParsedCommand(
            offset=offset,
            cmd_word=cmd_word,
            data_word=data_word,
            cmd_type=cmd_type,
            description=description,
            details=details,
            is_abnormal=is_abnormal,
            abnormal_reasons=abnormal_reasons,
        )

    def _decode_state_data(self, address: int, data: int) -> List[str]:
        """解码寄存器数据"""
        details = []

        if address == 0x0A00:  # VgControl
            blend = data & 0x00000F00
            blend_name = BLEND_MODES.get(blend, f"0x{blend:04X}")
            details.append(f"混合模式: {blend_name}")
            if self.parse_image:
                self._current_blend = blend_name

            if data & 0x01:
                details.append("启用Tiled模式")
            if data & 0x40:
                details.append("启用裁剪")
            if data & 0x80:
                details.append("启用遮罩")
            if data & 0x100000:
                details.append("启用颜色变换")
            if data & 0x200000:
                details.append("启用矩阵")

        elif address == 0x0A02:  # VgColor
            a = (data >> 24) & 0xFF
            r = (data >> 16) & 0xFF
            g = (data >> 8) & 0xFF
            b = data & 0xFF
            details.append(f"颜色: ARGB({a}, {r}, {g}, {b}) / #{data:08X}")

        elif address == 0x0A34:  # VgPathControl
            # 格式位在 bits [21:20]，参考 convert_path_format():
            # S8  = 0x000000 (00)
            # S16 = 0x100000 (01)
            # S32 = 0x200000 (10)
            # FP32 = 0x300000 (11)
            fmt = (data >> 20) & 0x3
            fmt_name = {0: "S8", 1: "S16", 2: "S32", 3: "FP32"}.get(fmt, "?")
            details.append(f"路径格式: {fmt_name}")
            # 更新当前路径格式
            self.current_path_format = fmt_name

            # 质量位在 bits [25:24]
            quality = (data >> 24) & 0x3
            quality_name = {0: "LOW", 1: "MEDIUM", 2: "HIGH", 3: "BETTER"}.get(
                quality, "?"
            )
            details.append(f"质量: {quality_name}")

            if data & 0x10:
                details.append("奇偶填充规则")
            else:
                details.append("非零填充规则")

            if data & 0x200:
                details.append("描边模式")

        elif address == 0x0A39:  # VgTessWindow
            x = data & 0xFFFF
            y = (data >> 16) & 0xFFFF
            details.append(f"窗口起点: ({x}, {y})")

        elif address == 0x0A3A:  # VgTessWindowSize
            w = data & 0xFFFF
            h = (data >> 16) & 0xFFFF
            details.append(f"窗口大小: {w} x {h}")

        elif address == 0x0A3B:  # VgPathScale
            # IEEE 754 单精度浮点数
            import struct

            try:
                scale = struct.unpack("f", struct.pack("I", data))[0]
                details.append(f"缩放: {scale}")
            except:
                details.append(f"缩放: 0x{data:08X}")

        elif address == 0x0A3C:  # VgPathBias
            import struct

            try:
                bias = struct.unpack("f", struct.pack("I", data))[0]
                details.append(f"偏移: {bias}")
            except:
                details.append(f"偏移: 0x{data:08X}")

        elif address == 0x0A01:  # VgTargetAddress
            details.append(f"目标地址: 0x{data:08X}")
            if self.parse_image:
                self._current_image.dst_address = data

        elif address == 0x0A29:  # VgSourceAddress (blit源图像地址)
            details.append(f"源图像地址: 0x{data:08X}")
            if self.parse_image:
                self._current_image.src_address = data

        elif address == 0x0A10:  # VgTargetStride
            details.append(f"目标步长: {data} 字节")
            if self.parse_image:
                self._current_image.dst_stride = data

        elif address == 0x0A11:  # VgTargetWidth
            details.append(f"目标宽度: {data}")
            if self.parse_image:
                self._current_image.dst_width = data

        elif address == 0x0A12:  # VgTargetHeight
            details.append(f"目标高度: {data}")
            if self.parse_image:
                self._current_image.dst_height = data

        elif address == 0x0A13:  # VgTargetConfig
            fmt = data & 0x3F
            fmt_name = IMAGE_FORMATS.get(fmt, f"0x{fmt:02X}")
            details.append(f"目标格式: {fmt_name}")
            if self.parse_image:
                self._current_image.dst_format = fmt_name
                self._current_image.dst_format_raw = fmt

        elif address == 0x0A25:  # VgSourceConfig (源格式配置)
            fmt = data & 0x3F
            filter_mode = (data >> 16) & 0x3
            filter_names = {0: "POINT", 1: "LINEAR", 2: "BI_LINEAR", 3: "GAUSSIAN"}
            fmt_name = IMAGE_FORMATS.get(fmt, f"0x{fmt:02X}")
            filter_name = filter_names.get(filter_mode, "?")
            details.append(f"源格式: {fmt_name}, 滤波: {filter_name}")
            if self.parse_image:
                self._current_image.src_format = fmt_name
                self._current_image.src_format_raw = fmt

        elif address == 0x0A2B:  # VgSourceStride
            stride = data & 0x0FFFFFFF
            tiled = (data >> 28) & 0x1
            details.append(f"源步长: {stride} 字节" + (", Tiled" if tiled else ""))
            if self.parse_image:
                self._current_image.src_stride = stride

        elif address == 0x0A2D:  # VgSourceOrigin
            x = data & 0xFFFF
            y = (data >> 16) & 0xFFFF
            details.append(f"源区域起点: ({x}, {y})")

        elif address == 0x0A2F:  # VgSourceSize
            w = data & 0xFFFF
            h = (data >> 16) & 0xFFFF
            details.append(f"源尺寸: {w} x {h}")
            if self.parse_image:
                self._current_image.src_width = w
                self._current_image.src_height = h

        elif address == 0x0A1B:  # VgTessControl
            details.append(f"细分控制: 0x{data:08X}")

        elif address == 0x0A3D:  # VgTessSize
            details.append(f"细分缓冲区: {data * 64} 字节")

        elif address in range(0x0A40, 0x0A46):  # Path Matrix
            import struct

            try:
                val = struct.unpack("f", struct.pack("I", data))[0]
                idx = address - 0x0A40
                row = idx // 3
                col = idx % 3
                details.append(f"路径矩阵[{row}][{col}]: {val}")
            except:
                details.append(f"矩阵值: 0x{data:08X}")

        elif address in range(0x0A30, 0x0A39):  # Image Matrix (0x0A30 - 0x0A38)
            import struct

            try:
                val = struct.unpack("f", struct.pack("I", data))[0]
                idx = address - 0x0A30
                details.append(f"图像矩阵[{idx}]: {val}")
                if self.parse_image:
                    self._image_matrix[idx] = val
            except:
                details.append(f"矩阵值: 0x{data:08X}")

        else:
            details.append(f"数据: 0x{data:08X}")

        return details

    def parse_log(self, log_text: str) -> List[ParsedCommand]:
        """解析完整日志

        注意：DATA命令后面会跟随路径数据，这些数据不是GPU命令。
        DATA命令格式: 0x4NNNNNNN，其中NNNNNNN是后续数据的DWORD数量。
        由于dump日志是每行打印2个DWORD（8字节），我们需要跳过 count/2 行。
        """
        self.commands = []
        self.command_sections = []
        self.current_section = None
        offset = 0
        skip_lines = 0  # 需要跳过的数据行数
        pending_path_data = []  # 收集路径数据用于显示

        def start_new_section(name: str):
            """开始新的命令段"""
            nonlocal offset
            # 保存当前段
            if self.current_section and self.current_section["commands"]:
                self.command_sections.append(self.current_section)
            # 创建新段
            self.current_section = {
                "name": name,
                "address": None,
                "size": None,
                "commands": [],
                "abnormal_commands": [],
            }
            offset = 0  # 重置偏移量

        lines = log_text.strip().split("\n")
        i = 0
        while i < len(lines):
            original_line = lines[i]
            line = self.clean_log_line(original_line)
            i += 1

            if not line:
                continue

            # 跳过头部信息 (检查清理后的内容)
            line_lower = line.lower()
            if "init command buffer" in line_lower:
                start_new_section("初始化命令缓冲区")
                continue
            if "last submit command" in line_lower:
                if "before hang" in line_lower:
                    start_new_section("挂起前最后提交的命令")
                else:
                    start_new_section("最后提交的命令")
                continue
            if "addr 0x" in line_lower and "size 0x" in line_lower:
                # 解析地址和大小信息
                addr_match = re.search(r"addr\s+(0x[0-9A-Fa-f]+)", line, re.IGNORECASE)
                size_match = re.search(r"size\s+(0x[0-9A-Fa-f]+)", line, re.IGNORECASE)
                if addr_match and size_match and self.current_section:
                    self.current_section["address"] = addr_match.group(1)
                    self.current_section["size"] = size_match.group(1)
                continue
            if "idle reg" in line_lower or "vg idle reg" in line_lower:
                # GPU空闲寄存器 - 不开始新段，只记录状态
                continue

            # 如果需要跳过路径数据行
            if skip_lines > 0:
                # 提取这行的数据作为路径数据
                pattern = r"(0x[0-9A-Fa-f]{8})\s+(0x[0-9A-Fa-f]{8})"
                match = re.search(pattern, line)
                if match:
                    d1 = int(match.group(1), 16)
                    d2 = int(match.group(2), 16)
                    pending_path_data.append((d1, d2))
                skip_lines -= 1
                offset += 8
                continue

            # 解析命令对
            cmd = self.parse_line(original_line, offset)
            if cmd:
                # 如果还没有任何段，创建一个默认段
                if self.current_section is None:
                    self.current_section = {
                        "name": "命令缓冲区",
                        "address": None,
                        "size": None,
                        "commands": [],
                        "abnormal_commands": [],
                    }

                # 如果有待处理的路径数据，附加到上一个DATA命令
                if pending_path_data and self.current_section["commands"]:
                    last_cmd = self.current_section["commands"][-1]
                    if last_cmd.cmd_type == "DATA":
                        last_cmd.path_data = self._flatten_path_data(pending_path_data)
                        # 解析路径数据
                        if self.parse_path:
                            path_parser = VGLitePathParser(self.current_path_format)
                            last_cmd.path_segments = path_parser.parse_path_data(
                                last_cmd.path_data
                            )
                pending_path_data = []

                # 添加到当前段
                self.current_section["commands"].append(cmd)
                if cmd.is_abnormal:
                    self.current_section["abnormal_commands"].append(cmd)
                # 同时添加到总命令列表（保持向后兼容）
                self.commands.append(cmd)
                if cmd.is_abnormal:
                    self.abnormal_commands.append(cmd)
                offset += 8

                # 如果是DATA命令，计算需要跳过的行数并开始收集数据
                if cmd.cmd_type == "DATA":
                    data_count = cmd.cmd_word & 0x0FFFFFFF
                    # DATA命令的结构 (参考 vg_lite_path.c):
                    # - buffer[0] = VG_LITE_DATA((path_length + 7) / 8) = DATA命令
                    # - buffer[1] = 0 (填充，不是路径数据)
                    # - buffer[2..N] = 实际路径数据 (memcpy复制)
                    #
                    # 所以：
                    # - cmd_word = 0x4NNNNNNN, N = 64-bit数据块数量
                    # - data_word = 0x00000000 (填充，不包含路径数据)
                    # - 路径数据在后续的行中
                    #
                    # data_count 个 64-bit 块 = data_count * 8 字节
                    # 每行有 2 个 32-bit DWORD = 8 字节
                    # 所以需要跳过 data_count 行

                    # 容错处理：如果数据量异常大，可能是路径数据被误识别为DATA命令
                    # 限制最大跳过行数，避免跳过整个文件
                    MAX_REASONABLE_DATA_COUNT = 0x1000  # 最大4096条目 = 32KB路径数据
                    if data_count > MAX_REASONABLE_DATA_COUNT:
                        # 数据量异常，不跳过任何行，继续正常解析
                        # 这样可以避免路径数据中的浮点数被误识别为DATA命令后跳过大量有效命令
                        skip_lines = 0
                        pending_path_data = []
                    else:
                        skip_lines = data_count  # 后续有 data_count 行，每行 8 字节
                        pending_path_data = []  # 路径数据从下一行开始

        # 保存最后一个段
        if self.current_section and self.current_section["commands"]:
            self.command_sections.append(self.current_section)

        # 处理最后可能残留的路径数据
        if (
            pending_path_data
            and self.current_section
            and self.current_section["commands"]
        ):
            last_cmd = self.current_section["commands"][-1]
            if last_cmd.cmd_type == "DATA":
                # 将pending_path_data转换为正确的格式
                last_cmd.path_data = self._flatten_path_data(pending_path_data)
                # 解析路径数据
                if self.parse_path:
                    path_parser = VGLitePathParser(self.current_path_format)
                    last_cmd.path_segments = path_parser.parse_path_data(
                        last_cmd.path_data
                    )

        # 为所有DATA命令解析路径数据
        if self.parse_path:
            for cmd in self.commands:
                if cmd.cmd_type == "DATA" and cmd.path_data and not cmd.path_segments:
                    path_parser = VGLitePathParser(self.current_path_format)
                    cmd.path_segments = path_parser.parse_path_data(cmd.path_data)

        return self.commands

    def _flatten_path_data(self, pending_data: list) -> List[Tuple[int, int]]:
        """将收集的路径数据转换为 (dword1, dword2) 对的列表"""
        # 展平所有数据
        flat = []
        for item in pending_data:
            if isinstance(item, tuple):
                flat.extend(item)
            else:
                flat.append(item)

        # 转换为对
        result = []
        for i in range(0, len(flat) - 1, 2):
            result.append((flat[i], flat[i + 1]))
        # 如果有落单的，补0
        if len(flat) % 2 == 1:
            result.append((flat[-1], 0))

        return result

    def format_output(self, cmd: ParsedCommand) -> str:
        """格式化输出"""
        lines = []

        # 异常命令标记
        abnormal_marker = "⚠️ " if cmd.is_abnormal else "   "

        lines.append(
            f"{abnormal_marker}[{cmd.offset:04X}] {cmd.cmd_word:08X} {cmd.data_word:08X}  "
            f"| {cmd.cmd_type:10s} | {cmd.description}"
        )

        if self.verbose and cmd.details:
            for detail in cmd.details:
                lines.append(f"{'':57s}  └─ {detail}")

        # 显示解析后的路径段 (--parse-path 模式)
        if self.parse_path and cmd.cmd_type == "DATA" and cmd.path_segments:
            lines.append(f"{'':57s}  └─ 路径 ({len(cmd.path_segments)} 段):")
            for seg in cmd.path_segments:
                lines.append(f"{'':57s}     {seg}")
        # 在verbose模式下显示原始路径数据
        elif self.verbose and cmd.cmd_type == "DATA" and cmd.path_data:
            lines.append(f"{'':57s}  └─ 路径数据:")
            for d1, d2 in cmd.path_data[:8]:  # 最多显示8条
                # 尝试解释为浮点数
                try:
                    f1 = struct.unpack("f", struct.pack("I", d1))[0]
                    f2 = struct.unpack("f", struct.pack("I", d2))[0]
                    lines.append(
                        f"{'':57s}     {d1:08X} {d2:08X} (float: {f1:.2f}, {f2:.2f})"
                    )
                except:
                    lines.append(f"{'':57s}     {d1:08X} {d2:08X}")
            if len(cmd.path_data) > 8:
                lines.append(f"{'':57s}     ... 还有 {len(cmd.path_data) - 8} 条数据")

        # 显示异常原因
        if cmd.is_abnormal and cmd.abnormal_reasons:
            for reason in cmd.abnormal_reasons:
                lines.append(f"{'':57s}  ⚠️ {reason}")

        return "\n".join(lines)

    def print_summary(self):
        """打印命令统计"""
        # 如果有分段，按段显示统计
        if self.command_sections:
            print("\n" + "=" * 80)
            print("命令缓冲区分析汇总")
            print("=" * 80)

            for section in self.command_sections:
                print(f"\n【{section['name']}】")
                if section["address"]:
                    print(f"  地址: {section['address']}")
                if section["size"]:
                    print(f"  大小: {section['size']}")

                cmd_counts = {}
                for cmd in section["commands"]:
                    cmd_counts[cmd.cmd_type] = cmd_counts.get(cmd.cmd_type, 0) + 1

                print("  命令统计:")
                for cmd_type, count in sorted(cmd_counts.items()):
                    print(f"    {cmd_type:15s}: {count:5d}")
                print(f"    {'总计':15s}: {len(section['commands']):5d}")

                # 打印该段的异常命令
                abnormal_in_section = [
                    cmd for cmd in section["commands"] if cmd.is_abnormal
                ]
                if abnormal_in_section:
                    print(f"\n  ⚠️  该段检测到 {len(abnormal_in_section)} 个异常命令:")
                    for cmd in abnormal_in_section:
                        print(
                            f"    [{cmd.offset:04X}] {cmd.cmd_word:08X} {cmd.data_word:08X} - {cmd.cmd_type}"
                        )
                        for reason in cmd.abnormal_reasons:
                            print(f"           └─ {reason}")
        else:
            # 兼容旧模式
            cmd_counts = {}
            for cmd in self.commands:
                cmd_counts[cmd.cmd_type] = cmd_counts.get(cmd.cmd_type, 0) + 1

            print("\n" + "=" * 70)
            print("命令统计")
            print("=" * 70)
            for cmd_type, count in sorted(cmd_counts.items()):
                print(f"  {cmd_type:15s}: {count:5d}")
            print(f"  {'总计':15s}: {len(self.commands):5d}")

            # 打印异常命令汇总
            if self.abnormal_commands:
                print("\n" + "=" * 70)
                print(f"⚠️  检测到 {len(self.abnormal_commands)} 个异常命令")
                print("=" * 70)
                for cmd in self.abnormal_commands:
                    print(
                        f"  [{cmd.offset:04X}] {cmd.cmd_word:08X} {cmd.data_word:08X} - {cmd.cmd_type}"
                    )
                    for reason in cmd.abnormal_reasons:
                        print(f"         └─ {reason}")


def parse_file(filename: str, verbose: bool = False) -> List[ParsedCommand]:
    """从文件解析日志 (旧版，保持兼容)"""
    return parse_file_v2(filename, verbose=verbose, parse_path=False)


def parse_string(log_text: str, verbose: bool = False) -> List[ParsedCommand]:
    """从字符串解析日志 (旧版，保持兼容)"""
    return parse_string_v2(log_text, verbose=verbose, parse_path=False)


def _create_command_table(title: str, address: str = None, size: str = None) -> Table:
    """创建命令表格"""
    table = Table(title=title, show_header=True, header_style="bold cyan")
    table.add_column("偏移", style="dim", width=8, justify="right")
    table.add_column("命令字", style="yellow", width=10)
    table.add_column("数据字", style="yellow", width=10)
    table.add_column("类型", style="green", width=12)
    table.add_column("描述", style="white")

    if address or size:
        info = []
        if address:
            info.append(f"地址: {address}")
        if size:
            info.append(f"大小: {size}")
        table.caption = " | ".join(info)

    return table


def _add_command_to_table(
    table: Table, cmd: ParsedCommand, parser: VGLiteCommandParser
):
    """将命令添加到表格"""
    # 异常命令使用红色
    if cmd.is_abnormal:
        style = "bold red"
        offset = f"⚠️ {cmd.offset:04X}"
    else:
        style = None
        offset = f"{cmd.offset:04X}"

    # 构建描述
    desc_parts = [cmd.description]

    # 添加详情
    if parser.verbose and cmd.details:
        for detail in cmd.details:
            desc_parts.append(f"  └─ {detail}")

    # 添加路径段
    if parser.parse_path and cmd.cmd_type == "DATA" and cmd.path_segments:
        desc_parts.append(f"  └─ 路径 ({len(cmd.path_segments)} 段):")
        for seg in cmd.path_segments:
            desc_parts.append(f"     {seg}")

    # 图片绘制信息：在 DATA(1) 命令（矩形绘制）时显示当前图片信息
    if parser.parse_image and cmd.cmd_type == "DATA":
        data_count = cmd.cmd_word & 0x0FFFFFFF
        if data_count == 1 and parser.image_draws:
            # 找到对应这个偏移的图片绘制记录
            for img in parser.image_draws:
                if img.offset == cmd.offset:
                    img_info = []
                    if img.src_address:
                        img_info.append(f"源: 0x{img.src_address:08X}")
                    if img.src_format != "UNKNOWN":
                        img_info.append(f"{img.src_format}")
                    if img.src_width and img.src_height:
                        img_info.append(f"{img.src_width}x{img.src_height}")
                    if img.src_stride:
                        img_info.append(f"步长:{img.src_stride}")
                    mem = img.calc_memory_size()
                    if mem > 0:
                        mem_str = f"{mem // 1024}KB" if mem >= 1024 else f"{mem}B"
                        img_info.append(f"({mem_str})")
                    if img.blend_mode != "SRC_OVER":
                        img_info.append(f"混合:{img.blend_mode}")
                    matrix_str = img.get_matrix_str()
                    if matrix_str != "Identity":
                        img_info.append(f"变换:{matrix_str}")
                    if img_info:
                        desc_parts.append(f"  🖼️ {' '.join(img_info)}")
                    break

    # 添加异常原因
    if cmd.is_abnormal and cmd.abnormal_reasons:
        for reason in cmd.abnormal_reasons:
            desc_parts.append(f"  ⚠️ {reason}")

    table.add_row(
        offset,
        f"{cmd.cmd_word:08X}",
        f"{cmd.data_word:08X}",
        cmd.cmd_type,
        "\n".join(desc_parts),
        style=style,
    )


def _print_summary_rich(parser: VGLiteCommandParser, console: Console):
    """使用 rich 打印统计汇总"""
    if parser.command_sections:
        summary_table = Table(
            title="命令缓冲区分析汇总", show_header=True, header_style="bold magenta"
        )
        summary_table.add_column("段落", style="cyan")
        summary_table.add_column("地址", style="dim")
        summary_table.add_column("大小", style="dim")
        summary_table.add_column("命令统计", style="green")
        summary_table.add_column("异常", style="red")

        for section in parser.command_sections:
            cmd_counts = {}
            for cmd in section["commands"]:
                cmd_counts[cmd.cmd_type] = cmd_counts.get(cmd.cmd_type, 0) + 1

            stats = ", ".join([f"{k}:{v}" for k, v in sorted(cmd_counts.items())])
            total = len(section["commands"])
            stats += f" (共{total}条)"

            abnormal_count = sum(1 for cmd in section["commands"] if cmd.is_abnormal)
            abnormal_str = f"{abnormal_count}" if abnormal_count > 0 else "-"

            summary_table.add_row(
                section["name"],
                section["address"] or "-",
                section["size"] or "-",
                stats,
                abnormal_str,
            )

        console.print()
        console.print(summary_table)

        # 打印异常命令详情
        all_abnormal = []
        for section in parser.command_sections:
            for cmd in section["commands"]:
                if cmd.is_abnormal:
                    all_abnormal.append((section["name"], cmd))

        if all_abnormal:
            console.print()
            abnormal_table = Table(
                title=f"⚠️ 异常命令详情 ({len(all_abnormal)} 个)",
                show_header=True,
                header_style="bold red",
            )
            abnormal_table.add_column("段落", style="cyan", width=20)
            abnormal_table.add_column("偏移", style="dim", width=8)
            abnormal_table.add_column("命令字", style="yellow", width=10)
            abnormal_table.add_column("数据字", style="yellow", width=10)
            abnormal_table.add_column("类型", width=10)
            abnormal_table.add_column("原因", style="red")

            for section_name, cmd in all_abnormal[:20]:  # 最多显示20个
                abnormal_table.add_row(
                    section_name,
                    f"{cmd.offset:04X}",
                    f"{cmd.cmd_word:08X}",
                    f"{cmd.data_word:08X}",
                    cmd.cmd_type,
                    "\n".join(cmd.abnormal_reasons),
                )

            if len(all_abnormal) > 20:
                console.print(
                    f"[dim]... 还有 {len(all_abnormal) - 20} 个异常命令未显示[/dim]"
                )

            console.print(abnormal_table)

        # 图片绘制统计
        if parser.parse_image and parser.image_draws:
            console.print()
            img_table = Table(
                title=f"🖼️ 图片绘制统计 ({len(parser.image_draws)} 次)",
                show_header=True,
                header_style="bold cyan",
            )
            img_table.add_column("指标", style="cyan")
            img_table.add_column("值", style="green")

            # 按格式统计
            format_counts = {}
            for img in parser.image_draws:
                fmt = img.src_format
                format_counts[fmt] = format_counts.get(fmt, 0) + 1

            # 按混合模式统计
            blend_counts = {}
            for img in parser.image_draws:
                blend_counts[img.blend_mode] = blend_counts.get(img.blend_mode, 0) + 1

            # 总内存
            total_mem = sum(img.calc_memory_size() for img in parser.image_draws)
            mem_str = f"{total_mem // 1024}KB" if total_mem >= 1024 else f"{total_mem}B"

            img_table.add_row(
                "格式分布",
                ", ".join(f"{k}:{v}" for k, v in sorted(format_counts.items())),
            )
            img_table.add_row(
                "混合模式",
                ", ".join(f"{k}:{v}" for k, v in sorted(blend_counts.items())),
            )
            img_table.add_row("图片总数据量", mem_str)

            # 检测重复绘制
            src_addr_counts = {}
            for img in parser.image_draws:
                src_addr_counts[img.src_address] = (
                    src_addr_counts.get(img.src_address, 0) + 1
                )
            repeated = sum(1 for c in src_addr_counts.values() if c > 1)
            if repeated > 0:
                img_table.add_row("重复绘制", f"{repeated} 个图片被多次绘制")

            console.print(img_table)
    else:
        # 兼容无段落模式
        cmd_counts = {}
        for cmd in parser.commands:
            cmd_counts[cmd.cmd_type] = cmd_counts.get(cmd.cmd_type, 0) + 1

        summary_table = Table(
            title="命令统计", show_header=True, header_style="bold magenta"
        )
        summary_table.add_column("类型", style="cyan")
        summary_table.add_column("数量", style="green", justify="right")

        for cmd_type, count in sorted(cmd_counts.items()):
            summary_table.add_row(cmd_type, str(count))
        summary_table.add_row("总计", str(len(parser.commands)), style="bold")

        console.print()
        console.print(summary_table)


def parse_file_v2(
    filename: str,
    verbose: bool = False,
    parse_path: bool = False,
    check_integrity: bool = False,
    parse_image: bool = False,
) -> List[ParsedCommand]:
    """从文件解析日志"""
    with open(filename, "r") as f:
        log_text = f.read()

    console = Console()

    # 日志完整性检测
    if check_integrity:
        lines = log_text.splitlines()
        checker = LogIntegrityChecker()
        checker.analyze(lines)
        checker.print_report(console)

    parser = VGLiteCommandParser(
        verbose=verbose, parse_path=parse_path, parse_image=parse_image
    )
    commands = parser.parse_log(log_text)

    # 按段输出
    if parser.command_sections:
        for section in parser.command_sections:
            table = _create_command_table(
                f"【{section['name']}】", section["address"], section["size"]
            )

            for cmd in section["commands"]:
                _add_command_to_table(table, cmd, parser)

            console.print(table)
            console.print()
    else:
        # 兼容无段落的日志
        table = _create_command_table("VGLite 命令缓冲区解析结果")
        for cmd in commands:
            _add_command_to_table(table, cmd, parser)
        console.print(table)

    _print_summary_rich(parser, console)

    return commands


def parse_string_v2(
    log_text: str, verbose: bool = False, parse_path: bool = False
) -> List[ParsedCommand]:
    """从字符串解析日志"""
    parser = VGLiteCommandParser(verbose=verbose, parse_path=parse_path)
    commands = parser.parse_log(log_text)
    console = Console()

    # 按段输出
    if parser.command_sections:
        for section in parser.command_sections:
            table = _create_command_table(
                f"【{section['name']}】", section["address"], section["size"]
            )

            for cmd in section["commands"]:
                _add_command_to_table(table, cmd, parser)

            console.print(table)
            console.print()
    else:
        # 兼容无段落的日志
        table = _create_command_table("VGLite 命令缓冲区解析结果")
        for cmd in commands:
            _add_command_to_table(table, cmd, parser)
        console.print(table)

    _print_summary_rich(parser, console)
    return commands


def interactive_mode(verbose: bool = False, parse_path: bool = False):
    """交互模式: 从标准输入读取日志"""
    print("VGLite 命令缓冲区解析器 - 交互模式")
    print("请粘贴日志内容，输入空行结束:")
    print("-" * 40)

    lines = []
    while True:
        try:
            line = input()
            if not line:
                break
            lines.append(line)
        except EOFError:
            break

    if lines:
        log_text = "\n".join(lines)
        parse_string_v2(log_text, verbose, parse_path)
    else:
        print("没有输入内容")


# ============================================================================
# GPU 寄存器分析功能
# ============================================================================


class GPURegisterAnalyzer:
    """GPU 硬件寄存器分析器"""

    def __init__(self):
        self.registers = {}  # 单值寄存器
        self.register_arrays = {}  # 数组形式的寄存器

    def parse_registers(self, log_text: str):
        """从日志中解析寄存器值"""
        lines = log_text.split("\n")

        for line in lines:
            # 跳过命令缓冲区部分
            if (
                "init command buffer" in line.lower()
                or "last submit command" in line.lower()
            ):
                break

            # 匹配 "idle = 0x..." 或 "AQHiClockControl = 0x..." 格式
            match = re.search(r"(\w+)\s*=\s*(0x[0-9a-fA-F]+)", line)
            if match:
                name = match.group(1)
                value = match.group(2).lower()
                self.registers[name] = value
                continue

            # 匹配 "0xXX = 0x..." 格式 (单个寄存器)
            match = re.search(
                r"\]\s*(?:\[ap\])?\s*(0x[0-9a-fA-F]+)\s*=\s*(0x[0-9a-fA-F]+)", line
            )
            if match:
                addr = match.group(1).lower()
                value = match.group(2).lower()
                self.registers[addr] = value
                continue

            # 匹配 "0xXX[N] = 0x..." 格式 (寄存器数组)
            match = re.search(
                r"\]\s*(?:\[ap\])?\s*(0x[0-9a-fA-F]+)\[(\d+)\]\s*=\s*(0x[0-9a-fA-F]+)",
                line,
            )
            if match:
                addr = match.group(1).lower()
                index = int(match.group(2))
                value = match.group(3).lower()
                if addr not in self.register_arrays:
                    self.register_arrays[addr] = {}
                self.register_arrays[addr][index] = value

    def analyze(self, console: Console):
        """分析并输出寄存器信息"""
        if not self.registers and not self.register_arrays:
            return

        console.print(Panel.fit("VGLite GPU 寄存器分析", style="bold magenta"))

        # 基本状态和芯片信息表
        self._print_basic_registers(console)

        # 命令队列寄存器
        self._print_cmd_queue_registers(console)

        # 内存和调试寄存器
        self._print_debug_registers(console)

        # MMU寄存器
        self._print_mmu_registers(console)

        # 寄存器数组
        self._print_register_arrays(console)

        # 关键状态分析
        self._print_analysis(console)

    def _print_basic_registers(self, console: Console):
        """打印基本状态寄存器"""
        table = Table(
            title="基本状态和芯片信息", show_header=True, header_style="bold cyan"
        )
        table.add_column("地址", style="yellow", width=20)
        table.add_column("名称", style="green", width=22)
        table.add_column("值", style="cyan", width=12)
        table.add_column("说明", style="white", width=40)

        basic_regs = [
            "idle",
            "AQHiClockControl",
            "0x0c",
            "0x10",
            "0x14",
            "0x18",
            "0x1c",
            "0x20",
            "0x24",
            "0x28",
            "0x2c",
            "0x30",
            "0x34",
        ]

        has_data = False
        for reg in basic_regs:
            if reg in self.registers and reg in GPU_REGISTER_INFO:
                name, desc = GPU_REGISTER_INFO[reg]
                table.add_row(reg, name, self.registers[reg], desc)
                has_data = True

        if has_data:
            console.print(table)
            console.print()

    def _print_cmd_queue_registers(self, console: Console):
        """打印命令队列寄存器"""
        table = Table(
            title="命令队列寄存器 (AQ)", show_header=True, header_style="bold cyan"
        )
        table.add_column("地址", style="yellow", width=8)
        table.add_column("名称", style="green", width=22)
        table.add_column("值", style="cyan", width=12)
        table.add_column("说明", style="white", width=40)

        cmd_regs = [
            "0x40",
            "0x44",
            "0x48",
            "0x4c",
            "0x50",
            "0x54",
            "0x58",
            "0x5c",
            "0x60",
        ]

        has_data = False
        for reg in cmd_regs:
            if reg in self.registers and reg in GPU_REGISTER_INFO:
                name, desc = GPU_REGISTER_INFO[reg]
                table.add_row(reg, name, self.registers[reg], desc)
                has_data = True

        if has_data:
            console.print(table)
            console.print()

    def _print_debug_registers(self, console: Console):
        """打印调试寄存器"""
        table = Table(
            title="内存和调试寄存器", show_header=True, header_style="bold cyan"
        )
        table.add_column("地址", style="yellow", width=8)
        table.add_column("名称", style="green", width=22)
        table.add_column("值", style="cyan", width=12)
        table.add_column("说明", style="white", width=40)

        mem_regs = [
            "0x98",
            "0xa4",
            "0xa8",
            "0xe8",
            "0x100",
            "0x104",
            "0x108",
            "0x438",
            "0x43c",
            "0x440",
            "0x444",
        ]

        has_data = False
        for reg in mem_regs:
            if reg in self.registers and reg in GPU_REGISTER_INFO:
                name, desc = GPU_REGISTER_INFO[reg]
                table.add_row(reg, name, self.registers[reg], desc)
                has_data = True

        if has_data:
            console.print(table)
            console.print()

    def _print_mmu_registers(self, console: Console):
        """打印MMU寄存器"""
        table = Table(title="MMU 寄存器", show_header=True, header_style="bold cyan")
        table.add_column("地址", style="yellow", width=8)
        table.add_column("名称", style="green", width=22)
        table.add_column("值", style="cyan", width=12)
        table.add_column("说明", style="white", width=40)

        mmu_regs = ["0x500", "0x504", "0x508"]

        has_data = False
        for reg in mmu_regs:
            if reg in self.registers and reg in GPU_REGISTER_INFO:
                name, desc = GPU_REGISTER_INFO[reg]
                table.add_row(reg, name, self.registers[reg], desc)
                has_data = True

        if has_data:
            console.print(table)
            console.print()

    def _print_register_arrays(self, console: Console):
        """打印寄存器数组"""
        if not self.register_arrays:
            return

        console.print(Panel.fit("调试寄存器组 (数组形式)", style="bold blue"))

        array_names = {
            "0x448": "模块调试寄存器组 (AQModuleDebug)",
            "0x450": "管线调试寄存器组 (AQPipeDebug)",
            "0x454": "获取调试寄存器组 (AQFetchDebug)",
            "0x45c": "渲染调试寄存器组 (AQRenderDebug)",
            "0x468": "细分调试寄存器组 (AQTessDebug)",
            "0x46c": "路径调试寄存器组 (AQPathDebug)",
        }

        for addr, values in sorted(self.register_arrays.items()):
            title = array_names.get(addr, f"寄存器组 {addr}")
            table = Table(
                title=f"{addr} {title}", show_header=True, header_style="bold cyan"
            )
            table.add_column("索引", style="yellow", width=6)
            table.add_column("值", style="cyan", width=12)
            table.add_column("备注", style="dim")

            for idx in sorted(values.keys()):
                val = values[idx]
                note = ""
                # 检测特殊值
                if val == "0xbabef00d":
                    note = "未初始化标记"
                elif val == "0x12345678":
                    note = "测试/调试标记"
                elif val == "0xaaaaaaaa":
                    note = "填充标记"
                table.add_row(str(idx), val, note)

            console.print(table)
            console.print()

    def _print_analysis(self, console: Console):
        """打印关键状态分析"""
        console.print(Panel.fit("关键状态分析", style="bold yellow"))

        analysis = Table(show_header=True, header_style="bold green")
        analysis.add_column("项目", style="cyan", width=25)
        analysis.add_column("分析", style="white", width=60)

        # 芯片信息
        if "0x1c" in self.registers:
            chip_id = int(self.registers["0x1c"], 16)
            analysis.add_row("芯片ID", f"0x{chip_id:08X} (Vivante GC系列)")

        if "0x20" in self.registers:
            chip_rev = int(self.registers["0x20"], 16)
            analysis.add_row("芯片版本", f"0x{chip_rev:04X}")

        if "0x28" in self.registers:
            chip_time = self.registers["0x28"]
            # 尝试解析日期格式
            try:
                val = int(chip_time, 16)
                year = val // 10000
                month = (val % 10000) // 100
                day = val % 100
                analysis.add_row(
                    "芯片日期", f"{chip_time} ({year}-{month:02d}-{day:02d})"
                )
            except:
                analysis.add_row("芯片日期", chip_time)

        # 命令缓冲区状态
        if "0x4c" in self.registers:
            cmd_start = int(self.registers["0x4c"], 16)
            analysis.add_row("命令缓冲区起始", f"0x{cmd_start:08X}")

        if "0x40" in self.registers:
            cmd_addr = int(self.registers["0x40"], 16)
            analysis.add_row("命令缓冲区当前", f"0x{cmd_addr:08X}")

        if "0x50" in self.registers:
            fetch_addr = int(self.registers["0x50"], 16)
            analysis.add_row("获取地址/PC", f"0x{fetch_addr:08X}")

        # GPU状态
        if "idle" in self.registers:
            idle_val = int(self.registers["idle"], 16)
            if idle_val == 0x7FFFFFFF:
                analysis.add_row("GPU状态", "[green]空闲 (所有模块idle)[/green]")
            else:
                analysis.add_row("GPU状态", f"[red]忙碌 (idle=0x{idle_val:08X})[/red]")

        # MMU状态
        if "0x504" in self.registers:
            mmu_status = int(self.registers["0x504"], 16)
            analysis.add_row("MMU状态", f"0x{mmu_status:03X}")

        if "0x508" in self.registers:
            mmu_exception = int(self.registers["0x508"], 16)
            if mmu_exception != 0:
                analysis.add_row(
                    "MMU异常地址", f"[bold red]0x{mmu_exception:08X}[/bold red]"
                )
                analysis.add_row("", "[red]⚠️ MMU异常! 可能是非法内存访问[/red]")

        console.print(analysis)
        console.print()


def parse_with_registers(
    filename: str,
    verbose: bool = False,
    parse_path: bool = False,
    check_integrity: bool = False,
    parse_image: bool = False,
):
    """解析日志文件，包括寄存器分析和命令缓冲区解析"""
    with open(filename, "r") as f:
        log_text = f.read()

    console = Console()

    # 日志完整性检测
    if check_integrity:
        lines = log_text.splitlines()
        checker = LogIntegrityChecker()
        checker.analyze(lines)
        checker.print_report(console)

    # 先分析寄存器
    reg_analyzer = GPURegisterAnalyzer()
    reg_analyzer.parse_registers(log_text)
    reg_analyzer.analyze(console)

    # 再解析命令缓冲区
    parser = VGLiteCommandParser(
        verbose=verbose, parse_path=parse_path, parse_image=parse_image
    )
    commands = parser.parse_log(log_text)

    # 按段输出
    if parser.command_sections:
        for section in parser.command_sections:
            table = _create_command_table(
                f"【{section['name']}】", section["address"], section["size"]
            )

            for cmd in section["commands"]:
                _add_command_to_table(table, cmd, parser)

            console.print(table)
            console.print()
    else:
        # 兼容无段落的日志
        table = _create_command_table("VGLite 命令缓冲区解析结果")
        for cmd in commands:
            _add_command_to_table(table, cmd, parser)
        console.print(table)

    _print_summary_rich(parser, console)

    return commands


def main():
    """主函数"""
    arg_parser = argparse.ArgumentParser(
        description="VGLite GPU 命令缓冲区解析器",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  # 从文件解析 (包含寄存器分析)
  python vglite_cmdbuf_parser.py -f dump.log

  # 解析并分析寄存器信息
  python vglite_cmdbuf_parser.py -f dump.log -r

  # 详细模式
  python vglite_cmdbuf_parser.py -f dump.log -v

  # 解析路径数据
  python vglite_cmdbuf_parser.py -f dump.log -p

  # 详细模式 + 解析路径 + 寄存器分析
  python vglite_cmdbuf_parser.py -f dump.log -v -p -r

  # 检测日志完整性问题
  python vglite_cmdbuf_parser.py -f dump.log -c

  # 分析图片绘制
  python vglite_cmdbuf_parser.py -f dump.log -I

  # 交互模式
  python vglite_cmdbuf_parser.py -i

  # 解析字符串
  python vglite_cmdbuf_parser.py -s "0x30010A00 0x00000100"
""",
    )

    arg_parser.add_argument("-f", "--file", help="日志文件路径")
    arg_parser.add_argument("-s", "--string", help="直接解析命令字符串")
    arg_parser.add_argument(
        "-i", "--interactive", action="store_true", help="交互模式,从标准输入读取"
    )
    arg_parser.add_argument("-v", "--verbose", action="store_true", help="详细输出模式")
    arg_parser.add_argument(
        "-p",
        "--parse-path",
        action="store_true",
        help="解析路径数据详情 (显示MOVE/LINE/CUBIC等指令)",
    )
    arg_parser.add_argument(
        "-r",
        "--regs",
        action="store_true",
        help="分析并打印GPU寄存器信息",
    )
    arg_parser.add_argument(
        "-c",
        "--check-integrity",
        action="store_true",
        help="检测日志完整性问题 (并发输出导致的数据损坏)",
    )
    arg_parser.add_argument(
        "-I",
        "--parse-image",
        action="store_true",
        help="分析图片绘制操作 (源/目标地址、格式、变换矩阵等)",
    )

    args = arg_parser.parse_args()

    if args.file:
        if args.regs:
            parse_with_registers(
                args.file,
                args.verbose,
                args.parse_path,
                args.check_integrity,
                args.parse_image,
            )
        else:
            parse_file_v2(
                args.file,
                args.verbose,
                args.parse_path,
                args.check_integrity,
                args.parse_image,
            )
    elif args.string:
        parse_string_v2(args.string, args.verbose, args.parse_path)
    elif args.interactive:
        interactive_mode(args.verbose, args.parse_path)
    else:
        # 默认交互模式
        interactive_mode(args.verbose, args.parse_path)


if __name__ == "__main__":
    main()
