#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite 日志完整性检测器
======================
检测日志输出时可能发生的并发冲突、缓冲区溢出等问题导致的数据损坏
"""

import re
from typing import List, Optional

from rich.console import Console
from rich.table import Table
from rich.panel import Panel

try:
    from .models import LogIntegrityIssue
except ImportError:
    from models import LogIntegrityIssue


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
