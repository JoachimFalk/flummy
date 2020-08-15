// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
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

#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/PowerMode.hpp>

#include "LookupPowerTimeModelImpl.hpp"
#include "../common.hpp"
#include "../DebugOStream.hpp"

#include <CoSupport/sassert.h>

#include <sstream>

namespace SystemC_VPC { namespace Detail { namespace ExecModelling {

  struct LookupPowerTimeModelImpl::PowerModeTiming {
    sc_core::sc_time guardDelay;
    sc_core::sc_time dii;
    sc_core::sc_time latency;
    Power            pwrRunning;
  };

  LookupPowerTimeModelImpl::LookupPowerTimeModelImpl()
    : AbstractExecModel(
        reinterpret_cast<char *>(static_cast<ExecModel         *>(this)) -
        reinterpret_cast<char *>(static_cast<AbstractExecModel *>(this)))
    , LookupPowerTimeModel(
         reinterpret_cast<char *>(static_cast<AbstractExecModel *>(this)) -
         reinterpret_cast<char *>(static_cast<ExecModel         *>(this)))
    , registeredActions(false)
    , startPowerMode(PowerMode::DEFAULT)
    {}

  LookupPowerTimeModelImpl::~LookupPowerTimeModelImpl() {
    for (PowerModeTiming *array : powerModeTimingArrays)
      delete[] array;
  }

  void LookupPowerTimeModelImpl::add(Timing timing) {
    assert(!registeredActions);
    PowerModeInfo &pmi = powerModes[timing.getPowerMode()];
#ifdef SYSTEMCVPC_ENABLE_DEBUG
    if (DBG_STREAM.isVisible(Debug::High)) {
      if (timing.getLatency() == timing.getDii()) {
        DBG_STREAM << "Got " << timing.getLatency()
            << " delay for action/guard "
            << timing.getFunction()
            << " in power mode " << timing.getPowerMode() << std::endl;
      } else {
        DBG_STREAM << "Got " << timing.getDii()
            << " dii and " << timing.getLatency()
            << "lat for action/guard "
            << timing.getFunction()
            << " in power mode " << timing.getPowerMode() << std::endl;
      }
    }
#endif //SYSTEMCVPC_ENABLE_DEBUG
    if (!pmi.timings.insert(std::make_pair(timing.getFunction(), timing)).second)
      throw ConfigException("Duplicate timing information for "+timing.getFunction());
  }

  void LookupPowerTimeModelImpl::addDefaultActorTiming(std::string actorName, Timing timing) {
    assert(!registeredActions);
    PowerModeInfo &pmi = powerModes[timing.getPowerMode()];
#ifdef SYSTEMCVPC_ENABLE_DEBUG
    if (DBG_STREAM.isVisible(Debug::High)) {
      if (timing.getLatency() == timing.getDii()) {
        DBG_STREAM << "Got default " << timing.getLatency()
            << " delay for actor " << actorName
            << " in power mode " << timing.getPowerMode() << std::endl;
      } else {
        DBG_STREAM << "Got default " << timing.getDii()
            << " dii and " << timing.getLatency()
            << "lat for actor " << actorName
            << " in power mode " << timing.getPowerMode() << std::endl;
      }
    }
#endif //SYSTEMCVPC_ENABLE_DEBUG
    if (!pmi.timings.insert(std::make_pair(actorName, timing)).second)
      throw ConfigException("Duplicate timing information for "+actorName);
  }

  bool LookupPowerTimeModelImpl::addAttribute(Attribute const &attr) {
//    if(attr->isType("governor")){
//      this->loadLocalGovernorPlugin(attr->getValue());
//      powerAttribute = powerAtt;
//      continue;
//    }

    if (attr.isType("startupPowermode")) {
      startPowerMode = attr.getValue();
      return true;
    }
    if (attr.isType("powermode")) {
      PowerModeInfo &pmi = powerModes[attr.getValue()];
      for (Attribute const &emAttr : attr.getAttributes()) {
        if (emAttr.isType("powerIdle")) {
          pmi.pwrIdle = Power(emAttr.getValue());
        } else if (emAttr.isType("powerRunning")) {
          pmi.pwrRunning = Power(emAttr.getValue());
        } else if (emAttr.isType("powerStalled")) {
          pmi.pwrStalled = Power(emAttr.getValue());

        } else if (emAttr.isType("guardComplexityFactor")) {
          pmi.guardComplexityFactor = createSC_Time(emAttr.getValue().c_str());
        } else {
          throw ConfigException("Unhandled attribute " + emAttr.getType()
              + " for power mode "+attr.getType()
              + " in execution model "+std::string(Type));
        }
      }
      return true;
    }

//    std::string powerMode = attr->getNextAttribute(i).first;
//    const PowerMode *power = this->translatePowerMode(powerMode);
//
//    if(powerTables.find(power) == powerTables.end()){
//      powerTables[power] = PowerTable();
//    }
//
//    PowerTable &powerTable=powerTables[power];
//
//    if(powerAtt->hasParameter("transaction_delay")) {
//      this->transactionDelays[power] =
//        createSC_Time(powerAtt->getParameter("transaction_delay").c_str());
//    }
//    if(powerAtt->hasParameter("transfer_delay")) {
//      this->transactionDelays[power] =
//        createSC_Time(powerAtt->getParameter("transfer_delay").c_str());
//    }

    return false;
  }


  /// Allocate opaque CompState object when attaching to an abstract component.
  void LookupPowerTimeModelImpl::attachToComponent(
      ComponentMixIn *comp)
  {
    setExecModel(comp, this);
    setCompState(comp, new CompState());
  }

  /// Change the power mode of a component. This should update the
  /// opaque object pointed to by execModelComponentState.
  void  LookupPowerTimeModelImpl::setPowerMode(
      ComponentMixIn     *comp
    , std::string const  &mode) const
  {
    PowerModes::const_iterator iter = powerModes.find(mode);
    assert(iter != powerModes.end());
    getCompState(comp)->powerMode = iter->second.index;
    setPowerIdle(comp, iter->second.pwrIdle);
  }

  LookupPowerTimeModelImpl::ActionInfo *LookupPowerTimeModelImpl::registerAction(
      ComponentMixIn       *comp
    , TaskInterface const  *actor
    , PossibleAction const *action)
  {
    if (!registeredActions) {
      registeredActions = true;
      // Fill in index information for PowerModeInfo objects.
      size_t index = 0;
      for (PowerModes::value_type &entry : powerModes)
        entry.second.index = index++;
      PowerModes::iterator iter = powerModes.find(startPowerMode);
      if (iter == powerModes.end())
        throw ConfigException("Startup power mode "+startPowerMode+" was not defined!");
      getCompState(comp)->powerMode = iter->second.index;
      setPowerIdle(comp, iter->second.pwrIdle);
    }

    size_t nrPowerModes = powerModes.size();

    PowerModeTiming *array = new PowerModeTiming[nrPowerModes]();
    powerModeTimingArrays.push_back(array);

    std::string const                   actorName       = actor->name();
    PossibleAction::FunctionNames const actionNames     = action->getActionNames();
    PossibleAction::FunctionNames const guardNames      = action->getGuardNames();
    size_t                        const guardComplexity = action->getGuardComplexity();

    for (PowerModes::value_type const &entry : powerModes) {
      size_t         index   = entry.second.index;
      Timings const &timings = entry.second.timings;

      sc_core::sc_time guardDelay, dii, latency;

      for (std::string const &actionName : actionNames) {
        Timings::const_iterator iter = timings.find(actionName);
        if (iter == timings.end())
          iter = timings.find(actorName);
        if (iter == timings.end()) {
          std::stringstream msg;
          msg << "No timing for action " << actionName
              << " of actor " << actorName
              << " in power mode " << entry.first << "!";
          throw ConfigException(msg.str());
        }
        dii     += iter->second.getDii();
        latency += iter->second.getLatency();
      }
      assert(latency >= dii);
      for (std::string const &guardName : guardNames) {
        Timings::const_iterator iter = timings.find(guardName);
        if (iter == timings.end())
          iter = timings.find(actorName);
        if (iter == timings.end()) {
          std::stringstream msg;
          msg << "No timing for guard " << guardName
              << " of actor " << actorName
              << " in power mode " << entry.first << "!";
          throw ConfigException(msg.str());
        }
        if (iter->second.getDii() != iter->second.getLatency()) {
          std::stringstream msg;
          msg << "Only delay is supported for guards, i.e., erroneous dii, latency pair"
              << " for guard " << guardName
              << " of actor " << actorName
              << " in power mode " << entry.first << "!";
          throw ConfigException(msg.str());
        }
        guardDelay += iter->second.getDii();
      }
      guardDelay += guardComplexity * entry.second.guardComplexityFactor;

      array[index].guardDelay = guardDelay;
      array[index].dii        = dii;
      array[index].latency    = latency;
      array[index].pwrRunning = entry.second.pwrRunning;
    }
    return reinterpret_cast<ActionInfo *>(array);

    /*
     *     FunctionTimingPtr timing =
        this->getTiming(this->getPowerMode(), taskImpl);

    if(!fLink->guardIds.empty())
      taskInstance.setDelay(
          timing->getDelay(fLink->guardIds) +
          fLink->complexity * sc_core::sc_time(0.1, sc_core::SC_NS));
    else
      taskInstance.setDelay(
          fLink->complexity * sc_core::sc_time(0.1, sc_core::SC_NS));
     */

    /*
    const char                        *actorName       = actor->name();
    smoc::SimulatorAPI::FunctionNames  actionNames     = fr->getActionNames();
    smoc::SimulatorAPI::FunctionNames  guardNames      = fr->getGuardNames();
    size_t                             guardComplexity = fr->getGuardComplexity();

    TaskImpl        *taskImpl = static_cast<TaskImpl *>(actor->getSchedulerInfo());
    ProcessId const  pid = taskImpl->getPid();

    assert(Director::getInstance().getProcessId(actorName) == pid);

    try {
      TimingsProvider::Ptr provider = this->getTimingsProvider();
      if (provider->hasDefaultActorTiming(actorName)) {

        SystemC_VPC::functionTimingsPM timingsPM = provider->getActionTimings(actorName);

        for (SystemC_VPC::functionTimingsPM::iterator it=timingsPM.begin() ; it != timingsPM.end(); it++ ) {
          std::string powermode = (*it).first;
          setTiming(provider->getActionTiming(actorName,powermode), taskImpl);
        }
      }
      for (std::string const &guard : guardNames) {
        if (provider->hasGuardTimings(guard)) {

          SystemC_VPC::functionTimingsPM timingsPM = provider->getGuardTimings(guard);
          for (SystemC_VPC::functionTimingsPM::iterator it=timingsPM.begin() ; it != timingsPM.end(); it++ )
          {
            std::string powermode = (*it).first;
            setTiming(provider->getGuardTiming(guard,powermode), taskImpl);
            ConfigCheck::configureTiming(pid, guard);
          }
        }
      }
      for (std::string const &action : actionNames) {
        if (provider->hasActionTimings(action)) {

          SystemC_VPC::functionTimingsPM timingsPM = provider->getActionTimings(action);
          for (SystemC_VPC::functionTimingsPM::iterator it=timingsPM.begin() ; it != timingsPM.end(); it++ )
          {
            std::string powermode = (*it).first;
            setTiming(provider->getActionTiming(action,powermode), taskImpl);
            ConfigCheck::configureTiming(pid, action);
          }
        }
      }
    } catch (std::exception &e) {
      std::cerr << "Actor registration failed for \"" << actorName <<
          "\". Got exception:\n" << e.what() << std::endl;
      exit(-1);
    }

    FunctionIds     actionIds;
    FunctionIds     guardIds;

    for (FunctionNames::const_iterator iter = actionNames.begin();
         iter != actionNames .end();
         ++iter){
      Director::getInstance().debugFunctionNames[pid].insert(*iter);
      //check if we have timing data for this function in the XML configuration
      if (Director::getInstance().hasFunctionId(*iter)) {
        actionIds.push_back(Director::getInstance().getFunctionId(*iter) );
      }
      ConfigCheck::modelTiming(pid, *iter);
    }
    for (FunctionNames::const_iterator iter = guardNames.begin();
         iter != guardNames.end();
         ++iter){
      Director::getInstance().debugFunctionNames[pid].insert(*iter);
      if(Director::getInstance().hasFunctionId(*iter)){
        guardIds.push_back(Director::getInstance().getFunctionId(*iter) );
      }
      ConfigCheck::modelTiming(pid, *iter);
    }

    fr->setSchedulerInfo(new FastLink(this, actionIds, guardIds, guardComplexity));
    */
  }

  /// Initialize ti with action or guard timing and power values.
  void  LookupPowerTimeModelImpl::initTaskInstance(
      ComponentMixIn   *comp
    , ActionInfo       *ai
    , TaskInstanceImpl *ti
    , bool              forGuard) const
  {
    PowerModeTiming &pmi = reinterpret_cast<PowerModeTiming *>(ai)
        [getCompState(comp)->powerMode];

    if (forGuard) {
      setDelay(ti, pmi.guardDelay);
      setLatency(ti, pmi.guardDelay);
      setPower(ti, pmi.pwrRunning);
    } else {
      setDelay(ti, pmi.dii);
      setLatency(ti, pmi.latency);
      setPower(ti, pmi.pwrRunning);
    }
  }

} } } // namespace SystemC_VPC::Detail::ExecModelling
