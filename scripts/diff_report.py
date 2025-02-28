import argparse
import csv
from collections import defaultdict


def parse_csv(file_path):
    """解析CSV文件，返回{测试用例: 时间数据}字典"""
    with open(file_path, "r") as f:
        reader = csv.reader(f)

        # 定位标题行
        headers = next(row for row in reader if row and row[0] == "Testcase")

        # 获取关键字段索引
        indices = {
            field: headers.index(field)
            for field in ["Setup Time(ms)", "Draw Time(ms)", "Finish Time(ms)"]
        }

        # 解析数据行
        return {
            row[0]: {
                "setup": safe_float(row[indices["Setup Time(ms)"]]),
                "draw": safe_float(row[indices["Draw Time(ms)"]]),
                "finish": safe_float(row[indices["Finish Time(ms)"]]),
            }
            for row in reader
            if row and not row[0].startswith("Test result")
        }


def safe_float(value):
    """安全转换为浮点数"""
    try:
        return float(value.strip()) if value.strip() else None
    except:
        return None


def compare_data(base, comp, pct_th, abs_th):
    """对比性能数据生成异常报告"""
    anomalies = defaultdict(list)

    for case in base:
        if case not in comp:
            continue

        base_data = base[case]
        comp_data = comp[case]

        for field in ["setup", "draw", "finish"]:
            b = base_data[field]
            c = comp_data[field]

            if None in (b, c):
                continue

            abs_diff = abs(c - b)
            pct_diff = abs((c - b) / b * 100) if b else float("inf")

            if pct_diff > pct_th and abs_diff >= abs_th:
                anomalies[case].append(
                    {
                        "field": field.upper(),
                        "pct": pct_diff,
                        "abs": abs_diff,
                        "base_val": b,
                        "comp_val": c,
                    }
                )

    return anomalies


def print_report(anomalies, pct_th, abs_th):
    """格式化输出对比报告"""
    if not anomalies:
        print(f"\n\033[32m所有用例正常 (阈值 >{pct_th}% 且 ≥{abs_th}ms)\033[0m")
        return

    header = f"{'测试用例':<25} | {'异常指标':<40} | 基准值/对比值"
    print(f"\n\033[31m发现异常 (阈值 >{pct_th}% 且 ≥{abs_th}ms):\033[0m")
    print("-" * 90)
    print(header)
    print("-" * 90)

    for case, details in anomalies.items():
        # 构建时间信息
        time_info = [
            (
                f"{field.upper()}: {details[0]['base_val']:.3f}/{details[0]['comp_val']:.3f}"
                if details
                else ""
            )
            for field in ["setup", "draw", "finish"]
        ]

        # 构建异常描述
        anomalies_str = " | ".join(
            [f"{d['field']}: {d['pct']:.2f}% ({d['abs']:.4f}ms)" for d in details]
        )

        print(f"{case:<25} | {anomalies_str:<40} | {' | '.join(time_info)}")


def main():
    parser = argparse.ArgumentParser(
        description="性能对比工具",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("-b", "--base", required=True, help="基准CSV文件")
    parser.add_argument("-c", "--compare", required=True, help="对比CSV文件")
    parser.add_argument(
        "-p", "--pct-threshold", type=float, default=10.0, help="百分比偏差阈值"
    )
    parser.add_argument(
        "-a", "--abs-threshold", type=float, default=0.01, help="绝对差值阈值(ms)"
    )

    args = parser.parse_args()

    try:
        # 打印生效参数
        print("\n\033[36m[当前配置]\033[0m")
        print(f"| 基准文件: {args.base}")
        print(f"| 对比文件: {args.compare}")
        print(f"| 百分比阈值: {args.pct_threshold:.2f}%")
        print(f"| 绝对差值阈值: {args.abs_threshold:.3f}ms")
        print("\033[36m" + "-" * 50 + "\033[0m\n")

        base = parse_csv(args.base)
        comp = parse_csv(args.compare)
        anomalies = compare_data(base, comp, args.pct_threshold, args.abs_threshold)
        print_report(anomalies, args.pct_threshold, args.abs_threshold)

    except Exception as e:
        print(f"\033[31m错误: {str(e)}\033[0m")


if __name__ == "__main__":
    main()
