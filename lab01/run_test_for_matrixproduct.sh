#!/bin/bash

# 测试n值范围
START_N=3000
END_N=4000
STEP=50

echo "n,cache-misses,cache-references,miss-ratio,cycles,instructions,ipc" > results.csv

for ((n=START_N; n<=END_N; n+=STEP)); do
    echo "Testing n=$n"
    perf stat -e cache-misses,cache-references,cycles,instructions \
        -o perf.tmp ./cache_test $n
    
    # 提取性能数据
    cache_misses=$(grep 'cache-misses' perf.tmp | awk '{print $1}' | tr -d ,)
    cache_refs=$(grep 'cache-references' perf.tmp | awk '{print $1}' | tr -d ,)
    cycles=$(grep 'cycles' perf.tmp | awk '{print $1}' | tr -d ,)
    instructions=$(grep 'instructions' perf.tmp | awk '{print $1}' | tr -d ,)
    
    # 计算指标
    if [ $cache_refs -ne 0 ]; then
        miss_ratio=$(echo "scale=4; $cache_misses/$cache_refs" | bc)
    else
        miss_ratio=0
    fi
    
    if [ $cycles -ne 0 ]; then
        ipc=$(echo "scale=4; $instructions/$cycles" | bc)
    else
        ipc=0
    fi
    
    # 写入CSV
    echo "$n,$cache_misses,$cache_refs,$miss_ratio,$cycles,$instructions,$ipc" >> results.csv
done

# 清理
rm perf.tmp