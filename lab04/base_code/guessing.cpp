#include "PCFG.h"
#include <mpi.h>
#include <vector>
#include <string.h>
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
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    PT pt_for_generate;
    if (rank == 0)
    {
        // 主进程从其队列的前端获取真实的PT。
        if (priority.empty()) return;
        pt_for_generate = priority.front();
    }
    Generate(pt_for_generate);
    if(rank == 0)
    {
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
    // 然后需要根据即将出队的PT，生成一系列新的PT
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
void PriorityQueue::Generate(PT pt)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    string guess;
    long long broadcast_data[3];
    
    if (rank == 0) 
    {
        if (!pt.content.empty()) 
        {
            //计算前缀
            if (pt.content.size() > 1) 
            {
                for (size_t i = 0; i < pt.content.size() - 1; ++i) 
                {
                    int seg_idx_val = pt.curr_indices[i];
                    if (pt.content[i].type == 1) guess += m.letters[m.FindLetter(pt.content[i])].ordered_values[seg_idx_val];
                    if (pt.content[i].type == 2) guess += m.digits[m.FindDigit(pt.content[i])].ordered_values[seg_idx_val];
                    if (pt.content[i].type == 3) guess += m.symbols[m.FindSymbol(pt.content[i])].ordered_values[seg_idx_val];
                }
            }
            // 准备最后一个segment的信息
            int last_idx = pt.content.size() - 1;
            const segment& last_seg = pt.content[last_idx];

            broadcast_data[0] = last_seg.type;
            broadcast_data[2] = pt.max_indices[last_idx];

            if(last_seg.type == 1) broadcast_data[1] = m.FindLetter(last_seg);
            else if(last_seg.type == 2) broadcast_data[1] = m.FindDigit(last_seg);
            else if(last_seg.type == 3) broadcast_data[1] = m.FindSymbol(last_seg);
            else broadcast_data[1] = -1;
        }
        else
        {
            broadcast_data[0] = -1; // 标记为无效PT
        }
        
    }
    MPI_Bcast(broadcast_data, 3, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    if (broadcast_data[0] == -1) 
    {
        return;
    }

    int guess_len = (rank == 0) ? guess.length() : 0;
    MPI_Bcast(&guess_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    vector<char> guess_buffer(guess_len + 1);
    if (rank == 0) 
    {
        strcpy(guess_buffer.data(), guess.c_str());
    }
    MPI_Bcast(guess_buffer.data(), guess_len + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    guess = string(guess_buffer.data());

    int last_seg_type = broadcast_data[0];
    int index_in_model = broadcast_data[1];
    int loop_bound = broadcast_data[2];
   
    segment* a;
    if(index_in_model != - 1)
    {
        if (last_seg_type == 1)       a = &m.letters[index_in_model];
        else if (last_seg_type == 2)  a = &m.digits[index_in_model];
        else if (last_seg_type == 3)  a = &m.symbols[index_in_model];
    }
    fill_preterminal(guess, a, loop_bound);
}