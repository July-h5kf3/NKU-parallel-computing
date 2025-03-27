#include <iostream>
#include <sys/time.h>
#include <vector>
using namespace std;

const int MAX_N = 5000;
vector<vector<double>> a(MAX_N, vector<double>(MAX_N));
vector<double> b(MAX_N);
vector<double> sum(MAX_N);

void init_data(int n) {
    for(int i = 0; i < n; i++) {
        b[i] = 2 * i + 1;
        for(int j = 0; j < n; j++) {
            a[i][j] = 2*i + j;
        }
    }
}

void normal_algo(int n) {
    for(int i = 0; i < n; i++) {
        sum[i] = 0;
        for(int j = 0; j < n; j++) {
            sum[i] += a[j][i] * b[j];
        }
    }
}

void optimizer_algo(int n) {
    fill(sum.begin(), sum.end(), 0);
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            sum[j] += a[i][j] * b[i];
        }
    }
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        cerr << "Usage: " << argv[0] << " <n>" << endl;
        return 1;
    }
    
    int n = atoi(argv[1]);
    init_data(n);
    
    // Warm up
    optimizer_algo(n);
    
    // Start profiling
    for(int i = 0; i < 1000; i++) {
        optimizer_algo(n);
    }
    
    return 0;
}