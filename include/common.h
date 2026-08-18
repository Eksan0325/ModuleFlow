/**
 * Information in this file is the intellectual property of Rokae Technology Co., Ltd,
 * And may contains trade secrets that must be stored and viewed confidentially.
 *
 * @file common.h
 * @brief  ModuleFlow interface
 * @author Eksan
 */
#ifndef MFLOW_COMMON_H
#define MFLOW_COMMON_H

#include "basic/data_manager_def.h"
#include "basic/cmd_manager_def.h"

namespace mflow {

extern const std::string global_mflow_uid;

class Module;
typedef Creator<Module*()> ModuleCreator;
ModuleCreator& moduleCreator();

typedef Creator<int()> VersionController;
VersionController& version();
int version(const std::string& key);

namespace entry {
Module* module(const std::string& key);
std::string createModuleName();
}

}

#endif