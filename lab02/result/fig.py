import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.signal import savgol_filter

# Savitzky-Golay 滤波平滑函数
def smooth_series_savgol(series, window_size=11, polyorder=3):
    """Savitzky-Golay滤波器平滑"""
    if len(series) < window_size:
        return series  # 如果数据太少，直接返回原数据
    return savgol_filter(series, window_length=window_size, polyorder=polyorder)

# 你的CSV文件名列表
csv_files = [
    'time_chuan.csv',
    #'time_SSE_2.csv',
    #'time_SSE_4.csv',
    #'time_AVX_8.csv',
    #'time_AVX_16.csv'
]

# 每个CSV对应的曲线标签
labels = [
    'BaseLine',
    #'Parallel Width 2',
    #'Parallel Width 4',
    #'Parallel Width 8',
    #'Parallel Width 16'
]

# 是否启用平滑
use_smoothing = 0  # 这里可以切换
use_savgol = True  # 切换使用Savitzky-Golay滤波器
window_size = 11  # Savitzky-Golay窗口大小（必须是奇数）
poly_order = 3  # 多项式阶数

# 创建画布
plt.figure(figsize=(10, 6))

# 画每一条曲线
for file, label in zip(csv_files, labels):
    data = pd.read_csv(file)
    
    if use_smoothing:
        if use_savgol:
            # 使用Savitzky-Golay滤波器
            time_smoothed = smooth_series_savgol(data['time(ms)'], window_size=window_size, polyorder=poly_order)
            plt.plot(data['size'], time_smoothed, label=f'{label}')
        else:
            # 使用简单的移动平均
            time_smoothed = data['time(ms)'].rolling(window=10, center=True).mean()
            plt.plot(data['size'], time_smoothed, label=f'{label} (Smoothed)')
    else:
        plt.plot(data['size'], data['time(ms)'], label=label)

# 设置标签和标题
plt.xlabel('Input Size (n)')
plt.ylabel('Time (s)')
plt.title('Performance Comparison with Different Parallel Widths')
plt.legend()
plt.grid(True)

# 保存并展示
plt.savefig('comparison_plot.png', dpi=300)
plt.show()
