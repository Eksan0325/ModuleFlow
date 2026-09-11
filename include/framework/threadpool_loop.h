/**
* @file threadpool_loop.h
* @brief inheritance of LoopObject and implementation of thread pool interface.
* @author Eksan0325
*/
#ifndef MFLOW_FRAMEWORK_THREADPOOL_LOOP_H
#define MFLOW_FRAMEWORK_THREADPOOL_LOOP_H

#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <pthread.h>
#include <functional>
#include <memory>

#include "basic/loop_manager_def.h"

constexpr int THREADPOOL_MAX_NUM = 16;
namespace mflow {
class ThreadPoolLoopObject : public mflow::LoopObject {
public:
    explicit ThreadPoolLoopObject(const std::string& name = "", unsigned short size = 4);
    virtual ~ThreadPoolLoopObject();

    explicit ThreadPoolLoopObject(const ThreadPoolLoopObject& other) = delete;
    explicit ThreadPoolLoopObject(ThreadPoolLoopObject&& other) = delete;
    ThreadPoolLoopObject& operator=(const ThreadPoolLoopObject&) = delete;
    ThreadPoolLoopObject& operator=(ThreadPoolLoopObject&&) = delete;
public:
    void submitTask(const std::function<void()>& func) override;
    bool isRunning() const override;
    bool hasTask() const override;
    int taskCount() const override;

    template<typename Func, typename... Args>
    auto submit(Func&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        if (!run_) return std::future<void>();
        using ResultType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<ResultType()>>(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
        std::future<ResultType> future = task->get_future();
        {
            std::lock_guard<std::mutex> lock{ lock_ };
            tasks_.emplace([task]() { (*task)(); });
        }
        if (idle_thr_num_ < 1 && pool_.size() < THREADPOOL_MAX_NUM) {
            addThread(1);
        }
        task_cv_.notify_one();
        return future;
    }
    void addThread(unsigned short size);
private:
    std::vector<std::thread> pool_;
    std::queue<std::function<void()>> tasks_;
    std::mutex lock_;
    std::condition_variable task_cv_;
    std::atomic<bool> run_{ true };
    std::atomic<int> idle_thr_num_{ 0 };
};
}


#endif