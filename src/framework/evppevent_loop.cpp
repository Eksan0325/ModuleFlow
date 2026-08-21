#include "framework/evppevent_loop.h"

#include <condition_variable>

#include <evpp/libevent.h>

using namespace mflow;

EvppLoopObject::EvppLoopObject(const std::string& name) : LoopObject(name),
    event_thread_(new evpp::EventLoopThread),
    loop_(event_thread_->loop()) {}
EvppLoopObject::EvppLoopObject(const std::string& name, evpp::EventLoop* current_loop) : LoopObject(name),
    event_thread_(nullptr),
    loop_(current_loop) {}
EvppLoopObject::~EvppLoopObject() {
    if (isRunning()) stop();
    delete event_thread_;
}

void EvppLoopObject::submitTask(const std::function<void()>& func) { loop_->RunInLoop(func); }
bool EvppLoopObject::isRunning() const { return loop_->IsRunning(); }
bool EvppLoopObject::hasTask() const { return taskCount() > 0; }
int EvppLoopObject::taskCount() const { return loop_->pending_functor_count(); }

bool EvppLoopObject::start() {
    if (!event_thread_) return false;
    if (event_thread_->IsRunning()) return true;
    auto res = event_thread_->Start();
    this->submitTask([this] {
        pthread_setname_np(pthread_self(), name().c_str());
        });
    return res;
}
bool EvppLoopObject::stop() {
    if (!event_thread_) return false;
    if (!event_thread_->IsRunning()) return true;
    event_thread_->Stop();  
    event_thread_->event_loop_.reset(loop_ = new evpp::EventLoop);
    return true;
}
void EvppLoopObject::runAfter(double delay_ms, const std::function<void()>& func) {
    loop_->RunAfter(delay_ms, func);
}