import matplotlib.pyplot as plt
import numpy as np
import glob

# 颜色方案 - 为每个优化级别分配独特颜色
color_palette = {
    'o0': '#1f77b4',  # 蓝色
    'o1': '#ff7f0e',  # 橙色
    'o2': '#2ca02c',  # 绿色
    'o3': '#d62728',  # 红色
}

def read_algorithm_data(algorithm_name):
    """读取指定算法在所有优化级别下的数据"""
    data = {}
    n_values = None
    for opt_level in ['o0', 'o1', 'o2', 'o3']:
        filename = f"{algorithm_name}_{opt_level}.txt"
        try:
            with open(filename, 'r') as f:
                n_values = []
                times = []
                for line in f:
                    n, time = line.strip().split(',')
                    n_values.append(int(n))
                    times.append(float(time))
                data[opt_level] = {'n': n_values, 'time': times}
        except FileNotFoundError:
            print(f"Warning: File {filename} not found")
        except ValueError:
            print(f"Error: Invalid data format in {filename}")
    return data

# 读取两个算法的数据
algoA_data = read_algorithm_data("normal_algo")
algoB_data = read_algorithm_data("optimizer_algo")

# 创建图表
plt.figure(figsize=(14, 8))

# 绘制算法A的各优化级别曲线
for opt_level in ['o0', 'o1', 'o2', 'o3']:
    if opt_level in algoA_data and 'n' in algoA_data[opt_level]:
        plt.plot(algoA_data[opt_level]['n'], algoA_data[opt_level]['time'], 
                label=f'sum_normal_{opt_level}', 
                color=color_palette[opt_level],
                linestyle='-', 
                linewidth=2,
                marker='o')


# 添加图表元素
plt.title('Algorithm Performance Across Compiler Optimization Levels', fontsize=16, pad=20)
plt.xlabel('Input Size (n)', fontsize=14)
plt.ylabel('Execution Time (ms)', fontsize=14)
plt.legend(fontsize=12, bbox_to_anchor=(1.05, 1), loc='upper left')

# 添加网格和调整布局
plt.grid(True, linestyle='--', alpha=0.6)
plt.tight_layout()

# 显示图表
plt.show()