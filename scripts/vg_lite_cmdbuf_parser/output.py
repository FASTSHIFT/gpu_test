#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite 输出格式化
=================
使用 Rich 库格式化输出命令解析结果
"""

from rich.console import Console
from rich.table import Table

try:
    from .models import ParsedCommand, ImageDrawInfo
except ImportError:
    from models import ParsedCommand, ImageDrawInfo
from typing import List, TYPE_CHECKING

if TYPE_CHECKING:
    try:
        from .command_parser import VGLiteCommandParser
    except ImportError:
        from command_parser import VGLiteCommandParser


def create_command_table(title: str, address: str = None, size: str = None) -> Table:
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


def add_command_to_table(
    table: Table, cmd: ParsedCommand, parser: "VGLiteCommandParser"
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

    # 添加路径段 (支持 DATA 和 CALL 命令)
    if parser.parse_path and cmd.cmd_type in ("DATA", "CALL") and cmd.path_segments:
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


def print_summary(parser: "VGLiteCommandParser", console: Console):
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
            _print_image_stats(parser.image_draws, console)
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


def _print_image_stats(image_draws: List[ImageDrawInfo], console: Console):
    """打印图片绘制统计"""
    console.print()
    img_table = Table(
        title=f"🖼️ 图片绘制统计 ({len(image_draws)} 次)",
        show_header=True,
        header_style="bold cyan",
    )
    img_table.add_column("指标", style="cyan")
    img_table.add_column("值", style="green")

    # 按格式统计
    format_counts = {}
    for img in image_draws:
        fmt = img.src_format
        format_counts[fmt] = format_counts.get(fmt, 0) + 1

    # 按混合模式统计
    blend_counts = {}
    for img in image_draws:
        blend_counts[img.blend_mode] = blend_counts.get(img.blend_mode, 0) + 1

    # 总内存
    total_mem = sum(img.calc_memory_size() for img in image_draws)
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
    for img in image_draws:
        src_addr_counts[img.src_address] = src_addr_counts.get(img.src_address, 0) + 1
    repeated = sum(1 for c in src_addr_counts.values() if c > 1)
    if repeated > 0:
        img_table.add_row("重复绘制", f"{repeated} 个图片被多次绘制")

    console.print(img_table)
