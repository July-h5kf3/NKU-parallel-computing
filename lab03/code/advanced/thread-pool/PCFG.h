#include <string>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <pthread.h>
#include <mutex>
#include <memory>
#include <unistd.h>
#include <condition_variable>
#include <omp.h>
#include <atomic>
// #include <chrono>   
// using namespace chrono;
using namespace std;

class segment
{
public:
    int type; // 0: 未设置, 1: 字母, 2: 数字, 3: 特殊字符
    int length; // 长度，例如S6的长度就是6
    segment(int type, int length)
    {
        this->type = type;
        this->length = length;
    };

    // 打印相关信息
    void PrintSeg();

    // 按照概率降序排列的value。例如，123是D3的一个具体value，其概率在D3的所有value中排名第三，那么其位置就是ordered_values[2]
    vector<string> ordered_values;

    // 按照概率降序排列的频数（概率）
    vector<int> ordered_freqs;

    // total_freq作为分母，用于计算每个value的概率
    int total_freq = 0;

    // 未排序的value，其中int就是对应的id
    unordered_map<string, int> values;

    // 根据id，在freqs中查找/修改一个value的频数
    unordered_map<int, int> freqs;


    void insert(string value);
    void order();
    void PrintValues();
};

class PT
{
public:
    // 例如，L6D1的content大小为2，content[0]为L6，content[1]为D1
    vector<segment> content;

    // pivot值，参见PCFG的原理
    int pivot = 0;
    void insert(segment seg);
    void PrintPT();

    // 导出新的PT
    vector<PT> NewPTs();

    // 记录当前每个segment（除了最后一个）对应的value，在模型中的下标
    vector<int> curr_indices;

    // 记录当前每个segment（除了最后一个）对应的value，在模型中的最大下标（即最大可以是max_indices[x]-1）
    vector<int> max_indices;
    // void init();
    float preterm_prob;
    float prob;
};

class model
{
public:
    // 对于PT/LDS而言，序号是递增的
    // 训练时每遇到一个新的PT/LDS，就获取一个新的序号，并且当前序号递增1
    int preterm_id = -1;
    int letters_id = -1;
    int digits_id = -1;
    int symbols_id = -1;
    int GetNextPretermID()
    {
        preterm_id++;
        return preterm_id;
    };
    int GetNextLettersID()
    {
        letters_id++;
        return letters_id;
    };
    int GetNextDigitsID()
    {
        digits_id++;
        return digits_id;
    };
    int GetNextSymbolsID()
    {
        symbols_id++;
        return symbols_id;
    };

    // C++上机和数据结构实验中，一般不允许使用stl
    // 这就导致大家对stl不甚熟悉。现在是时候体会stl的便捷之处了
    // unordered_map: 无序映射
    int total_preterm = 0;
    vector<PT> preterminals;
    int FindPT(PT pt);

    vector<segment> letters;
    vector<segment> digits;
    vector<segment> symbols;
    int FindLetter(segment seg);
    int FindDigit(segment seg);
    int FindSymbol(segment seg);

    unordered_map<int, int> preterm_freq;
    unordered_map<int, int> letters_freq;
    unordered_map<int, int> digits_freq;
    unordered_map<int, int> symbols_freq;

    vector<PT> ordered_pts;

    // 给定一个训练集，对模型进行训练
    void train(string train_path);

    // 对已经训练的模型进行保存
    void store(string store_path);

    // 从现有的模型文件中加载模型
    void load(string load_path);

    // 对一个给定的口令进行切分
    void parse(string pw);

    void order();

    // 打印模型
    void print();
};

struct TaskBase
{
    virtual ~TaskBase(){}
    virtual void run() = 0;
    static void* runTask(void* arg) 
    {
        unique_ptr<TaskBase> task(static_cast<TaskBase*>(arg));
        task->run();
        return nullptr;
    }
};

class ThreadPool
{
    public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();
    void enqueue(TaskBase* task);
    void stop();

    private:
        vector<pthread_t> workers;
        queue<TaskBase*> tasks;

        mutex queue_mutex;
        condition_variable_any condition;
        bool stop_flag = 0;

        static void* worker_thread(void* arg);
        void run();
};

// 优先队列，用于按照概率降序生成口令猜测
// 实际上，这个class负责队列维护、口令生成、结果存储的全部过程
class PriorityQueue
{
public:
    // 用vector实现的priority queue
    vector<PT> priority;
    PriorityQueue() : pool(4), THREAD_NUM(4) {}  // 默认用4个线程
    ~PriorityQueue() { pool.stop(); }
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//这个是锁 因为有全局变量guesses 和 total_guesses所以要加锁不然会访问冲突
    // 模型作为成员，辅助猜测生成
    model m;
    std::atomic<int> remaining_tasks{0};
    std::mutex done_mutex;
    condition_variable_any done_condition;
    int THREAD_NUM;
    int Threadshold;
    // 计算一个pt的概率
    void CalProb(PT &pt);
    static void* fill_range(void* arg);
    // 优先队列的初始化
    void init();

    // 对优先队列的一个PT，生成所有guesses
    void Generate(PT pt);

    // 将优先队列最前面的一个PT
    void PopNext();

    void submit_task(segment* a,const string& pre_terminal,int start_index,int end_index);

    int total_guesses = 0;

    vector<string> guesses;
private:
    ThreadPool pool;
};

class TaskGuard {
    public:
        TaskGuard(PriorityQueue* q) : queue(q), dismissed(false) {}
        ~TaskGuard() {
            if (!dismissed && queue) {
                if (--queue->remaining_tasks == 0) {
                    std::lock_guard<std::mutex> lock(queue->done_mutex);
                    queue->done_condition.notify_all();
                }
            }
        }
        void dismiss() { dismissed = true; }
    
    private:
        PriorityQueue* queue;
        bool dismissed;
    };


class Fill_Task : public TaskBase
{
    public:
        Fill_Task(PriorityQueue* s,segment* seg,const string& pt,int start,int end)
        :self(s),a(seg),pre_terminal(pt),start_index(start),end_index(end) {}

        void run() override {
            TaskGuard guard(self); // 确保任务完成时递减 remaining_tasks
        
            vector<string> local_guesses;
            local_guesses.reserve(end_index - start_index);
            int local_count = 0;
        
            for (int i = start_index; i < end_index; ++i) {
                if (i >= a->ordered_values.size()) {
                    cerr << "Index out of bounds: " << i << " >= " << a->ordered_values.size() << endl;
                    break;
                }
                string& suffix = a->ordered_values[i];
                string guess = pre_terminal + suffix;
                local_guesses.emplace_back(guess);
                ++local_count;
            }
        
            pthread_mutex_lock(&self->mutex);
            self->guesses.insert(self->guesses.end(), local_guesses.begin(), local_guesses.end());
            self->total_guesses += local_count;
            pthread_mutex_unlock(&self->mutex);
        
            // guard 会自动触发剩余任务计数递减
        }
    private:
    PriorityQueue*  self;
    segment* a;
    string pre_terminal;
    int start_index;
    int end_index;
};

    