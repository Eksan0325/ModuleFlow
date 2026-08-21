#include "framework/threadpool_loop.h"

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

namespace {
    inline bool TurnThreadCpu(int cpu_id) {
        cpu_set_t cpuset;
        auto thread_id = pthread_self();
        CPU_SET(cpu_id, &cpuset);

        if (0 != pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset)) {
            return false;
        }
        if (0 != pthread_getaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset)) {
            return false;
        }
        return true;
    }
    inline pid_t GetThreadID() {
        static bool can_using_gettid = true;
        if (can_using_gettid) {
            pid_t tid = syscall(__NR_gettid);
            if (tid != -1) return tid;
            can_using_gettid = false;
        }
        return static_cast<pid_t>(pthread_self());
    }
}

using namespace mflow;

ThreadPoolLoopObject::ThreadPoolLoopObject(const std::string& name, unsigned short size)
    : LoopObject(name) {
    addThread(size);
}
ThreadPoolLoopObject::~ThreadPoolLoopObject() {
    run_ = false;
    task_cv_.notify_all();
    for (auto& t : pool_) {
        if (t.joinable()) t.join();
    }
}
void ThreadPoolLoopObject::submitTask(const std::function<void()>& func) { submit([=]{func();}); }
bool ThreadPoolLoopObject::isRunning() const { return run_; }
bool ThreadPoolLoopObject::hasTask() const { return !tasks_.empty(); }
int ThreadPoolLoopObject::taskCount() const { return static_cast<int>(tasks_.size()); }

void ThreadPoolLoopObject::addThread(unsigned short size) {
    for (; pool_.size() < THREADPOOL_MAX_NUM && size > 0; --size) {
        pool_.emplace_back([this] {
            auto _name = "ORGINAL_THREAD_POOL" + std::to_string(GetThreadID());
            pthread_setname_np(pthread_self(), _name.c_str());

            while (run_) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock{ lock_ };
                    task_cv_.wait(lock, [this] { return !run_ || !tasks_.empty(); });
                    if (!run_ && tasks_.empty()) return;
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                idle_thr_num_--;
                task();
                idle_thr_num_++;
            }
        });
        idle_thr_num_++;
    }
}