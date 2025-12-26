#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite 命令缓冲区解析器
======================
解析 VGLite GPU 命令缓冲区数据
"""

import re
import struct
from typing import List, Optional, Tuple

try:
    from .constants import (
        CommandType,
        REGISTER_MAP,
        BLEND_MODES,
        IMAGE_FORMATS,
        ADDRESS_REGISTERS,
        VALID_REG_RANGES,
        SUSPICIOUS_ADDRESSES,
    )
    from .models import ParsedCommand, ImageDrawInfo, PathSegment
    from .path_parser import VGLitePathParser
except ImportError:
    from constants import (
        CommandType,
        REGISTER_MAP,
        BLEND_MODES,
        IMAGE_FORMATS,
        ADDRESS_REGISTERS,
        VALID_REG_RANGES,
        SUSPICIOUS_ADDRESSES,
    )
    from models import ParsedCommand, ImageDrawInfo, PathSegment
    from path_parser import VGLitePathParser


class VGLiteCommandParser:
    """VGLite命令缓冲区解析器"""

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
        self.command_sections: List[dict] = []
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
        """清理日志行，移除时间戳、ANSI颜色码等前缀"""
        # 移除ANSI转义序列 (颜色码等)
        ansi_pattern = r"(\x1b|\033|\^\[)\[[0-9;]*m"
        line = re.sub(ansi_pattern, "", line)

        # 移除常见的日志前缀格式
        log_prefix_pattern = r"^.*?\[ap\]\s*"
        line = re.sub(log_prefix_pattern, "", line, flags=re.IGNORECASE)

        # 如果上面的模式没匹配，尝试更通用的方式
        timestamp_pattern = r"^\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}[.\d]*\s*"
        line = re.sub(timestamp_pattern, "", line)

        # 移除方括号标签
        bracket_tags_pattern = r"\[[^\]]*\]\s*"
        if not re.match(r"^\s*0x", line):
            line = re.sub(bracket_tags_pattern, "", line)

        return line.strip()

    def parse_line(self, line: str, offset: int = 0) -> Optional[ParsedCommand]:
        """解析单行日志"""
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
                        start <= address <= end for start, end in VALID_REG_RANGES
                    )
                    if not is_valid_range:
                        is_abnormal = True
                        abnormal_reasons.append(f"未知寄存器地址: 0x{address:04X}")

                # 检查地址寄存器写入可疑值
                if address in ADDRESS_REGISTERS:
                    if data_word in SUSPICIOUS_ADDRESSES:
                        is_abnormal = True
                        abnormal_reasons.append(
                            f"可疑地址值: 0x{data_word:08X} (可能为空指针)"
                        )
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

            if sem_id > 31:
                is_abnormal = True
                abnormal_reasons.append(f"信号量ID超出范围: {sem_id}")

        elif opcode == CommandType.STALL:
            cmd_type = "STALL"
            stall_id = cmd_word & 0x0FFFFFFF
            description = f"停顿等待 ID={stall_id}"
            if stall_id == 7:
                details.append("等待所有路径渲染完成")

            if stall_id > 31:
                is_abnormal = True
                abnormal_reasons.append(f"停顿ID超出范围: {stall_id}")

        elif opcode == CommandType.DATA:
            cmd_type = "DATA"
            data_count = cmd_word & 0x0FFFFFFF

            if data_count == 1:
                description = f"矩形绘制数据 (Blit)"
                if self.parse_image and self._current_image.src_address != 0:
                    self._finalize_image_draw(offset)
            else:
                description = f"路径数据 ({data_count * 8} 字节, {data_count} 条目)"

            if data_count > 0x10000:
                is_abnormal = True
                abnormal_reasons.append(f"数据量异常大: {data_count * 8} 字节")

        elif opcode == CommandType.CALL:
            cmd_type = "CALL"
            call_count = cmd_word & 0x0FFFFFFF
            call_bytes = call_count * 8
            description = f"调用上传路径 (Uploaded Path)"
            details.append(f"地址: 0x{data_word:08X}")
            details.append(f"长度: {call_bytes} 字节 ({call_count} 条目)")

            if data_word in SUSPICIOUS_ADDRESSES:
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
            fmt = (data >> 20) & 0x3
            fmt_name = {0: "S8", 1: "S16", 2: "S32", 3: "FP32"}.get(fmt, "?")
            details.append(f"路径格式: {fmt_name}")
            self.current_path_format = fmt_name

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
            try:
                scale = struct.unpack("f", struct.pack("I", data))[0]
                details.append(f"缩放: {scale}")
            except:
                details.append(f"缩放: 0x{data:08X}")

        elif address == 0x0A3C:  # VgPathBias
            try:
                bias = struct.unpack("f", struct.pack("I", data))[0]
                details.append(f"偏移: {bias}")
            except:
                details.append(f"偏移: 0x{data:08X}")

        elif address == 0x0A01:  # VgTargetAddress
            details.append(f"目标地址: 0x{data:08X}")
            if self.parse_image:
                self._current_image.dst_address = data

        elif address == 0x0A29:  # VgSourceAddress
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

        elif address == 0x0A25:  # VgSourceConfig
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
            try:
                val = struct.unpack("f", struct.pack("I", data))[0]
                idx = address - 0x0A40
                row = idx // 3
                col = idx % 3
                details.append(f"路径矩阵[{row}][{col}]: {val}")
            except:
                details.append(f"矩阵值: 0x{data:08X}")

        elif address in range(0x0A30, 0x0A39):  # Image Matrix
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
        """解析完整日志"""
        self.commands = []
        self.command_sections = []
        self.current_section = None
        offset = 0
        skip_lines = 0
        pending_path_data = []

        def start_new_section(name: str):
            nonlocal offset
            if self.current_section and self.current_section["commands"]:
                self.command_sections.append(self.current_section)
            self.current_section = {
                "name": name,
                "address": None,
                "size": None,
                "commands": [],
                "abnormal_commands": [],
            }
            offset = 0

        lines = log_text.strip().split("\n")
        i = 0
        while i < len(lines):
            original_line = lines[i]
            line = self.clean_log_line(original_line)
            i += 1

            if not line:
                continue

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
                addr_match = re.search(r"addr\s+(0x[0-9A-Fa-f]+)", line, re.IGNORECASE)
                size_match = re.search(r"size\s+(0x[0-9A-Fa-f]+)", line, re.IGNORECASE)
                if addr_match and size_match and self.current_section:
                    self.current_section["address"] = addr_match.group(1)
                    self.current_section["size"] = size_match.group(1)
                continue
            if "idle reg" in line_lower or "vg idle reg" in line_lower:
                continue

            if skip_lines > 0:
                pattern = r"(0x[0-9A-Fa-f]{8})\s+(0x[0-9A-Fa-f]{8})"
                match = re.search(pattern, line)
                if match:
                    d1 = int(match.group(1), 16)
                    d2 = int(match.group(2), 16)
                    pending_path_data.append((d1, d2))
                skip_lines -= 1
                offset += 8
                continue

            cmd = self.parse_line(original_line, offset)
            if cmd:
                if self.current_section is None:
                    self.current_section = {
                        "name": "命令缓冲区",
                        "address": None,
                        "size": None,
                        "commands": [],
                        "abnormal_commands": [],
                    }

                if pending_path_data and self.current_section["commands"]:
                    last_cmd = self.current_section["commands"][-1]
                    if last_cmd.cmd_type == "DATA":
                        last_cmd.path_data = self._flatten_path_data(pending_path_data)
                        if self.parse_path:
                            path_parser = VGLitePathParser(self.current_path_format)
                            last_cmd.path_segments = path_parser.parse_path_data(
                                last_cmd.path_data
                            )
                pending_path_data = []

                self.current_section["commands"].append(cmd)
                if cmd.is_abnormal:
                    self.current_section["abnormal_commands"].append(cmd)
                self.commands.append(cmd)
                if cmd.is_abnormal:
                    self.abnormal_commands.append(cmd)
                offset += 8

                if cmd.cmd_type == "DATA":
                    data_count = cmd.cmd_word & 0x0FFFFFFF
                    MAX_REASONABLE_DATA_COUNT = 0x1000
                    if data_count > MAX_REASONABLE_DATA_COUNT:
                        skip_lines = 0
                        pending_path_data = []
                    else:
                        skip_lines = data_count
                        pending_path_data = []

        if self.current_section and self.current_section["commands"]:
            self.command_sections.append(self.current_section)

        if (
            pending_path_data
            and self.current_section
            and self.current_section["commands"]
        ):
            last_cmd = self.current_section["commands"][-1]
            if last_cmd.cmd_type == "DATA":
                last_cmd.path_data = self._flatten_path_data(pending_path_data)
                if self.parse_path:
                    path_parser = VGLitePathParser(self.current_path_format)
                    last_cmd.path_segments = path_parser.parse_path_data(
                        last_cmd.path_data
                    )

        if self.parse_path:
            for cmd in self.commands:
                if cmd.cmd_type == "DATA" and cmd.path_data and not cmd.path_segments:
                    path_parser = VGLitePathParser(self.current_path_format)
                    cmd.path_segments = path_parser.parse_path_data(cmd.path_data)

        return self.commands

    def _flatten_path_data(self, pending_data: list) -> List[Tuple[int, int]]:
        """将收集的路径数据转换为 (dword1, dword2) 对的列表"""
        flat = []
        for item in pending_data:
            if isinstance(item, tuple):
                flat.extend(item)
            else:
                flat.append(item)

        result = []
        for i in range(0, len(flat) - 1, 2):
            result.append((flat[i], flat[i + 1]))
        if len(flat) % 2 == 1:
            result.append((flat[-1], 0))

        return result

    def format_output(self, cmd: ParsedCommand) -> str:
        """格式化输出"""
        lines = []

        abnormal_marker = "⚠️ " if cmd.is_abnormal else "   "

        lines.append(
            f"{abnormal_marker}[{cmd.offset:04X}] {cmd.cmd_word:08X} {cmd.data_word:08X}  "
            f"| {cmd.cmd_type:10s} | {cmd.description}"
        )

        if self.verbose and cmd.details:
            for detail in cmd.details:
                lines.append(f"{'':57s}  └─ {detail}")

        if self.parse_path and cmd.cmd_type == "DATA" and cmd.path_segments:
            lines.append(f"{'':57s}  └─ 路径 ({len(cmd.path_segments)} 段):")
            for seg in cmd.path_segments:
                lines.append(f"{'':57s}     {seg}")
        elif self.verbose and cmd.cmd_type == "DATA" and cmd.path_data:
            lines.append(f"{'':57s}  └─ 路径数据:")
            for d1, d2 in cmd.path_data[:8]:
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

        if cmd.is_abnormal and cmd.abnormal_reasons:
            for reason in cmd.abnormal_reasons:
                lines.append(f"{'':57s}  ⚠️ {reason}")

        return "\n".join(lines)

    def print_summary(self):
        """打印命令统计"""
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
            cmd_counts = {}
            for cmd in self.commands:
                cmd_counts[cmd.cmd_type] = cmd_counts.get(cmd.cmd_type, 0) + 1

            print("\n" + "=" * 70)
            print("命令统计")
            print("=" * 70)
            for cmd_type, count in sorted(cmd_counts.items()):
                print(f"  {cmd_type:15s}: {count:5d}")
            print(f"  {'总计':15s}: {len(self.commands):5d}")

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
