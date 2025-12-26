#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite 数据模型
===============
包含解析过程中使用的数据类定义
"""

from dataclasses import dataclass, field
from typing import List, Tuple, Optional


@dataclass
class LogIntegrityIssue:
    """日志完整性问题"""

    line_number: int  # 行号 (1-based)
    original_line: str  # 原始行内容
    issue_type: str  # 问题类型
    description: str  # 问题描述


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


@dataclass
class CommandSection:
    """命令段"""

    name: str
    address: str = ""
    size: str = ""
    commands: List[ParsedCommand] = field(default_factory=list)
    abnormal_commands: List[ParsedCommand] = field(default_factory=list)
