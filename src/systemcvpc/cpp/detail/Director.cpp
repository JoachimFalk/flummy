// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include "PluggablePowerGovernor.hpp"
#include "TaskImpl.hpp"
#include "SelectFastestPowerModeGlobalGovernor.hpp"
#include "TaskInstanceImpl.hpp"
#include "dynload/dll.hpp"

#include <CoSupport/SystemC/systemc_time.hpp>

#include <systemc>

#include "AbstractRoute.hpp"

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
    : checkVpcConfig(true)
    , topPowerGov(new InternalSelectFastestPowerModeGovernor)
    , topPowerGovFactory(NULL)
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

  /// begin section: VpcApi.hpp related stuff

  void Director::beforeVpcFinalize()
  {
  }
  /// end section: VpcApi.hpp related stuff

  //
  void Director::endOfVpcFinalize()
  {
    if (checkVpcConfig) {
      ConfigCheck::check();
    }
  }

  void Director::loadGlobalGovernorPlugin(std::string plugin,
                                          Attribute const &attr){
    //std::cerr << "Director::loadGlobalGovernorPlugin" << std::endl;
    topPowerGovFactory =
      new DLLFactory<PlugInFactory<PluggableGlobalPowerGovernor> >
        (plugin.c_str());
    if( topPowerGovFactory->factory){
      delete topPowerGov;

      topPowerGovFactory->factory->processAttributes(attr);
      topPowerGov = topPowerGovFactory->factory->createPlugIn();
    }
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
