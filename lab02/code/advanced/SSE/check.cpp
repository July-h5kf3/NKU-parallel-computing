#include "md5.h"
#include<iomanip>
#include<assert.h>
#include<chrono>
#include<stdio.h>
#include<stdlib.h>
#include<immintrin.h>
#include<math.h>
int main()
{
    string pwd[4] = {"abc","def","hij","knb"};
    __m128i state_simd[4];
    //auto start_hash = system_clock::now();
    MD5Hash_SIMD(pwd,state_simd);
    //auto end_hash = system_clock::now();
    //cout<<end_hash - start_hash<<endl;
    bit32 state_show[4][4];
    for (int i = 0; i < 4; i++) {
    uint32_t tmp0[4], tmp1[4], tmp2[4], tmp3[4];
    _mm_storeu_si128((__m128i*)tmp0, state_simd[0]);
    _mm_storeu_si128((__m128i*)tmp1, state_simd[1]);
    _mm_storeu_si128((__m128i*)tmp2, state_simd[2]);
    _mm_storeu_si128((__m128i*)tmp3, state_simd[3]);

    state_show[i][0] = tmp0[i];
    state_show[i][1] = tmp1[i];
    state_show[i][2] = tmp2[i];
    state_show[i][3] = tmp3[i];
}

    for(int i = 0;i < 4;i++)
    {
        bit32 state[4];
        MD5Hash(pwd[i],state);
        cout<<"correct:";
        for(int j = 0;j < 4;j++)
        {
            cout << std::setw(8) << std::setfill('0') << hex << state[j];    
        }
        puts("");
        cout<<"me:";
        for(int j = 0;j < 4;j++)
        {
            cout << std::setw(8) << std::setfill('0') << hex << state_show[i][j];
        }
        puts("");
    }
    return 0;
}