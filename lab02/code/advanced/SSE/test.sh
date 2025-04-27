#!/bin/bash

# 定义参数
input_file="/home/lorn/NKU-parallel-computing/prob-hashcat/pcfg.outfile"
output_file="time.csv"
start_size=100000
step_size=100000
max_size=100000000

# 写入CSV头
echo "size,time(ms)" > "$output_file"

# 循环测试不同规模
size=$start_size
while [ "$size" -le "$max_size" ]; do
    echo "正在测试规模: $size"
    
    # 运行程序并捕获输出
    output=$(./benchmark "$input_file" "$size" 2>&1)
    
    # 从输出中提取时间（假设时间格式为"耗时: xxx 微秒"）
    time=$(echo "$output" | grep "耗时:" | awk '{print $(NF-2)}')
    
    # 写入CSV文件
    echo "$size,$time" >> "$output_file"
    
    echo "测试完成: size=$size, time=$time s"
    
    # 增加步长
    size=$((size + step_size))
done

echo "所有测试完成，结果已保存到 $output_file"