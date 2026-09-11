#include "basic/loop_manager_def.h"

#include "framework/threadpool_loop.h"

namespace mflow {

namespace loop {
ObjectManager& mgr() {
	static ObjectManager _("mflow_global_loop_manager");
	return _;
}

LoopObject * get(const std::string& name, bool default_pool) {
	LoopObject* lobj = mgr().get<LoopObject>(name);
	if (lobj || !default_pool) return lobj;
	static LoopObject* nullobj = new ThreadPoolLoopObject();
	return nullobj;
}

LoopObject * create(const std::string& name, LoopEventType type) {
	LoopObject* newobj = nullptr;
	switch (type) {
	case LOOP_POOL: newobj = new ThreadPoolLoopObject("mflow_global_thread_pool"); break;
	default: newobj = nullptr; break;
	}
	if (newobj) mgr().add(newobj);
	return newobj;
}

}
}