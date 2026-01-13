# VGLite Command Buffer Parser

A powerful Python tool for parsing and visualizing VGLite GPU command buffer data. This tool helps debug GPU rendering issues by analyzing command sequences, registers, paths, and generating visual representations.

## Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [Usage](#usage)
  - [Parsing Log Files](#parsing-log-files)
  - [Parsing Coredump Files](#parsing-coredump-files)
  - [HTML/SVG Visualization](#htmlsvg-visualization)
  - [Interactive Mode](#interactive-mode)
- [Command Line Options](#command-line-options)
- [Output Examples](#output-examples)
- [Architecture](#architecture)
- [Technical Details](#technical-details)

## Installation

### Prerequisites

- Python 3.7+
- pip (Python package manager)

### Install Dependencies

```bash
cd gpu_test/scripts/vg_lite_cmdbuf_parser
pip install -r requirements.txt
```

Required packages:
- `rich>=13.0.0` - Terminal formatting and colorful output
- `pyelftools>=0.29` - ELF file parsing (for coredump analysis)
- `Pillow` (optional) - Image generation for HTML visualization

## Quick Start

```bash
# Parse a log file with default settings (path and image parsing enabled)
python main.py -f dump.log

# Parse with verbose output
python main.py -f dump.log -v

# Parse a coredump file and export HTML visualization
python main.py --elf firmware.elf --core crash.core --export-html out.html
```

## Usage

### Parsing Log Files

The parser can analyze VGLite command buffer dumps from log files. These typically contain hexadecimal command-data pairs:

```bash
# Basic parsing (default: path and image parsing enabled)
python main.py -f dump.log

# Enable register analysis
python main.py -f dump.log -r

# Enable verbose mode for detailed output
python main.py -f dump.log -v

# Check log integrity (detect corrupted lines)
python main.py -f dump.log -c

# Disable path parsing (faster)
python main.py -f dump.log --no-parse-path

# Disable image analysis
python main.py -f dump.log --no-parse-image
```

#### Expected Log Format

The parser expects log lines containing command-data pairs:

```
0x30010A00 0x00000100
0x30010A01 0x2F000000
0x40000008 0x00000000
...
```

Lines can include timestamps and log prefixes - these are automatically stripped:

```
[01/13 10:30:45] [ap] 0x30010A00 0x00000100
```

### Parsing Coredump Files

Extract and analyze command buffers directly from coredump files:

```bash
# Parse coredump with ELF symbols
python main.py --elf firmware.elf --core crash.core

# Specify which command buffer to analyze (0 or 1)
python main.py --elf firmware.elf --core crash.core --cmdbuf 0

# Verbose mode with path parsing
python main.py --elf firmware.elf --core crash.core -v
```

The coredump parser automatically:
- Locates `s_context` and command buffer symbols in ELF
- Extracts command buffer data from core memory segments
- Retrieves target buffer information (dimensions, format, address)
- Reads VGLite context state (blend mode, scissor, etc.)

### HTML/SVG Visualization

Generate interactive visualizations of the parsed commands:

```bash
# Export to HTML (default filename: out.html)
python main.py --elf firmware.elf --core crash.core --export-html

# Specify output filename
python main.py --elf firmware.elf --core crash.core --export-html result.html

# Export as pure SVG
python main.py --elf firmware.elf --core crash.core --export-html result.svg

# Customize canvas dimensions
python main.py --elf firmware.elf --core crash.core --export-html out.html \
    --canvas-width 800 --canvas-height 600
```

The HTML output includes:
- SVG rendering of all path commands
- Interactive hover for command details
- Context state information panel
- Target buffer image (if available from coredump)

### Interactive Mode

Enter commands interactively from stdin:

```bash
# Start interactive mode
python main.py -i

# Then paste log content and press Enter twice to process
```

### Parse String Directly

```bash
# Parse a single command string
python main.py -s "0x30010A00 0x00000100"
```

## Command Line Options

| Option | Short | Description |
|--------|-------|-------------|
| `--file` | `-f` | Log file path to parse |
| `--string` | `-s` | Parse command string directly |
| `--interactive` | `-i` | Interactive mode (read from stdin) |
| `--verbose` | `-v` | Enable verbose output mode |
| `--regs` | `-r` | Analyze and print GPU register information |
| `--check-integrity` | `-c` | Check log integrity for corruption |
| `--no-parse-path` | | Disable path data parsing (enabled by default) |
| `--no-parse-image` | | Disable image draw analysis (enabled by default) |
| `--elf` | | ELF file path (for coredump parsing) |
| `--core` | | Coredump file path |
| `--cmdbuf` | | Command buffer index (0 or 1), default: backup buffer |
| `--export-html` | `-S` | Export HTML/SVG visualization |
| `--canvas-width` | | SVG canvas width (default: 466) |
| `--canvas-height` | | SVG canvas height (default: 466) |

## Output Examples

### Command Table Output

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ 【Last Submit Command】                                                          ┃
┣━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ Offset ┃ Cmd Word   ┃ Data Word  ┃ Type       ┃ Description                      ┃
┡━━━━━━━━╇━━━━━━━━━━━━╇━━━━━━━━━━━━╇━━━━━━━━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩
│   0000 │ 30010A00   │ 00000100   │ STATE      │ Write register VgControl         │
│   0008 │ 30010A01   │ 2F000000   │ STATE      │ Write register VgTargetAddress   │
│   0010 │ 40000008   │ 00000000   │ DATA       │ Path data (8 bytes)              │
│   0018 │ 00000000   │ 00000000   │ END        │ End command                      │
└────────┴────────────┴────────────┴────────────┴──────────────────────────────────┘
```

### Abnormal Command Detection

Commands with potential issues are highlighted in red with warning indicators:

```
⚠️ 0020 │ 30010A99   │ 00000000   │ STATE      │ Write register REG_0x0A99
                                                │   ⚠️ Unknown register address: 0x0A99
```

## Architecture

```
vg_lite_cmdbuf_parser/
├── __init__.py           # Package exports
├── main.py               # CLI entry point
├── constants.py          # Register maps, opcodes, formats
├── models.py             # Data classes (ParsedCommand, PathSegment, etc.)
├── command_parser.py     # Core command parsing logic
├── path_parser.py        # VGLite path data parser (VLC opcodes)
├── register_analyzer.py  # GPU register analysis
├── integrity.py          # Log integrity checker
├── coredump_parser.py    # Coredump/ELF parsing
├── svg_exporter.py       # HTML/SVG visualization generator
├── output.py             # Rich terminal output formatting
└── requirements.txt      # Python dependencies
```

## Technical Details

### VGLite Command Format

VGLite commands are 64-bit (two 32-bit words):

| Bits 31-28 | Bits 27-16 | Bits 15-0 | Description |
|------------|------------|-----------|-------------|
| Opcode     | Count      | Address   | Command word structure |

**Command Types:**

| Opcode | Name | Description |
|--------|------|-------------|
| `0x0` | END | End of command buffer, optionally trigger interrupt |
| `0x1` | SEMAPHORE | Signal semaphore for synchronization |
| `0x2` | STALL | Wait for semaphore |
| `0x3` | STATE | Write GPU registers (single or batch) |
| `0x4` | DATA | Inline data (path coordinates, etc.) |
| `0x6` | CALL | Call subroutine at address |
| `0x7` | RETURN | Return from subroutine |
| `0x8` | NOP | No operation |

### Register Decoding

The parser recognizes VGLite GPU registers in the `0x0A00-0x0AFF` range:

- `0x0A00` - VgControl (render control)
- `0x0A01` - VgTargetAddress (destination buffer)
- `0x0A04` - VgImageAddress (source image)
- `0x0A30-0x0A38` - Image transformation matrix
- `0x0A40-0x0A45` - Path transformation matrix
- And many more...

### Path Data Parsing

VGLite path commands use VLC (Vector Line Commands) opcodes:

| Opcode | Name | Coordinates | Description |
|--------|------|-------------|-------------|
| `0x00` | END | 0 | End path |
| `0x01` | CLOSE | 0 | Close subpath |
| `0x02` | MOVE | 2 | Move to (x, y) |
| `0x04` | LINE | 2 | Line to (x, y) |
| `0x06` | QUAD | 4 | Quadratic Bézier curve |
| `0x08` | CUBIC | 6 | Cubic Bézier curve |
| `0x0B` | HLINE | 1 | Horizontal line |
| `0x0D` | VLINE | 1 | Vertical line |
| `0x13-0x1A` | *ARC | 5 | Arc commands (various) |

Supported path formats: `S8`, `S16`, `S32`, `FP32`

### Coredump Parsing

The coredump parser extracts data from memory by:

1. **Symbol Resolution**: Reading symbol table from ELF to find `s_context` address
2. **Memory Mapping**: Parsing PT_LOAD segments in coredump to build memory map
3. **Structure Traversal**: Following known offsets in `vg_lite_context_t` structure:
   - `command_buffer[2]` at offset `0x6D0`
   - `command_buffer_size` at offset `0x6D8`
   - `rtbuffer` (target buffer pointer) at offset `0x720`
   - Blend mode, scissor, filter settings, etc.

### Integrity Checking

The integrity checker detects common log corruption issues:

- **Duplicate timestamps**: Multiple `[MM/DD HH:MM:SS]` in one line
- **Line merging**: Multiple `[ap]` tags indicating concurrent output collision
- **Data truncation**: Incomplete hex values at line boundaries
- **Missing separators**: Concatenated hex values without whitespace

### HTML/SVG Export

The SVG exporter generates visualizations by:

1. Processing all STATE commands to track render state
2. Collecting path segments from DATA commands
3. Applying transformation matrices to paths
4. Rendering paths as SVG `<path>` elements with proper fill colors
5. Wrapping in interactive HTML with metadata panels

## Python API Usage

You can also use the parser as a Python library:

```python
from vg_lite_cmdbuf_parser import (
    VGLiteCommandParser,
    CoredumpParser,
    SVGExporter,
    parse_coredump,
)

# Parse log text
parser = VGLiteCommandParser(verbose=True, parse_path=True)
commands = parser.parse_log(log_text)

for cmd in commands:
    print(f"{cmd.cmd_type}: {cmd.description}")

# Parse coredump
commands, target_info, context_state = parse_coredump(
    elf_path="firmware.elf",
    core_path="crash.core",
    verbose=True,
)

# Export visualization
exporter = SVGExporter(width=800, height=600)
exporter.set_target_info(target_info)
exporter.process_commands(commands)
exporter.export_html("output.html")
```

## License

This tool is part of the VGLite GPU testing infrastructure.
