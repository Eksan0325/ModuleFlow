/**
* @file object.cpp
* @brief basical Object impl of ModuleFlow Framework
* @author Eksan0325
*/

#include "basic/object.h"



namespace mflow {

class Object::PrivateImpl {
public:
	std::string name_;
	Object* parent_ = nullptr;
	container::HashMap<int, container::HashMap<Object*, container::List<Task>>> connections_;
};

Object::Object(const std::string& name) : pimpl_(new PrivateImpl) {
	pimpl_->name_ = name.empty() ? MFLOW_UNDEFINED_OBJECT_NAME : name;
}

Object::~Object() {
	emit(DELETE);
	if (auto _mgr = dynamic_cast<ObjectManager*>(pimpl_->parent_)) _mgr->remove(this, false);
	delete pimpl_;
}

Object* Object::parent() const { return pimpl_->parent_; }
void Object::setParent(Object* p) { pimpl_->parent_ = p; }
std::string Object::name() const { return pimpl_->name_; }

bool Object::hasConnection(int signal, Object* observer) {
	return pimpl_->connections_.has(signal) && pimpl_->connections_[signal].has(observer);
}
void Object::connect(int signal, Object* observer, const Task& action) {
	if (!pimpl_->connections_.has(signal)) {
		pimpl_->connections_[signal] = { {observer, {action}} };
	} else if (!pimpl_->connections_[signal].has(observer)) {
		pimpl_->connections_[signal][observer] = { action };
	} else {
		pimpl_->connections_[signal][observer].push_back(action);
	}
	if (observer && observer != this && signal != Signal::DELETE) {
		observer->connect(DELETE, this, [=]() noexcept { disconnect(observer); });
		this->connect(DELETE, observer, [=]() noexcept { observer->disconnect(this); });
	}
}

void Object::disconnect(int signal, Object* observer) {
	if (pimpl_->connections_.has(signal)) pimpl_->connections_[signal].erase(observer);
}
void Object::disconnect(Object* observer) {
	for (auto& kv : pimpl_->connections_) kv.second.erase(observer);
}
void Object::emit(int signal) {
	if (!pimpl_->connections_.has(signal)) return;
	auto _observer_map_copy = pimpl_->connections_[signal]; //task maybe will call disconnect, needed to use copy
	for (auto& kv : _observer_map_copy) {
		if (auto obj = kv.first) (void)obj; // @todo async task queue maybe.
		for (auto& func : kv.second) func();
	}
}

ObjectManager::ObjectManager(const std::string& name) : Object(name) {}

ObjectManager::~ObjectManager() {
	for (auto& kv : *this) {
		Object* obj = kv.second;
		obj->setParent(nullptr);
		delete obj;
	}
}

Object* ObjectManager::add(Object* obj, bool delete_if_failed_add) {
	if (!obj) return nullptr;
	if (this->has(obj->name())) {
		if (delete_if_failed_add) delete obj;
		return nullptr;
	}
	obj->setParent(this);
	(*this)[obj->name()] = obj;
	return obj;
}

bool ObjectManager::remove(Object* obj, bool auto_delete) {
	if (!obj) return false;
	bool _find_iterator = erase(obj->name()) > 0;
	if (!_find_iterator) {
		for (const auto& kv : *this) {
			if (obj == kv.second) {
				_find_iterator = erase(kv.first) > 0;
				break;
			}
		}
	}
	if (_find_iterator) {
		obj->setParent(nullptr);
		if (auto_delete) delete obj;
	}
	return _find_iterator;
}

bool ObjectManager::remove(const std::string& name, bool auto_delete) {
	Object* obj = get(name);
	if (!obj || erase(name) == 0) return false;
	obj->setParent(nullptr);
	if (auto_delete) delete obj;
	return true;
}

Object* ObjectManager::get(const std::string& name) const {
	auto iterator = find(name); return iterator == end() ? nullptr : iterator->second;
}
void ObjectManager::fix() {
	for (auto& kv : *this) { kv.second->pimpl_->name_ = kv.first; kv.second->setParent(this); };
}

}