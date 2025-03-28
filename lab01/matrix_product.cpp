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

void optimizer_algo_with_unroll_2(int n)
{
    for(int i = 0;i < n;i++) sum[i] = 0;
    for(int i = 0;i < n;i++)
    {
        int j;
       for(j = 0;j <= n - 2;j += 2)
       {
        sum[j] += a[i][j] * b[i];
        sum[j + 1] += a[i][j + 1] * b[i];
       }
       for(;j < n;j++)
       {
        sum[j] += a[i][j] * b[i];
       }
    }
    return;
}

void optimizer_algo_with_unroll_4(int n)
{
    for(int i = 0;i < n;i++) sum[i] = 0;
    for(int i = 0;i < n;i++)
    {
        int j;
       for(j = 0;j <= n - 4;j += 4)
       {
        sum[j] += a[i][j] * b[i];
        sum[j + 1] += a[i][j + 1] * b[i];
        sum[j + 2] += a[i][j + 2] * b[i];
        sum[j + 3] += a[i][j + 3] * b[i];
       }
       for(;j < n;j++)
       {
        sum[j] += a[i][j] * b[i];
       }
    }
    return;
}

void optimizer_algo_with_unroll_8(int n)
{
    for(int i = 0;i < n;i++) sum[i] = 0;
    for(int i = 0;i < n;i++)
    {
        int j;
       for(j = 0;j <= n - 8;j += 8)
       {
        sum[j] += a[i][j] * b[i];
        sum[j + 1] += a[i][j + 1] * b[i];
        sum[j + 2] += a[i][j + 2] * b[i];
        sum[j + 3] += a[i][j + 3] * b[i];
        sum[j + 4] += a[i][j + 4] * b[i];
        sum[j + 5] += a[i][j + 5] * b[i];
        sum[j + 6] += a[i][j + 6] * b[i];
        sum[j + 7] += a[i][j + 7] * b[i];
       }
       for(;j < n;j++)
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
    ofstream optimizer_algo_with_unroll_2_file("optimizer_algo_with_unroll_2.txt");
    ofstream optimizer_algo_with_unroll_4_file("optimizer_algo_with_unroll_4.txt");
    ofstream optimizer_algo_with_unroll_8_file("optimizer_algo_with_unroll_8.txt");
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

            gettimeofday(&start, nullptr);
            optimizer_algo_with_unroll_2(n);
            gettimeofday(&end, nullptr);
            
    
            seconds = end.tv_sec - start.tv_sec;
            micro_seconds = end.tv_usec - start.tv_usec;
            elapsed = seconds*1e6 + micro_seconds;
            optimizer_algo_with_unroll_2_file<<n<<","<<elapsed<<endl;

            gettimeofday(&start, nullptr);
            optimizer_algo_with_unroll_4(n);
            gettimeofday(&end, nullptr);
            
    
            seconds = end.tv_sec - start.tv_sec;
            micro_seconds = end.tv_usec - start.tv_usec;
            elapsed = seconds*1e6 + micro_seconds;
            optimizer_algo_with_unroll_4_file<<n<<","<<elapsed<<endl;

            gettimeofday(&start, nullptr);
            optimizer_algo_with_unroll_8(n);
            gettimeofday(&end, nullptr);
            
    
            seconds = end.tv_sec - start.tv_sec;
            micro_seconds = end.tv_usec - start.tv_usec;
            elapsed = seconds*1e6 + micro_seconds;
            optimizer_algo_with_unroll_8_file<<n<<","<<elapsed<<endl;
        }
        else
        {
            double avg_time_1 = 0;
            double avg_time_2 = 0;
            double avg_time_3 = 0;
            double avg_time_4 = 0;
            double avg_time_5 = 0;
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

            gettimeofday(&start, nullptr);
            for(int cnt = 0;cnt < 100;cnt++)
            {
                optimizer_algo_with_unroll_2(n);
            }
            gettimeofday(&end, nullptr);
            seconds = end.tv_sec - start.tv_sec;
            micro_seconds = end.tv_usec - start.tv_usec;
            elapsed = seconds*1e6 + micro_seconds;
            avg_time_3 = elapsed * 0.01;
            optimizer_algo_with_unroll_2_file<<n<<","<<avg_time_3<<endl;

            gettimeofday(&start, nullptr);
            for(int cnt = 0;cnt < 100;cnt++)
            {
                optimizer_algo_with_unroll_4(n);
            }
            gettimeofday(&end, nullptr);
            seconds = end.tv_sec - start.tv_sec;
            micro_seconds = end.tv_usec - start.tv_usec;
            elapsed = seconds*1e6 + micro_seconds;
            avg_time_4 = elapsed * 0.01;
            optimizer_algo_with_unroll_4_file<<n<<","<<avg_time_4<<endl;

            gettimeofday(&start, nullptr);
            for(int cnt = 0;cnt < 100;cnt++)
            {
                optimizer_algo_with_unroll_8(n);
            }
            gettimeofday(&end, nullptr);
            seconds = end.tv_sec - start.tv_sec;
            micro_seconds = end.tv_usec - start.tv_usec;
            elapsed = seconds*1e6 + micro_seconds;
            avg_time_5 = elapsed * 0.01;
            optimizer_algo_with_unroll_8_file<<n<<","<<avg_time_5<<endl;
        }
            //printf("normal_algo n = %d,cost = %.2lf us\n",n,avg_time_1);
            //printf("optimizer_algo n = %d,cost = %.2lf us\n",n,avg_time_2);
    }
    return 0;
}