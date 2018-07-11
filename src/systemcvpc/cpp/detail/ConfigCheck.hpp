// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_CONFIGCHECK_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_CONFIGCHECK_HPP

#include <systemcvpc/datatypes.hpp>

#include <systemc>

#include <set>
#include <map>

namespace SystemC_VPC { namespace Detail {

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

  static std::map<ProcessId, std::string> & processNames();
  static std::map<ProcessId, std::pair<std::string, std::string> > & routeNames();

  static void setProcessName(ProcessId pid, std::string name);
  static void setRouteName(ProcessId pid, std::string src, std::string dest);
  static bool hasProcessName(ProcessId pid);
  static bool hasRouteName(ProcessId pid);
  static std::string getProcessName(ProcessId pid);
  static std::pair<std::string, std::string> getRouteName(ProcessId pid);

private:
  TaskConfigs taskConfigs;
};

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_CONFIGCHECK_HPP */
