#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include<iomanip>
#include "md5.h"
using namespace std;
using namespace chrono;

void benchmark(const std::string& filename, size_t test_size) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "错误：无法打开文件 " << filename << std::endl;
        exit(1);
    }

    std::string line;
    size_t processed = 0;
    double hash_time = 0
    ;auto start = high_resolution_clock::now();
    //cout<<1<<endl;
    while (processed < test_size && std::getline(file, line)) {
        processed++;
        size_t tab_pos = line.find(' ');
        if (tab_pos == std::string::npos) continue;
        
        bit32 state[4];
          // 更精确的时钟
        MD5Hash(line.substr(0, tab_pos), state);
       
    }
    auto end = high_resolution_clock::now();
        
    auto duration = duration_cast<microseconds>(end - start);
    hash_time += 1.0 * duration.count() / 1000000;  // 直接累加微秒数
    std::cout << "\n测试完成! 规模: " << processed << " 行 | "
              << "耗时: " <<hash_time << " 秒 | "<<"\n";
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "用法: " << argv[0] << " <密码文件> <测试行数>\n";
        return 1;
    }
    
    benchmark(argv[1], std::stoul(argv[2]));
    return 0;
}