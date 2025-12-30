#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite SVG/HTML 导出器
====================
将解析的 VGLite 命令导出为 SVG 或交互式 HTML 可视化文件
"""

from dataclasses import dataclass, field
from typing import List, Optional, Tuple
import html


@dataclass
class DrawCommand:
    """绘制命令数据"""

    path_segments: List = field(default_factory=list)  # 路径段列表
    color: int = 0xFFFFFFFF  # ARGB 颜色
    opacity: float = 1.0  # 不透明度
    fill_rule: str = "nonzero"  # 填充规则: nonzero, evenodd
    matrix: Optional[List[float]] = None  # 变换矩阵 [m00, m01, m02, m10, m11, m12]
    path_scale: float = 1.0  # 路径缩放
    path_bias: float = 0.0  # 路径偏移


class SVGExporter:
    """SVG/HTML 导出器"""

    def __init__(self, width: int = 466, height: int = 466):
        """
        初始化导出器

        Args:
            width: 画布宽度
            height: 画布高度
        """
        self.width = width
        self.height = height
        self.draw_commands: List[DrawCommand] = []
        self.background_color = "#000000"  # 纯黑色背景

    def process_commands(self, commands: list):
        """
        处理解析的命令列表，提取绘制信息

        Args:
            commands: ParsedCommand 列表
        """
        current_draw = DrawCommand()
        current_matrix = None
        current_path_scale = 1.0
        current_path_bias = 0.0

        for cmd in commands:
            if cmd.cmd_type == "STATE":
                reg_addr = cmd.cmd_word & 0xFFFF

                # VgPathScale (0x0A28)
                if reg_addr == 0x0A28:
                    import struct

                    current_path_scale = struct.unpack(
                        "f", struct.pack("I", cmd.data_word)
                    )[0]

                # VgPathBias (0x0A2C)
                elif reg_addr == 0x0A2C:
                    import struct

                    current_path_bias = struct.unpack(
                        "f", struct.pack("I", cmd.data_word)
                    )[0]

                # VgColor (0x0A02) - 主要使用这个
                elif reg_addr == 0x0A02:
                    current_draw.color = cmd.data_word

                # VgFillColor (0x0A18) - 备用
                elif reg_addr == 0x0A18:
                    current_draw.color = cmd.data_word

                # VgFillRule (0x0A30)
                elif reg_addr == 0x0A30:
                    rule_val = cmd.data_word & 0x1
                    current_draw.fill_rule = "evenodd" if rule_val else "nonzero"

                # 变换矩阵 VgPathMatrix0-VgPathMatrix5 (0x0A40-0x0A45)
                elif 0x0A40 <= reg_addr <= 0x0A45:
                    if current_matrix is None:
                        current_matrix = [1.0, 0.0, 0.0, 0.0, 1.0, 0.0]
                    idx = reg_addr - 0x0A40
                    if idx < 6:
                        import struct

                        current_matrix[idx] = struct.unpack(
                            "f", struct.pack("I", cmd.data_word)
                        )[0]

            # 处理所有带路径数据的命令 (DATA, DRAW_PATH, CALL 等)
            if cmd.path_segments:
                current_draw.path_segments = cmd.path_segments
                current_draw.matrix = current_matrix.copy() if current_matrix else None
                current_draw.path_scale = current_path_scale
                current_draw.path_bias = current_path_bias
                self.draw_commands.append(current_draw)
                current_draw = DrawCommand()

    def _color_to_rgba(self, color: int) -> Tuple[int, int, int, float]:
        """将 ARGB 颜色转换为 RGBA 元组"""
        a = (color >> 24) & 0xFF
        r = (color >> 16) & 0xFF
        g = (color >> 8) & 0xFF
        b = color & 0xFF
        return (r, g, b, a / 255.0)

    def _path_to_svg(self, draw_cmd: DrawCommand) -> str:
        """将路径段转换为 SVG path d 属性"""
        if not draw_cmd.path_segments:
            return ""

        d_parts = []
        scale = draw_cmd.path_scale
        bias = draw_cmd.path_bias

        for seg in draw_cmd.path_segments:
            # PathSegment 是 dataclass，有 op_name 和 coords 属性
            op = seg.op_name if hasattr(seg, "op_name") else seg.get("op", "")
            coords = seg.coords if hasattr(seg, "coords") else seg.get("coords", [])

            # 应用 scale 和 bias
            scaled_coords = [c * scale + bias for c in coords]

            if op == "MOVE":
                if len(scaled_coords) >= 2:
                    d_parts.append(f"M {scaled_coords[0]:.2f} {scaled_coords[1]:.2f}")
            elif op == "LINE":
                if len(scaled_coords) >= 2:
                    d_parts.append(f"L {scaled_coords[0]:.2f} {scaled_coords[1]:.2f}")
            elif op == "QUAD":
                if len(scaled_coords) >= 4:
                    d_parts.append(
                        f"Q {scaled_coords[0]:.2f} {scaled_coords[1]:.2f} {scaled_coords[2]:.2f} {scaled_coords[3]:.2f}"
                    )
            elif op == "CUBIC":
                if len(scaled_coords) >= 6:
                    d_parts.append(
                        f"C {scaled_coords[0]:.2f} {scaled_coords[1]:.2f} {scaled_coords[2]:.2f} {scaled_coords[3]:.2f} {scaled_coords[4]:.2f} {scaled_coords[5]:.2f}"
                    )
            elif op == "CLOSE":
                d_parts.append("Z")
            elif op == "END":
                pass  # 忽略 END

        return " ".join(d_parts)

    def _matrix_to_svg(self, matrix: Optional[List[float]]) -> str:
        """将 VGLite 矩阵转换为 SVG transform 属性"""
        if matrix is None:
            return ""

        # VGLite 矩阵格式: [m00, m01, m02, m10, m11, m12]
        m00, m01, m02, m10, m11, m12 = matrix

        # 如果是无效矩阵（会导致图形消失），不应用变换
        # 无效情况：m00 和 m11 同时为 0，或者 m00 和 m10 同时为 0 且 m01 和 m11 同时为 0
        if (abs(m00) < 0.0001 and abs(m01) < 0.0001) or (
            abs(m10) < 0.0001 and abs(m11) < 0.0001
        ):
            # 矩阵会导致 x 或 y 坐标丢失，跳过
            return ""

        # 如果是单位矩阵，不需要 transform
        if (
            abs(m00 - 1.0) < 0.0001
            and abs(m11 - 1.0) < 0.0001
            and abs(m01) < 0.0001
            and abs(m10) < 0.0001
            and abs(m02) < 0.0001
            and abs(m12) < 0.0001
        ):
            return ""

        # SVG matrix: matrix(a, b, c, d, e, f)
        # 对应变换: x' = a*x + c*y + e, y' = b*x + d*y + f
        # VGLite: x' = m00*x + m01*y + m02, y' = m10*x + m11*y + m12
        # 所以: a=m00, b=m10, c=m01, d=m11, e=m02, f=m12
        return f'transform="matrix({m00:.6f} {m10:.6f} {m01:.6f} {m11:.6f} {m02:.2f} {m12:.2f})"'

    def export_svg(self, filename: str):
        """
        导出为 SVG 文件

        Args:
            filename: 输出文件路径
        """
        svg_content = self._generate_svg()
        with open(filename, "w", encoding="utf-8") as f:
            f.write(svg_content)

    def export_html(self, filename: str):
        """
        导出为交互式 HTML 文件

        Args:
            filename: 输出文件路径
        """
        svg_content = self._generate_svg()
        html_content = self._wrap_in_html(svg_content)
        with open(filename, "w", encoding="utf-8") as f:
            f.write(html_content)

    def _generate_svg(self) -> str:
        """生成 SVG 内容"""
        paths = []

        for i, draw_cmd in enumerate(self.draw_commands):
            d = self._path_to_svg(draw_cmd)
            if not d:
                continue

            r, g, b, a = self._color_to_rgba(draw_cmd.color)
            fill_color = f"rgb({r},{g},{b})"
            opacity = a * draw_cmd.opacity

            transform = self._matrix_to_svg(draw_cmd.matrix)

            path_elem = f'<path d="{d}" fill="{fill_color}" fill-opacity="{opacity:.2f}" fill-rule="{draw_cmd.fill_rule}" {transform} data-index="{i}"/>'
            paths.append(path_elem)

        svg = f"""<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" width="{self.width}" height="{self.height}" viewBox="0 0 {self.width} {self.height}">
  <rect width="100%" height="100%" fill="{self.background_color}"/>
  <g id="paths">
    {chr(10).join("    " + p for p in paths)}
  </g>
</svg>"""
        return svg

    def _wrap_in_html(self, svg_content: str) -> str:
        """将 SVG 包装为交互式 HTML"""
        # 移除 XML 声明
        svg_content = svg_content.replace(
            '<?xml version="1.0" encoding="UTF-8"?>\n', ""
        )

        html_template = f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>VGLite Visualization</title>
    <style>
        body {{
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            margin: 0;
            padding: 20px;
            background: #f5f5f5;
        }}
        .container {{
            max-width: 1200px;
            margin: 0 auto;
        }}
        h1 {{
            color: #333;
            margin-bottom: 20px;
        }}
        .svg-container {{
            background: white;
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 20px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            display: inline-block;
        }}
        .controls {{
            margin-bottom: 20px;
        }}
        .controls label {{
            margin-right: 20px;
        }}
        .info {{
            margin-top: 20px;
            padding: 15px;
            background: #e8f4fc;
            border-radius: 4px;
        }}
        svg {{
            display: block;
        }}
        svg path {{
            cursor: pointer;
            vector-effect: non-scaling-stroke;
        }}
        svg path:hover {{
            stroke: red;
            stroke-width: 2;
        }}
        svg path.selected {{
            stroke: #00ff00;
            stroke-width: 2;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>VGLite GPU 渲染可视化</h1>
        <div class="controls">
            <label>
                <input type="checkbox" id="showGrid" onchange="toggleGrid()"> 显示网格
            </label>
            <label>
                缩放: <input type="range" id="zoom" min="0.5" max="3" step="0.1" value="1" oninput="updateZoom()">
                <span id="zoomValue">100%</span>
            </label>
            <label>
                背景色: <input type="color" id="bgColor" value="#000000" oninput="updateBgColor()">
            </label>
        </div>
        <div class="svg-container" id="svgContainer">
            {svg_content}
        </div>
        <div class="info">
            <strong>统计信息:</strong> 共 {len(self.draw_commands)} 个绘制命令<br>
            <strong>画布尺寸:</strong> {self.width} x {self.height}<br>
            <span id="pathInfo"></span>
        </div>
    </div>
    <script>
        function toggleGrid() {{
            const svg = document.querySelector('svg');
            const showGrid = document.getElementById('showGrid').checked;
            let grid = document.getElementById('gridPattern');
            
            if (showGrid && !grid) {{
                const defs = document.createElementNS('http://www.w3.org/2000/svg', 'defs');
                defs.innerHTML = `
                    <pattern id="gridPattern" width="20" height="20" patternUnits="userSpaceOnUse">
                        <path d="M 20 0 L 0 0 0 20" fill="none" stroke="#ddd" stroke-width="0.5"/>
                    </pattern>
                `;
                svg.insertBefore(defs, svg.firstChild);
                
                const gridRect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
                gridRect.setAttribute('id', 'gridRect');
                gridRect.setAttribute('width', '100%');
                gridRect.setAttribute('height', '100%');
                gridRect.setAttribute('fill', 'url(#gridPattern)');
                const bgRect = svg.querySelector('rect');
                bgRect.parentNode.insertBefore(gridRect, bgRect.nextSibling);
            }} else if (!showGrid) {{
                const gridRect = document.getElementById('gridRect');
                if (gridRect) gridRect.remove();
                const defs = svg.querySelector('defs');
                if (defs) defs.remove();
            }}
        }}
        
        function updateZoom() {{
            const zoom = document.getElementById('zoom').value;
            document.getElementById('zoomValue').textContent = Math.round(zoom * 100) + '%';
            const svg = document.querySelector('svg');
            svg.style.transform = `scale(${{zoom}})`;
            svg.style.transformOrigin = 'top left';
        }}
        
        function updateBgColor() {{
            const color = document.getElementById('bgColor').value;
            const bgRect = document.querySelector('svg > rect');
            if (bgRect) bgRect.setAttribute('fill', color);
        }}
        
        // 路径点击事件
        document.querySelectorAll('svg path').forEach(path => {{
            path.addEventListener('click', function() {{
                // 移除之前的选中状态
                document.querySelectorAll('svg path.selected').forEach(p => p.classList.remove('selected'));
                // 添加选中状态
                this.classList.add('selected');
                
                const index = this.getAttribute('data-index');
                const d = this.getAttribute('d');
                const fill = this.getAttribute('fill');
                const transform = this.getAttribute('transform') || '无';
                document.getElementById('pathInfo').innerHTML = 
                    `<br><strong>选中路径 #${{index}}:</strong><br>` +
                    `颜色: ${{fill}}<br>` +
                    `变换: ${{transform}}<br>` +
                    `路径: <code style="font-size:11px;word-break:break-all">${{d.substring(0, 200)}}${{d.length > 200 ? '...' : ''}}</code>`;
            }});
        }});
    </script>
</body>
</html>"""
        return html_template


def export_commands_to_svg(
    commands: list, filename: str, width: int = 466, height: int = 466
):
    """
    便捷函数：将命令列表导出为 SVG/HTML

    Args:
        commands: ParsedCommand 列表
        filename: 输出文件路径 (以 .svg 结尾则导出 SVG，否则导出 HTML)
        width: 画布宽度
        height: 画布高度
    """
    exporter = SVGExporter(width=width, height=height)
    exporter.process_commands(commands)

    if filename.endswith(".svg"):
        exporter.export_svg(filename)
    else:
        exporter.export_html(filename)
