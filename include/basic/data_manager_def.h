/**
 * @file data_manager_def.h
 * @brief global data storage
 * @author Eksan0325
 */
#ifndef MFLOW_BASIC_DATA_MANAGER_DEF_H
#define MFLOW_BASIC_DATA_MANAGER_DEF_H

#include <jsoncpp/json.h>

#include "basic/base.h"
#include "basic/object.h"

namespace mflow {

namespace atomic {
template<typename T> class AtomicData;
template<typename T> class MutexData;
}

struct ControllableFlag {
	enum Flags : int { READABLE = 0x0001, WRITEABLE = 0x0002};
	template<Flags F> inline bool flag() const { return flags_ | F; }
	template<Flags F> inline void setFlag(bool B) { flags_ = B ? flags_ | F : flags_ & !F; }

	virtual int beforeRead() { return 0; }
	virtual int afterWrite() { return 0; }

	int flags_ = !(0);
	int ctrl_ = 0;
};

template<typename T> class TemplateDataInterface {
	using DataType = meta::_t_remove_rc<T>;
public:
	typedef typename meta::MetaType<T> Meta_T;
	TemplateDataInterface() {}
	TemplateDataInterface(const DataType& t) : data_(t) {}
	TemplateDataInterface(DataType&& t) : data_(t) {}
	
	const DataType& get() const { return data_; };
	DataType& ref() { return data_; }
	DataType* ptr() { return &data_; }
	void set(const DataType& t) { data_ = t; }
	void set(DataType&& t) { data_ = std::move(t); }
protected:
	DataType data_;
};
template<> class TemplateDataInterface<void> {
public:
	TemplateDataInterface() {}
	typedef typename meta::MetaType<void> Meta_T;
};
class JsonRefrence {
public:
	JsonRefrence(const Json::Value& value);
	JsonRefrence(Json::Value&& value);
	virtual ~JsonRefrence();

	const Json::Value& value() const;
	explicit operator bool() const;

	size_t size() const;
	int arraySize() const;
	int objectSize() const;

	container::Vector<std::string> objectKeys() const;

	bool has(int index) const;
	bool has(const std::string& key) const;

	JsonRefrence at(int index, bool* founded = nullptr) const;
	JsonRefrence at(const std::string& key, bool use_index = false, bool* founded = nullptr) const;
	JsonRefrence get(const std::string& key, bool* founded = nullptr) const;

	JsonRefrence operator[](int index) const;
	JsonRefrence operator[](const std::string& key) const;

	bool toBool(bool default_bool = false, bool* ok = nullptr) const;
	int  toInt(int default_int = 0, bool* ok = nullptr) const;
	unsigned int toUint(unsigned int default_uint = 0, bool* ok = nullptr) const;
	double toDouble(double default_double = 0.0, bool* ok = nullptr) const;
	std::string toString(const std::string& default_str = "", bool* ok = nullptr) const;

	template<typename T> T toT(bool* ok = nullptr) const;
	template<typename T> T toT(T& t, bool* ok = nullptr) const { return t = toT<T>(ok); }

	double operator()(int index) const;
	double operator()(const std::string& key) const;
private:
	TemplateDataInterface<Json::Value>* holder_;
	const Json::Value* jptr_;
};

struct JsonInterface {
	virtual Json::Value serialize() const;
	virtual bool deserialize(const JsonRefrence& value);
};

class DataObject : public Object, public ControllableFlag {
public:
	enum Signal {
		CHANGED = 0x0200,
		BEFORE_GET = 0x0201,
		AFTER_SET = 0x0202
	};
public:
	explicit DataObject(const std::string& name = "");
	virtual ~DataObject();
	virtual std::string dataTypeName() const;
};
using DataObjectPointer = std::shared_ptr<DataObject>;

template<typename T>
class TDataObject : public DataObject, public TemplateDataInterface<T> {
	using DataType = meta::_t_remove_rc<T>;
public:
	typedef std::function<DataType()> SourceFunction;
	typedef std::function<int(DataType&)> ValidatorFunction;
public:
	explicit TDataObject(const std::string& name = "") : DataObject(name) {}
	TDataObject(const std::string& name, const DataType& t) : DataObject(name), TemplateDataInterface<T>(t) {}
	TDataObject(const std::string& name, DataType&& t) : DataObject(name), TemplateDataInterface<T>(std::move(t)) {}

	std::string dataTypeName() const override { return TemplateDataInterface<T>::Meta_T::typeName(); }

	int beforeRead() override { ctrl_ = 0; emit(BEFORE_GET); return ctrl_; }
	int afterWrite() override { emit(AFTER_SET); return ctrl_; }
	TDataObject<T>* readonly() { setFlag<WRITEABLE>(false); return this; }

	void update() { emit(CHANGED); }
	void update(const DataType& data) { this->set(data); this->update(); }
	void update(DataType&& data) { this->set(std::move(data)); this->update(); }

	TDataObject<T>* link(DataType* dt_ptr) {
		disconnect(BEFORE_GET, this);
		disconnect(AFTER_SET, this);
		if (dt_ptr) {
			connect(BEFORE_GET, this, [=]() noexcept { this->set(*dt_ptr); });
			connect(AFTER_SET, this, [=]() noexcept { *dt_ptr = this->get(); });
		}
		return this;
	}
	TDataObject<T>* link(SourceFunction&& from_function = NULL, ValidatorFunction&& to_function = NULL) {
		disconnect(BEFORE_GET, this);
		disconnect(AFTER_SET, this);
		if (from_function) connect(BEFORE_GET, this, [=]() noexcept { this->set(from_function()); });
		if (to_function) connect(AFTER_SET, this, [=]() noexcept { this->ctrl_ = to_function(this->ref()); });
		return this;
	}
};
template<>
class TDataObject<void> : public DataObject, public TemplateDataInterface<void> {
public:
	explicit TDataObject(const std::string& name = "") : DataObject(name) {}
	std::string dataTypeName() const override { return TemplateDataInterface<void>::Meta_T::typeName(); }
	void update() { emit(CHANGED); }
};

namespace standard {

template<typename T, class = void>
struct JsonTraits {
	enum { VALID = false };
	using FromInvalid = bool;
	using ToInvalid = Json::Value;
};
template<typename T>
struct JsonTraits<T, meta::enable_if_void<std::is_base_of<JsonInterface, T>::value>> { 
	enum { VALID = true };
	typedef bool FromSerializable;
	typedef Json::Value ToSerializable;
};
template<typename T>
struct JsonTraits<T, meta::enable_if_void<std::is_convertible<T, DataObject*>::value>> {
	enum { VALID = true};
	typedef bool FromObject;
	typedef Json::Value ToObject;
};
template<typename T>
struct JsonTraits<T, meta::enable_if_void < std::is_convertible<T, std::string>::value>> {
	enum { VALID = true };
	typedef bool FromStdString;
	typedef Json::Value ToStdString;
};
template<typename T>
struct JsonTraits<T, meta::enable_if_void<(!std::is_pointer<T>::value&& std::is_convertible<T, Json::Value>::value)>> {
	enum { VALID = true };
	typedef bool FromBasic;
	typedef Json::Value ToBasic;
};
template<typename T>
struct JsonTraits<atomic::AtomicData<T>, meta::enable_if_void < std::is_convertible<T, std::string>::value>> {
	enum { VALID = true };
	typedef bool FromStdString;
	typedef Json::Value ToStdString;
};
template<typename T>
struct JsonTraits<atomic::MutexData<T>, meta::enable_if_void < std::is_convertible<T, std::string>::value>> {
	enum { VALID = true };
	typedef bool FromStdString;
	typedef Json::Value ToStdString;
};
template<typename T>
struct JsonTraits<atomic::AtomicData<T>, meta::enable_if_void<(!std::is_pointer<T>::value&& std::is_convertible<T, Json::Value>::value)>> {
	enum { VALID = true };
	typedef bool FromBasic;
	typedef Json::Value ToBasic;
};
template<typename T>
struct JsonTraits<atomic::MutexData<T>, meta::enable_if_void<(!std::is_pointer<T>::value&& std::is_convertible<T, Json::Value>::value)>> {
	enum { VALID = true };
	typedef bool FromBasic;
	typedef Json::Value ToBasic;
};
template<typename T>
struct JsonTraits<T, meta::enable_if_void<T::_list_likely::value>> {
	enum { VALID = true };
	typedef bool FromList;
	typedef Json::Value ToList;
};
template<typename T>
struct JsonTraits<T, meta::enable_if_void<T::_map_likely::value>>
{
	enum { VALID = true };
	typedef bool FromMap;
	typedef Json::Value ToMap;
};

bool fromJson(const JsonRefrence& jref, DataObject* const d);
Json::Value toJson(DataObject* const d);
bool fromJson(const JsonRefrence& jref, DataObjectPointer dptr);
Json::Value toJson(DataObjectPointer dptr);

template<typename T>
typename JsonTraits<T>::FromInvalid inline fromJson(const JsonRefrence& jref, T& d) { return false; }

template<typename T>
typename JsonTraits<T>::ToInvalid inline toJson(const T& d) { return Json::Value(); }

template<typename T>
typename JsonTraits<T>::FromSerializable fromJson(const JsonRefrence& jref, T& d) { return d.deserialize(jref); }

template<typename T>
typename JsonTraits<T>::ToSerializable toJson(const T& d) { return d.serialize(); }

template<typename T>
typename JsonTraits<T>::FromStdString inline fromJson(const JsonRefrence& jref, T& d) { bool ok = false; jref.toT(d, &ok); return ok; }

template<typename T>
typename JsonTraits<T>::ToStdString inline toJson(const T& d) { return Json::Value(d); }

template<typename T>
typename JsonTraits<T>::FromBasic inline fromJson(const JsonRefrence& jref, T& d) { bool ok = false; jref.toT(d, &ok); return ok; }

template<typename T>
typename JsonTraits<T>::ToBasic inline toJson(const T& d) { return Json::Value(d); }

template<typename T>
typename JsonTraits<T>::FromStdString inline fromJson(const JsonRefrence& jref, atomic::AtomicData<T>& d) {
	bool ok = false;
	d.AtomicDataStore(jref.toT<T>(&ok));
	return ok;
}

template<typename T>
typename JsonTraits<T>::ToStdString inline toJson(const atomic::AtomicData<T>& d) {
	return Json::Value(const_cast<atomic::AtomicData<T>&>(d).AtomicDataLoad());
}

template<typename T>
typename JsonTraits<T>::FromStdString inline fromJson(const JsonRefrence& jref, atomic::MutexData<T>& d) {
	bool ok = false;
	d.MutexDataStore(jref.toT<T>(&ok));
	return ok;
}

template<typename T>
typename JsonTraits<T>::ToStdString inline toJson(const atomic::MutexData<T>& d) {
	return Json::Value(const_cast<atomic::MutexData<T>&>(d).MutexDataLoad());
}

template<typename T>
typename JsonTraits<T>::FromBasic inline fromJson(const JsonRefrence& jref, atomic::AtomicData<T>& d) {
	bool ok = false;
	d.AtomicDataStore(jref.toT<T>(&ok));
	return ok;
}

template<typename T>
typename JsonTraits<T>::ToBasic inline toJson(const atomic::AtomicData<T>& d) {
	return Json::Value(const_cast<atomic::AtomicData<T>&>(d).AtomicDataLoad());
}

template<typename T>
typename JsonTraits<T>::FromBasic inline fromJson(const JsonRefrence& jref, atomic::MutexData<T>& d) {
	bool ok = false;
	d.MutexDataStore(jref.toT<T>(&ok));
	return ok;
}

template<typename T>
typename JsonTraits<T>::ToBasic inline toJson(const atomic::MutexData<T>& d) {
	return Json::Value(const_cast<atomic::MutexData<T>&>(d).MutexDataLoad());
}

template<typename T>
typename JsonTraits<T>::FromList inline fromJson(const JsonRefrence& jref, T& list) {
	if (jref.arraySize() < 0) return false;
	size_t _cnt = 0;
	if (jref.arraySize() != list.sizeInt()) {
		if (!(std::is_constructible<typename T::value_type>::value && !std::is_pointer<typename T::value_type>::value)) return false;
		list.resize(jref.arraySize());
	}
	for (auto& _iterator : list) { if (!fromJson(jref.at(_cnt++), _iterator)) { return false; } }
	return true;
}

template<typename T>
typename JsonTraits<T>::ToList inline toJson(const T& list) {
	Json::Value result(Json::arrayValue);
	result.resize(list.size());
	int _cnt = 0;
	for (const auto& _iterator : list) result[_cnt++] = toJson(_iterator);
	return result;
}

template<typename T>
typename JsonTraits<T>::FromMap inline fromJson(const JsonRefrence& jref, T& map) {
	for (auto& kv : map) { 
		if (!fromJson(jref.at(kv.first), kv.second))  return false; 
	}
	return true;
}

template<typename T>
typename JsonTraits<T>::ToMap inline toJson(const T& map) {
	Json::Value result;
	for (const auto& kv : map) result[kv.first] = toJson(kv.second);
	return result;
}

template<typename T>
inline T fromJson(const JsonRefrence& jref) { T d; fromJson(jref, d); return d; }

}

template<typename T>
class StandardDataObject : public TDataObject<T>, public JsonInterface {
public:
	using TDataObject<T>::TDataObject;
	virtual Json::Value serialize() const override { return standard::toJson(this->get()); }
	virtual bool deserialize(const JsonRefrence& jref) override { return standard::fromJson(jref, this->ref()); }
};
template<>
class StandardDataObject<void> : public TDataObject<void>, public JsonInterface {
public:
	using TDataObject<void>::TDataObject;
};
namespace custom {
template<typename T> bool fromJson(const JsonRefrence& jref, T& d);
template<typename T> Json::Value toJson(const T& d);
}
template<typename T>
class UserDefineDataObject : public StandardDataObject<T> {
public:
	typedef typename meta::FuncInfo<decltype(&custom::toJson<T>)>::FunctionStdType Serializer;
	typedef typename meta::FuncInfo<decltype(&custom::fromJson<T>)>::FunctionStdType Deserializer;
public:
	UserDefineDataObject(const std::string& name, const Serializer& s, const Deserializer& d) : StandardDataObject<T>(name), s_(s), d_(d) {}
	UserDefineDataObject(const std::string& name, const T& d) : StandardDataObject<T>(name, d), s_(&custom::toJson<T>), d_(&custom::fromJson<T>) {}
	UserDefineDataObject(const std::string& name, T&& d) : StandardDataObject<T>(name, std::move(d)), s_(&custom::toJson<T>), d_(&custom::fromJson<T>) {}

	Serializer serializer() const { return s_; }
	Deserializer deserializer() const { return d_; }

	Json::Value serialize() const override { return s_(this->get()); }
	bool deserialize(const JsonRefrence& jref) override { return d_ ? d_(jref, this->ref()) : false; }

private:
	Serializer s_;
	Deserializer d_;
};
template<>
class UserDefineDataObject<void> : public StandardDataObject<void> {
public:
	typedef std::function<Json::Value()> Serializer;
	typedef std::function<bool(const JsonRefrence&)> Deserializer;
public:
	UserDefineDataObject(const std::string& name, const Serializer& s, const Deserializer& d) : StandardDataObject<void>(name), s_(s), d_(d) {}

	Serializer serializer() const { return s_; }
	Deserializer deserializer() const { return d_; }

	Json::Value serialize() const override { return s_(); }
	bool deserialize(const JsonRefrence& jref) override { return d_ ? d_(jref) : false; }

private:
	Serializer s_;
	Deserializer d_;
};

namespace data {

ObjectManager& mgr();
DataObject* get(const std::string& path);
template<typename T>
TDataObject<T>* getT(const std::string& key) { return dynamic_cast<TDataObject<T>*>(get(key)); }

template<typename T>
inline TemplateDataInterface<T>* holder(DataObject* d) { 
	return dynamic_cast<TemplateDataInterface<T>*>(d); 
}
template<typename T>
inline TemplateDataInterface<T>* holder(const std::string& path) { 
	return holder<T>(get(path)); 
}
inline JsonInterface* json_holder(DataObject* d) {
	return dynamic_cast<JsonInterface*>(d);
}
inline JsonInterface* json_holder(const std::string& name) {
	return json_holder(get(name));
}

template<typename T, typename = void>
struct PrivateDataCreator {
	typedef StandardDataObject<T> NewData_T;
};
template<typename T>
struct PrivateDataCreator<T, meta::enable_if_void<std::is_convertible<T, DataObject*>::value>> {
	typedef std::remove_pointer<T> MoveData_T;
};
template<typename T>
struct PrivateDataCreator<T, meta::enable_if_void<std::is_convertible<T, const char*>::value>> {
	typedef StandardDataObject<std::string> NewData_T;
};
template<typename T>
struct PrivateDataCreator<T, meta::enable_if_void<!standard::JsonTraits<T>::VALID>> {
	typedef TDataObject<T> NewData_T;
};

template<typename T>
using StandardT = typename PrivateDataCreator<meta::_t_remove_rc<T>>::NewData_T;
template<typename T>
using UserDefineT = UserDefineDataObject<meta::_t_remove_rc<T>>;
template<typename T>
using AutoT = typename std::conditional<standard::JsonTraits<meta::_t_remove_rc<T>>::VALID, StandardT<T>, UserDefineT<T>>::type;

template<typename T>
inline StandardT<T>* from(T&& d, const std::string& name = "") {
	return new StandardT<T>(name, std::forward<T>(d));
}
template<typename T>
inline typename PrivateDataCreator<T>::MoveData_T* from(T d) { return d; }
template<typename T>
inline UserDefineT<T>* from_user_define(T&& d, const std::string& name = "") {
	return new UserDefineT<T>(name, d);
}
template<typename T>
inline meta::enable_if_t<!meta::FuncInfo<T>::isFunction, AutoT<T>*> create(const std::string& name = "", T&& d = T()) {
	return mgr().add(new AutoT<T>(name, std::forward<T>(d)), true);
}
template<typename SerializeFunc, typename T = meta::_t_remove_rc<typename meta::FuncInfo<SerializeFunc>::ArgsT::FirstT>>
inline UserDefineT<T>* create(const std::string& name, SerializeFunc sf, typename UserDefineT<T>::Deserializer df = NULL) {
	return mgr().add(new UserDefineT<T>(name, sf, df), true);
}
}

template<typename DerivedT, typename KeyT, template<typename> typename Pointer = std::shared_ptr>
struct PrivateDataContainerBase {
	template<typename T> using ValueT = data::AutoT<T>;

	template<typename T> inline static Pointer<ValueT<T>> make_data_ptr() {
		return Pointer<ValueT<T>>(new ValueT<T>);
	}
	template<typename T> inline static Pointer<ValueT<T>> make_data_ptr(T&& d) {
		return Pointer<ValueT<T>>(new ValueT<T>("", std::forward<T>(d)));
	}
	inline const DerivedT* dptr() const { return static_cast<const DerivedT*>(this); }
	inline DerivedT* dptr() { return static_cast<DerivedT*>(this); }

	DataObject* getDataObject(const KeyT& key) const {
		return dptr()->value(key).get();
	}
	template<typename T>
	ValueT<T>* getDataT(const KeyT& key) const { return dynamic_cast<ValueT<T>*>(getDataObject(key)); }
	template<typename T>
	ValueT<T>* insertDataT(const KeyT& key) { auto _nptr = make_data_ptr<T>(); dptr()->insertOne(key, _nptr); return _nptr.get(); }
	template<typename T>
	ValueT<T>* insertDataT(const KeyT& key, T&& d) { auto _nptr = make_data_ptr<T>(std::forward<T>(d)); dptr()->insertOne(key, _nptr); return _nptr.get(); }
	
	template<typename T>
	void getValue(const KeyT& key, T& d) const { d = getDataT<T>(key).get(); }
	template<typename T>
	const T& getValue(const KeyT& key) const { return getDataT<T>(key).get(); }
	template<typename T>
	void setValue(const KeyT& key, const T& d) const { getDataT<T>(key).set(d); }
	template<typename T>
	void setValue(const KeyT& key, T&& d) const { getDataT<T>(key).set(std::move(d)); }
	template<typename T>
	void replace(const KeyT& key, T&& d) { dptr()->operator[](key) = make_data_ptr(std::forward<T>(d)); }

};
class DataList : public container::Vector<DataObjectPointer>, public PrivateDataContainerBase<DataList, int> {
public:
	using container::Vector<DataObjectPointer>::Vector;

	template<typename T> 
	void append_raw(T&& d) { append(make_data_ptr(std::forward<T>(d))); }
	template<typename T, typename... Tn>
	void append_raw(T&& d, Tn&&... dn) { append(std::forward<T>(d)); append_raw(std::forward<T>(dn)...); }
	template<typename T>
	void append_empty() { append(make_data_ptr<T>()); }
	template<typename T, typename... Tn> 
	meta::enable_if_void<(sizeof...(Tn) >0)> append_empty() { append_empty<T>(); append_empty<Tn...>(); }
	
	template<typename... Tn> 
	static DataList from_raw(Tn&&... dn) {
		DataList _; _.reserve(sizeof...(dn)); _.append_raw(std::forward<Tn>(dn)...);
		return _;
	}
	template<typename ...Tn>
	static DataList from_raw() {
		DataList _; _.reserve(sizeof...(Tn)); _.append_empty<Tn...>(); 
		return _;
	}
};
class DataDict : public container::HashMap<std::string, DataObjectPointer>, public PrivateDataContainerBase<DataDict, std::string> {
public:
	using container::HashMap<std::string, DataObjectPointer>::HashMap;
	template<typename... Tn> 
	static DataDict fromRaw(const container::Vector<std::string>& keys, Tn... dn) { 
		return DataDict(keys, DataList::from_raw(dn...)); 
	};
};
#define MFLOW_DATA_QUICK_IMPL(...) auto _d = data::getT<T>(name); if (_d) __VA_ARGS__; return _d;
template<typename T>
TDataObject<T>* d_get(const std::string& name, T& d) { MFLOW_DATA_QUICK_IMPL(d = _d->get()); }
template<typename T> 
TDataObject<T>* d_set(const std::string& name, const T& d) { MFLOW_DATA_QUICK_IMPL(_d->set(d)); }
template<typename T> 
TDataObject<T>* d_update(const std::string& name, const T& d) { MFLOW_DATA_QUICK_IMPL(_d->update(d)); }

}

#endif // !MFLOW_BASIC_DATA_MANAGER_DEF_H
