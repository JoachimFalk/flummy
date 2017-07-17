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

#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/ComponentObserver.hpp>
#include <systemcvpc/ComponentInfo.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/PluggablePowerGovernor.hpp>
#include <systemcvpc/HysteresisLocalGovernor.hpp>

#include "tracing/TracerIf.hpp"

namespace SystemC_VPC {

  AbstractComponent::Factories AbstractComponent::factories;


  AbstractComponent::~AbstractComponent() {
    this->timingPools.clear();
    if (this->taskTracer_)
      delete this->taskTracer_;
  }

  void AbstractComponent::addTracer(Trace::TracerIf *tracer) {
    assert(taskTracer_ == nullptr);
    taskTracer_ = tracer;
  }

  bool AbstractComponent::processPower(AttributePtr attPtr)
  {
    // hierarchical format
    if(!attPtr->isType("powermode")) {
      return false;
    }

    for(size_t i=0; i<attPtr->getAttributeSize();++i){
      AttributePtr powerAtt = attPtr->getNextAttribute(i).second;
      if(powerAtt->isType("governor")){
        this->loadLocalGovernorPlugin(powerAtt->getValue());
        powerAttribute = powerAtt;
        continue;
      }

      std::string powerMode = attPtr->getNextAttribute(i).first;
      const PowerMode *power = this->translatePowerMode(powerMode);

      if(powerTables.find(power) == powerTables.end()){
        powerTables[power] = PowerTable();
      }

      PowerTable &powerTable=powerTables[power];

      if(powerAtt->hasParameter("IDLE")){
        std::string v = powerAtt->getParameter("IDLE");
        const double value = atof(v.c_str());
        powerTable[ComponentState::IDLE] = value;
      }
      if(powerAtt->hasParameter("RUNNING")){
        std::string v = powerAtt->getParameter("RUNNING");
        const double value = atof(v.c_str());
        powerTable[ComponentState::RUNNING] = value;
      }
      if(powerAtt->hasParameter("STALLED")){
        std::string v = powerAtt->getParameter("STALLED");
        const double value = atof(v.c_str());
        powerTable[ComponentState::STALLED] = value;
      }
      if(powerAtt->hasParameter("transaction_delay")) {
        this->transactionDelays[power] =
          Director::createSC_Time(powerAtt->getParameter("transaction_delay"));
      }
      if(powerAtt->hasParameter("transfer_delay")) {
        this->transactionDelays[power] =
          Director::createSC_Time(powerAtt->getParameter("transfer_delay"));
      }

    }

    return true;
  }

  bool AbstractComponent::processMCG(AttributePtr attPtr)
  {
    // hierarchical format
    if(!attPtr->isType("multicastgroup")) {
      return false;
    }
    MultiCastGroup mcg = attPtr->getValue();

    for(size_t i=0; i<attPtr->getAttributeSize();++i){
         AttributePtr mcgAtt = attPtr->getNextAttribute(i).second;
         if(mcgAtt->isType("task")){
             std::string task = mcgAtt->getValue();
             ProcessId pid = Director::getInstance().getProcessId(task);
             multiCastGroups[pid] = mcg;
         }
    }
    return true;
  }


  /**
   *
   */
  bool AbstractComponent::setAttribute(AttributePtr attribute){
    if(processPower(attribute)){
      return true;
    }else if(processMCG(attribute)){
        return true;
    }

    return false;
  }

  void AbstractComponent::loadLocalGovernorPlugin(std::string plugin){
    //std::cerr << "Component::loadLocalGovernorPlugin" << std::endl;

    if(plugin == "") return;

    if(AbstractComponent::factories.find(plugin)
        == AbstractComponent::factories.end()){
      AbstractComponent::factories[plugin] =
        new DLLFactory<PlugInFactory<PluggableLocalPowerGovernor> >
          (plugin.c_str());
    }

    localGovernorFactory = AbstractComponent::factories[plugin]->factory;
  }

  /*
   * This function sets the appropriate execution state of the component according to the component powerstate
   * (component's power state info is not encapsulated here, so it is the responsability of the powerState object to call this
   * function whenever a powermode change takes place.
   *
   * Assumptions:
   * "Disabling" a component (i.e. SLEEPING execution state) will remember the previous state and come back to it
   * when leaving SLEEPING state)
   *
  */
  void AbstractComponent::forceComponentState(const PowerMode * newPowerMode)
  {
	  //TODO: Generelize for new powermodes

	  // Moving into powerGated mode and not already there? then change to sleeping mode
	  if(newPowerMode->getName() == PowerMode::powerGated && this->powerMode->getName() != PowerMode::powerGated)
	  {
		  //this->previousComponentState = this->componentState;
		  //this->componentState = ComponentState::SLEEPING;
	  }
	  else
	  {
		  //Leaving powerGated? then return to previous execution state
		  if(this->powerMode->getName() == PowerMode::powerGated && newPowerMode->getName() != PowerMode::powerGated)
		  {
			 // this->componentState = this->previousComponentState;
			 // this->previousComponentState = ComponentState::SLEEPING;
		  }

	  }
  }

  void AbstractComponent::setPowerMode(const PowerMode* mode){
    this->powerMode = translatePowerMode(mode->getName());
    this->forceComponentState(mode);
    this->updatePowerConsumption();

    if(timingPools.find(powerMode) == timingPools.end()){
      timingPools[powerMode].reset(new FunctionTimingPool());
    }
    this->timingPool = timingPools[powerMode];
  }


  FunctionTimingPtr AbstractComponent::getTiming(const PowerMode *mode, ProcessId pid){

	  if(timingPools.find(mode) == timingPools.end()){
          timingPools[mode].reset(new FunctionTimingPool());
        }
        FunctionTimingPoolPtr pool = this->timingPools[mode];
        if(pool->find(pid) == pool->end()){
          (*pool)[pid].reset(new FunctionTiming());
          (*pool)[pid]->setBaseDelay(this->transactionDelays[mode]);
          //sc_core::sc_time a = this->transactionDelays[mode];
          //std::cout << a;
        }
        return (*pool)[pid];
      }

  const ComponentId Delayer::getComponentId() const{
    return this->componentId_;
  }

  void Delayer::addObserver(ComponentObserver *obs)
  {
    observers.push_back(obs);
  }

  void Delayer::removeObserver(ComponentObserver *obs)
  {
    for(Observers::iterator iter = observers.begin();
        iter != observers.end();
        ++iter)
    {
      if(*iter == obs) {
        observers.erase(iter);
        break;
      }
    }
  }
      
  void Delayer::fireNotification(ComponentInfo *compInf)
  {
    for(Observers::iterator iter = observers.begin();
        iter != observers.end();
        ++iter)
    {
      (*iter)->notify(compInf);
    }
  }

} //namespace SystemC_VPC
