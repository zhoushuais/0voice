#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <thread>   // 编译需要加上这个动态库 -lpthread
#include <condition_variable>

// 模板声明
template <typename T>
class BlockingQueue{
public:

    BlockingQueue(bool nonblock = false) : nonblock_(nonblock) {}

    // 入队操作
    void Push(const T &value) {
        std::lock_guard<std::mutex> lock(mutex_);        // 构造lock_guard时自动对mutex_加锁，析构时自动解锁
        queue_.push(value);
        not_empty_.notify_one();
    }

    // 正常pop 弹出元素
    // 异常pop 没有弹出元素
    bool Pop(T &value){
        std::unique_lock<std::mutex> lock(mutex_);      // 与lock_guard不同，unique_lock可以手动加锁和解锁

        // 1. mutex_.unlock()
        // 2. queue_.empty() && nonblock_ 线程在 wait 中阻塞
        // notify_one notify_all 唤醒线程
        // 3. 假设满足条件 mutex_.lock()
        // 4. 不满足条件 回到2
        // [this]：捕获当前类的this指针，使得 Lambda 内部能访问类的成员，无参数列表：不需要传入参数；返回值自动推导：编译器推导为bool（因为返回布尔表达式）
        not_empty_.wait(lock, [this]{ return !queue_.empty() || nonblock_; });
        if (queue_.empty()) return false;

        //  condition_variable::wait 需要临时释放锁（否则其他线程无法入队元素，当前线程会永久阻塞），unique_lock 支持这种 “自动解锁 - 重新加锁” 的逻辑，是实现条件变量等待的必需选择。

        value = queue_.front();         // 取出队列的队首元素（仅读取，不删除），并赋值给传入的引用参数 value（通过引用将元素传递给外部）。
        queue_.pop();                   // 删除队列的队首元素，完成 “取出” 的完整操作（front() 是 “读”，pop() 是 “删”，两步结合才是完整的取元素）。
        return true;
        
    }

    // 解除阻塞在当前队列的线程
    void Cancel() {
        std::lock_guard<std::mutex> lock(mutex_);
        nonblock_ = true;
        not_empty_.notify_all();
    }

private:
    bool nonblock_;
    std::queue<T> queue_;
    std::mutex mutex_;                      // 互斥锁
    std::condition_variable not_empty_;     // 条件变量,std::condition_variable not_empty_用于线程间同步，解决 “生产者 - 消费者” 模型中的等待 / 通知问题：
    // 消费者线程（线程池工作线程）可通过not_empty_.wait()等待队列非空，避免空轮询浪费 CPU；
    // 生产者线程（调用Push的线程）添加任务后，可通过not_empty_.notify_one()/notify_all()唤醒等待线程。
};


template <typename T>
class BlockingQueuePro {
public:
    BlockingQueuePro(bool nonblock = false) : nonblock_(nonblock) {}

    void Push(const T &value) {
        std::lock_guard<std::mutex> lock(prod_mutex_);
        prod_queue_.push(value);
        not_empty_.notify_one();
    }

    bool Pop(T &value) {
        std::unique_lock<std::mutex> lock(cons_mutex_);
        if (cons_queue_.empty() && SwapQueue_() == 0) {
            return false;
        }
        value = cons_queue_.front();
        cons_queue_.pop();
        return true;
    }

    void Cancel() {
        std::lock_guard<std::mutex> lock(prod_mutex_);
        nonblock_ = true;
        not_empty_.notify_all();
    }

private:

    int SwapQueue_() {
        std::unique_lock<std::mutex> lock(prod_mutex_);
        not_empty_.wait(lock, [this] {return !prod_mutex_.empty() || nonblock_;});
        std::swap(prod_queue_, cons_queue_);
        return cons_queue_.size();
    }


    bool nonblock_;
    std::queue<T> prod_queue_;
    std::queue<T> cons_queue_;
    std::mutex prod_mutex_;
    std::mutex cons_mutex_;
    std::condition_variable not_empty_;
};


