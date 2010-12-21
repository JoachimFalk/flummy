/*
 * ConfigCheck.hpp
 *
 *  Created on: 15 Dec 2010
 *      Author: streubue
 */

#ifndef __INCLUDED__CONFIGCHECK_HPP__
#define __INCLUDED__CONFIGCHECK_HPP__

#include <systemcvpc/Director.hpp>
#include <systemcvpc/Timing.hpp>

#include <systemc>

#include <set>
#include <iterator>

namespace SystemC_VPC
{
typedef std::set<std::string> FunctionSet;

class TaskConfig
{
public:
  void addConfiguredFunction(std::string fid)
  {
    configuredFunctionSet.insert(fid);
  }

  void addModelledFunction(std::string fid)
  {
    modelledFunctionSet.insert(fid);
  }

  void check();

private:

  // the set of functions configured in the XMl file
  FunctionSet configuredFunctionSet;

  // the set of SysteMoC actions CALLed from transitions
  FunctionSet modelledFunctionSet;

  // Has this task a mapping in the configuration file?
  bool isConfigured;

  // Is this task modelled as an SysteMoC actor?
  bool isModelled;

};

class ConfigCheck
{
public:
  typedef std::map<ProcessId, TaskConfig*> TaskConfigs;

  static void check()
  {
    static bool isChecked = false;
    if (!isChecked) {
      isChecked = true;
      ConfigCheck::getInstance().runCheck(); // run check only once
    }
  }

  static ConfigCheck &
  getInstance()
  {
    static ConfigCheck configCheck;
    return configCheck;
  }

  static void configureTiming(ProcessId pid, std::string function)
  {
    ConfigCheck & cc = getInstance();
    TaskConfig & taskConfig = cc.getTaskConfig(pid);
    taskConfig.addConfiguredFunction(function);

  }

  static void modelTiming(ProcessId pid, std::string function)
  {
    ConfigCheck & cc = getInstance();
    TaskConfig & taskConfig = cc.getTaskConfig(pid);
    taskConfig.addModelledFunction(function);

  }

  TaskConfig &
  getTaskConfig(ProcessId pid)
  {
    if (taskConfigs.end() == taskConfigs.find(pid)) {
      taskConfigs[pid] = new TaskConfig();
    }
    TaskConfig * ret = taskConfigs[pid];
    return *ret;
  }

  void runCheck()
  {
    for (TaskConfigs::iterator iter = taskConfigs.begin(); iter
        != taskConfigs.end(); iter++) {
      iter->second->check();
    }
  }
private:
  TaskConfigs taskConfigs;
};
}

#endif /* __INCLUDED__CONFIGCHECK_HPP__ */
