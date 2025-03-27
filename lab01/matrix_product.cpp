#include<bits/stdc++.h>
#include<sys/time.h>
using namespace std;
double a[5005][5005];
double b[5005];
double sum[5005];
void normal_algo(int n)
{
    for(int i = 0;i < n;i++)
    {
        sum[i] = 0;
        for(int j = 0;j < n;j++)
        {
            sum[i] += a[j][i] * b[j];
        }
    }
    return;
}
void optimizer_algo(int n)
{
    for(int i = 0;i < n;i++) sum[i] = 0;
    for(int i = 0;i < n;i++)
    {
       for(int j = 0;j < n;j++)
       {
        sum[j] += a[i][j] * b[i];
       }
    }
    return;
}
int main()
{
    ofstream normal_algo_file("normal_algo.txt");
    ofstream optimizer_algo_file("optimizer_algo.txt");
    struct timeval start,end;
    for(int n = 10;n <= 5000;n<300 ? n+=10 : n+=100)
    {
        for(int i = 0;i < n;i++)
        {
            for(int j = 0;j < n;j++)
            a[i][j] = i + j;
        }
        for(int i = 0;i < n;i++)b[i] = i;
        if(n>300)
        {
            gettimeofday(&start, nullptr);
            normal_algo(n);
            gettimeofday(&end, nullptr);
            
    
            long seconds = end.tv_sec - start.tv_sec;
            long micro_seconds = end.tv_usec - start.tv_usec;
            double elapsed = seconds*1e6 + micro_seconds;
            normal_algo_file<<n<<","<<elapsed<<endl;
            //printf("normal_algo n = %d,cost = %.2lf us\n",n,elapsed);
    
            gettimeofday(&start, nullptr);
            optimizer_algo(n);
            gettimeofday(&end, nullptr);
    
            seconds = end.tv_sec - start.tv_sec;
            micro_seconds = end.tv_usec - start.tv_usec;
            elapsed = seconds*1e6+ micro_seconds;
            optimizer_algo_file<<n<<","<<elapsed<<endl;
            //printf("optimizer_algo n = %d,cost = %.2lf us\n",n,elapsed);
        }
        else
        {
            double avg_time_1 = 0;
            double avg_time_2 = 0;
            gettimeofday(&start, nullptr);
            for(int cnt = 0;cnt < 100;cnt++)
            {
                normal_algo(n);
            }
            gettimeofday(&end, nullptr);
            long seconds = end.tv_sec - start.tv_sec;
            long micro_seconds = end.tv_usec - start.tv_usec;
            double elapsed = seconds*1e6 + micro_seconds;
            avg_time_1 = elapsed * 0.01;
            gettimeofday(&start, nullptr);
            for(int cnt = 0;cnt < 100;cnt++)
            {
                optimizer_algo(n);
            }
            gettimeofday(&end, nullptr);
            seconds = end.tv_sec - start.tv_sec;
            micro_seconds = end.tv_usec - start.tv_usec;
            elapsed = seconds*1e6 + micro_seconds;
            avg_time_2 = (elapsed * 0.01);
            normal_algo_file<<n<<","<<avg_time_1<<endl;
            optimizer_algo_file<<n<<","<<avg_time_2<<endl;
        }
            //printf("normal_algo n = %d,cost = %.2lf us\n",n,avg_time_1);
            //printf("optimizer_algo n = %d,cost = %.2lf us\n",n,avg_time_2);
    }
    return 0;
}