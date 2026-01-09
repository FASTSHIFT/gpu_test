#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite 命令缓冲区解析器
======================
解析 VGLite GPU 命令缓冲区数据

用法:
  python main.py -f dump.log              # 解析日志文件 (默认解析路径和图片)
  python main.py -f dump.log -r           # 包含寄存器分析
  python main.py -f dump.log -v           # 详细模式
  python main.py -f dump.log --no-parse-path   # 禁用路径解析
  python main.py -f dump.log --no-parse-image  # 禁用图片分析
  python main.py -f dump.log -c           # 检测日志完整性
  python main.py -i                       # 交互模式
"""

import argparse
import sys
import os
from typing import List

from rich.console import Console

# 支持直接运行和作为包导入两种方式
try:
    from .constants import (
        REGISTER_MAP,
        BLEND_MODES,
        IMAGE_FORMATS,
    )
    from .models import ParsedCommand
    from .integrity import LogIntegrityChecker
    from .command_parser import VGLiteCommandParser
    from .register_analyzer import GPURegisterAnalyzer
    from .output import (
        create_command_table,
        add_command_to_table,
        print_summary,
    )
    from .coredump_parser import parse_coredump
    from .svg_exporter import SVGExporter
except ImportError:
    from constants import (
        REGISTER_MAP,
        BLEND_MODES,
        IMAGE_FORMATS,
    )
    from models import ParsedCommand
    from integrity import LogIntegrityChecker
    from command_parser import VGLiteCommandParser
    from register_analyzer import GPURegisterAnalyzer
    from output import (
        create_command_table,
        add_command_to_table,
        print_summary,
    )
    from coredump_parser import parse_coredump
    from svg_exporter import SVGExporter


def parse_file_v2(
    filename: str,
    verbose: bool = False,
    parse_path: bool = False,
    check_integrity: bool = False,
    parse_image: bool = False,
    export_html: str = None,
    canvas_width: int = 466,
    canvas_height: int = 466,
) -> List[ParsedCommand]:
    """
    从文件解析日志 (v2版本，使用Rich表格输出)

    Args:
        filename: 日志文件路径
        verbose: 是否显示详细信息
        parse_path: 是否解析路径数据
        check_integrity: 是否检测日志完整性
        parse_image: 是否分析图片绘制

    Returns:
        解析后的命令列表
    """
    with open(filename, "r", encoding="utf-8", errors="ignore") as f:
        log_text = f.read()

    console = Console()

    # 日志完整性检测
    if check_integrity:
        lines = log_text.splitlines()
        checker = LogIntegrityChecker()
        checker.analyze(lines)
        checker.print_report(console)

    # 解析命令缓冲区
    parser = VGLiteCommandParser(
        verbose=verbose, parse_path=parse_path, parse_image=parse_image
    )
    commands = parser.parse_log(log_text)

    # 按段输出
    if parser.command_sections:
        for section in parser.command_sections:
            table = create_command_table(
                f"【{section['name']}】", section["address"], section["size"]
            )

            for cmd in section["commands"]:
                add_command_to_table(table, cmd, parser)

            console.print(table)
            console.print()
    else:
        # 兼容无段落的日志
        table = create_command_table("VGLite 命令缓冲区解析结果")
        for cmd in commands:
            add_command_to_table(table, cmd, parser)
        console.print(table)

    print_summary(parser, console)

    return commands


def parse_string_v2(
    log_text: str, verbose: bool = False, parse_path: bool = False
) -> List[ParsedCommand]:
    """
    从字符串解析日志

    Args:
        log_text: 日志文本
        verbose: 是否显示详细信息
        parse_path: 是否解析路径数据

    Returns:
        解析后的命令列表
    """
    parser = VGLiteCommandParser(verbose=verbose, parse_path=parse_path)
    commands = parser.parse_log(log_text)
    console = Console()

    # 按段输出
    if parser.command_sections:
        for section in parser.command_sections:
            table = create_command_table(
                f"【{section['name']}】", section["address"], section["size"]
            )

            for cmd in section["commands"]:
                add_command_to_table(table, cmd, parser)

            console.print(table)
            console.print()
    else:
        # 兼容无段落的日志
        table = create_command_table("VGLite 命令缓冲区解析结果")
        for cmd in commands:
            add_command_to_table(table, cmd, parser)
        console.print(table)

    print_summary(parser, console)
    return commands


def parse_with_registers(
    filename: str,
    verbose: bool = False,
    parse_path: bool = False,
    check_integrity: bool = False,
    parse_image: bool = False,
) -> List[ParsedCommand]:
    """
    解析日志文件，包括寄存器分析和命令缓冲区解析

    Args:
        filename: 日志文件路径
        verbose: 是否显示详细信息
        parse_path: 是否解析路径数据
        check_integrity: 是否检测日志完整性
        parse_image: 是否分析图片绘制

    Returns:
        解析后的命令列表
    """
    with open(filename, "r", encoding="utf-8", errors="ignore") as f:
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
            table = create_command_table(
                f"【{section['name']}】", section["address"], section["size"]
            )

            for cmd in section["commands"]:
                add_command_to_table(table, cmd, parser)

            console.print(table)
            console.print()
    else:
        # 兼容无段落的日志
        table = create_command_table("VGLite 命令缓冲区解析结果")
        for cmd in commands:
            add_command_to_table(table, cmd, parser)
        console.print(table)

    print_summary(parser, console)

    return commands


def interactive_mode(verbose: bool = False, parse_path: bool = False):
    """
    交互模式: 从标准输入读取日志

    Args:
        verbose: 是否显示详细信息
        parse_path: 是否解析路径数据
    """
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


def main():
    """主函数"""
    arg_parser = argparse.ArgumentParser(
        description="VGLite GPU 命令缓冲区解析器",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  # 从文件解析 (包含寄存器分析)
  python main.py -f dump.log

  # 解析并分析寄存器信息
  python main.py -f dump.log -r

  # 详细模式
  python main.py -f dump.log -v

  # 解析路径数据
  python main.py -f dump.log -p

  # 详细模式 + 解析路径 + 寄存器分析
  python main.py -f dump.log -v -p -r

  # 检测日志完整性问题
  python main.py -f dump.log -c

  # 分析图片绘制
  python main.py -f dump.log -I

  # 从 coredump 解析命令缓冲区
  python main.py --elf firmware.elf --core crash.core

  # 交互模式
  python main.py -i

  # 解析字符串
  python main.py -s "0x30010A00 0x00000100"
""",
    )

    arg_parser.add_argument("-f", "--file", help="日志文件路径")
    arg_parser.add_argument("-s", "--string", help="直接解析命令字符串")
    arg_parser.add_argument(
        "-i", "--interactive", action="store_true", help="交互模式,从标准输入读取"
    )
    arg_parser.add_argument("-v", "--verbose", action="store_true", help="详细输出模式")
    arg_parser.add_argument(
        "--no-parse-path",
        action="store_true",
        help="禁用路径数据解析 (默认开启)",
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
        "--no-parse-image",
        action="store_true",
        help="禁用图片绘制分析 (默认开启)",
    )
    arg_parser.add_argument(
        "--elf",
        help="ELF 文件路径 (用于 coredump 解析)",
    )
    arg_parser.add_argument(
        "--core",
        help="Coredump 文件路径",
    )
    arg_parser.add_argument(
        "--cmdbuf",
        type=int,
        choices=[0, 1],
        default=None,
        help="指定分析 s_context.command_buffer[0|1]，默认使用 backup_command_buffer",
    )
    arg_parser.add_argument(
        "--export-html",
        "-S",
        nargs="?",
        const="out.html",
        default=None,
        help="导出 HTML 可视化文件 (默认: out.html，以 .svg 结尾则导出纯 SVG)",
    )
    arg_parser.add_argument(
        "--canvas-width",
        type=int,
        default=466,
        help="SVG 画布宽度 (默认: 466)",
    )
    arg_parser.add_argument(
        "--canvas-height",
        type=int,
        default=466,
        help="SVG 画布高度 (默认: 466)",
    )

    args = arg_parser.parse_args()

    # 处理默认开启的选项
    parse_path = not args.no_parse_path
    parse_image = not args.no_parse_image

    # Coredump 解析模式
    if args.elf and args.core:
        # 确定要分析的 command buffer 索引
        cmdbuf_index = args.cmdbuf if args.cmdbuf is not None else -1

        commands, target_info, context_state = parse_coredump(
            elf_path=args.elf,
            core_path=args.core,
            verbose=args.verbose,
            parse_path=parse_path,
            cmdbuf_index=cmdbuf_index,
        )

        # HTML/SVG 导出
        if args.export_html:
            if not commands:
                console = Console()
                console.print(f"[yellow]警告: 没有解析到任何命令，无法导出 HTML 文件[/yellow]")
            else:
                # 使用 target buffer 的尺寸（如果可用）
                canvas_width = args.canvas_width
                canvas_height = args.canvas_height
                if target_info:
                    if target_info.width > 0:
                        canvas_width = target_info.width
                    if target_info.height > 0:
                        canvas_height = target_info.height

                exporter = SVGExporter(
                    width=canvas_width,
                    height=canvas_height,
                )
                exporter.set_target_info(target_info)
                exporter.set_context_state(context_state)
                exporter.process_commands(commands)
                try:
                    if args.export_html.endswith(".svg"):
                        exporter.export_svg(args.export_html)
                    else:
                        exporter.export_html(args.export_html)
                    console = Console()
                    console.print(f"[green]已导出可视化文件: {args.export_html}[/green]")
                except Exception as e:
                    console = Console()
                    console.print(f"[red]错误: 导出 HTML 文件失败: {e}[/red]")
    elif args.file:
        if args.regs:
            parse_with_registers(
                args.file,
                args.verbose,
                parse_path,
                args.check_integrity,
                parse_image,
            )
        else:
            parse_file_v2(
                args.file,
                args.verbose,
                parse_path,
                args.check_integrity,
                parse_image,
                args.export_html,
                args.canvas_width,
                args.canvas_height,
            )
    elif args.string:
        parse_string_v2(args.string, args.verbose, parse_path)
    elif args.interactive:
        interactive_mode(args.verbose, parse_path)
    else:
        # 默认交互模式
        interactive_mode(args.verbose, parse_path)


if __name__ == "__main__":
    main()
