/**
 * @file cmd_manager_def.h
 * @brief global cmd pipeline storage
 * @author Eksan0325
 */
#ifndef MFLOW_BASIC_CMD_MANAGER_DEF_H
#define MFLOW_BASIC_CMD_MANAGER_DEF_H

#include "basic/data_manager_def.h"
#define MFLOW_COMMAND_INPUT "_input"

namespace mflow {

class Module;
class LoopObject;

class CommandResult : public JsonInterface {
public:
	enum ResultCode : int { SUCCESS = 0, FAILED = -1, ASYNC_ACCEPT = 1};
	CommandResult(int code);
	CommandResult(bool flag, int error_code = FAILED);
	CommandResult(void);
public:
	int code() const;
	void setCode(int code);
	DataObject* data() const;
	template<typename T> CommandResult& setData(T d) { data_.reset(data::from(d)); return *this; }

	bool isSuccess() const;
	bool isError() const;
	bool isAsyncAccept() const;
	explicit operator bool() const;

	Json::Value serialize() const override;
	bool deserialize(const JsonRefrence& jref) override;

private:
	int code_;
	DataObjectPointer data_;
};

class CommandObject : public Object {
public:
	class Procedure {
	public:
		typedef std::function<CommandResult(CommandObject*)> Functor;
	public:
		explicit Procedure(const Functor& f, LoopObject* loop = nullptr);
		explicit Procedure(Functor&& f, LoopObject* loop = nullptr);
		
		template<typename P = Procedure>
		static Procedure from(const Procedure& p) { return p; }
		template<typename F>
		static meta::FuncIfSame<F, Functor, Procedure> from(F f, LoopObject* loop = nullptr) { return Procedure(std::move(f), loop); }

		template<typename F> 
		using FuncIn = typename meta::FuncInfo<F>::ArgsType::FirstT;
		template<typename F>
		using FuncOut = typename meta::FuncInfo<F>::ResultType;
		template<typename D>
		using DataFunctor = typename std::enable_if<!std::is_pointer<D>::value, std::function<CommandResult(D)>>::type;
		template<typename T> 
		using NArgsFunctor = std::function<T()>;
		using VoidFunctor = std::function<void()>;

		template<typename F>
		static meta::FuncIfConvertible<F, DataFunctor<FuncIn<F>>, Procedure> from(F f, LoopObject* loop = nullptr) {
			return Procedure([=](CommandObject* c) noexcept -> CommandResult { 
				return f(c->template inputData<meta::_t_remove_rc<FuncIn<F>>>());
			}, loop);
		}
		template<typename F>
		static meta::FuncIfConvertible<F, DataFunctor<FuncIn<F>>, Procedure> from(F f, FuncIn<F> in, LoopObject* loop = nullptr) {
			return Procedure([=](CommandObject*) noexcept { return f(in); }, loop); 
		}
		template<typename F>
		static meta::FuncIfSame<F, NArgsFunctor<typename meta::FuncInfo<F>::ResultType>, 
			typename std::enable_if<!std::is_same<typename meta::FuncInfo<F>::ResultType, void>::value, Procedure>::type> from(F f, LoopObject * loop = nullptr) { 
			return Procedure([=](CommandObject*) noexcept -> CommandResult { return f(); }, loop);
		}
		template<typename F>
		static meta::FuncIfSame<F, VoidFunctor, Procedure> from(F f, LoopObject* loop = nullptr) {
			return Procedure([=](CommandObject*) noexcept { return f(); }, loop);
		}

		Functor functor() const { return functor_; }
		LoopObject* loop() const { return loop_; }

		CommandResult run(CommandObject* c) const;
	private:
		Functor functor_;
		LoopObject* loop_;
		bool blocked_;
	};
	typedef std::function<void(const CommandResult&)> CommandResultHandler;
public:
	explicit CommandObject(const std::string& name);
	virtual ~CommandObject();
	CommandObject* clone() const;

	ObjectManager& getDataMgr();

	template<typename T>
	T getData(const std::string& name, bool* ok = nullptr) {
		TDataObject<T>* _td = mgr_.get<TDataObject<T>>(name); if (ok) *ok = (_td != nullptr); return _td ? _td->get() : T();
	}
	template<typename T>
	T getData(const std::string& name, const T default_t, bool* ok = nullptr) {
		TDataObject<T>* _td = mgr_.get<TDataObject<T>>(name); if (ok) *ok = (_td != nullptr); return _td ? _td->get() : default_t;
	}
	template<typename T>
	bool setData(const std::string& name, const T& d) { return mgr_.add(new TDataObject<T>(name, d), true); }
	template<typename T>
	T inputData() const { return this->getData<T>(MFLOW_COMMAND_INPUT); }
	template<typename T>
	bool setInputData(const T& d) { return setData(MFLOW_COMMAND_INPUT, d); }

	void setResultHandler(const CommandResultHandler& handler);

	CommandObject* prependProc(const Procedure& p);
	CommandObject* appendProc(const Procedure& p);

	CommandResult start(bool auto_delete = true);

	void report(const CommandResult& result);
	void finish(const CommandResult& result, bool auto_delete = true);
private:
	ObjectManager mgr_;
	CommandResultHandler handler_;
	mflow::container::List<Procedure> procedure_list_;
};

namespace command {
ObjectManager& mgr();
CommandObject* copy(const std::string& name);
CommandObject* copy(const std::string& name, const CommandObject::CommandResultHandler& handler);
bool registerCommandObject(Module* context, const std::string& key, CommandObject* c, bool delete_if_failed = false);
}

}

#endif // !MFLOW_BASIC_CMD_MANAGER_DEF_H
