#include<bits/stdc++.h>
#include<pthread.h>
using namespace std;
#define THREAD_NUM 8
const int MAX = 10;
char specials[MAX];
typedef struct {
    const char* base;
    int start_index;
    int end_index;
} ThreadTask;

void* fill_range(void* arg)
{
    ThreadTask* task = (ThreadTask*)arg;
    for(int i = task->start_index;i < task->end_index;i++)
    {
        char result[256];
        snprintf(result,sizeof(result),"%s%c",task->base,specials[i]);
        printf("Generated: %s\n",result);
    }
    free(task);
    return NULL;
}
void parallel_fill_specials(const char* base)
{
    pthread_t threads[THREAD_NUM];
    int chunk = MAX / THREAD_NUM;
    for(int i = 0;i < THREAD_NUM;i++)
    {
        ThreadTask* task = (ThreadTask*)malloc(sizeof(ThreadTask));
        task->base = base;
        task->start_index = i * chunk;
        task->end_index = (i == THREAD_NUM - 1) ? MAX : (i + 1) * chunk;
        pthread_create(&threads[i],NULL,fill_range,task);
    }
    for(int i = 0;i < THREAD_NUM;i++)
    {
        pthread_join(threads[i],NULL);
    }
}
int main()
{
    for(int i = 0;i < MAX;i++)
    specials[i] = (char)(48 + (i % 10));
    parallel_fill_specials("password1");
    return 0;
}