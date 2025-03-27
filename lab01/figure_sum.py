import matplotlib.pyplot as plt
import numpy as np

# 文件列表和对应的标签
files = [
    ('sum_chain_2_algo.txt', 'Chain 2'),
    ('sum_chain_4_algo.txt', 'Chain 4'),
    ('sum_chain_8_algo.txt', 'Chain 8'),
    ('sum_normal_algo.txt', 'Normal'),
    ('sum_recursion_algo.txt', 'Recursion')
]

plt.figure(figsize=(12, 7))  # 主图大小

# 绘制主图数据
for filename, label in files:
    try:
        data = np.loadtxt(filename)
        n = data[:, 0]
        time = data[:, 1]
        log_n = np.log2(n)
        plt.plot(log_n, time, marker='o', label=label)
    except Exception as e:
        print(f"Error processing file {filename}: {e}")

# 主图设置
plt.xlabel('log2(n)')
plt.ylabel('Time (us)')
plt.title('Algorithm Performance Comparison')
plt.legend(loc='upper left')  # 主图图例放在左上角
plt.grid(True, which="both", ls="--")

# 手动添加子图（位置和大小完全自定义）
# 参数说明：[left, bottom, width, height]，范围在0~1之间
ax_inset = plt.axes([0.55, 0.55, 0.35, 0.35])  # 调整这4个值！

# 绘制子图数据
for filename, label in files:
    try:
        data = np.loadtxt(filename)
        n = data[:, 0]
        time = data[:, 1]
        log_n = np.log2(n)
        mask = (log_n >= 25) & (log_n <= 30) & (time > 0) & (time < 200000)
        ax_inset.plot(log_n[mask], time[mask], marker='o', label=label)
    except Exception as e:
        print(f"Error processing file {filename}: {e}")

# 子图设置
ax_inset.set_xlim(25, 30)
ax_inset.set_ylim(0, 200000)
ax_inset.grid(True, which="both", ls="--")
ax_inset.set_title('Zoom: 25 ≤ log2(n) ≤ 30')

plt.tight_layout()
plt.show()