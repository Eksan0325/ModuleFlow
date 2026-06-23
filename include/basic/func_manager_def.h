#ifndef MFLOW_BASIC_FUNC_MANAGER_DEF_H
#define MFLOW_BASIC_FUNC_MANAGER_DEF_H
#include <memory>

#include "basic/cmd_manager_def.h"
#include "basic/base.h"

namespace mflow {
class Module;

ObjectManager& mgr();
bool registerFuncObject(Module* context, const std::string& key, Object* f, bool delete_if_failed);

template<typename FT> class FuncObject;
template<typename FT> class Func;

template<typename RT, typename... ArgsT>
class FuncObject<std::function<RT(ArgsT...)>> : public Object {
	using FuncT = std::function<RT(ArgsT...)>;
public:
	explicit FuncObject(const std::string& name, const FuncT& func) 
		: Object(name), ft_(std::make_shared<FuncT>(func)) {}
	virtual ~FuncObject(){}
	std::shared_ptr<FuncT> get() { return ft_; }
private:
	std::shared_ptr<FuncT> ft_;
};

template<typename RT, typename... ArgsT>
class Func<RT(ArgsT...)> {
	using FuncT = std::function<RT(ArgsT...)>;
	using FuncObjectT = FuncObject<FuncT>;
public:
	Func() : ft_(nullptr){}
	Func(const std::string& name) { connect(name); }
	virtual ~Func() {}
public:
	static std::shared_ptr<FuncT> get(const std::string& name) {
		auto* _obj = mgr().get(name);
		if (_obj != nullptr) {
			if (auto* _fobj = dynamic_cast<FuncObject<FuncT>*>(_obj)) return _fobj->get();
		}
		return nullptr;
	}
	bool connect(const std::string& name) {
		ft_ = get(name);
		return ft_ != nullptr;
	}
	RT operator()(ArgsT... args) {
		if (ft_) return (*ft_)(std::forward<ArgsT>(args)...);
		return static_cast<RT>(NULL);
	}
private:
	std::shared_ptr<FuncT> ft_;
};

template<typename... ArgsT>
class Func<CommandResult(ArgsT...)> {
	using FuncT = std::function<CommandResult(ArgsT...)>;
public:
	Func() : ft_(nullptr) {}
	Func(const std::string& name) { connect(name); }
	virtual ~Func(){}
public:
	static std::shared_ptr<FuncT> get(const std::string& name) {
		auto* _obj = mgr().get(name);
		if (_obj != nullptr) {
			if (auto* _fobj = dynamic_cast<FuncObject<FuncT>*>(_obj)) return _fobj->get();
		}
		return nullptr;
	}
	bool connect(const std::string& name) {
		ft_ = get(name);
		return ft_ != nullptr;
	}
	CommandResult operator()(ArgsT... args) {
		if (ft_) return (*ft_)(std::forward<ArgsT>(args)...);
		return NULL;
	}
private:
	std::shared_ptr<FuncT> ft_;
};

}

#endif

