#include<bits/stdc++.h>
#include<sys/time.h>
using namespace std;
const long long N (2<<29);
int a[N + 5];
int b[N + 5];
void normal_algo(int n)
{
    int sum = 0;
    for(int i = 0;i < n;i++)
    sum += a[i];
}

void chain_add_2(int n)
{
    int sum = 0;
    int sum1 = 0;
    int sum2 = 0;
    for(int i = 0;i < n;i += 2)
    sum1 += a[i],sum2 += a[i + 1];
    sum = sum1 + sum2;
}

void chain_add_4(int n)
{
    int sum = 0;
    int sum_mid[4];
    for(int i = 0;i < n;i += 4)
    {
        sum_mid[0] += a[i];
        sum_mid[1] += a[i + 1];
        sum_mid[2] += a[i + 2];
        sum_mid[3] += a[i + 3];
    }
    sum = sum_mid[0] + sum_mid[1] + sum_mid[2] + sum_mid[3];
}
void chain_add_8(int n)
{
    int sum = 0;
    int sum_mid[8];
    for(int i = 0;i < n;i += 8)
    {
        sum_mid[0] += a[i];
        sum_mid[1] += a[i + 1];
        sum_mid[2] += a[i + 2];
        sum_mid[3] += a[i + 3];
        sum_mid[4] += a[i + 4];
        sum_mid[5] += a[i + 5];
        sum_mid[6] += a[i + 6];
        sum_mid[7] += a[i + 7];
    }
    sum += sum_mid[0] + sum_mid[1] + sum_mid[2] + sum_mid[3] + sum_mid[4] + sum_mid[5] + sum_mid[6] + sum_mid[7];
} 
void recursion(int n)
{
    if(n == 1)
    return;
    for(int i = 0;i < n/2;i++)
    {
        a[i] += a[n - i - 1];
    }
    n /= 2;
    recursion(n);
}
int main()
{
    ofstream normal_algo_file("sum_normal_algo.txt");

    struct timeval start,end;
    for(int i = 0;i < N;i++)a[i] = 2 * i + 1;
    for(int n = 8;n < N;n = n * 2)
    {
        gettimeofday(&start,nullptr);
        for(int cnt = 0;cnt < 100;cnt++)
        {
            normal_algo(n);
        }
        gettimeofday(&end,nullptr);

        long seconds = end.tv_sec - start.tv_sec;
        long micro_seconds = end.tv_usec - start.tv_usec;
        double elapsed = seconds * 1e6 + micro_seconds;
        double avg_time = elapsed * 0.01;
        normal_algo_file<<n<<" "<<avg_time<<endl;
    }
    return 0;
}