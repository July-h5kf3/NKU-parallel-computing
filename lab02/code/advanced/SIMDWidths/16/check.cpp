#include "md5.h"
#include <iomanip>
#include <assert.h>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <math.h>

int main()
{
    // 假设密码数组有16个密码
    string pwd[16] = {"abc", "def", "hij", "knb", "123", "456", "qwe", "adasf", 
                      "xyzz", "abcd", "efgh", "ijkl", "mnop", "qrst", "uvwx", "yz12"};

    // 16路并行处理，使用AVX2 SIMD指令集
    __m512i state_simd[4]; // 这里存储 256 位的状态，每次处理4个元素（256位 = 4*64位）

    // 处理16个密码，假设你有一个合适的 SIMD MD5 处理函数
    MD5Hash_SIMD(pwd, state_simd);

    // 用于显示的状态
    bit32 state_show[16][4];
    
    // 从 SIMD 状态中提取出哈希结果
    for (int i = 0; i < 16; i++) {
        uint32_t tmp0[16], tmp1[16], tmp2[16], tmp3[16];
        _mm512_storeu_si512((__m512i*)tmp0, state_simd[0]);
        _mm512_storeu_si512((__m512i*)tmp1, state_simd[1]);
        _mm512_storeu_si512((__m512i*)tmp2, state_simd[2]);
        _mm512_storeu_si512((__m512i*)tmp3, state_simd[3]);

        // 将SIMD的哈希结果存储到数组中
        state_show[i][0] = tmp0[i];
        state_show[i][1] = tmp1[i];
        state_show[i][2] = tmp2[i];
        state_show[i][3] = tmp3[i];
    }

    // 对比和输出结果
    for (int i = 0; i < 16; i++) {
        bit32 state[4];  // 存储常规 MD5 的哈希结果
        MD5Hash(pwd[i], state);  // 计算常规的 MD5 哈希

        // 输出常规的 MD5 哈希结果
        cout << "correct: ";
        for (int j = 0; j < 4; j++) {
            cout << std::setw(8) << std::setfill('0') << std::hex << state[j];
        }
        puts("");

        // 输出使用 SIMD 计算的哈希结果
        cout << "me: ";
        for (int j = 0; j < 4; j++) {
            cout << std::setw(8) << std::setfill('0') << std::hex << state_show[i][j];
        }
        puts("");
    }

    return 0;
}
