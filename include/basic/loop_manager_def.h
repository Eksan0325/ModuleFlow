/**
 * Information in this file is the intellectual property of Rokae Technology Co., Ltd,
 * And may contains trade secrets that must be stored and viewed confidentially.
 *
 * @file loop_manager_def.h
 * @brief global loop manager
 * @author Eksan
 */

#ifndef MFLOW_BASIC_LOOP_MANAGER_DEF_H
#define MFLOW_BASIC_LOOP_MANAGER_DEF_H

#include "basic/object.h"

#ifndef MFLOW_LOOP_TIMEOUT_MS
#define MFLOW_LOOP_TIMEOUT_MS 30000.0
#endif
#ifndef MFLOW_LOOP_INTERVAL_MS
#define MFLOW_LOOP_INTERVAL_MS 0.5
#endif

namespace mflow {
class LoopObject : public Object {
public:
	explicit LoopObject(const std::string& name = "") {}
	virtual void submitTask(const std::function<void()>& func) {}
	virtual bool isRunning() const { return false; }
	virtual bool hasTask() const { return false; }
	virtual int taskCount() const { return 0; }

	virtual bool start() { return false; }
	virtual bool stop() { return false; }

	virtual void setPeriodTask(const std::function<void()>& func, double period_ms = 2.0) {}
};

enum LoopEventType { LOOP_EVPP, LOOP_POOL, LOOP_ASIO };

namespace loop {
ObjectManager& mgr();
LoopObject* get(const std::string& name, bool default_pool = true);
LoopObject* create(const std::string& name, LoopEventType type = LOOP_EVPP);
}

}


#endif