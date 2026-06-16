/**
* @file object.h
* @brief basical core of ModuleFlow Framework
* @author Eksan0325
*/
#ifndef MFLOW_BASIC_OBJECT_H
#define MFLOW_BASIC_OBJECT_H

#include "basic/base.h"
#define MFLOW_UNDEFINED_OBJECT_NAME "@undefined"
namespace mflow {

/**
* @brief The Simple Type of Task and Condition Declaration in here,
* @todo  No typedef forward declaration for function type, need to support any function symbol.
*/
typedef std::function<void()> Task;
typedef std::function<bool()> Condition;

/**
* @brief The base class of all objects in ModuleFlow Framework
* @note likely QObject in Qt Framework, but we don't need to use meta-object system, so we just provide a base class for all objects in ModuleFlow Framework, 
* and we can use it to implement some common functionalities for all objects in ModuleFlow Framework.
* also support simple Signal-Slot
*/
class Object {
	MFLOW_PIMPL_POINTER_DECLARE
public:
	explicit Object(const std::string& name = "");
	virtual ~Object();

	Object* parent() const;
	void setParent(Object * p);
	virtual std::string name() const;

	enum Signal : int { DELETE = 0x00ff };
	
	bool hasConnection(int signal, Object* observer);
	void connect(int signal, Object* observer, const Task& action);
	void disconnect(int signal, Object* observer);
	void disconnect(Object* observer);

	void emit(int signal);
private:
	friend class ObjectManager;
};

class ObjectManager : public Object, public container::HashMap<std::string, Object*> {
public:
	explicit ObjectManager(const std::string& name);
	virtual ~ObjectManager();

	Object* add(Object* obj, bool delete_if_failed_add = false);
	template<typename SubObject> 
	meta::enable_if_t<std::is_base_of<Object, SubObject>::value, SubObject*> add(SubObject* obj, bool delete_if_failed_add = false) {
		return add(static_cast<Object*>(obj), delete_if_failed_add) ? obj : nullptr; 
	}
	bool remove(Object* object, bool auto_delete = true);
	bool remove(const std::string& name, bool auto_delete = true);

	Object* get(const std::string& name) const;
	template<typename SubObject>
	meta::enable_if_t<std::is_base_of<Object, SubObject>::value, SubObject*> get(const std::string& name) const {
		return static_cast<SubObject*>(get(name));
	}
	/**
	* @brief fix() will be fix object's keys map and parent tree.
	* use when need to sync all object states.
	*/
	void fix();
};

template<typename Signature>
class Creator : public Object, public container::HashMap <std::string, std::function<Signature>> {
public:
	explicit Creator(const std::string& name) : Object(name) {}
	virtual ~Creator() {}

	typedef meta::FuncInfo<std::function<Signature>> FuncInfoT;
	typedef typename FuncInfoT::ResultType RetT;
	
	template<typename... Args>
	RetT exec(const std::string& key, Args... args) { return this->has(key) ? (*this)[key](args...) : (RetT)(NULL); }
	RetT exec(const std::string& key) { return this->has(key) ? (*this)[key]() : (RetT)(NULL); }
};

}

#endif // !MFLOW_BASIC_OBJECT_H
