#!/bin/bash

# 事件列表
EVENTS="L1-dcache-loads,L1-dcache-load-misses,cycles,instructions"

# 函数列表
FUNCTIONS=("chain_add_512")

# 遍历每个函数
for func in "${FUNCTIONS[@]}"; do
    echo "Profiling function: $func"
    sudo perf stat -e $EVENTS \
        -o "perf_${func}.txt" \
        -- ./sum "$func"  # 传递函数名作为参数
done

# 提取结果并计算 miss-ratio 和 CPI
for func in "${FUNCTIONS[@]}"; do
    echo "Results for $func:"
    
    # 移除数字中的逗号以便计算
    loads=$(grep "L1-dcache-loads" "perf_${func}.txt" | awk '{print $1}' | tr -d ',')
    misses=$(grep "L1-dcache-load-misses" "perf_${func}.txt" | awk '{print $1}' | tr -d ',')
    cycles=$(grep "cycles" "perf_${func}.txt" | awk '{print $1}' | tr -d ',')
    instructions=$(grep "instructions" "perf_${func}.txt" | awk '{print $1}' | tr -d ',')

    # 安全检查并计算
    if [[ $loads -eq 0 ]]; then
        miss_ratio="N/A (zero loads)"
    else
        miss_ratio=$(echo "scale=4; $misses / $loads" | bc)
    fi

    if [[ $instructions -eq 0 ]]; then
        cpi="N/A (zero instructions)"
    else
        cpi=$(echo "scale=4; $cycles / $instructions" | bc)
    fi

    # 打印结果
    echo "L1-dcache-loads: $loads"
    echo "L1-dcache-misses: $misses"
    echo "Miss-ratio: $miss_ratio"
    echo "Cycles: $cycles"
    echo "Instructions: $instructions"
    echo "CPI: $cpi"
    echo "----------------------------------"
done