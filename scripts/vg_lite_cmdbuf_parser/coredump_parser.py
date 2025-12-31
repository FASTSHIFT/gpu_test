#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite Coredump 解析器
======================
从 coredump 文件中提取并解析 VGLite 命令缓冲区数据

依赖:
  pip install pyelftools

用法:
  python coredump_parser.py --elf firmware.elf --core dump.core
"""

import struct
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from rich.console import Console
from rich.panel import Panel
from rich.table import Table

try:
    from elftools.elf.elffile import ELFFile
    from elftools.elf.sections import SymbolTableSection

    HAS_ELFTOOLS = True
except ImportError:
    HAS_ELFTOOLS = False

try:
    from .command_parser import VGLiteCommandParser
    from .output import create_command_table, add_command_to_table, print_summary
    from .path_parser import VGLitePathParser
except ImportError:
    from command_parser import VGLiteCommandParser
    from output import create_command_table, add_command_to_table, print_summary
    from path_parser import VGLitePathParser


@dataclass
class SymbolInfo:
    """符号信息"""

    name: str
    address: int
    size: int


@dataclass
class CoreSegment:
    """Coredump 内存段"""

    vaddr: int  # 虚拟地址
    filesz: int  # 文件中大小
    memsz: int  # 内存中大小
    offset: int  # 文件偏移


@dataclass
class TargetBufferInfo:
    """渲染目标缓冲区信息"""

    width: int = 0
    height: int = 0
    stride: int = 0
    format: int = 0
    address: int = 0  # GPU 地址
    memory: int = 0  # CPU 逻辑地址
    pixel_data: bytes = None  # 像素数据


class CoredumpParser:
    """Coredump 解析器

    从 ELF 文件获取符号地址，从 coredump 文件读取内存数据
    """

    # 需要查找的 VGLite 符号
    VGLITE_SYMBOLS = [
        "backup_command_buffer_klogical",
        "backup_command_buffer_physical",
        "backup_command_buffer_size",
        "init_cmd_buffer",
        "init_cmd_buffer_size",
        "global_power_context",
        # 也搜索 command buffer 相关符号
        "s_context",
    ]

    # vg_lite_context_t 结构体偏移量 (通过 GDB 获取)
    CONTEXT_OFFSETS = {
        "rtbuffer": 0x720,  # vg_lite_buffer_t* 指针
        "target_width": 0x7A4,
        "target_height": 0x7A8,
    }

    # vg_lite_buffer_t 结构体偏移量
    BUFFER_OFFSETS = {
        "width": 0x0,
        "height": 0x4,
        "stride": 0x8,
        "format": 0xE,
        "memory": 0x14,
        "address": 0x18,
    }

    def __init__(self, elf_path: str, core_path: str):
        """
        Args:
            elf_path: ELF 文件路径
            core_path: Coredump 文件路径
        """
        self.elf_path = Path(elf_path)
        self.core_path = Path(core_path)
        self.console = Console()

        self.symbols: Dict[str, SymbolInfo] = {}
        self.segments: List[CoreSegment] = []
        self.core_data: bytes = b""

        self._elf_file = None
        self._core_file = None

    def parse(self) -> bool:
        """解析 ELF 和 coredump 文件"""
        if not HAS_ELFTOOLS:
            self.console.print(
                "[red]错误: 需要安装 pyelftools 库[/red]\n"
                "请运行: pip install pyelftools"
            )
            return False

        if not self.elf_path.exists():
            self.console.print(f"[red]错误: ELF 文件不存在: {self.elf_path}[/red]")
            return False

        if not self.core_path.exists():
            self.console.print(
                f"[red]错误: Coredump 文件不存在: {self.core_path}[/red]"
            )
            return False

        try:
            # 解析 ELF 获取符号
            self._parse_elf_symbols()

            # 解析 coredump 获取内存段
            self._parse_core_segments()

            return True
        except Exception as e:
            self.console.print(f"[red]解析失败: {e}[/red]")
            import traceback

            traceback.print_exc()
            return False

    def _parse_elf_symbols(self):
        """从 ELF 文件解析符号表"""
        with open(self.elf_path, "rb") as f:
            elf = ELFFile(f)

            for section in elf.iter_sections():
                if not isinstance(section, SymbolTableSection):
                    continue

                for symbol in section.iter_symbols():
                    name = symbol.name
                    if name in self.VGLITE_SYMBOLS:
                        self.symbols[name] = SymbolInfo(
                            name=name,
                            address=symbol["st_value"],
                            size=symbol["st_size"],
                        )

        # 打印找到的符号
        if self.symbols:
            table = Table(title="找到的 VGLite 符号", show_header=True)
            table.add_column("符号名", style="cyan")
            table.add_column("地址", style="yellow")
            table.add_column("大小", style="green")

            for sym in self.symbols.values():
                table.add_row(sym.name, f"0x{sym.address:08X}", str(sym.size))

            self.console.print(table)
            self.console.print()

    def _parse_core_segments(self):
        """解析 coredump 文件的内存段"""
        with open(self.core_path, "rb") as f:
            self.core_data = f.read()

            # 重新打开解析 ELF 结构
            f.seek(0)
            try:
                core = ELFFile(f)

                # 遍历 program headers 获取 LOAD 段
                for segment in core.iter_segments():
                    if segment["p_type"] == "PT_LOAD":
                        self.segments.append(
                            CoreSegment(
                                vaddr=segment["p_vaddr"],
                                filesz=segment["p_filesz"],
                                memsz=segment["p_memsz"],
                                offset=segment["p_offset"],
                            )
                        )

                # 打印内存段信息
                if self.segments:
                    table = Table(title="Coredump 内存段", show_header=True)
                    table.add_column("虚拟地址", style="yellow")
                    table.add_column("文件大小", style="cyan")
                    table.add_column("文件偏移", style="green")

                    for seg in self.segments[:10]:  # 只显示前10个
                        table.add_row(
                            f"0x{seg.vaddr:08X}",
                            f"0x{seg.filesz:X}",
                            f"0x{seg.offset:X}",
                        )
                    if len(self.segments) > 10:
                        table.add_row("...", f"共 {len(self.segments)} 段", "...")

                    self.console.print(table)
                    self.console.print()

            except Exception as e:
                self.console.print(
                    f"[yellow]警告: 无法解析 coredump ELF 结构: {e}[/yellow]"
                )
                self.console.print("[yellow]尝试作为原始内存转储处理...[/yellow]")

    def read_memory(self, address: int, size: int) -> Optional[bytes]:
        """从 coredump 读取指定地址的内存

        Args:
            address: 虚拟地址
            size: 读取大小

        Returns:
            内存数据，失败返回 None
        """
        # 首先尝试从 LOAD 段读取
        for seg in self.segments:
            if seg.vaddr <= address < seg.vaddr + seg.filesz:
                offset = seg.offset + (address - seg.vaddr)
                end_offset = offset + size
                if end_offset <= len(self.core_data):
                    return self.core_data[offset:end_offset]

        # 如果没有找到对应段，尝试直接作为偏移读取（适用于原始内存转储）
        if address < len(self.core_data):
            return self.core_data[address : address + size]

        return None

    def read_u32(self, address: int) -> Optional[int]:
        """读取 32 位无符号整数"""
        data = self.read_memory(address, 4)
        if data and len(data) == 4:
            return struct.unpack("<I", data)[0]
        return None

    def read_pointer(self, address: int) -> Optional[int]:
        """读取指针值（32位系统）"""
        return self.read_u32(address)

    def get_backup_command_buffer(
        self,
    ) -> Tuple[Optional[int], Optional[int], Optional[bytes]]:
        """获取 backup_command_buffer 的内容

        Returns:
            (physical_addr, size, data) 元组
        """
        # 获取 backup_command_buffer_klogical 指针值
        klogical_sym = self.symbols.get("backup_command_buffer_klogical")
        size_sym = self.symbols.get("backup_command_buffer_size")
        physical_sym = self.symbols.get("backup_command_buffer_physical")

        if not klogical_sym or not size_sym:
            self.console.print(
                "[yellow]警告: 未找到 backup_command_buffer 相关符号[/yellow]"
            )
            return None, None, None

        # 读取指针值和大小
        klogical_ptr = self.read_pointer(klogical_sym.address)
        buf_size = self.read_u32(size_sym.address)
        physical_addr = self.read_u32(physical_sym.address) if physical_sym else None

        if klogical_ptr is None or buf_size is None:
            self.console.print(
                "[yellow]警告: 无法读取 backup_command_buffer 变量值[/yellow]"
            )
            self.console.print(f"  klogical 地址: 0x{klogical_sym.address:08X}")
            self.console.print(f"  size 地址: 0x{size_sym.address:08X}")
            return None, None, None

        self.console.print(
            Panel.fit(
                f"backup_command_buffer_klogical: 0x{klogical_ptr:08X}\n"
                f"backup_command_buffer_physical: {f'0x{physical_addr:08X}' if physical_addr else 'N/A'}\n"
                f"backup_command_buffer_size: {buf_size} bytes",
                title="Backup Command Buffer 信息",
                style="cyan",
            )
        )

        if buf_size == 0 or buf_size > 0x100000:  # 超过 1MB 认为异常
            self.console.print(f"[yellow]警告: buffer 大小异常 ({buf_size})[/yellow]")
            return physical_addr, buf_size, None

        # 读取缓冲区数据
        buf_data = self.read_memory(klogical_ptr, buf_size)
        if buf_data is None:
            self.console.print(
                f"[yellow]警告: 无法读取地址 0x{klogical_ptr:08X} 的数据[/yellow]"
            )

        return physical_addr, buf_size, buf_data

    def get_init_command_buffer(self) -> Tuple[Optional[int], Optional[bytes]]:
        """获取 init_cmd_buffer 的内容

        Returns:
            (size, data) 元组
        """
        buf_sym = self.symbols.get("init_cmd_buffer")
        size_sym = self.symbols.get("init_cmd_buffer_size")

        if not buf_sym or not size_sym:
            return None, None

        buf_size = self.read_u32(size_sym.address)
        if buf_size is None or buf_size == 0 or buf_size > 0x10000:
            return None, None

        buf_data = self.read_memory(buf_sym.address, buf_size)

        self.console.print(
            Panel.fit(
                f"init_cmd_buffer: 0x{buf_sym.address:08X}\n"
                f"init_cmd_buffer_size: {buf_size} bytes",
                title="Init Command Buffer 信息",
                style="green",
            )
        )

        return buf_size, buf_data

    def get_target_buffer_info(self) -> Optional[TargetBufferInfo]:
        """获取渲染目标缓冲区信息

        从 s_context.rtbuffer 读取 target buffer 信息

        Returns:
            TargetBufferInfo 或 None
        """
        s_context_sym = self.symbols.get("s_context")
        if not s_context_sym:
            self.console.print("[yellow]警告: 未找到 s_context 符号[/yellow]")
            return None

        s_context_addr = s_context_sym.address

        # 读取 rtbuffer 指针
        rtbuffer_ptr = self.read_u32(s_context_addr + self.CONTEXT_OFFSETS["rtbuffer"])
        if rtbuffer_ptr is None or rtbuffer_ptr == 0:
            # 尝试从 target_width/target_height 获取
            target_width = self.read_u32(
                s_context_addr + self.CONTEXT_OFFSETS["target_width"]
            )
            target_height = self.read_u32(
                s_context_addr + self.CONTEXT_OFFSETS["target_height"]
            )
            if target_width and target_height:
                info = TargetBufferInfo(width=target_width, height=target_height)
                self.console.print(
                    Panel.fit(
                        f"target_width: {target_width}\n"
                        f"target_height: {target_height}\n"
                        f"(rtbuffer 指针为空，仅从 context 获取尺寸)",
                        title="Target Buffer 信息",
                        style="yellow",
                    )
                )
                return info
            return None

        # 读取 vg_lite_buffer_t 结构
        width = self.read_u32(rtbuffer_ptr + self.BUFFER_OFFSETS["width"])
        height = self.read_u32(rtbuffer_ptr + self.BUFFER_OFFSETS["height"])
        stride = self.read_u32(rtbuffer_ptr + self.BUFFER_OFFSETS["stride"])
        fmt = self.read_u16(rtbuffer_ptr + self.BUFFER_OFFSETS["format"])
        memory = self.read_u32(rtbuffer_ptr + self.BUFFER_OFFSETS["memory"])
        address = self.read_u32(rtbuffer_ptr + self.BUFFER_OFFSETS["address"])

        if width is None or height is None:
            self.console.print(
                f"[yellow]警告: 无法读取 rtbuffer (0x{rtbuffer_ptr:08X}) 数据[/yellow]"
            )
            return None

        info = TargetBufferInfo(
            width=width or 0,
            height=height or 0,
            stride=stride or 0,
            format=fmt or 0,
            address=address or 0,
            memory=memory or 0,
        )

        # 尝试读取像素数据
        if memory and width and height and stride:
            pixel_size = height * stride
            pixel_data = self.read_memory(memory, pixel_size)
            if pixel_data:
                info.pixel_data = pixel_data
                self.console.print(
                    f"[green]已读取目标缓冲区像素数据: {len(pixel_data)} 字节[/green]"
                )

        # 格式名称映射 (VGLite 格式: base | (1 << 10))
        base_fmt = fmt & 0x3FF
        format_names = {
            0: "RGBA8888",
            1: "BGRA8888",
            2: "RGBX8888",
            3: "BGRX8888",
            4: "RGB565",
            5: "BGR565",
            6: "RGBA4444",
            7: "BGRA4444",
            10: "A8",
            11: "L8",
            31: "ABGR8888",
            32: "ARGB8888",
        }
        fmt_name = format_names.get(base_fmt, f"Unknown({fmt})")

        self.console.print(
            Panel.fit(
                f"rtbuffer: 0x{rtbuffer_ptr:08X}\n"
                f"width: {width}\n"
                f"height: {height}\n"
                f"stride: {stride}\n"
                f"format: {fmt_name}\n"
                f"address (GPU): 0x{address:08X}\n"
                f"memory (CPU): 0x{memory:08X}",
                title="目标缓冲区信息",
                style="blue",
            )
        )

        return info

    def read_u16(self, address: int) -> Optional[int]:
        """读取 16 位无符号整数"""
        data = self.read_memory(address, 2)
        if data and len(data) >= 2:
            return struct.unpack("<H", data[:2])[0]
        return None

    def buffer_to_log_format(self, data: bytes) -> str:
        """将缓冲区数据转换为日志格式

        Args:
            data: 原始缓冲区数据

        Returns:
            类似 "0xXXXXXXXX 0xXXXXXXXX" 格式的日志文本
        """
        lines = []
        for i in range(0, len(data) - 7, 8):
            word1 = struct.unpack("<I", data[i : i + 4])[0]
            word2 = struct.unpack("<I", data[i + 4 : i + 8])[0]
            lines.append(f"0x{word1:08X} 0x{word2:08X}")
        return "\n".join(lines)

    def analyze(
        self, verbose: bool = False, parse_path: bool = False
    ) -> Tuple[list, Optional[TargetBufferInfo]]:
        """分析命令缓冲区

        Args:
            verbose: 详细模式
            parse_path: 解析路径数据

        Returns:
            (所有解析出的命令列表, target buffer 信息) 元组
        """
        all_commands = []

        # 获取 target buffer 信息
        target_info = self.get_target_buffer_info()

        # 获取 backup command buffer
        physical, size, backup_data = self.get_backup_command_buffer()

        if backup_data:
            self.console.print()
            self.console.print(
                Panel.fit(
                    "解析 Backup Command Buffer (最后一次提交的命令)",
                    style="bold magenta",
                )
            )

            log_text = self.buffer_to_log_format(backup_data)
            parser = VGLiteCommandParser(verbose=verbose, parse_path=parse_path)
            commands = parser.parse_log(log_text)

            # 处理 CALL 命令 (upload path)，尝试从 coredump 读取路径数据
            self._resolve_uploaded_paths(commands, parser)

            # 输出解析结果
            table = create_command_table(
                "最后提交的命令缓冲区",
                f"0x{physical:08X}" if physical else None,
                f"0x{size:X}" if size else None,
            )

            for cmd in commands:
                add_command_to_table(table, cmd, parser)

            self.console.print(table)
            print_summary(parser, self.console)

            all_commands.extend(commands)

        # 获取 init command buffer
        init_size, init_data = self.get_init_command_buffer()

        if init_data:
            self.console.print()
            self.console.print(
                Panel.fit(
                    "解析 Init Command Buffer (GPU 初始化命令)", style="bold green"
                )
            )

            log_text = self.buffer_to_log_format(init_data)
            parser = VGLiteCommandParser(verbose=verbose, parse_path=parse_path)
            commands = parser.parse_log(log_text)

            table = create_command_table("初始化命令缓冲区", None, f"0x{init_size:X}")

            for cmd in commands:
                add_command_to_table(table, cmd, parser)

            self.console.print(table)
            print_summary(parser, self.console)

            # Init commands 不包含绘图命令，可以选择是否加入
            # all_commands.extend(commands)

        return all_commands, target_info

    def _resolve_uploaded_paths(self, commands: list, parser: VGLiteCommandParser):
        """解析 CALL 命令指向的上传路径数据

        Upload path 内存布局 (参考 vg_lite_upload_path 和 lv_vg_lite_path_finish_upload):
        [0]: DATA 命令 (0x40000000 | path_data_count)
        [1]: 0
        [2..n-2]: 实际路径数据 (原始字节流，格式取决于 VgPathControl)
        [n-2]: RETURN 命令 (0x70000000)
        [n-1]: 0

        路径数据格式 (参考 lv_vg_lite_path.c):
        - S8: 操作码 1 字节，每个坐标 1 字节
        - S16: 操作码 2 字节，每个坐标 2 字节
        - S32: 操作码 4 字节，每个坐标 4 字节
        - FP32: 操作码 4 字节，每个坐标 4 字节 (浮点数)

        Args:
            commands: 命令列表
            parser: 命令解析器（用于获取当前路径格式）
        """
        call_count = 0
        resolved_count = 0

        # 记录每个 CALL 命令对应的路径格式
        current_path_format = "FP32"  # 默认格式

        for cmd in commands:
            # 跟踪路径格式变化 (VgPathControl 寄存器 0x0A34)
            if cmd.cmd_type == "STATE":
                reg_addr = cmd.cmd_word & 0xFFFF
                if reg_addr == 0x0A34:  # VgPathControl
                    # 格式位在 bit 20-21 (2 位)
                    fmt_bits = (cmd.data_word >> 20) & 0x3
                    format_map = {0: "S8", 1: "S16", 2: "S32", 3: "FP32"}
                    current_path_format = format_map.get(fmt_bits, "FP32")

            if cmd.cmd_type != "CALL":
                continue

            call_count += 1

            # CALL 命令格式: cmd_word = 0x60000000 | count, data_word = address
            call_data_count = cmd.cmd_word & 0x0FFFFFFF
            path_address = cmd.data_word
            path_size = call_data_count * 8  # 每个条目 8 字节

            if path_address == 0 or path_size == 0:
                continue

            # 尝试从 coredump 读取路径数据
            path_data = self.read_memory(path_address, path_size)

            if path_data is None:
                cmd.details.append(f"[无法读取地址 0x{path_address:08X}]")
                continue

            # 验证包头: DATA 命令 (0x40000000 | count)
            if len(path_data) < 16:  # 至少需要包头(8) + 包尾(8)
                cmd.details.append("[数据太短]")
                continue

            header_word = struct.unpack("<I", path_data[0:4])[0]
            header_opcode = header_word & 0xF0000000
            if header_opcode != 0x40000000:
                cmd.details.append(
                    f"[包头校验失败: 期望 DATA(0x4xxxxxxx), 实际 0x{header_word:08X}]"
                )
                continue

            # 验证包尾: RETURN 命令 (0x70000000)
            tail_word = struct.unpack("<I", path_data[-8:-4])[0]
            if tail_word != 0x70000000:
                cmd.details.append(
                    f"[包尾校验失败: 期望 RETURN(0x70000000), 实际 0x{tail_word:08X}]"
                )
                continue

            resolved_count += 1

            # 从包头获取实际路径数据的 DWORD 数
            path_data_dword_count = header_word & 0x0FFFFFFF
            # 实际路径数据字节数 = (path_data_dword_count * 8) 但这是向上对齐的
            # 实际路径数据从偏移 8 开始（跳过包头），到包尾前结束
            actual_path_bytes = path_data[8:-8]

            cmd.details.append(
                f"[Upload Path: 格式={current_path_format}, "
                f"包头 DATA({path_data_dword_count}), 包尾 RETURN]"
            )

            # 直接将字节流传给路径解析器
            path_parser = VGLitePathParser(current_path_format)
            cmd.path_segments = path_parser._parse_bytes(bytearray(actual_path_bytes))

            if cmd.path_segments:
                cmd.details.append(f"[解析出 {len(cmd.path_segments)} 个路径段]")

        if call_count > 0:
            self.console.print(
                f"[cyan]已解析 {resolved_count}/{call_count} 个上传路径 (CALL 命令)[/cyan]"
            )


def parse_coredump(
    elf_path: str,
    core_path: str,
    verbose: bool = False,
    parse_path: bool = False,
) -> Tuple[list, Optional[TargetBufferInfo]]:
    """解析 coredump 文件中的 VGLite 命令缓冲区

    Args:
        elf_path: ELF 文件路径
        core_path: Coredump 文件路径
        verbose: 详细模式
        parse_path: 解析路径数据

    Returns:
        (解析出的命令列表, target buffer 信息) 元组，如果解析失败返回 ([], None)
    """
    parser = CoredumpParser(elf_path, core_path)

    if not parser.parse():
        return [], None

    return parser.analyze(verbose=verbose, parse_path=parse_path)


def main():
    """命令行入口"""
    import argparse

    arg_parser = argparse.ArgumentParser(
        description="从 coredump 文件解析 VGLite 命令缓冲区",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  # 基本用法
  python coredump_parser.py --elf firmware.elf --core crash.core

  # 详细模式
  python coredump_parser.py --elf firmware.elf --core crash.core -v

  # 解析路径数据
  python coredump_parser.py --elf firmware.elf --core crash.core -p

依赖:
  pip install pyelftools rich
""",
    )

    arg_parser.add_argument(
        "--elf", "-e", required=True, help="ELF 文件路径 (用于获取符号地址)"
    )
    arg_parser.add_argument("--core", "-c", required=True, help="Coredump 文件路径")
    arg_parser.add_argument("-v", "--verbose", action="store_true", help="详细输出模式")
    arg_parser.add_argument(
        "-p", "--parse-path", action="store_true", help="解析路径数据详情"
    )

    args = arg_parser.parse_args()

    parse_coredump(
        elf_path=args.elf,
        core_path=args.core,
        verbose=args.verbose,
        parse_path=args.parse_path,
    )


if __name__ == "__main__":
    main()
