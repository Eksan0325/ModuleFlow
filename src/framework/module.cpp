#include "framework/module.h"
#include "common.h"

namespace mflow {

Module::Module() : Object() {}
Module::~Module() {}

std::string Module::name() const { return Object::name() == MFLOW_UNDEFINED_OBJECT_NAME ? mflow::entry::createModuleName() : Object::name(); }
std::string Module::name(const std::string& suffix) const { return suffix.empty() ? name() : name() + "." +  suffix; }

Module::ModuleStateSignal Module::state() const { return state_.load(); }

void Module::startToInit() {
	emit(state_ = MODULE_GONNA_INIT);
	onInit();
	if(mflow::entry::module(name())) emit(state_ = MODULE_INITED);
}
void Module::startToIdle() {
	emit(state_ = MODULE_GONNA_IDLE);
	onInit();
	if (mflow::entry::module(name())) emit(state_ = MODULE_IDLE);
}
void Module::startToDeinit() {
	emit(state_ = MODULE_DEINIT);
	onInit();
	if (mflow::entry::module(name())) emit(state_ = MODULE_DEINITED);
}

void Module::adoptObject(Object* obj) {
	connect(DELETE, obj, [=]() noexcept { delete obj; });
}

void Module::abandonObject(Object* obj, bool auto_delete) {
	disconnect(DELETE, obj);
	if (auto_delete) delete obj;
}

void Module::onInit() {}
void Module::onIdle() {}
void Module::onDeinit() {}

}