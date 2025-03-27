import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

# 读取数据
def read_data(filename):
    n_values = []
    time_values = []
    with open(filename, 'r') as file:
        for line in file:
            n, time = line.strip().split(',')
            n_values.append(int(n))
            time_values.append(float(time))
    return n_values, time_values

# 读取 normal_algo 和 optimizer_algo 的数据
n_normal, time_normal = read_data('normal_algo.txt')
n_optimizer, time_optimizer = read_data('optimizer_algo.txt')

# 创建主图
plt.figure(figsize=(12, 8))

# 绘制主图
plt.plot(n_normal, time_normal, label='Normal Algorithm', marker='o', linestyle='-', color='blue')
plt.plot(n_optimizer, time_optimizer, label='Optimized Algorithm', marker='s', linestyle='--', color='red')

# 添加矩形框标出 200-400 范围
rect = Rectangle((0, -2000),  # 左下角坐标
                500,  # 宽度
                8000,  # 高度
                 linewidth=2, edgecolor='green', facecolor='none',linestyle='--')  # 边框颜色和样式
plt.gca().add_patch(rect)  # 将矩形添加到主图

# 添加标题和标签
plt.title('Algorithm Performance Comparison')
plt.xlabel('Matrix Size (n)')
plt.ylabel('Execution Time (us)')
plt.legend()  # 显示图例
plt.grid(True)  # 添加网格

# 创建放大的子图
ax_inset = plt.axes([0.28, 0.45, 0.31, 0.35])  # 子图位置和大小

# 筛选 n=200-400 范围内的数据
zoom_n_normal = [n for n in n_normal if 0 <= n <= 300]
zoom_time_normal = [time for n, time in zip(n_normal, time_normal) if 0 <= n <= 300]
zoom_n_optimizer = [n for n in n_optimizer if 0 <= n <= 300]
zoom_time_optimizer = [time for n, time in zip(n_optimizer, time_optimizer) if 0 <= n <= 300]

# 绘制子图
ax_inset.plot(zoom_n_normal, zoom_time_normal, label='Normal Algorithm', marker='o', linestyle='-', color='blue')
ax_inset.plot(zoom_n_optimizer, zoom_time_optimizer, label='Optimized Algorithm', marker='s', linestyle='--', color='red')

# 设置子图范围
ax_inset.set_xlim(0, 300)  # 设置子图横轴范围
ax_inset.set_ylim(min(min(zoom_time_normal), min(zoom_time_optimizer)),  # 设置子图纵轴范围
                  max(max(zoom_time_normal), max(zoom_time_optimizer)))
ax_inset.set_title('Zoomed View (n=0-300)')
ax_inset.set_xlabel('n')
ax_inset.set_ylabel('Time (us)')
ax_inset.grid(True)  # 添加网格

# 显示图表
plt.show()