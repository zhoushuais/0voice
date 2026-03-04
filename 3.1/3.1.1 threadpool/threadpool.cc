
#include "threadpool.h"
#include "blockingqueue.h"   


ThreadPool::ThreadPool(int threads_num) {
    task_queue_ = std::make_unique<BlockingQueue<std::function<void()>>>();
    for (size_t i = 0; i < threads_num; ++i) {

        // 当 emplace_back 构造 std::thread 时，会立即启动新线程，执行 lambda 表达式中的 Worker() 函数
        workers_.emplace_back([this]-> void {Worker();});       // emplace_back：容器的高效构造方法，直接在容器的内存空间中构造 std::thread 对象
    }
}


// 停止线程池
ThreadPool::~ThreadPool() {
    task_queue_->Cancel();
    for (auto &worker: workers_) {      // 范围基于 for 循环，遍历 workers_ 容器,使用 auto &（引用）
        if (worker.joinable())          // joinable() 是 std::thread 的成员函数，返回 bool 值：
            worker.join();          // 等待线程结束：worker.join(),join() 是 std::thread 的核心成员函数，作用是
                                    // 阻塞当前线程（调用析构函数的线程，比如主线程）；
                                    // 等待 worker 对应的工作线程执行完毕；
                                    // 线程执行完毕后，worker 变为非 joinable 状态，资源被释放。
    }
}

// 向线程池中添加任务             // std::function 是一个模板类，可以存储、调用和复制任何可调用对象  
void ThreadPool::Post(std::function<void()> task) {
    task_queue_->Push(task);
}


void ThreadPool::Worker() {
    while (true) {
        std::function<void()> task;
        if (!task_queue_->Pop(task)) {
            break;
        }
        task();
    }
}