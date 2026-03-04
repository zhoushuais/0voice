#pragma once

#include <functional>
#include <vector>
#include <thread>


// #include "blockingqueue.h"

// 避免头文件循环依赖，使用前置声明的做法, 但是blockingqueue 仅仅只能用作指针或引用，不能直接实例化
template <typename T>
class BlockingQueue;

class ThreadPool {
public:
    // 用了explicit是为了不要隐式转换，比如ThreadPool tp = 12；不希望出现这样写，我们的要求是ThreadPool tp = ThreadPool(12);
    // 初始化线程池
    explicit ThreadPool(int threads_num);

    // 停止线程池
    ~ThreadPool();

    // 向线程池中添加任务             // std::function 是一个模板类，可以存储、调用和复制任何可调用对象
    void Post(std::function<void()> task);

private:

    void Worker();

    std::unique_ptr<BlockingQueue<std::function<void()>>> task_queue_;

    std::vector<std::thread> workers_;
};