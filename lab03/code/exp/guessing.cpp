#include "PCFG.h"
#include<pthread.h>
#include<omp.h>
using namespace std;
struct last_segment lseg[1005];                // 真正定义并分配空间
std::map<segment*, int> find_last_segment;     // 真正定义 map
int tot = 0; 
typedef struct
{
    string pre_terminal;
    segment* a;
    PriorityQueue* self;
    int start_index;
    int end_index;
}ThreadTask;


void* PriorityQueue::fill_range(void* arg)
{
    ThreadTask* task = (ThreadTask*)arg;
    // if (!task || !task->a || !task->self) {
    //     std::cerr << "Null pointer in task!" << std::endl;
    //     return NULL;
    // }
    segment* a = task->a;
    PriorityQueue* self = task->self;

    vector<string> local_guesses;
    local_guesses.reserve(task->end_index - task->start_index);
    int local_count = 0;
    // std::cout << "Thread running: start=" << task->start_index
    //           << ", end=" << task->end_index
    //           << ", total=" << task->a->ordered_values.size() << std::endl;
    for(int i = task->start_index;i < task->end_index;i++)
    {
        string& suffix = a->ordered_values[i];
        string guess;
        guess.reserve(task->pre_terminal.size() + suffix.size());
        guess += task->pre_terminal + suffix;
        local_guesses.emplace_back(guess);
        local_count++;
    }
    pthread_mutex_lock(&self->mutex);
    self->guesses.insert(self->guesses.end(),local_guesses.begin(),local_guesses.end());
    self->total_guesses += local_count;
    pthread_mutex_unlock(&self->mutex);
    free(task);
    return NULL;
}

void PriorityQueue::CalProb(PT &pt)
{
    // 计算PriorityQueue里面一个PT的流程如下：
    // 1. 首先需要计算一个PT本身的概率。例如，L6S1的概率为0.15
    // 2. 需要注意的是，Queue里面的PT不是“纯粹的”PT，而是除了最后一个segment以外，全部被value实例化的PT
    // 3. 所以，对于L6S1而言，其在Queue里面的实际PT可能是123456S1，其中“123456”为L6的一个具体value。
    // 4. 这个时候就需要计算123456在L6中出现的概率了。假设123456在所有L6 segment中的概率为0.1，那么123456S1的概率就是0.1*0.15

    // 计算一个PT本身的概率。后续所有具体segment value的概率，直接累乘在这个初始概率值上
    pt.prob = pt.preterm_prob;

    // index: 标注当前segment在PT中的位置
    int index = 0;


    for (int idx : pt.curr_indices)
    {
        // pt.content[index].PrintSeg();
        if (pt.content[index].type == 1)
        {
            // 下面这行代码的意义：
            // pt.content[index]：目前需要计算概率的segment
            // m.FindLetter(seg): 找到一个letter segment在模型中的对应下标
            // m.letters[m.FindLetter(seg)]：一个letter segment在模型中对应的所有统计数据
            // m.letters[m.FindLetter(seg)].ordered_values：一个letter segment在模型中，所有value的总数目
            pt.prob *= m.letters[m.FindLetter(pt.content[index])].ordered_freqs[idx];
            pt.prob /= m.letters[m.FindLetter(pt.content[index])].total_freq;
            // cout << m.letters[m.FindLetter(pt.content[index])].ordered_freqs[idx] << endl;
            // cout << m.letters[m.FindLetter(pt.content[index])].total_freq << endl;
        }
        if (pt.content[index].type == 2)
        {
            pt.prob *= m.digits[m.FindDigit(pt.content[index])].ordered_freqs[idx];
            pt.prob /= m.digits[m.FindDigit(pt.content[index])].total_freq;
            // cout << m.digits[m.FindDigit(pt.content[index])].ordered_freqs[idx] << endl;
            // cout << m.digits[m.FindDigit(pt.content[index])].total_freq << endl;
        }
        if (pt.content[index].type == 3)
        {
            pt.prob *= m.symbols[m.FindSymbol(pt.content[index])].ordered_freqs[idx];
            pt.prob /= m.symbols[m.FindSymbol(pt.content[index])].total_freq;
            // cout << m.symbols[m.FindSymbol(pt.content[index])].ordered_freqs[idx] << endl;
            // cout << m.symbols[m.FindSymbol(pt.content[index])].total_freq << endl;
        }
        index += 1;
    }
    // cout << pt.prob << endl;
}

void PriorityQueue::init()
{
    // cout << m.ordered_pts.size() << endl;
    // 用所有可能的PT，按概率降序填满整个优先队列
    for (PT pt : m.ordered_pts)
    {
        for (segment seg : pt.content)
        {
            if (seg.type == 1)
            {
                // 下面这行代码的意义：
                // max_indices用来表示PT中各个segment的可能数目。例如，L6S1中，假设模型统计到了100个L6，那么L6对应的最大下标就是99
                // （但由于后面采用了"<"的比较关系，所以其实max_indices[0]=100）
                // m.FindLetter(seg): 找到一个letter segment在模型中的对应下标
                // m.letters[m.FindLetter(seg)]：一个letter segment在模型中对应的所有统计数据
                // m.letters[m.FindLetter(seg)].ordered_values：一个letter segment在模型中，所有value的总数目
                pt.max_indices.emplace_back(m.letters[m.FindLetter(seg)].ordered_values.size());
            }
            if (seg.type == 2)
            {
                pt.max_indices.emplace_back(m.digits[m.FindDigit(seg)].ordered_values.size());
            }
            if (seg.type == 3)
            {
                pt.max_indices.emplace_back(m.symbols[m.FindSymbol(seg)].ordered_values.size());
            }
        }
        pt.preterm_prob = float(m.preterm_freq[m.FindPT(pt)]) / m.total_preterm;
        // pt.PrintPT();
        // cout << " " << m.preterm_freq[m.FindPT(pt)] << " " << m.total_preterm << " " << pt.preterm_prob << endl;

        // 计算当前pt的概率
        CalProb(pt);
        // 将PT放入优先队列
        priority.emplace_back(pt);
    }
    // cout << "priority size:" << priority.size() << endl;
}

void PriorityQueue::PopNext()
{

    // 对优先队列最前面的PT，首先利用这个PT生成一系列猜测
    Generate(priority.front());

    // 然后需要根据即将出队的PT，生成一系列新的PT
    vector<PT> new_pts = priority.front().NewPTs();
    for (PT pt : new_pts)
    {
        // 计算概率
        CalProb(pt);
        // 接下来的这个循环，作用是根据概率，将新的PT插入到优先队列中
        for (auto iter = priority.begin(); iter != priority.end(); iter++)
        {
            // 对于非队首和队尾的特殊情况
            if (iter != priority.end() - 1 && iter != priority.begin())
            {
                // 判定概率
                if (pt.prob <= iter->prob && pt.prob > (iter + 1)->prob)
                {
                    priority.emplace(iter + 1, pt);
                    break;
                }
            }
            if (iter == priority.end() - 1)
            {
                priority.emplace_back(pt);
                break;
            }
            if (iter == priority.begin() && iter->prob < pt.prob)
            {
                priority.emplace(iter, pt);
                break;
            }
        }
    }

    // 现在队首的PT善后工作已经结束，将其出队（删除）
    priority.erase(priority.begin());
}

// 这个函数你就算看不懂，对并行算法的实现影响也不大
// 当然如果你想做一个基于多优先队列的并行算法，可能得稍微看一看了
vector<PT> PT::NewPTs()
{
    // 存储生成的新PT
    vector<PT> res;

    // 假如这个PT只有一个segment
    // 那么这个segment的所有value在出队前就已经被遍历完毕，并作为猜测输出
    // 因此，所有这个PT可能对应的口令猜测已经遍历完成，无需生成新的PT
    if (content.size() == 1)
    {
        return res;
    }
    else
    {
        // 最初的pivot值。我们将更改位置下标大于等于这个pivot值的segment的值（最后一个segment除外），并且一次只更改一个segment
        // 上面这句话里是不是有没看懂的地方？接着往下看你应该会更明白
        int init_pivot = pivot;

        // 开始遍历所有位置值大于等于init_pivot值的segment
        // 注意i < curr_indices.size() - 1，也就是除去了最后一个segment（这个segment的赋值预留给并行环节）
        for (int i = pivot; i < curr_indices.size() - 1; i += 1)
        {
            // curr_indices: 标记各segment目前的value在模型里对应的下标
            curr_indices[i] += 1;

            // max_indices：标记各segment在模型中一共有多少个value
            if (curr_indices[i] < max_indices[i])
            {
                // 更新pivot值
                pivot = i;
                res.emplace_back(*this);
            }

            // 这个步骤对于你理解pivot的作用、新PT生成的过程而言，至关重要
            curr_indices[i] -= 1;
        }
        pivot = init_pivot;
        return res;
    }

    return res;
}


// 这个函数是PCFG并行化算法的主要载体
// 尽量看懂，然后进行并行实现
void PriorityQueue::Generate(PT pt)
{
    // 计算PT的概率，这里主要是给PT的概率进行初始化
    CalProb(pt);

    // 对于只有一个segment的PT，直接遍历生成其中的所有value即可
    if (pt.content.size() == 1)
    {
        // 指向最后一个segment的指针，这个指针实际指向模型中的统计数据
        segment *a;
        // 在模型中定位到这个segment
        if (pt.content[0].type == 1)
        {
            a = &m.letters[m.FindLetter(pt.content[0])];
        }
        if (pt.content[0].type == 2)
        {
            a = &m.digits[m.FindDigit(pt.content[0])];
        }
        if (pt.content[0].type == 3)
        {
            a = &m.symbols[m.FindSymbol(pt.content[0])];
        }
       if(find_last_segment.find(a) == find_last_segment.end())
       {
        lseg[++tot].a = a;
        lseg[tot].num = 1;
        find_last_segment.insert({a,tot});
       }
       else
       {
            lseg[find_last_segment[a]].num += 1;
       }
        // Multi-thread TODO：
        // 这个for循环就是你需要进行并行化的主要部分了，特别是在多线程&GPU编程任务中
        // 可以看到，这个循环本质上就是把模型中一个segment的所有value，赋值到PT中，形成一系列新的猜测
        // 这个过程是可以高度并行化的
        // omp_set_num_threads(THREAD_NUM);
        // #pragma omp parallel
        // {
        //     vector<string> local_guesses;
        //     #pragma omp for 
        //     for (int i = 0; i < pt.max_indices[0]; i += 1)
        //     {
        //         string guess = a->ordered_values[i];
        //         // cout << guess << endl;
        //         //guesses.emplace_back(guess);
        //         local_guesses.emplace_back(guess);
        //         //total_guesses += 1;
        //     }
        //     #pragma omp critical
        //     {
        //         guesses.insert(guesses.end(),local_guesses.begin(),local_guesses.end());
        //         total_guesses += local_guesses.size();
        //     }
        // }

        for (int i = 0; i < pt.max_indices[0]; i += 1)
        {
            string guess = a->ordered_values[i];
            // cout << guess << endl;
            guesses.emplace_back(guess);
            total_guesses += 1;
        }
        // pthread_t threads[THREAD_NUM];
        // int all = pt.max_indices[0];
        // int chunk = all / THREAD_NUM;
        // //cout<<"chunk = "<<chunk<<endl;
        // for(int i = 0;i < THREAD_NUM;i++)
        // {
        //     ThreadTask* task = new ThreadTask();
        //     task->pre_terminal = "";
        //     //cout<<"xxx"<<endl;
        //     task->a = a;
        //     task->self = this;
        //     task->start_index = i * chunk;
        //     task->end_index = (i == THREAD_NUM - 1) ? all : (i + 1) * chunk;
        //     pthread_create(&threads[i], NULL, PriorityQueue::fill_range, task);
        //     // if (ret != 0) 
        //     // {
        //     //     std::cerr << "Failed to create thread " << i << ", error: " << ret << std::endl;
        //     // }
        // }
        // for(int i = 0;i < THREAD_NUM;i++)
        // {
        //     pthread_join(threads[i],NULL);
        // }
    }
    else
    {
        string guess;
        int seg_idx = 0;
        // 这个for循环的作用：给当前PT的所有segment赋予实际的值（最后一个segment除外）
        // segment值根据curr_indices中对应的值加以确定
        // 这个for循环你看不懂也没太大问题，并行算法不涉及这里的加速
        for (int idx : pt.curr_indices)
        {
            if (pt.content[seg_idx].type == 1)
            {
                guess += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
            }
            if (pt.content[seg_idx].type == 2)
            {
                guess += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
            }
            if (pt.content[seg_idx].type == 3)
            {
                guess += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];
            }
            seg_idx += 1;
            if (seg_idx == pt.content.size() - 1)
            {
                break;
            }
        }

        // 指向最后一个segment的指针，这个指针实际指向模型中的统计数据
        segment *a;
        if (pt.content[pt.content.size() - 1].type == 1)
        {
            a = &m.letters[m.FindLetter(pt.content[pt.content.size() - 1])];
        }
        if (pt.content[pt.content.size() - 1].type == 2)
        {
            a = &m.digits[m.FindDigit(pt.content[pt.content.size() - 1])];
        }
        if (pt.content[pt.content.size() - 1].type == 3)
        {
            a = &m.symbols[m.FindSymbol(pt.content[pt.content.size() - 1])];
        }
        if(find_last_segment.find(a) == find_last_segment.end())
       {
        lseg[++tot].a = a;
        lseg[tot].num = 1;
        find_last_segment.insert({a,tot});
       }
       else
       {
        lseg[find_last_segment[a]].num += 1;
       }
        // Multi-thread TODO：
        // 这个for循环就是你需要进行并行化的主要部分了，特别是在多线程&GPU编程任务中
        // 可以看到，这个循环本质上就是把模型中一个segment的所有value，赋值到PT中，形成一系列新的猜测
        // 这个过程是可以高度并行化的
        // omp_set_num_threads(THREAD_NUM);
        // #pragma omp parallel
        // {
        //     vector<string> local_guesses;
        //     #pragma omp for 
        //     for (int i = 0; i < pt.max_indices[pt.content.size() - 1]; i += 1)
        //     {
        //         string temp = guess + a->ordered_values[i];
        //         // cout << guess << endl;
        //         //guesses.emplace_back(guess);
        //         local_guesses.emplace_back(temp);
        //         //total_guesses += 1;
        //     }
        //     #pragma omp critical
        //     {
        //         guesses.insert(guesses.end(),local_guesses.begin(),local_guesses.end());
        //         total_guesses += local_guesses.size();
        //     }
        // }
       for (int i = 0; i < pt.max_indices[pt.content.size() - 1]; i += 1)
        {
            string temp = guess + a->ordered_values[i];
            // cout << temp << endl;
            guesses.emplace_back(temp);
            total_guesses += 1;
        }
        // pthread_t threads[THREAD_NUM];
        // int all = pt.max_indices[pt.content.size() - 1];
        // int chunk = all / THREAD_NUM;
        // for(int i = 0;i < THREAD_NUM;i++)
        // {
        //     ThreadTask* task = new ThreadTask();
        //     // cout<<"guess = "<<guess<<endl;
        //     task->pre_terminal = guess;
        //     task->a = a;
        //     task->self = this;
        //     task->start_index = i * chunk;
        //     task->end_index = (i == THREAD_NUM - 1) ?all : (i + 1) * chunk;
        //     pthread_create(&threads[i], NULL, PriorityQueue::fill_range, task);
        //     // if (ret != 0) 
        //     // {
        //     //     std::cerr << "Failed to create thread " << i << ", error: " << ret << std::endl;
        //     // }
        // }
        // for(int i = 0;i < THREAD_NUM;i++)
        // {
        //     pthread_join(threads[i],NULL);
        // }
    }
}