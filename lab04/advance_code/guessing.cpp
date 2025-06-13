#include "PCFG.h"
#include <mpi.h>
#include <vector>
#include <string.h>
#include <algorithm>
#include <omp.h>
using namespace std;
#define MAX_RESULT_LEN 128

void PriorityQueue::fill_preterminal(const std::string& base,segment* a,int num)
{
    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    vector<string> local_guesses;

    if (num <= 0 || a == nullptr) 
    {} 
    else 
    {
        long long n_total = num;
        long long start_idx = (long long)rank * n_total / size;
        long long end_idx = (long long)(rank + 1) * n_total / size;

        local_guesses.reserve(end_idx - start_idx);

        for (long long i = start_idx; i < end_idx; ++i) 
        {
            local_guesses.emplace_back(base + a->ordered_values[i]);
        }
    }
    //将每个进程的local_guesses链接
    size_t total_local_len = 0;
    for (const auto& s : local_guesses) 
    {
        total_local_len += s.length() + 1;
    }
    string concat_str;
    if (total_local_len > 0) 
    {
        concat_str.reserve(total_local_len);
        for (const auto& s : local_guesses) 
        {
            concat_str.append(s);
            concat_str.push_back('\0');
        }
    }
    //在主线程收集这些链接后的字符串大小
    int local_size = concat_str.length();
    vector<int> all_sizes;
    if (rank == 0) 
    {
        all_sizes.resize(size);
    }
    MPI_Gather(&local_size, 1, MPI_INT, all_sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    //接受数据
    vector<int> displacements;
    vector<char> recv_buffer;
    if (rank == 0) 
    {
        displacements.resize(size, 0);
        size_t total_recv_size = 0;
        for (int i = 0; i < size; ++i) 
        {
            displacements[i] = total_recv_size;
            total_recv_size += all_sizes[i];
        }
        if (total_recv_size > 0) 
        {
            recv_buffer.resize(total_recv_size);
        }
    }
    MPI_Gatherv(concat_str.data(), local_size, MPI_CHAR,
    recv_buffer.data(), all_sizes.data(), displacements.data(),
    MPI_CHAR, 0, MPI_COMM_WORLD);
    //主进程解包缓冲区并将其添加到主'guesses'
    if (rank == 0 && !recv_buffer.empty()) 
    {
        size_t current_pos = 0;
        while (current_pos < recv_buffer.size()) 
        {
            string s(&recv_buffer[current_pos]);
            guesses.emplace_back(std::move(s));
            current_pos += guesses.back().length() + 1;
        }
    }
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
    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    vector<int> send_buffer;
    vector<int> sendcounts(size,0);
    vector<int> displacements(size,0);
    int batch_size = 0;
    if (rank == 0)
    {
        vector<PT> pt_for_generate;
        // 主进程从其队列的前端获取真实的PT。
       batch_size = min((int)priority.size(),size);
    //    cout<<batch_size<<endl;
       for(int i = 0;i < batch_size;i++)
       {
        PT now_pt = priority.front();
        pt_for_generate.push_back(now_pt);
        
        vector<PT> new_pts = now_pt.NewPTs();
        for(PT insert_pt : new_pts)
        {
            CalProb(insert_pt);
            // for(auto iter = priority.begin(); iter != priority.end();iter++)
            // {
            //     // 对于非队首和队尾的特殊情况
            //     if (iter != priority.end() - 1 && iter != priority.begin())
            //     {
            //         // 判定概率
            //         if (insert_pt.prob <= iter->prob && insert_pt.prob > (iter + 1)->prob)
            //         {
            //             priority.emplace(iter + 1, insert_pt);
            //             break;
            //         }
            //     }
            //     if (iter == priority.end() - 1)
            //     {
            //         priority.emplace_back(insert_pt);
            //         break;
            //     }
            //     if (iter == priority.begin() && iter->prob < insert_pt.prob)
            //     {
            //         priority.emplace(iter, insert_pt);
            //         break;
            //     }
            // }
            priority.push_back(insert_pt);
        }
        priority.erase(priority.begin());
       }
       sort(priority.begin(), priority.end(), [](const PT& a, const PT& b){return a.prob > b.prob;});

       int current_displacement = 0;
       for(int i = 0;i < pt_for_generate.size();i++)
       {
        const auto& pt = pt_for_generate[i];

        vector<int> temp_buffer;
        temp_buffer.push_back(pt.content.size());
        temp_buffer.push_back(pt.curr_indices.size());
        temp_buffer.push_back(pt.max_indices.size());

        for(const auto& seg : pt.content)
        {
            temp_buffer.push_back(seg.type);
            temp_buffer.push_back(seg.length);
        }
        temp_buffer.insert(temp_buffer.end(),pt.curr_indices.begin(),pt.curr_indices.end());
        temp_buffer.insert(temp_buffer.end(),pt.max_indices.begin(),pt.max_indices.end());
        send_buffer.insert(send_buffer.end(),temp_buffer.begin(),temp_buffer.end());
        sendcounts[i] = temp_buffer.size();
        displacements[i] = current_displacement;
        current_displacement += sendcounts[i];
       }
    }
    MPI_Bcast(&batch_size,1,MPI_INT,0,MPI_COMM_WORLD);
    if(batch_size == 0)return;
    int recv_count;
    MPI_Scatter(sendcounts.data(), 1, MPI_INT, &recv_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    vector<int> recv_buffer(recv_count);
    MPI_Scatterv(send_buffer.data(),sendcounts.data(),displacements.data(),MPI_INT,recv_buffer.data(),recv_count,MPI_INT,0,MPI_COMM_WORLD);
    vector<string> local_guesses;
    if(recv_count > 0)
    {
        PT pt;
        int content_size = recv_buffer[0];
        int curr_indices_size = recv_buffer[1];
        int max_indices_size = recv_buffer[2];

        int pos = 3;
        for(int i = 0;i < content_size;i++)
        {
            pt.content.emplace_back(recv_buffer[pos],recv_buffer[pos + 1]);
            pos += 2;
        }
        pt.curr_indices.assign(recv_buffer.begin() + pos,recv_buffer.begin() + pos + max_indices_size);
        pos += curr_indices_size;
        pt.max_indices.assign(recv_buffer.begin() + pos,recv_buffer.begin() + pos + max_indices_size);
        local_guesses = Generate(pt);

    }
    size_t total_local_len = 0;
    for(const auto& s : local_guesses)
    {
        total_local_len += s.length() + 1;
    }
    string concat_str;
    for(const auto& s : local_guesses)
    {
        concat_str.append(s);
        concat_str.push_back('\0');
    }
    int local_size = concat_str.length();
    vector<int> all_sizes(size);
    MPI_Gather(&local_size,1,MPI_INT,all_sizes.data(),1,MPI_INT,0,MPI_COMM_WORLD);

    vector<char> gathered_buffer;
    vector<int> recv_displacements(size,0);
    if(rank == 0)
    {
        size_t total_recv_size = 0;
        for(int i = 0;i < size;i++)
        {
            recv_displacements[i] = total_recv_size;
            total_recv_size += all_sizes[i];
        }
        gathered_buffer.resize(total_recv_size);
    }
    MPI_Gatherv(concat_str.data(), local_size, MPI_CHAR,
                gathered_buffer.data(), all_sizes.data(), recv_displacements.data(),
                MPI_CHAR, 0, MPI_COMM_WORLD);
    if (rank == 0) 
    {
        size_t current_pos = 0;
        while (current_pos < gathered_buffer.size()) 
        {
            char* start = &gathered_buffer[current_pos];
            if (*start == '\0') 
            {
                current_pos++;
                continue;
            }
            string s(start);
            guesses.emplace_back(s);
            total_guesses++;
            current_pos += s.length() + 1;
        }
    }
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
vector<string> PriorityQueue::Generate(PT pt)
{
    vector<string> final_guesses;
    if (pt.content.empty()) {
        return final_guesses;
    }

    // 用一个 vector 来存储每个线程的私有结果容器
    vector<vector<string>> private_results;
    int num_threads = 0;

    if (pt.content.size() == 1)
    {
        segment* a = nullptr;
        if (pt.content[0].type == 1)         a = &m.letters[m.FindLetter(pt.content[0])];
        else if (pt.content[0].type == 2)    a = &m.digits[m.FindDigit(pt.content[0])];
        else if (pt.content[0].type == 3)    a = &m.symbols[m.FindSymbol(pt.content[0])];

        if (a && pt.max_indices.size() > 0)
        {
            const int n_total = pt.max_indices[0];

            #pragma omp parallel
            {
                // 在并行区域的开始处，获取线程总数并初始化私有结果容器
                // 这个代码块只会被每个线程执行一次
                #pragma omp single
                {
                    num_threads = omp_get_num_threads();
                    private_results.resize(num_threads);
                }

                int thread_id = omp_get_thread_num();
                
                // 每个线程在自己的私有 vector 中工作
                #pragma omp for schedule(static) nowait
                for (int i = 0; i < n_total; i++) {
                    private_results[thread_id].emplace_back(a->ordered_values[i]);
                }
            } // 并行区域结束

            // 合并结果 (串行执行)
            size_t total_size = 0;
            for(int i = 0; i < num_threads; ++i) total_size += private_results[i].size();
            final_guesses.reserve(total_size);
            for(int i = 0; i < num_threads; ++i) {
                final_guesses.insert(final_guesses.end(), private_results[i].begin(), private_results[i].end());
            }
        }
    }
    else // 多 segment 的情况
    {
        string guess_prefix;
        // ... (准备 guess_prefix 的代码不变) ...
        int seg_idx = 0;
        for(int idx : pt.curr_indices)
        {
           if(seg_idx == pt.content.size() - 1) break;
           if(pt.content[seg_idx].type == 1)    guess_prefix += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
           if (pt.content[seg_idx].type == 2)   guess_prefix += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
           if (pt.content[seg_idx].type == 3)   guess_prefix += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];
           seg_idx += 1;
        }

        segment* a = nullptr;
        int last_idx = pt.content.size() - 1;
        if(pt.content[last_idx].type == 1)  a = &m.letters[m.FindLetter(pt.content[last_idx])];
        if (pt.content[last_idx].type == 2) a = &m.digits[m.FindDigit(pt.content[last_idx])];
        if (pt.content[last_idx].type == 3) a = &m.symbols[m.FindSymbol(pt.content[last_idx])];

        if (a && pt.max_indices.size() > last_idx)
        {
            const int n_total = pt.max_indices[last_idx];
            
            #pragma omp parallel
            {
                #pragma omp single
                {
                    num_threads = omp_get_num_threads();
                    private_results.resize(num_threads);
                }
                
                int thread_id = omp_get_thread_num();
                
                #pragma omp for schedule(static) nowait
                for (int i = 0; i < n_total; i++) {
                    private_results[thread_id].emplace_back(guess_prefix + a->ordered_values[i]);
                }
            } // 并行区域结束

            // 合并结果 (串行执行)
            size_t total_size = 0;
            for(int i = 0; i < num_threads; ++i) total_size += private_results[i].size();
            final_guesses.reserve(total_size);
            for(int i = 0; i < num_threads; ++i) {
                final_guesses.insert(final_guesses.end(), private_results[i].begin(), private_results[i].end());
            }
        }
    }
    return final_guesses;
}