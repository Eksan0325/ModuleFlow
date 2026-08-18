#include "basic/func_manager_def.h"
#include "framework/module.h"

namespace mflow {

ObjectManager& mgr() {
	static ObjectManager _("mflow_global_func_manager");
	return _;
}

bool registerFuncObject(Module* context, const std::string& key, Object* f, bool delete_if_failed) {
	if (context == nullptr || f == nullptr) return false;
	if (mgr().add(f, delete_if_failed) == nullptr) return false;
	context->adoptObject(f);
	return true;
}
}