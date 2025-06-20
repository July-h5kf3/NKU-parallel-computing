#include "PCFG.h"
#include <chrono>
#include <fstream>
#include "md5.h"
#include <iomanip>
#include <mpi.h>
#include <arm_neon.h>
using namespace std;
using namespace chrono;

// 编译指令如下
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o test.exe
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o test.exe -O1
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o test.exe -O2

int main(int argc, char** argv)
{
    // ofstream logFile("top1e6.txt");
    MPI_Init(&argc, &argv);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    double time_hash = 0;  // 用于MD5哈希的时间
    double time_guess = 0; // 哈希和猜测的总时长
    double time_train = 0; // 模型训练的总时长
    PriorityQueue q;
    auto start_train = system_clock::now();
    q.m.train("/home/s2312810/homework/data/Rockyou.txt");
    q.m.order();
    auto end_train = system_clock::now();
    if(world_rank == 0)
    {
        auto duration_train = duration_cast<microseconds>(end_train - start_train);
        time_train = double(duration_train.count()) * microseconds::period::num / microseconds::period::den;
        q.init();
        cout << "here" << endl;
    }
    int curr_num = 0;
    auto start = system_clock::now();
    // 由于需要定期清空内存，我们在这里记录已生成的猜测总数
    int history = 0;
    // std::ofstream a("./output/results.txt");
    bool master_is_done = false;
    while (1)
    {
        // master_is_done = false;
        MPI_Bcast(&master_is_done, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
        if (master_is_done) 
        {
            break;
        }
        q.PopNext();
        if(world_rank == 0)
        {
            q.total_guesses = q.guesses.size();
        if (q.total_guesses - curr_num >= 100000)
        {
            cout << "Guesses generated: " <<history + q.total_guesses << endl;
            curr_num = q.total_guesses;

            // 在此处更改实验生成的猜测上限
            int generate_n=10000000;
            if (history + q.total_guesses > generate_n && master_is_done != 1)
            {
                auto end = system_clock::now();
                auto duration = duration_cast<microseconds>(end - start);
                time_guess = double(duration.count()) * microseconds::period::num / microseconds::period::den;
                cout << "Guess time:" << time_guess - time_hash << "seconds"<< endl;
                cout << "Hash time:" << time_hash << "seconds"<<endl;
                cout << "Train time:" << time_train <<"seconds"<<endl;
                master_is_done = 1;
            }
        }
        // 为了避免内存超限，我们在q.guesses中口令达到一定数目时，将其中的所有口令取出并且进行哈希
        // 然后，q.guesses将会被清空。为了有效记录已经生成的口令总数，维护一个history变量来进行记录
        if (curr_num > 1000000)
        {
            //uint32x4_t state[4];
            //int num = q.guesses.size();
            auto start_hash = system_clock::now();
            bit32 state[4];
            
            for(int i = 0;i < q.guesses.size();i++)
            {
                // logFile<<q.guesses[i]<<endl;

                MD5Hash(q.guesses[i],state);
            }
            /*
            for (int i = 0;i < num;i += 4)
            {
                string pw1 = q.guesses[i];
                string pw2 = i+1 >= num ? "" : q.guesses[i + 1];
                string pw3 = i+2 >= num ? "" : q.guesses[i + 2];
                string pw4 = i+3 >= num ? "" : q.guesses[i + 3];
                string pw_batch[4] = {pw1,pw2,pw3,pw4};
                // TODO：对于SIMD实验，将这里替换成你的SIMD MD5函数
                //MD5Hash(pw1, state);
                MD5Hash_SIMD(pw_batch,state);
                //Hash time:14.1481seconds
                // 以下注释部分用于输出猜测和哈希，但是由于自动测试系统不太能写文件，所以这里你可以改成cout
                // a<<pw<<"\t";
                // for (int i1 = 0; i1 < 4; i1 += 1)
                // {
                //     a << std::setw(8) << std::setfill('0') << hex << state[i1];
                // }
                // a << endl;
            }*/

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
    // printf("Rank %d before MPI_Finalize\n", world_rank);
    MPI_Finalize();
    return 0;
}