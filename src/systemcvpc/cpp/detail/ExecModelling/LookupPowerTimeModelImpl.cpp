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

#include <systemcvpc/ConfigException.hpp>

#include "LookupPowerTimeModelImpl.hpp"

#include <CoSupport/sassert.h>

#include <sstream>

namespace SystemC_VPC { namespace Detail { namespace ExecModelling {

  struct LookupPowerTimeModelImpl::PowerModeTiming {
    sc_core::sc_time guardDelay;
    sc_core::sc_time dii;
    sc_core::sc_time latency;
  };

  LookupPowerTimeModelImpl::LookupPowerTimeModelImpl()
    : AbstractExecModel(
        reinterpret_cast<char *>(static_cast<ExecModel         *>(this)) -
        reinterpret_cast<char *>(static_cast<AbstractExecModel *>(this)))
    , LookupPowerTimeModel(
         reinterpret_cast<char *>(static_cast<AbstractExecModel *>(this)) -
         reinterpret_cast<char *>(static_cast<ExecModel         *>(this)))
    , registeredActions(false)
    {}

  LookupPowerTimeModelImpl::~LookupPowerTimeModelImpl() {
    for (PowerModeTiming *array : powerModeTimingArrays)
      delete[] array;
  }

  void LookupPowerTimeModelImpl::add(Timing timing) {
    assert(!registeredActions);
    powerModeDependentTimings[timing.getPowerMode()][timing.getFunction()] = timing;
  }

  void LookupPowerTimeModelImpl::addDefaultActorTiming(std::string actorName, Timing timing) {
    assert(!registeredActions);
    powerModeDependentTimings[timing.getPowerMode()][actorName] = timing;
  }

  bool LookupPowerTimeModelImpl::addAttribute(AttributePtr attr) {
//    if(attr->isType("governor")){
//      this->loadLocalGovernorPlugin(attr->getValue());
//      powerAttribute = powerAtt;
//      continue;
//    }

//    std::string powerMode = attr->getNextAttribute(i).first;
//    const PowerMode *power = this->translatePowerMode(powerMode);
//
//    if(powerTables.find(power) == powerTables.end()){
//      powerTables[power] = PowerTable();
//    }
//
//    PowerTable &powerTable=powerTables[power];
//
//    if(powerAtt->hasParameter("IDLE")){
//      std::string v = powerAtt->getParameter("IDLE");
//      const double value = atof(v.c_str());
//      powerTable[ComponentState::IDLE] = value;
//    }
//    if(powerAtt->hasParameter("RUNNING")){
//      std::string v = powerAtt->getParameter("RUNNING");
//      const double value = atof(v.c_str());
//      powerTable[ComponentState::RUNNING] = value;
//    }
//    if(powerAtt->hasParameter("STALLED")){
//      std::string v = powerAtt->getParameter("STALLED");
//      const double value = atof(v.c_str());
//      powerTable[ComponentState::STALLED] = value;
//    }
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
  LookupPowerTimeModelImpl::CompState  *LookupPowerTimeModelImpl::attachToComponent(
      AbstractComponent *comp)
  {
    return new CompState();
  }

  /// Change the power mode of a component. This should update the
  /// opaque object pointed to by execModelComponentState.
  void  LookupPowerTimeModelImpl::setPowerMode(
      AbstractExecModel::CompState *&execModelComponentState
    , std::string const             &mode) const
  {
    CompState &cs = *static_cast<CompState *>(execModelComponentState);

    PowerModeToIndex::const_iterator iter = powerModeToIndex.find(mode);
    assert(iter != powerModeToIndex.end());
    cs.powerMode = iter->second;
  }

  LookupPowerTimeModelImpl::ActionInfo *LookupPowerTimeModelImpl::registerAction(
      AbstractExecModel::CompState *&execModelComponentState
    , TaskInterface const           *actor
    , PossibleAction const          *action)
  {
    if (!registeredActions) {
      registeredActions = true;
      for (PowerModeDependentTimings::value_type const &entry : powerModeDependentTimings) {
        size_t index = powerModeToIndex.size();
        sassert(powerModeToIndex.insert(std::make_pair(entry.first, index)).second);
      }
    }

    size_t nrPowerModes = powerModeToIndex.size();

    PowerModeTiming *array = new PowerModeTiming[nrPowerModes]();
    powerModeTimingArrays.push_back(array);

    std::string const                   actorName   = actor->name();
    PossibleAction::FunctionNames const actionNames = action->getActionNames();
    PossibleAction::FunctionNames const guardNames  = action->getGuardNames();

    for (PowerModeToIndex::value_type const &entry : powerModeToIndex) {
      size_t         index = entry.second;

      PowerModeDependentTimings::const_iterator iter = powerModeDependentTimings.find(entry.first);
      assert(iter != powerModeDependentTimings.end());
      Timings const &timings = iter->second;

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
      array[index].guardDelay = guardDelay;
      array[index].dii        = dii;
      array[index].latency    = latency;
    }
    return reinterpret_cast<ActionInfo *>(array);

    /*
     *     FunctionTimingPtr timing =
        this->getTiming(this->getPowerMode(), pcb);

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

    ProcessControlBlock       *pcb = static_cast<ProcessControlBlock *>(actor->getSchedulerInfo());
    ProcessId           const  pid = pcb->getPid();

    assert(Director::getInstance().getProcessId(actorName) == pid);

    try {
      TimingsProvider::Ptr provider = this->getTimingsProvider();
      if (provider->hasDefaultActorTiming(actorName)) {

        SystemC_VPC::functionTimingsPM timingsPM = provider->getActionTimings(actorName);

        for (SystemC_VPC::functionTimingsPM::iterator it=timingsPM.begin() ; it != timingsPM.end(); it++ ) {
          std::string powermode = (*it).first;
          setTiming(provider->getActionTiming(actorName,powermode), pcb);
        }
      }
      for (std::string const &guard : guardNames) {
        if (provider->hasGuardTimings(guard)) {

          SystemC_VPC::functionTimingsPM timingsPM = provider->getGuardTimings(guard);
          for (SystemC_VPC::functionTimingsPM::iterator it=timingsPM.begin() ; it != timingsPM.end(); it++ )
          {
            std::string powermode = (*it).first;
            setTiming(provider->getGuardTiming(guard,powermode), pcb);
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
            setTiming(provider->getActionTiming(action,powermode), pcb);
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
      AbstractExecModel::CompState *&execModelComponentState
    , ActionInfo                    *ai
    , TaskInstance                  *ti
    , bool                           forGuard) const
  {
    CompState &cs = *static_cast<CompState *>(execModelComponentState);

    PowerModeTiming *array = reinterpret_cast<PowerModeTiming *>(ai);

    if (forGuard) {
      ti->setDelay(array[cs.powerMode].guardDelay);
      ti->setRemainingDelay(array[cs.powerMode].guardDelay);
      ti->setLatency(array[cs.powerMode].guardDelay);
    } else {
      ti->setDelay(array[cs.powerMode].dii);
      ti->setRemainingDelay(array[cs.powerMode].dii);
      ti->setLatency(array[cs.powerMode].latency);
    }
  }

} } } // namespace SystemC_VPC::Detail::ExecModelling
