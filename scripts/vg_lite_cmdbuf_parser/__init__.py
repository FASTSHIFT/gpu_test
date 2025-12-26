#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VGLite Command Buffer Parser
============================
解析 VGLite GPU 命令缓冲区数据的工具包
"""

from .constants import (
    CommandType,
    GPU_REGISTER_INFO,
    VLC_OP_CODES,
    REGISTER_MAP,
    BLEND_MODES,
    IMAGE_FORMATS,
    PATH_FORMATS,
    PATH_QUALITY,
    ADDRESS_REGISTERS,
    VALID_REG_RANGES,
    SUSPICIOUS_ADDRESSES,
)

from .models import (
    LogIntegrityIssue,
    ImageDrawInfo,
    PathSegment,
    ParsedCommand,
    CommandSection,
)

from .integrity import LogIntegrityChecker
from .path_parser import VGLitePathParser
from .command_parser import VGLiteCommandParser
from .register_analyzer import GPURegisterAnalyzer
from .output import (
    create_command_table,
    add_command_to_table,
    print_summary,
)

__version__ = "2.0.0"
__all__ = [
    # Constants
    "CommandType",
    "GPU_REGISTER_INFO",
    "VLC_OP_CODES",
    "REGISTER_MAP",
    "BLEND_MODES",
    "IMAGE_FORMATS",
    "PATH_FORMATS",
    "PATH_QUALITY",
    "ADDRESS_REGISTERS",
    "VALID_REG_RANGES",
    "SUSPICIOUS_ADDRESSES",
    # Models
    "LogIntegrityIssue",
    "ImageDrawInfo",
    "PathSegment",
    "ParsedCommand",
    "CommandSection",
    # Classes
    "LogIntegrityChecker",
    "VGLitePathParser",
    "VGLiteCommandParser",
    "GPURegisterAnalyzer",
    # Functions
    "create_command_table",
    "add_command_to_table",
    "print_summary",
]
