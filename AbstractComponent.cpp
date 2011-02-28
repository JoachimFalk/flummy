/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/ComponentObserver.hpp>
#include <systemcvpc/ComponentInfo.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/PluggablePowerGovernor.hpp>
#include <systemcvpc/HysteresisLocalGovernor.hpp>


namespace SystemC_VPC{

  //
  const ComponentId Delayer::getComponentId() const{
    return this->componentId_;
  }

  AbstractComponent::Factories AbstractComponent::factories;

  const char* AbstractComponent::getName() const {
    return this->basename();
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


  /**
   *
   */
  bool AbstractComponent::setAttribute(AttributePtr attribute){
    if(processPower(attribute)){
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
