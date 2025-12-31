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
import hashlib
import base64
import io

# 尝试导入 PIL，用于生成目标缓冲区图像
try:
    from PIL import Image

    HAS_PIL = True
except ImportError:
    HAS_PIL = False


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
    split_count: int = 1  # VGLite SPLIT 策略分割次数
    bounding_box: Optional[Tuple[int, int, int, int]] = (
        None  # (x, y, width, height) 路径裁剪区域
    )

    def get_hash_key(self) -> str:
        """生成路径的唯一哈希键，用于去重"""
        # 基于路径数据、颜色、变换矩阵等生成哈希
        key_parts = []

        # 路径段数据
        for seg in self.path_segments:
            op = seg.op_name if hasattr(seg, "op_name") else seg.get("op", "")
            coords = seg.coords if hasattr(seg, "coords") else seg.get("coords", [])
            key_parts.append(f"{op}:{','.join(f'{c:.4f}' for c in coords)}")

        # 颜色
        key_parts.append(f"color:{self.color}")

        # 变换矩阵
        if self.matrix:
            key_parts.append(f"matrix:{','.join(f'{m:.6f}' for m in self.matrix)}")

        # path scale 和 bias
        key_parts.append(f"scale:{self.path_scale:.6f}")
        key_parts.append(f"bias:{self.path_bias:.6f}")

        key_str = "|".join(key_parts)
        return hashlib.md5(key_str.encode()).hexdigest()


class SVGExporter:
    """SVG/HTML 导出器"""

    def __init__(self, width: int = 466, height: int = 466, deduplicate: bool = True):
        """
        初始化导出器

        Args:
            width: 画布宽度
            height: 画布高度
            deduplicate: 是否去重 VGLite SPLIT 策略产生的重复路径（默认开启）
        """
        self.width = width
        self.height = height
        self.draw_commands: List[DrawCommand] = []
        self.background_color = "#000000"  # 纯黑色背景
        self.deduplicate = deduplicate
        self.target_info = None  # 渲染目标缓冲区信息
        self.context_state = None  # VGLite 上下文状态信息

    def set_target_info(self, target_info):
        """设置渲染目标缓冲区信息"""
        self.target_info = target_info

    def set_context_state(self, context_state):
        """设置 VGLite 上下文状态信息"""
        self.context_state = context_state

    def _generate_target_buffer_image(self) -> Optional[str]:
        """生成目标缓冲区的 base64 编码图像

        Returns:
            base64 编码的 PNG 图像数据 URL，或 None
        """
        if not HAS_PIL:
            return None

        if not self.target_info or not self.target_info.pixel_data:
            return None

        width = self.target_info.width
        height = self.target_info.height
        stride = self.target_info.stride
        fmt = self.target_info.format & 0x3FF  # 去掉标志位
        data = self.target_info.pixel_data

        try:
            # 创建图像
            img = Image.new("RGBA", (width, height))
            pixels = img.load()

            for y in range(height):
                row_offset = y * stride
                for x in range(width):
                    if fmt == 3:  # BGRX8888
                        pixel_offset = row_offset + x * 4
                        if pixel_offset + 4 <= len(data):
                            b = data[pixel_offset]
                            g = data[pixel_offset + 1]
                            r = data[pixel_offset + 2]
                            # X (忽略)
                            pixels[x, y] = (r, g, b, 255)
                    elif fmt == 1:  # BGRA8888
                        pixel_offset = row_offset + x * 4
                        if pixel_offset + 4 <= len(data):
                            b = data[pixel_offset]
                            g = data[pixel_offset + 1]
                            r = data[pixel_offset + 2]
                            a = data[pixel_offset + 3]
                            pixels[x, y] = (r, g, b, a)
                    elif fmt == 0:  # RGBA8888
                        pixel_offset = row_offset + x * 4
                        if pixel_offset + 4 <= len(data):
                            r = data[pixel_offset]
                            g = data[pixel_offset + 1]
                            b = data[pixel_offset + 2]
                            a = data[pixel_offset + 3]
                            pixels[x, y] = (r, g, b, a)
                    elif fmt == 2:  # RGBX8888
                        pixel_offset = row_offset + x * 4
                        if pixel_offset + 4 <= len(data):
                            r = data[pixel_offset]
                            g = data[pixel_offset + 1]
                            b = data[pixel_offset + 2]
                            pixels[x, y] = (r, g, b, 255)

            # 转换为 base64
            buffer = io.BytesIO()
            img.save(buffer, format="PNG")
            img_base64 = base64.b64encode(buffer.getvalue()).decode()
            return f"data:image/png;base64,{img_base64}"

        except Exception as e:
            print(f"[警告] 生成目标缓冲区图像失败: {e}")
            return None

    def _deduplicate_commands(self):
        """
        去重 VGLite SPLIT 策略产生的重复路径

        VGLite 使用 SPLIT 策略将大型/复杂路径分割成多个 tessellation 窗口渲染，
        这会导致相同的路径数据被发送多次。此方法识别并合并这些重复路径，
        同时记录分割次数用于显示，并合并所有 tile 的 bounding box。
        """
        if not self.draw_commands:
            return

        # 使用哈希键来识别重复路径
        seen = {}  # hash_key -> (index, count)
        deduplicated = []
        original_count = len(self.draw_commands)

        for draw_cmd in self.draw_commands:
            hash_key = draw_cmd.get_hash_key()
            if hash_key in seen:
                # 找到重复，增加计数并合并 bounding box
                idx = seen[hash_key]
                deduplicated[idx].split_count += 1
                # 合并 bounding box (取并集)
                if draw_cmd.bounding_box and deduplicated[idx].bounding_box:
                    ox, oy, ow, oh = deduplicated[idx].bounding_box
                    nx, ny, nw, nh = draw_cmd.bounding_box
                    # 计算两个矩形的并集
                    min_x = min(ox, nx)
                    min_y = min(oy, ny)
                    max_x = max(ox + ow, nx + nw)
                    max_y = max(oy + oh, ny + nh)
                    deduplicated[idx].bounding_box = (
                        min_x,
                        min_y,
                        max_x - min_x,
                        max_y - min_y,
                    )
                elif draw_cmd.bounding_box:
                    deduplicated[idx].bounding_box = draw_cmd.bounding_box
            else:
                # 新路径
                seen[hash_key] = len(deduplicated)
                deduplicated.append(draw_cmd)

        self.draw_commands = deduplicated
        deduplicated_count = len(deduplicated)

        if original_count != deduplicated_count:
            print(
                f"[去重] 原始路径: {original_count}, 去重后: {deduplicated_count}, "
                f"移除重复: {original_count - deduplicated_count}"
            )

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
        current_tess_x = 0
        current_tess_y = 0
        current_tess_w = 0
        current_tess_h = 0

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

                # VgTessWindow (0x0A39) - 裁剪窗口起点
                elif reg_addr == 0x0A39:
                    current_tess_x = cmd.data_word & 0xFFFF
                    current_tess_y = (cmd.data_word >> 16) & 0xFFFF

                # VgTessWindowSize (0x0A3A) - 裁剪窗口大小
                elif reg_addr == 0x0A3A:
                    current_tess_w = cmd.data_word & 0xFFFF
                    current_tess_h = (cmd.data_word >> 16) & 0xFFFF

            # 处理所有带路径数据的命令 (DATA, DRAW_PATH, CALL 等)
            if cmd.path_segments:
                current_draw.path_segments = cmd.path_segments
                current_draw.matrix = current_matrix.copy() if current_matrix else None
                current_draw.path_scale = current_path_scale
                current_draw.path_bias = current_path_bias
                # 保存当前的 bounding box
                if current_tess_w > 0 and current_tess_h > 0:
                    current_draw.bounding_box = (
                        current_tess_x,
                        current_tess_y,
                        current_tess_w,
                        current_tess_h,
                    )
                self.draw_commands.append(current_draw)
                current_draw = DrawCommand()

        # 不再在此处去重，改为在 HTML 中动态控制

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

        # 统计每个 hash 出现的次数，用于标记重复路径
        hash_count = {}
        hash_first_index = {}  # 记录每个 hash 第一次出现的索引
        for i, draw_cmd in enumerate(self.draw_commands):
            h = draw_cmd.get_hash_key()
            if h not in hash_count:
                hash_count[h] = 0
                hash_first_index[h] = i
            hash_count[h] += 1

        for i, draw_cmd in enumerate(self.draw_commands):
            d = self._path_to_svg(draw_cmd)
            if not d:
                continue

            r, g, b, a = self._color_to_rgba(draw_cmd.color)
            fill_color = f"rgb({r},{g},{b})"
            opacity = a * draw_cmd.opacity

            transform = self._matrix_to_svg(draw_cmd.matrix)

            # 生成 hash 用于去重
            path_hash = draw_cmd.get_hash_key()[:8]  # 只取前8位
            is_duplicate = hash_first_index.get(draw_cmd.get_hash_key(), i) != i
            dup_attr = 'data-duplicate="true"' if is_duplicate else ""

            # 添加 split-count 属性用于显示 VGLite SPLIT 策略信息
            split_count = hash_count.get(draw_cmd.get_hash_key(), 1)
            split_attr = f'data-split-count="{split_count}"' if split_count > 1 else ""

            # 添加 bounding box 属性
            bbox_attr = ""
            if draw_cmd.bounding_box:
                bx, by, bw, bh = draw_cmd.bounding_box
                bbox_attr = f'data-bbox-x="{bx}" data-bbox-y="{by}" data-bbox-w="{bw}" data-bbox-h="{bh}"'

            path_elem = f'<path d="{d}" fill="{fill_color}" fill-opacity="{opacity:.2f}" fill-rule="{draw_cmd.fill_rule}" {transform} data-index="{i}" data-hash="{path_hash}" {dup_attr} {split_attr} {bbox_attr}/>'
            paths.append(path_elem)

        # 生成 bounding box 矩形组 (每个路径一个，初始隐藏)
        bbox_rects = []
        for i, draw_cmd in enumerate(self.draw_commands):
            if draw_cmd.bounding_box:
                bx, by, bw, bh = draw_cmd.bounding_box
                bbox_rects.append(
                    f'<rect class="bbox-rect" data-path-index="{i}" x="{bx}" y="{by}" width="{bw}" height="{bh}" fill="none" stroke="#ff00ff" stroke-width="1" stroke-dasharray="3,3" style="display: none;"/>'
                )
        # 添加动态合并边界矩形（用于去重模式）
        bbox_rects.append(
            '<rect id="mergedBbox" fill="none" stroke="#ff00ff" stroke-width="2" stroke-dasharray="4,4" style="display: none;"/>'
        )
        bbox_group = (
            f'<g id="bboxGroup">{chr(10).join(bbox_rects)}</g>' if bbox_rects else ""
        )

        # 生成 scissor 矩形
        scissor_rect = ""
        if self.context_state and self.context_state.scissor:
            sx, sy, sr, sb = self.context_state.scissor
            sw = sr - sx  # width = right - x
            sh = sb - sy  # height = bottom - y
            if sw > 0 and sh > 0:
                scissor_rect = f'<rect id="scissorRect" x="{sx}" y="{sy}" width="{sw}" height="{sh}" fill="none" stroke="#00ff00" stroke-width="2" stroke-dasharray="5,5" style="display: none;"/>'

        svg = f"""<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" width="{self.width}" height="{self.height}" viewBox="0 0 {self.width} {self.height}">
  <rect width="100%" height="100%" fill="{self.background_color}"/>
  <g id="paths">
    {chr(10).join("    " + p for p in paths)}
  </g>
  {bbox_group}
  {scissor_rect}
</svg>"""
        return svg

    def _wrap_in_html(self, svg_content: str) -> str:
        """将 SVG 包装为交互式 HTML"""
        # 移除 XML 声明
        svg_content = svg_content.replace(
            '<?xml version="1.0" encoding="UTF-8"?>\n', ""
        )

        # 生成 target buffer 信息 HTML
        target_info_html = ""
        target_buffer_image_html = ""
        if self.target_info:
            # VGLite 格式: base_value | (1 << 10)
            # 提取 base 值
            fmt = self.target_info.format
            base_fmt = fmt & 0x3FF  # 去掉 1<<10 标志位
            format_names = {
                0: "RGBA8888",
                1: "BGRA8888",
                2: "RGBX8888",
                3: "BGRX8888",
                4: "RGB565",
                5: "BGR565",
                6: "RGBA4444",
                7: "BGRA4444",
                8: "BGRA5551",
                9: "A4",
                10: "A8",
                11: "L8",
                25: "RGBA2222",
                26: "BGRA2222",
                31: "ABGR8888",
                32: "ARGB8888",
                36: "XBGR8888",
                37: "XRGB8888",
            }
            fmt_name = format_names.get(base_fmt, f"Unknown({fmt})")
            target_info_html = f"""
            <strong>目标缓冲区:</strong> {self.target_info.width}x{self.target_info.height}, {fmt_name}, stride={self.target_info.stride}<br>"""

            # 生成目标缓冲区图像
            image_data = self._generate_target_buffer_image()
            if image_data:
                target_buffer_image_html = f"""
                <div class="canvas-panel">
                    <h3>目标缓冲区</h3>
                    <div class="image-container">
                        <img src="{image_data}" alt="Target Buffer" style="max-width: 100%; border: 1px solid #666;">
                    </div>
                </div>"""

        # 生成上下文状态信息 HTML
        context_state_html = ""
        if self.context_state:
            # 混合模式名称映射
            blend_names = {
                0: "NONE",
                1: "SRC_OVER",
                2: "DST_OVER",
                3: "SRC_IN",
                4: "DST_IN",
                5: "MULTIPLY",
                6: "SCREEN",
                7: "DARKEN",
                8: "LIGHTEN",
                9: "ADDITIVE",
                10: "SUBTRACT",
                11: "NORMAL_LVGL",
                12: "ADDITIVE_LVGL",
                13: "SUBTRACT_LVGL",
                14: "MULTIPLY_LVGL",
                15: "PREMULTIPLY_SRC_OVER",
            }
            blend_name = blend_names.get(
                self.context_state.blend_mode,
                f"Unknown({self.context_state.blend_mode})",
            )

            # 滤波器名称映射
            filter_names = {
                0: "POINT",
                0x1000: "LINEAR",
                0x2000: "BI_LINEAR",
                0x3000: "GAUSSIAN",
            }
            filter_name = filter_names.get(
                self.context_state.filter,
                f"Unknown(0x{self.context_state.filter:X})",
            )

            scissor_str = (
                f"({self.context_state.scissor[0]}, {self.context_state.scissor[1]}) - "
                f"({self.context_state.scissor[2]}, {self.context_state.scissor[3]})"
            )

            context_state_html = f"""
            <strong>上下文状态:</strong><br>
            &nbsp;&nbsp;• blend_mode (混合模式): {blend_name}<br>
            &nbsp;&nbsp;• filter (滤波器): {filter_name}<br>
            &nbsp;&nbsp;• scissor (裁剪区域): {scissor_str}<br>
            &nbsp;&nbsp;• alpha_mode (Alpha模式): src={self.context_state.src_alpha_mode}, dst={self.context_state.dst_alpha_mode}<br>
            &nbsp;&nbsp;• premultiply (预乘): src={self.context_state.premultiply_src}, dst={self.context_state.premultiply_dst}<br>
            &nbsp;&nbsp;• tess_size (曲面细分尺寸): {self.context_state.tess_width}x{self.context_state.tess_height}<br>
            &nbsp;&nbsp;• color_transform (颜色变换): {self.context_state.color_transform}<br>
            &nbsp;&nbsp;• path_counter (路径计数): {self.context_state.path_counter}<br>"""

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
            max-width: 2000px;
            margin: 0 auto;
        }}
        h1 {{
            color: #333;
            margin-bottom: 20px;
        }}
        .canvas-row {{
            display: flex;
            gap: 20px;
            flex-wrap: wrap;
            align-items: flex-start;
        }}
        .canvas-panel {{
            background: white;
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 20px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        .canvas-panel h3 {{
            margin: 0 0 15px 0;
            color: #555;
            font-size: 14px;
            border-bottom: 1px solid #eee;
            padding-bottom: 10px;
        }}
        .svg-container {{
            display: inline-block;
            overflow: visible;
        }}
        .image-container {{
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
        .path-data {{
            max-height: 300px;
            overflow-y: auto;
            background: #f0f0f0;
            padding: 10px;
            border-radius: 4px;
            margin-top: 10px;
            font-family: monospace;
            font-size: 12px;
            word-break: break-all;
            white-space: pre-wrap;
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
                <input type="checkbox" id="showCrosshair" onchange="toggleCrosshair()"> 显示光标
            </label>
            <label>
                <input type="checkbox" id="showScissor" onchange="toggleScissor()"> 显示裁剪区域
            </label>
            <label>
                <input type="checkbox" id="showBbox" onchange="toggleBbox()"> 显示路径边界
            </label>
            <label>
                <input type="checkbox" id="dedup" checked onchange="toggleDedup()"> SPLIT去重
            </label>
            <label>
                背景色: <input type="color" id="bgColor" value="#000000" oninput="updateBgColor()">
            </label>
            <label>
                渲染进度: <input type="range" id="drawOrder" min="0" max="{len(self.draw_commands)}" step="1" value="{len(self.draw_commands)}" oninput="updateDrawOrder()" style="width: 120px;">
                <span id="drawOrderValue">{len(self.draw_commands)}/{len(self.draw_commands)}</span>
            </label>
        </div>
        <div class="canvas-row">
            <div class="canvas-panel">
                <h3>渲染命令重放</h3>
                <div class="svg-container" id="svgContainer">
                    {svg_content}
                </div>
            </div>
            {target_buffer_image_html}
        </div>
        <div class="info">
            <strong>统计信息:</strong> 共 {len(self.draw_commands)} 个绘制命令<br>
            <strong>画布尺寸:</strong> {self.width} x {self.height}<br>{target_info_html}{context_state_html}
            <strong>鼠标位置:</strong> <span id="mousePos">X: -, Y: -</span><br>
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
        
        function updateBgColor() {{
            const color = document.getElementById('bgColor').value;
            const bgRect = document.querySelector('svg > rect');
            if (bgRect) bgRect.setAttribute('fill', color);
        }}
        
        // 渲染进度控制
        function updateDrawOrder() {{
            const value = parseInt(document.getElementById('drawOrder').value);
            const total = {len(self.draw_commands)};
            document.getElementById('drawOrderValue').textContent = `${{value}}/${{total}}`;
            
            const paths = document.querySelectorAll('svg path');
            paths.forEach((path, index) => {{
                const pathIndex = parseInt(path.getAttribute('data-index'));
                if (pathIndex < value) {{
                    path.style.display = '';
                }} else {{
                    path.style.display = 'none';
                }}
            }});
        }}
        
        // 光标功能
        let crosshairEnabled = false;
        let crosshairH = null;
        let crosshairV = null;
        
        function toggleCrosshair() {{
            crosshairEnabled = document.getElementById('showCrosshair').checked;
            if (!crosshairEnabled) {{
                if (crosshairH) {{ crosshairH.remove(); crosshairH = null; }}
                if (crosshairV) {{ crosshairV.remove(); crosshairV = null; }}
            }}
        }}
        
        function toggleScissor() {{
            const showScissor = document.getElementById('showScissor').checked;
            const scissorRect = document.getElementById('scissorRect');
            if (scissorRect) {{
                scissorRect.style.display = showScissor ? '' : 'none';
            }}
        }}
        
        // 当前选中的路径索引
        var selectedPathIndex = -1;
        
        function toggleBbox() {{
            updateSelectedBbox();
        }}
        
        function updateSelectedBbox() {{
            const showBbox = document.getElementById('showBbox').checked;
            const dedup = document.getElementById('dedup').checked;
            
            // 先隐藏所有 bbox
            document.querySelectorAll('.bbox-rect').forEach(rect => {{
                rect.style.display = 'none';
            }});
            const mergedBbox = document.getElementById('mergedBbox');
            if (mergedBbox) mergedBbox.style.display = 'none';
            
            if (!showBbox || selectedPathIndex < 0) {{
                return;
            }}
            
            // 找到选中的路径元素
            const selectedPath = document.querySelector(`svg path[data-index="${{selectedPathIndex}}"]`);
            if (!selectedPath) return;
            
            const selectedHash = selectedPath.getAttribute('data-hash');
            
            if (dedup && selectedHash) {{
                // 去重模式：计算所有相同 hash 路径的 bbox 并集
                const samePaths = document.querySelectorAll(`svg path[data-hash="${{selectedHash}}"]`);
                let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
                let hasValidBbox = false;
                
                samePaths.forEach(path => {{
                    const pathIndex = path.getAttribute('data-index');
                    const bboxRect = document.querySelector(`.bbox-rect[data-path-index="${{pathIndex}}"]`);
                    if (bboxRect) {{
                        const bx = parseFloat(bboxRect.getAttribute('x'));
                        const by = parseFloat(bboxRect.getAttribute('y'));
                        const bw = parseFloat(bboxRect.getAttribute('width'));
                        const bh = parseFloat(bboxRect.getAttribute('height'));
                        if (!isNaN(bx) && !isNaN(by) && !isNaN(bw) && !isNaN(bh)) {{
                            minX = Math.min(minX, bx);
                            minY = Math.min(minY, by);
                            maxX = Math.max(maxX, bx + bw);
                            maxY = Math.max(maxY, by + bh);
                            hasValidBbox = true;
                        }}
                    }}
                }});
                
                if (hasValidBbox && mergedBbox) {{
                    mergedBbox.setAttribute('x', minX);
                    mergedBbox.setAttribute('y', minY);
                    mergedBbox.setAttribute('width', maxX - minX);
                    mergedBbox.setAttribute('height', maxY - minY);
                    mergedBbox.style.display = '';
                }}
            }} else if (!dedup && selectedHash) {{
                // 不去重模式：显示所有相同 hash 路径的边界（各个tile）
                const samePaths = document.querySelectorAll(`svg path[data-hash="${{selectedHash}}"]`);
                samePaths.forEach(path => {{
                    const pathIndex = path.getAttribute('data-index');
                    const bboxRect = document.querySelector(`.bbox-rect[data-path-index="${{pathIndex}}"]`);
                    if (bboxRect) {{
                        bboxRect.style.display = '';
                    }}
                }});
            }} else {{
                // 没有 hash 或单独的路径，只显示选中路径的边界
                const bboxRect = document.querySelector(`.bbox-rect[data-path-index="${{selectedPathIndex}}"]`);
                if (bboxRect) {{
                    bboxRect.style.display = '';
                }}
            }}
        }}
        
        function toggleDedup() {{
            const dedup = document.getElementById('dedup').checked;
            const paths = document.querySelectorAll('svg path[data-duplicate]');
            paths.forEach(path => {{
                path.style.display = dedup ? 'none' : '';
            }});
            // 更新统计信息
            updateStats();
            // 更新边界显示
            updateSelectedBbox();
        }}
        
        function updateStats() {{
            const dedup = document.getElementById('dedup').checked;
            const allPaths = document.querySelectorAll('svg path');
            const dupPaths = document.querySelectorAll('svg path[data-duplicate]');
            const visibleCount = dedup ? (allPaths.length - dupPaths.length) : allPaths.length;
            // 可以在这里更新显示的统计信息
        }}
        
        // 页面加载时自动应用去重
        document.addEventListener('DOMContentLoaded', function() {{
            toggleDedup();
        }});
        
        function updateCrosshair(x, y) {{
            const svg = document.querySelector('svg');
            if (!crosshairEnabled) return;
            
            if (!crosshairH) {{
                crosshairH = document.createElementNS('http://www.w3.org/2000/svg', 'line');
                crosshairH.setAttribute('stroke', '#00ff00');
                crosshairH.setAttribute('stroke-width', '1');
                crosshairH.setAttribute('stroke-dasharray', '4,4');
                crosshairH.style.pointerEvents = 'none';
                svg.appendChild(crosshairH);
            }}
            if (!crosshairV) {{
                crosshairV = document.createElementNS('http://www.w3.org/2000/svg', 'line');
                crosshairV.setAttribute('stroke', '#00ff00');
                crosshairV.setAttribute('stroke-width', '1');
                crosshairV.setAttribute('stroke-dasharray', '4,4');
                crosshairV.style.pointerEvents = 'none';
                svg.appendChild(crosshairV);
            }}
            
            // 水平线
            crosshairH.setAttribute('x1', '0');
            crosshairH.setAttribute('y1', y);
            crosshairH.setAttribute('x2', '{self.width}');
            crosshairH.setAttribute('y2', y);
            
            // 垂直线
            crosshairV.setAttribute('x1', x);
            crosshairV.setAttribute('y1', '0');
            crosshairV.setAttribute('x2', x);
            crosshairV.setAttribute('y2', '{self.height}');
        }}
        
        // 鼠标位置追踪
        const svg = document.querySelector('svg');
        svg.addEventListener('mousemove', function(e) {{
            const rect = svg.getBoundingClientRect();
            // 计算相对于 SVG 画布的坐标
            const x = e.clientX - rect.left;
            const y = e.clientY - rect.top;
            document.getElementById('mousePos').textContent = `X: ${{Math.round(x)}}, Y: ${{Math.round(y)}}`;
            updateCrosshair(x, y);
        }});
        
        svg.addEventListener('mouseleave', function() {{
            document.getElementById('mousePos').textContent = 'X: -, Y: -';
            if (crosshairH) {{ crosshairH.remove(); crosshairH = null; }}
            if (crosshairV) {{ crosshairV.remove(); crosshairV = null; }}
        }});
        
        // 路径点击事件
        document.querySelectorAll('svg path').forEach(path => {{
            path.addEventListener('click', function() {{
                // 移除之前的选中状态
                document.querySelectorAll('svg path.selected').forEach(p => p.classList.remove('selected'));
                // 添加选中状态
                this.classList.add('selected');
                
                const index = this.getAttribute('data-index');
                selectedPathIndex = parseInt(index);
                
                // 更新选中路径的 bbox 显示
                updateSelectedBbox();
                
                const d = this.getAttribute('d');
                const fill = this.getAttribute('fill');
                const fillOpacity = this.getAttribute('fill-opacity');
                const fillRule = this.getAttribute('fill-rule');
                const transform = this.getAttribute('transform') || '无';
                const splitCount = this.getAttribute('data-split-count');
                const bboxX = this.getAttribute('data-bbox-x');
                const bboxY = this.getAttribute('data-bbox-y');
                const bboxW = this.getAttribute('data-bbox-w');
                const bboxH = this.getAttribute('data-bbox-h');
                
                // 将SVG路径格式转换为 MOVE,x,y 格式
                function formatPath(svgPath) {{
                    return svgPath
                        .replace(/M\\s*([\\d.-]+)\\s+([\\d.-]+)/g, 'MOVE,$1,$2,')
                        .replace(/L\\s*([\\d.-]+)\\s+([\\d.-]+)/g, 'LINE,$1,$2,')
                        .replace(/Q\\s*([\\d.-]+)\\s+([\\d.-]+)\\s+([\\d.-]+)\\s+([\\d.-]+)/g, 'QUAD,$1,$2,$3,$4,')
                        .replace(/C\\s*([\\d.-]+)\\s+([\\d.-]+)\\s+([\\d.-]+)\\s+([\\d.-]+)\\s+([\\d.-]+)\\s+([\\d.-]+)/g, 'CUBIC,$1,$2,$3,$4,$5,$6,')
                        .replace(/Z/g, 'CLOSE,')
                        .replace(/,\\s*/g, ',')
                        .split(/(MOVE|LINE|QUAD|CUBIC|CLOSE),/)
                        .filter(s => s.trim())
                        .reduce((acc, curr, i, arr) => {{
                            if (['MOVE', 'LINE', 'QUAD', 'CUBIC', 'CLOSE'].includes(curr)) {{
                                acc.push(curr + ',' + (arr[i+1] || ''));
                            }}
                            return acc;
                        }}, [])
                        .join('\\n') + '\\nEND,';
                }}
                
                const formattedD = formatPath(d);
                
                // 生成 split 信息（如果存在）
                let splitInfo = '';
                if (splitCount && parseInt(splitCount) > 1) {{
                    splitInfo = `<span style="color: #ff9800; font-weight: bold;">⚠ VGLite SPLIT x${{splitCount}}</span><br>`;
                }}
                
                // 生成 bounding box 信息
                let bboxInfo = '';
                if (bboxX && bboxY && bboxW && bboxH) {{
                    bboxInfo = `路径边界: (${{bboxX}}, ${{bboxY}}) - ${{bboxW}}x${{bboxH}}<br>`;
                }}
                
                document.getElementById('pathInfo').innerHTML = 
                    `<br><strong>选中路径 #${{index}}:</strong><br>` +
                    splitInfo +
                    `颜色: ${{fill}} (透明度: ${{fillOpacity}})<br>` +
                    `填充规则: ${{fillRule}}<br>` +
                    bboxInfo +
                    `变换矩阵: <code>${{transform}}</code><br>` +
                    `路径长度: ${{d.length}} 字符<br>` +
                    `<div class="path-data">${{formattedD}}</div>`;
            }});
        }});
    </script>
</body>
</html>"""
        return html_template


def export_commands_to_svg(
    commands: list,
    filename: str,
    width: int = 466,
    height: int = 466,
    deduplicate: bool = True,
):
    """
    便捷函数：将命令列表导出为 SVG/HTML

    Args:
        commands: ParsedCommand 列表
        filename: 输出文件路径 (以 .svg 结尾则导出 SVG，否则导出 HTML)
        width: 画布宽度
        height: 画布高度
        deduplicate: 是否去重 VGLite SPLIT 策略产生的重复路径（默认开启）
    """
    exporter = SVGExporter(width=width, height=height, deduplicate=deduplicate)
    exporter.process_commands(commands)

    if filename.endswith(".svg"):
        exporter.export_svg(filename)
    else:
        exporter.export_html(filename)
