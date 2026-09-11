/**
 * Information in this file is the intellectual property of Rokae Technology Co., Ltd,
 * And may contains trade secrets that must be stored and viewed confidentially.
 *
 * @file module.h
 * @brief module interface
 * @author Eksan
 */

#ifndef MFLOW_FRAMEWORK_MODULE_H
#define MFLOW_FRAMEWORK_MODULE_H

#include "basic/object.h"

namespace mflow {
class Module : public Object {
public:
	explicit Module();
	virtual ~Module();

	std::string name() const override;
	std::string name(const std::string& suffix) const;

	enum ModuleStateSignal : int {
		MODULE_GONNA_INIT = 0x0110,
		MODULE_INITED     = 0x0100,
		MODULE_GONNA_IDLE = 0x0111,
		MODULE_IDLE		  = 0x0101,
		MODULE_DEINIT     = 0x0112,
		MODULE_DEINITED   = 0x0102
	};

	ModuleStateSignal state() const;

	void startToInit();
	void startToIdle();
	void startToDeinit();

	void adoptObject(Object* obj);
	void abandonObject(Object* obj, bool auto_delete = false);

protected:
	virtual void onInit(); 
	virtual void onIdle();
	virtual void onDeinit();
private:
	std::atomic<ModuleStateSignal> state_;
};
}

#endif // !MFLOW_FRAMEWORK_MODULE_H
