#include "basic/cmd_manager_def.h"
#define MFLOW_MAX_COMMAND_PROC_CNT 1024
#define MFLOW_COMMAND_DATA_MANAGER_STR "_cmd_data_mgr"

namespace mflow {

container::Map<std::string, int> global_cmd_object_cnt;

CommandResult::CommandResult(int code) 
	: code_(code), data_(nullptr) {}
CommandResult::CommandResult(bool flag, int error_code) 
	: CommandResult(flag ? SUCCESS : error_code) {}
CommandResult::CommandResult(void) 
	: CommandResult(SUCCESS) {}

int CommandResult::code() const { return code_; }
void CommandResult::setCode(int code) { code_ = code; }
DataObject* CommandResult::data() const { return data_.get(); }

bool CommandResult::isSuccess() const 
{ return code_ == SUCCESS; }
bool CommandResult::isError() const 
{ return code_ < FAILED; }
bool CommandResult::isAsyncAccept() const 
{ return code_ > ASYNC_ACCEPT; }
CommandResult::operator bool() const 
{ return isSuccess(); }

Json::Value CommandResult::serialize() const {
	const JsonInterface* _interface = dynamic_cast<const JsonInterface*>(data());
	if (_interface == nullptr)  return code_; 
	Json::Value result;
	if (isError()) result["error_code"] = code_;
	if (_interface) {
		result["data"] = _interface->serialize();
		return result;
	}
	return result;
}

bool CommandResult::deserialize(const JsonRefrence& jref) {
	bool flag = false;
	if (jref.objectSize() < 0) {
		code_ = jref.toInt(SUCCESS, &flag);
		return flag;
	}
	if (auto err = jref.at("error_code")) {
		code_ = err.toInt(FAILED, &flag);
	}
	if (auto dt = jref.at("data")) {
		JsonInterface* _interface = dynamic_cast<JsonInterface*>(data());
		if (_interface != nullptr) flag = _interface->deserialize(dt);
	}
	return flag;
}

CommandObject::Procedure::Procedure(const Functor& ft, LoopObject * loop)
	: functor_(ft), loop_(loop), blocked_(true) {}
CommandObject::Procedure::Procedure(Functor&& ft, LoopObject* loop)
	: functor_(std::move(ft)), loop_(loop), blocked_(true) {}
CommandResult CommandObject::Procedure::run(CommandObject* c) const { return functor_(c); }

CommandObject::CommandObject(const std::string& name)
	: Object(name), mgr_(name + MFLOW_COMMAND_DATA_MANAGER_STR), handler_(NULL) {
	int _current_index = global_cmd_object_cnt.value(this->name());
	global_cmd_object_cnt[this->name()] = ++_current_index;
}
CommandObject::~CommandObject()
{
	global_cmd_object_cnt[this->name()] = global_cmd_object_cnt.value(this->name()) - 1;
}
CommandObject* CommandObject::clone() const {
	auto* _cmdobj = new CommandObject(name() + "_copy");
	for (auto& _iterator : procedure_list_) { _cmdobj->appendProc(_iterator); }
	return _cmdobj;
}
ObjectManager& CommandObject::getDataMgr() { return mgr_; }
void CommandObject::setResultHandler(const CommandResultHandler& handler) { handler_ = handler; }

CommandObject* CommandObject::prependProc(const Procedure& p) {
	if (procedure_list_.sizeInt() < MFLOW_MAX_COMMAND_PROC_CNT * 2) {
		procedure_list_.push_front(p);
		return this;
	}
	return nullptr;
}
CommandObject* CommandObject::appendProc(const Procedure& p) {
	if (procedure_list_.sizeInt() < MFLOW_MAX_COMMAND_PROC_CNT * 2) {
		procedure_list_.push_back(p);
		return this;
	}
	return nullptr;
}

CommandResult CommandObject::start(bool auto_delete) { return CommandResult(CommandResult::SUCCESS); }
void CommandObject::report(const CommandResult& result) {}
void CommandObject::finish(const CommandResult& result, bool auto_delete) {}
namespace command {
ObjectManager& mgr() {
	static ObjectManager _("command_manager");
	return _;
}
CommandObject* copy(const std::string& name) {
	if (auto _cmdobj = mgr().get<CommandObject>(name)) {
		_cmdobj = _cmdobj->clone();
		return _cmdobj;
	}
	return nullptr;
}
CommandObject* copy(const std::string& name, const CommandObject::CommandResultHandler& handler) {
	auto _cmdobj = copy(name);
	if (_cmdobj != nullptr) _cmdobj->setResultHandler(handler);
	return _cmdobj;
}
bool registerCommand(Module* context, const std::string& key, CommandObject* c, bool delete_if_failed) {
	if (context == nullptr || c == nullptr) return false;
	if (mgr().add(c, delete_if_failed) == nullptr) return false;
	//context
	return true;
}

}
}