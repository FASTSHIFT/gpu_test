#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite 路径数据解析器
====================
解析VGLite路径绘制命令中的路径数据
"""

import struct
from typing import List, Optional, Tuple

try:
    from .constants import VLC_OP_CODES
    from .models import PathSegment
except ImportError:
    from constants import VLC_OP_CODES
    from models import PathSegment


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
