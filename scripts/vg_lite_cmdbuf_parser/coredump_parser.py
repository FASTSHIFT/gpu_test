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
except ImportError:
    from command_parser import VGLiteCommandParser
    from output import create_command_table, add_command_to_table, print_summary


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

    def analyze(self, verbose: bool = False, parse_path: bool = False):
        """分析命令缓冲区

        Args:
            verbose: 详细模式
            parse_path: 解析路径数据
        """
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


def parse_coredump(
    elf_path: str,
    core_path: str,
    verbose: bool = False,
    parse_path: bool = False,
):
    """解析 coredump 文件中的 VGLite 命令缓冲区

    Args:
        elf_path: ELF 文件路径
        core_path: Coredump 文件路径
        verbose: 详细模式
        parse_path: 解析路径数据
    """
    parser = CoredumpParser(elf_path, core_path)

    if not parser.parse():
        return

    parser.analyze(verbose=verbose, parse_path=parse_path)


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
