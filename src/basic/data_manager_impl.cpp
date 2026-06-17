/**
 * @file data_manager_def.h
 * @brief global data storage
 * @author Eksan0325
 */
#include "basic/data_manager_def.h"
#include "basic/object.h"

namespace mflow {

Json::Value global_empty_json_ptr;

JsonRefrence::JsonRefrence(const Json::Value& value) : holder_(nullptr), jptr_(&value) {}
JsonRefrence::JsonRefrence(Json::Value&& value) : holder_(new TemplateDataInterface<Json::Value>(std::move(value))), jptr_(&holder_->ref()){}
JsonRefrence::~JsonRefrence() { delete holder_; }

const Json::Value& JsonRefrence::value() const { return *jptr_; }
JsonRefrence::operator bool() const { return jptr_ != &global_empty_json_ptr; }

size_t JsonRefrence::size() const { return jptr_->size(); }
int JsonRefrence::arraySize() const { return jptr_->isArray() ? static_cast<int>(jptr_->size()) : 0; }
int JsonRefrence::objectSize() const { return jptr_->isObject() ? static_cast<int>(jptr_->size()) : -1; }

container::Vector<std::string> JsonRefrence::objectKeys() const {
	return jptr_->isObject() ? jptr_->getMemberNames() : container::Vector<std::string>();
}
bool JsonRefrence::has(int index) const { return index < arraySize() && index >= 0; }
bool JsonRefrence::has(const std::string& key) const { return jptr_->isObject() && jptr_->isMember(key); }

JsonRefrence JsonRefrence::at(int index, bool* founded) const { 
	if (!has(index)) {
		if (founded) *founded = false;
		return global_empty_json_ptr;
	}
	if (founded) *founded = true;
	return (*jptr_)[index];
}

JsonRefrence JsonRefrence::at(const std::string& key, bool use_index, bool* founded) const {
    if (use_index && key.length() > 1 && key[0] == '#') {
        return at(atoi(key.c_str() + sizeof(char)), founded);
    }
    if (!has(key)) {
        if (founded) *founded = false;
        return global_empty_json_ptr;
    }
    if (founded) *founded = true;
    return (*jptr_)[key];
}

JsonRefrence JsonRefrence::get(const std::string& path, bool* founded) const
{
    if (jptr_->isNull()) {
        if (founded) *founded = false;
        return global_empty_json_ptr;
    }
    int len = path.find('.');
    if (len < 0) {
        return at(path, true, founded);
    }
    else if (jptr_->isObject()) {
        return at(path.substr(0, len), true, founded).get(path.substr(len + 1, path.size() - len - 1), founded);
    }
    if (founded) *founded = false;
    return global_empty_json_ptr;
}

JsonRefrence JsonRefrence::operator[](int index) const { return at(index); }
JsonRefrence JsonRefrence::operator[](const std::string& key) const { return get(key); }

#define _F_to(JSON_TYPE, JSON_FUNC, DEFAULT) \
if (!jptr_->isNull() && jptr_->isConvertibleTo(Json::JSON_TYPE##Value)) { \
    if (ok) *ok = true; \
    return jptr_->as##JSON_FUNC(); \
} \
if (ok) *ok = false; \
return DEFAULT \

bool JsonRefrence::toBool(bool default_bool, bool* ok) const { _F_to(boolean, Bool, default_bool); }
int JsonRefrence::toInt(int default_int, bool* ok) const { _F_to(int, Int, default_int); }
unsigned int JsonRefrence::toUint(unsigned int default_uint, bool* ok) const { _F_to(uint, UInt, default_uint); }
double JsonRefrence::toDouble(double default_double, bool* ok) const { _F_to(real, Double, default_double); }
std::string JsonRefrence::toString(const std::string& default_string, bool* ok) const { _F_to(string, String, default_string); }

template<> 
bool JsonRefrence::toT(bool* ok) const { return toBool(false, ok); }
template<> 
int JsonRefrence::toT(bool* ok) const { return toInt(0, ok); }
template<> 
unsigned int JsonRefrence::toT(bool* ok) const { return toUint(0, ok); }
template<> 
float JsonRefrence::toT(bool* ok) const { return toDouble(0, ok); }
template<> 
double JsonRefrence::toT(bool* ok) const { return toDouble(0, ok); }
template<> 
std::string JsonRefrence::toT(bool* ok) const { return toString("", ok); }
template<> 
Json::Value JsonRefrence::toT(bool* ok) const { if (ok) *ok = true; return value(); }
template<> 
short JsonRefrence::toT(bool* ok) const { return toInt(0, ok); }
template<> 
unsigned short JsonRefrence::toT(bool* ok) const { return toUint(0, ok); }
template<> 
long JsonRefrence::toT(bool* ok) const { _F_to(int, LargestInt, 0l); }
template<> 
unsigned long JsonRefrence::toT(bool* ok) const { _F_to(int, LargestInt, 0l); }
template<> 
long long JsonRefrence::toT(bool* ok) const { _F_to(int, LargestInt, 0l); }
template<> 
unsigned long long JsonRefrence::toT(bool* ok) const { _F_to(int, LargestUInt, 0l); }

double JsonRefrence::operator()(int index) const {
    return at(index).toDouble();
}
double JsonRefrence::operator()(const std::string& key) const {
    return get(key).toDouble();
}

Json::Value JsonInterface::serialize() const {
    return Json::Value();
}
bool JsonInterface::deserialize(const JsonRefrence&) {
    return false;
}

DataObject::DataObject(const std::string& name) : Object(name) {}
DataObject::~DataObject() {}
std::string DataObject::dataTypeName() const { return MFLOW_UNDEFINED_OBJECT_NAME; }

namespace standard {
bool fromJson(const JsonRefrence& jref, DataObject* const d) {
    if (auto _js_interface = data::json_holder(d)) return _js_interface->deserialize(jref);
    return false;
}
Json::Value toJson(DataObject* const d) {
    if (auto _js_interface = data::json_holder(d)) return _js_interface->serialize();
    return Json::Value();
}
bool fromJson(const JsonRefrence& jr, std::shared_ptr<DataObject> dptr) { return fromJson(jr, dptr.get()); }
Json::Value toJson(std::shared_ptr<DataObject> dptr) { return toJson(dptr.get()); }
}

namespace custom {
template<> 
bool fromJson(const JsonRefrence& jref, std::vector<double>& d) {
    container::Vector<double> _ds = std::move(d);
    bool _result = standard::fromJson(jref, d);
    _ds = std::move(d);
    return _result;
}
template<> 
Json::Value toJson(const std::vector<double>& d) {
    return standard::toJson(container::Vector<double>(d));
}
template<typename DA> bool json2DoubleArray(const JsonRefrence& jref, DA& da)
{
    constexpr const int len = sizeof(DA) / sizeof(double);
    if (jref.arraySize() != len) return false;
    bool ok = false;
    for (int i = 0; i < len; i++) {
        da[i] = jref.at(i).toDouble(0, &ok);
        if (!ok) return false;
    }
    return true;
}

template<typename DA> Json::Value doubleArray2Json(const DA& da)
{
    constexpr const int len = sizeof(DA) / sizeof(double);
    Json::Value v;
    v.resize(len);
    for (int i = 0; i < len; i++) {
        v[i] = da[i];
    }
    return v;
}

template<> bool fromJson(const JsonRefrence& jref, std::array<double, 6>& da) { return json2DoubleArray(jref, da); }
template<> Json::Value toJson(const std::array<double, 6>& ds) { return doubleArray2Json(ds); }
template<> bool fromJson(const JsonRefrence& jref, std::array<double, 7>& da) { return json2DoubleArray(jref, da); }
template<> Json::Value toJson(const std::array<double, 7>& ds) { return doubleArray2Json(ds); }
}

namespace data {

ObjectManager& mgr() {
	static ObjectManager mgr("mflow_global_data_manager");
	return mgr;
}
DataObject* get(const std::string& path) {
	auto _d = mgr().get<DataObject>(path);
	if (_d) return _d;
	size_t _length = path.rfind(".");
	if (_length >= path.length() || _length == 0) return nullptr;
	_d = get(path.substr(0, _length));
	if (!_d) return nullptr;
    auto _sub_key = path.substr(_length + 1);
    if (_sub_key == "#") {
        if (auto _d_list = dynamic_cast<StandardT<DataList>*>(_d)) return _d_list->ref().getDataObject(std::stoi(_sub_key.substr(1)));
        return nullptr;
    }
    if (auto _d_dict = dynamic_cast<StandardT<DataDict>*>(_d)) return _d_dict->ref().getDataObject(_sub_key);
    return nullptr;
}

}
}