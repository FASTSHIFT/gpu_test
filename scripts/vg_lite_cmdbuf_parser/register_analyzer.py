#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite GPU 寄存器分析器
======================
分析 GPU 硬件寄存器状态
"""

import re
from rich.console import Console
from rich.table import Table
from rich.panel import Panel

try:
    from .constants import GPU_REGISTER_INFO
except ImportError:
    from constants import GPU_REGISTER_INFO


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
