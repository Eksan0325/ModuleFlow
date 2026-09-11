/**
* @file evppevent_loop.h
* @brief inheritance of LoopObject and implementation of libevpp event loop interface.
* @author Eksan0325
*/
#ifndef MFLOW_FRAMEWORK_EVPPEVENT_LOOP_H
#define MFLOW_FRAMEWORK_EVPPEVENT_LOOP_H

#include <evpp/event_loop.h>
#include <evpp/event_loop_thread.h>

#include "basic/loop_manager_def.h"

namespace mflow {

class EvppLoopObject : public mflow::LoopObject {
public:
    explicit EvppLoopObject(const std::string& name = "");
    explicit EvppLoopObject(const std::string& name, evpp::EventLoop* current_loop);
    virtual ~EvppLoopObject();

    void submitTask(const std::function<void()>& func) override;
    bool isRunning() const override;
    bool hasTask() const override;
    int taskCount() const override;

    bool start() override;
    bool stop() override;

    void runAfter(double delay_ms, const std::function<void()>& func);
private:
    evpp::EventLoopThread* event_thread_;
    evpp::EventLoop* loop_;
};

}

#endif  