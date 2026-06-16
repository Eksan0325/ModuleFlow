/**
* @file base.h
* @brief basical core of ModuleFlow Framework
* @author Eksan0325
*/

#ifndef MFLOW_BASIC_BASE_HPP
#define MFLOW_BASIC_BASE_HPP

#include <type_traits>
#include <functional>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <string>
#include <cxxabi.h>

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <ostream>
#include <cstdlib>

#define MFLOW_PRIVATE_DECLARE(type, name) \
	private: \
		type name; \
	public: \
		const type& get_##name() const { return name; } \
		void set_##name(const type& value) { name = value; }
#define MFLOW_PRIVATE_DECLARE_MOVE(type, name) \
	private: \
		type name; \
	public: \
		const type& get_##name() const { return name; } \
		void set_##name(type&& value) { name = std::move(value); }
#define MFLOW_PROTECTED_DECLARE(type, name) \
	protected: \
		type name; \
	public: \
		const type& get_##name() const { return name; } \
		void set_##name(const type& value) { name = value; }

#define MFLOW_PIMPL_POINTER_DECLARE protected: class PrivateImpl; PrivateImpl* pimpl_;
#define MFLOW_INHERIT_CONSTRUCTORS_IMPL(CONSTRUCTOR, CLASS, ...) \
public: \
using __VA_ARGS__::CONSTRUCTOR; \
CLASS() : __VA_ARGS__() {} \
CLASS(const __VA_ARGS__& other) noexcept : __VA_ARGS__(other) {} \
CLASS(__VA_ARGS__&& other) noexcept : __VA_ARGS__(std::move(other)) {}

namespace mflow {
namespace meta {

/*
* @note enable_if_t and enable_if_void for C++14, and _void_t for SFINAE, and this project based on C++11, so we need to define them by ourselves.
* @see https://en.cppreference.com/w/cpp/types/enable_if
*/
template<bool B, typename T>
using enable_if_t = typename std::enable_if<B, T>::type;
template<bool B>
using enable_if_void = typename std::enable_if<B, void>::type;
template<typename ...>
using _void_t = void;
/*
* @note is_comparable for SFINAE, to check if the type can be compared by operator==, and this project based on C++11, so we need to define it by ourselves.
*/
template<typename L, typename R, typename = void>
struct is_comparable : std::false_type {};
template<typename L, typename R>
using _t_comparability = decltype(std::declval<L>() == std::declval<R>());
template<typename L, typename R>
struct is_comparable<L, R, _void_t<_t_comparability<L, R>>> : std::true_type {};
/*
* @note equals for SFINAE, to check if the type can be compared by operator==, and this project based on C++11, so we need to define it by ourselves.
*/
template<typename T> inline static typename std::enable_if<is_comparable<T, T>::value, bool>::type 
equals(const T& t1, const T& t2) { return t1 == t2; }
template<typename T> inline static typename std::enable_if<!is_comparable<T, T>::value, bool>::type 
equals(const T& t1, const T& t2) { return false; }
/*
* @note _t_list for type list, to store a list of types.
*/
template<typename ...>
struct _t_list { typedef void FirstT; };
template<typename T0, typename... Ts>
struct _t_list<T0, Ts...> { typedef T0 FirstT; typedef _t_list<Ts...> RestT; };
template<typename T0, typename T1, typename... Ts>
struct _t_list<T0, T1, Ts...> { typedef T0 FirstT; typedef T1 SecondT; typedef _t_list<Ts...> RestT; };

template<typename...> 
struct _t_contains : std::false_type {};
template<typename T0, typename T1, typename... T> 
struct _t_contains<T0, T1, T...> : std::integral_constant<bool, std::is_same<T0, T1>::value || _t_contains<T0, T...>::value> {};

/*
* @note FuncInfo for function traits, 
* to get the return type and argument types of a function.
*/

template<typename T> static decltype(&T::operator()) _t_functional(int) { return &T::operator(); }
template<typename T> static void _t_functional(short) {}

template<typename Func>
struct FuncInfo;

template<>
struct FuncInfo<void> { enum { isFunction = false }; };

template<typename Func>
struct FuncInfo : public FuncInfo<decltype(_t_functional<Func>(0))> {};

template<typename Result, typename... Args>
struct FuncInfo<Result(*)(Args...)> {
	typedef Result ResultType; typedef _t_list<Args...> ArgsType; typedef Result(*FuncPointerType)(Args...); typedef Result(*FuncType)(Args...); 
	typedef std::function<Result(Args...)> FunctionStdType;
	enum { isFunction = true, isMember = false, ArgsCount = sizeof...(Args) };
};
template<typename Result, typename... Args>
struct FuncInfo<std::function<Result(Args...)>> {
	typedef Result ResultType; typedef _t_list<Args...> ArgsType; typedef Result(*FuncPointerType)(Args...); typedef Result(*FuncType)(Args...); 
	typedef std::function<Result(Args...)> FunctionStdType;
	enum { isFunction = true, isMember = false, ArgsCount = sizeof...(Args) };
};
template<typename Result, typename Class, typename... Args>
struct FuncInfo<Result(Class::*)(Args...)> {
	typedef Result ResultType; typedef Class ClassType; typedef _t_list<Args...> ArgsType; typedef Result(*FuncPointerType)(Args...); typedef Result(Class::* FuncType)(Args...);
	typedef std::function<Result(Args...)> FunctionStdType;
	enum { isFunction = true, isMember = true, ArgsCount = sizeof...(Args) };
};
template<typename Result, typename Class, typename... Args>
struct FuncInfo<Result(Class::*)(Args...) const> {
	typedef Result ResultType; typedef Class ClassType; typedef _t_list<Args...> ArgsType; typedef Result(*FuncPointerType)(Args...); typedef Result(Class::* FuncType)(Args...) const;
	typedef std::function<Result(Args...)> FunctionStdType;
	enum { isFunction = true, isMember = true, ArgsCount = sizeof...(Args) };
};

template<typename T>
using _t_remove_rc = typename std::remove_const<typename std::remove_reference<T>::type>::type;

std::string _t_demangle(const char* name) {
	int status = -4; std::string _result = "";
	char* _demangled = abi::__cxa_demangle(name, NULL, NULL, &status);
	if (status != 0 || _demangled == nullptr) _result = name; else _result = _demangled;
	std::free(_demangled);
	return _result;
}

template<typename T>
struct MetaType {
	typedef T TypeT;
	static const char* typeInfoName() { return typeid(T).name(); }
	static std::string typeName() { return _t_demangle(typeid(T).name()); }
};

template<typename F1, typename F2, typename Type>
using FuncIfSame = typename std::enable_if<std::is_same<typename FuncInfo<F1>::FuncPointerType, typename FuncInfo<F2>::FuncPointerType>::value, Type>::type;
template<typename F1, typename F2, typename Type>
using FuncIfConvertible = typename std::enable_if<std::is_convertible<F1, F2>::value && std::is_constructible<typename FuncInfo<F2>::FuncType, F1>::value, Type>::type;
}
namespace atomic {
template<typename T> class AtomicData {
private:
	using Type = mflow::meta::_t_remove_rc<T>;
public:
	AtomicData() : data_() {}
	AtomicData(const Type& D) : data_(D) {}
	AtomicData(Type&& D) : data_(std::move(D)) {}
	AtomicData(const AtomicData& other) { data_ = other.data_.load(); }

	inline Type AtomicDataLoad() const { return data_.load(); }
	inline void AtomicDataStore(const Type& D) { data_ = (D); }
private:
	std::atomic<Type> data_;
};

template<typename T> class MutexData {
private:
	using Type = mflow::meta::_t_remove_rc<T>;
	using Lock = std::lock_guard<std::mutex>;
public:
	MutexData() : data_() {}
	MutexData(const Type& D) : data_(D) {}
	MutexData(Type&& D) : data_(std::move(D)) {}
	MutexData(const MutexData& other) { Lock _(mtx_); data_ = other.data_; }

	inline Type MutexDataLoad() const { Lock _(mtx_); return data_; }
	inline void MutexDataStore(const Type& D) { Lock _(mtx_); data_ = (D); }

private:
	Type data_;
	std::mutex mtx_;
};
}
namespace container {
template<typename DerivedT, typename ValueT>
class PrivateTContainerBase {
public:
	using _list_likely = std::true_type;
protected:
	inline const DerivedT* dptr() const { return static_cast<const DerivedT*>(this); }
	inline DerivedT* dptr() { return static_cast<DerivedT*>(this); }
public:
	template<typename Foreach> DerivedT& foreach(Foreach&& func) {
		auto* _dptr = dptr(); std::for_each(_dptr->begin(), _dptr->end(), std::forward<Foreach>(func)); return *_dptr;
	}
	template<typename Foreach> const DerivedT& foreach(Foreach&& func) const {
		auto* _dptr = dptr(); std::for_each(_dptr->cbegin(), _dptr->cend(), std::forward<Foreach>(func)); return *_dptr;
	}
	int size() const { return static_cast<int>(dptr()->size()); }
	bool has(int index) const { return index >= 0 && index < this->size(); }
	ValueT value(int index, const ValueT& default_value) const { 
		return this->has(index) ? dptr()->operator[](index) : default_value; }
	const ValueT& valueref(int index, const ValueT& default_value) const { 
		return this->has(index) ? dptr()->operator[](index) : default_value; }
	DerivedT& insert(int index, const ValueT& value) {
		auto* _dptr = dptr(); _dptr->insert(_dptr->begin() + index, value); return *_dptr; }
	DerivedT& insert(int index, ValueT&& value) {
		auto* _dptr = dptr(); _dptr->insert(_dptr->begin() + index, std::move(value)); return *_dptr; }
	DerivedT& prepend(const ValueT& value) {
		auto* _dptr = dptr(); _dptr->insert(_dptr->begin(), value); return *_dptr; }
	DerivedT& prepend(ValueT&& value) {
		auto* _dptr = dptr(); _dptr->insert(_dptr->begin(), std::move(value)); return *_dptr; }
	DerivedT& append(const ValueT& value) {
		auto* _dptr = dptr(); _dptr->push_back(value); return *_dptr; }
	DerivedT& append(ValueT&& value) {
		auto* _dptr = dptr(); _dptr->push_back(std::move(value)); return *_dptr; }
};

template<typename T> class Vector : public std::vector<T>, public PrivateTContainerBase<Vector<T>, T> {
	MFLOW_INHERIT_CONSTRUCTORS_IMPL(vector, Vector, std::vector<T>)
};
template<typename T> class List : public std::list<T>, public PrivateTContainerBase<List<T>, T> {
	MFLOW_INHERIT_CONSTRUCTORS_IMPL(list, List, std::list<T>)
};

template<typename DerivedT, typename KeyT, typename ValueT>
class PrivateKVContainerBase {
public:
	using _map_likely = std::true_type;
	using _dict_likely = std::integral_constant<bool, std::is_convertible<KeyT, std::string>::value>;
protected:
	inline const DerivedT* dptr() const { return static_cast<const DerivedT*>(this); }
	inline DerivedT* dptr() { return static_cast<DerivedT*>(this); }

	inline void fromMultiKV(const Vector<KeyT>& keys, const Vector<ValueT>& values) {
		for (size_t i = 0; i < std::min(keys.size(), values.size()); ++i) static_cast<DerivedT*>(this)->operator[](keys[i]) = values[i];
	}

public:
	int size() const { return static_cast<int>(dptr()->size()); }
	bool has(const KeyT& key) const { return dptr()->find(key) != dptr()->end(); }

	Vector<KeyT> kyes() const {
		Vector<KeyT> _keys; _keys.reserve(dptr()->size()); 
		for (const auto& kv : *dptr()) _keys.push_back(kv.first); 
		return _keys;
	}
	Vector<ValueT> values() const {
		Vector<ValueT> _values; _values.reserve(dptr()->size()); 
		for (const auto& kv : *dptr()) _values.push_back(kv.second); 
		return _values;
	}
	ValueT value(const KeyT& key) const { 
		const auto _iterator = dptr()->find(key); 
		return _iterator == dptr()->end() ? ValueT() : _iterator->second;
	}
	const ValueT& valueref(const KeyT& key, const ValueT& defualt_value) const { 
		const auto _iterator = dptr()->find(key); 
		return _iterator == dptr()->end() ? defualt_value : _iterator->second;
	}
	DerivedT& insertOne(const KeyT& key, const ValueT& value) {  auto* _dptr = dptr(); _dptr->operator[](key) = value; return *_dptr;}
	DerivedT& insertOne(const KeyT& key, ValueT&& value) {  auto* _dptr = dptr(); _dptr->operator[](key) = std::move(value); return *_dptr; }
};

template<typename K, typename V>
class Map : public std::map<K, V>, public PrivateKVContainerBase<Map<K, V>, K, V> {
	MFLOW_INHERIT_CONSTRUCTORS_IMPL(map, Map, std::map<K, V>)
	Map(const Vector<K>& keys, const Vector<V>& values) : std::map<K, V>() { this->fromMultiKV(keys, values); }
};

template<typename K, typename V>
class HashMap : public std::unordered_map<K, V>, public PrivateKVContainerBase<HashMap<K, V>, K, V> {
	MFLOW_INHERIT_CONSTRUCTORS_IMPL(unordered_map, HashMap, std::unordered_map<K, V>)
	HashMap(const Vector<K>& keys, const Vector<V>& values) : std::unordered_map<K, V>() { this->fromMultiKV(keys, values); }
};

}

}

#define MFLOW_CONTAINER_T_OUTSTREAM_DECL(CONTAINER) \
template<typename T> std::ostream& operator<<(std::ostream& os, const CONTAINER<T>& container)
#define MFLOW_CONTAINER_KV_OUTSTREAM_DECL(CONTAINER) \
template<typename K, typename V> std::ostream& operator<<(std::ostream& os, const CONTAINER<K, V>& container)
#define MFLOW_CONTAINER_OUTSTREAM_IMPL(BEGIN, END, ...) \
os << BEGIN; \
size_t _cnt = 0; \
for (const auto& _iterator : container) { os << __VA_ARGS__; if (++_cnt != container.size()) os << ", "; } \
os << END; \
return os;
MFLOW_CONTAINER_T_OUTSTREAM_DECL(mflow::container::Vector) { MFLOW_CONTAINER_OUTSTREAM_IMPL("[", "]", _iterator); }
MFLOW_CONTAINER_T_OUTSTREAM_DECL(mflow::container::List) { MFLOW_CONTAINER_OUTSTREAM_IMPL("[", "]", _iterator); }
MFLOW_CONTAINER_KV_OUTSTREAM_DECL(mflow::container::Map) { MFLOW_CONTAINER_OUTSTREAM_IMPL("{", "}", _iterator.first << ": " << _iterator.second); }
MFLOW_CONTAINER_KV_OUTSTREAM_DECL(mflow::container::HashMap) { MFLOW_CONTAINER_OUTSTREAM_IMPL("{", "}", _iterator.first << ": " << _iterator.second); }
#endif
