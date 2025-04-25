#include "PCFG.h"
#include <chrono>
#include <fstream>
#include "md5.h"
#include <iomanip>
#include <immintrin.h>
using namespace std;
using namespace chrono;

// 编译指令如下
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o test.exe
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o test.exe -O1
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o test.exe -O2

int main()
{
    double time_hash = 0;  // 用于MD5哈希的时间
    double time_guess = 0; // 哈希和猜测的总时长
    double time_train = 0; // 模型训练的总时长
    PriorityQueue q;
    auto start_train = system_clock::now();
    q.m.train("/home/lorn/NKU-parallel-computing/data/Rockyou-singleLined-full.txt");
    q.m.order();
    auto end_train = system_clock::now();
    auto duration_train = duration_cast<microseconds>(end_train - start_train);
    time_train = double(duration_train.count()) * microseconds::period::num / microseconds::period::den;

    q.init();
    cout << "here" << endl;
    int curr_num = 0;
    auto start = system_clock::now();
    // 由于需要定期清空内存，我们在这里记录已生成的猜测总数
    int history = 0;
    // std::ofstream a("./output/results.txt");
    while (!q.priority.empty())
    {
        q.PopNext();
        q.total_guesses = q.guesses.size();
        if (q.total_guesses - curr_num >= 100000)
        {
            cout << "Guesses generated: " <<history + q.total_guesses << endl;
            curr_num = q.total_guesses;

            // 在此处更改实验生成的猜测上限
            int generate_n=10000000;
            if (history + q.total_guesses > 10000000)
            {
                auto end = system_clock::now();
                auto duration = duration_cast<microseconds>(end - start);
                time_guess = double(duration.count()) * microseconds::period::num / microseconds::period::den;
                cout << "Guess time:" << time_guess - time_hash << "seconds"<< endl;
                cout << "Hash time:" << time_hash << "seconds"<<endl;
                cout << "Train time:" << time_train <<"seconds"<<endl;
                break;
            }
        }
        // 为了避免内存超限，我们在q.guesses中口令达到一定数目时，将其中的所有口令取出并且进行哈希
        // 然后，q.guesses将会被清空。为了有效记录已经生成的口令总数，维护一个history变量来进行记录
        if (curr_num > 1000000)
        {
            __m512i state[4];
            int num = q.guesses.size();
            auto start_hash = system_clock::now();
            for (int i = 0; i < num; i += 16)
            {
                string pw1 = i >= num ? "" : q.guesses[i];
                string pw2 = i+1 >= num ? "" : q.guesses[i + 1];
                string pw3 = i+2 >= num ? "" : q.guesses[i + 2];
                string pw4 = i+3 >= num ? "" : q.guesses[i + 3];
                string pw5 = i+4 >= num ? "" : q.guesses[i + 4];
                string pw6 = i+5 >= num ? "" : q.guesses[i + 5];
                string pw7 = i+6 >= num ? "" : q.guesses[i + 6];
                string pw8 = i+7 >= num ? "" : q.guesses[i + 7];
                string pw9 = i+8 >= num ? "" : q.guesses[i + 8];
                string pw10 = i+9 >= num ? "" : q.guesses[i + 9];
                string pw11 = i+10 >= num ? "" : q.guesses[i + 10];
                string pw12 = i+11 >= num ? "" : q.guesses[i + 11];
                string pw13 = i+12 >= num ? "" : q.guesses[i + 12];
                string pw14 = i+13 >= num ? "" : q.guesses[i + 13];
                string pw15 = i+14 >= num ? "" : q.guesses[i + 14];
                string pw16 = i+15 >= num ? "" : q.guesses[i + 15];
            
                // 将这些密码组成一个16元素的批次
                string pw_batch[16] = {pw1, pw2, pw3, pw4, pw5, pw6, pw7, pw8, pw9, pw10, pw11, pw12, pw13, pw14, pw15, pw16};
            
                // 使用SIMD MD5函数处理这个批次
                MD5Hash_SIMD(pw_batch, state);
                // Hash time: 14.1481 seconds
                // 以下代码用于输出猜测和哈希（可选）
                // a << pw1 << "\t"; // 输出第一个密码
                // for (int i1 = 0; i1 < 4; i1++) {
                //     a << std::setw(8) << std::setfill('0') << std::hex << state[i1];
                // }
                // a << std::endl;
            }            

            // 在这里对哈希所需的总时长进行计算
            auto end_hash = system_clock::now();
            auto duration = duration_cast<microseconds>(end_hash - start_hash);
            time_hash += double(duration.count()) * microseconds::period::num / microseconds::period::den;

            // 记录已经生成的口令总数
            history += curr_num;
            curr_num = 0;
            q.guesses.clear();
        }
    }
}