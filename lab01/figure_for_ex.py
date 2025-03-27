import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('results.csv')

plt.figure(figsize=(12, 8))

# 缓存未命中率
plt.subplot(2, 1, 1)
plt.plot(df['n'], df['miss-ratio'], 'b-o')
plt.title('Cache Miss Ratio vs Problem Size (n)')
plt.xlabel('Problem Size (n)')
plt.ylabel('Cache Miss Ratio')
plt.grid(True)

# IPC
plt.subplot(2, 1, 2)
plt.plot(df['n'], df['ipc'], 'r-o')
plt.title('IPC vs Problem Size (n)')
plt.xlabel('Problem Size (n)')
plt.ylabel('Instructions Per Cycle (IPC)')
plt.grid(True)

plt.tight_layout()
plt.savefig('cache_analysis.png')
plt.show()