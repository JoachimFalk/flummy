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

#include "config.h"
//#include <systemcvpc/vpc_config.h>

#include <systemcvpc/Timing.hpp>
#include <systemcvpc/VpcApi.hpp>
#include <systemcvpc/InvalidArgumentException.hpp>
#include <systemcvpc/ScheduledTask.hpp>

#include "Director.hpp"
#include "Configuration.hpp"
#include "ConfigCheck.hpp"
#include "DebugOStream.hpp"
#include "HysteresisLocalGovernor.hpp"
#include "FastLink.hpp"

#include "PluggablePowerGovernor.hpp"
#include "PowerSumming.hpp"
#include "ProcessControlBlock.hpp"
#include "SelectFastestPowerModeGlobalGovernor.hpp"
#include "TaskInstance.hpp"
#include "dynload/dll.hpp"

#include <CoSupport/SystemC/systemc_time.hpp>

#include <systemc>

#include "detail/AbstractRoute.hpp"

#include <iostream>
#include <map>
#include <sstream>
#include <vector>

namespace SystemC_VPC { namespace Detail {

  std::unique_ptr<Director> Director::singleton;

  sc_core::sc_time Director::end = sc_core::SC_ZERO_TIME;

  /**
   *
   */
  Director::Director()
    : FALLBACKMODE(false)
    , defaultRoute(false)
    , checkVpcConfig(true)
    , topPowerGov(new InternalSelectFastestPowerModeGovernor)
    , topPowerGovFactory(NULL)
#ifndef NO_POWER_SUM
    , powerConsStream("powerconsumption.dat")
    , powerSumming(NULL)
#endif // NO_POWER_SUM
    , taskPool(new TaskPool())
  {
    sc_core::sc_report_handler::set_actions(
        sc_core::SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_,
        sc_core::SC_DO_NOTHING);
    //sc_core::sc_report_handler::set_actions(SC_ID_OBJECT_EXISTS_,
    //                               SC_DO_NOTHING);
    sc_core::sc_report_handler::set_actions(
        sc_core::SC_WARNING,
        sc_core::SC_DO_NOTHING);
  }

  /**
   *
   */
  Director::~Director(){

    sc_core::sc_time start = sc_core::SC_ZERO_TIME;
    sc_core::sc_time end = this->end;
#ifdef DBG_DIRECTOR
    std::cerr << "start: " << start << " end: " << end  << std::endl;
#endif //DBG_DIRECTOR

    if(0 != this->vpc_result_file.compare("")) {
#ifdef DBG_DIRECTOR
      std::cerr << "Director> result_file: "
                << this->vpc_result_file << std::endl;
#endif //DBG_DIRECTOR
      std::ofstream resultFile;
      resultFile.open(this->vpc_result_file.c_str());
      if(resultFile){
        resultFile << (end-start).to_default_time_units();
      }
      resultFile.flush();
      resultFile.close();
    }else{
      std::cerr << "[VPC] overall simulated time: " << end - start << std::endl;
    }

#ifndef NO_POWER_SUM
    for (SystemC_VPC::Components::value_type const &v : getComponents()) {
      v.second->removeObserver(powerSumming);
    }
    delete powerSumming;
#endif // NO_POWER_SUM
    delete taskPool;
  }

  TaskInstance* Director::allocateTask(ProcessId pid){
    return this->taskPool->allocate(pid);
  }

  //
  const Delayer * Director::getComponent(FastLink const *vpcLink) const {
    assert(vpcLink->component);
    return vpcLink->component;
  }

  static
  ProcessId getNextProcessId() {
    static ProcessId       globalProcessId = 0;
    return globalProcessId++;
  }

  ProcessId Director::getProcessId(std::string process_or_source,
      std::string destination)
  {
    typedef std::map<std::string, ProcessId> ProcessIdMap;
    static ProcessIdMap processIdMap;

    if (destination == "") {
      std::string & process = process_or_source;
      ProcessIdMap::const_iterator iter = processIdMap.find(process);
      if (iter == processIdMap.end()) {
        ProcessId id = getNextProcessId();
        processIdMap[process] = id;
        ConfigCheck::setProcessName(id, process);
      }
      iter = processIdMap.find(process);
      return iter->second;

    } else {
      std::string & source = process_or_source;
      std::string name_hack = "msg_" + source + "_2_" + destination;
      ProcessIdMap::const_iterator iter = processIdMap.find(name_hack);
      if (iter == processIdMap.end()) {
        ProcessId id = getNextProcessId();
        processIdMap[name_hack] = id;
        ConfigCheck::setRouteName(id, source, destination);
      }
      iter = processIdMap.find(name_hack);
      return iter->second;
    }
  }

  typedef std::map<std::string, FunctionId>  FunctionIdMap;

  FunctionId uniqueFunctionId() {
    static FunctionId globalFunctionId = 1;
    return globalFunctionId++;
  }


  FunctionIdMap& getFunctionIdMap(){
    static FunctionIdMap functionIdMap;
    return functionIdMap;
  }

  // FunctionIds are created by VPC during XML parsing.
  FunctionId Director::createFunctionId(const std::string& function) {
    FunctionIdMap::const_iterator iter = getFunctionIdMap().find(function);
    if( iter == getFunctionIdMap().end() ) {
      // map empty function names to default timing
      if(function == "") {
        getFunctionIdMap()[function] = defaultFunctionId;
      }else{
        getFunctionIdMap()[function] = uniqueFunctionId();
      }
    }
    iter = getFunctionIdMap().find(function);
    return iter->second;
  }

  bool Director::hasFunctionId(const std::string& function){
    FunctionIdMap::const_iterator iter = getFunctionIdMap().find(function);
    return (iter != getFunctionIdMap().end());
  }

  // FunctionIds are used (get) by SysteMoC.
  // The default ID (and a default timing) is used if no ID was created during
  // parsing. (The function name was not given in the XML.)
  FunctionId Director::getFunctionId(const std::string& function){
    FunctionIdMap::const_iterator iter = getFunctionIdMap().find(function);

    // the function name was not set in configuration
    // -> we have to use the default delay
    if( iter == getFunctionIdMap().end() ) {
      return defaultFunctionId;
    }
    return iter->second;
    
  }

  /// begin section: VpcApi.hpp related stuff

  void Director::beforeVpcFinalize()
  {
#ifndef NO_POWER_SUM
    powerSumming = new PowerSumming(powerConsStream);
#endif // NO_POWER_SUM
    for (Components::value_type const &v : Configuration::getInstance().getComponents()) {
#ifndef NO_POWER_SUM
      v.second->addObserver(powerSumming);
#endif // NO_POWER_SUM
      v.second->initialize(this);
    }
  }
  /// end section: VpcApi.hpp related stuff

  //
  void Director::endOfVpcFinalize()
  {
    if (checkVpcConfig) {
      ConfigCheck::check();
    }
  }

  //
  bool Director::hasValidConfig() const
  {
    return !FALLBACKMODE;
  }

  void Director::loadGlobalGovernorPlugin(std::string plugin,
                                          AttributePtr attPtr){
    //std::cerr << "Director::loadGlobalGovernorPlugin" << std::endl;
    topPowerGovFactory =
      new DLLFactory<PlugInFactory<PluggableGlobalPowerGovernor> >
        (plugin.c_str());
    if( topPowerGovFactory->factory){
      delete topPowerGov;

      topPowerGovFactory->factory->processAttributes(attPtr);
      topPowerGov = topPowerGovFactory->factory->createPlugIn();
    }
  }

  void Director::debugUnknownNames( ) const {
    bool route = false;
    bool mappings = false;
    for(std::map<ProcessId, std::pair<std::string, std::string> >::const_iterator
          iter = ConfigCheck::routeNames().begin();
        iter != ConfigCheck::routeNames().end();
        ++iter){
      if( !taskPool->contains( iter->first ) ){
           if(!route){
             std::cout << "Found unknown routes.\n"
                       <<" Please add the following routes to the config file:"
                       << std::endl;
             route = true;
           }

        std::cout << "  <route  source=\""
                  << iter->second.first
                  << "\" destination=\""
                  << iter->second.second
                  << "\">\n   <hop name=\"?\">  </hop>\n  </route>"
                  << std::endl;
      }
    }
    for (VpcTasks::value_type const &v :
            Configuration::getInstance().getVpcTasks()) {
      if (!v.second->getComponent()) {
        if(!mappings){
          std::cout << "\n" << std::endl;
          std::cout << "Unknown mapping for tasks.\n"
                    <<" Please add the following mapping to the config file:"
                    << std::endl;
          mappings = true;
        }
        std::cout << "  <mapping source=\""
                  << v.first
                  << "\" target=\"?\">\n"
                  << "    <!-- we may use a default delay: "
                  << "    <timing dii=\"? us\""
                  << " latency=\"? us\" />"
                  << " --> "
                  << std::endl;

        const std::set<std::string> &functionNames =
          debugFunctionNames.find(getProcessId(v.first))->second;

        for(std::set<std::string>::const_iterator fiter =
            functionNames.begin();
            fiter != functionNames.end();
            ++fiter){
          if(0 == fiter->compare("???")){
            std::cout << "    <!-- the \"???\" is caused by a SysteMoC"
                      << " transition without function CALL  -->" << std::endl;
          }
          std::cout << "    <timing fname=\""
                    << *fiter
                    << "\" dii=\"? us\""
                    << " latency=\"? us\" />"
                    << std::endl;
        }
        std::cout <<"  </mapping>" << std::endl;
      }
    }
    if(route || mappings){
      std::cout << "\n" << std::endl;
    }
    exit(-1);
  }

  //
  std::string Director::getTaskName(ProcessId id) {
    if(ConfigCheck::hasProcessName(id)){
      return ConfigCheck::getProcessName(id);
    }else{
      return "Route from " + ConfigCheck::getRouteName(id).first +
        " to: " + ConfigCheck::getRouteName(id).second;
    }
  }

} } // namespace SystemC_VPC::Detail
