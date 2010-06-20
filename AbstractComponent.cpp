/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * AbstractComponent.cpp
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/ComponentObserver.hpp>
#include <systemcvpc/ComponentInfo.hpp>

namespace SystemC_VPC{

  //
  ComponentId Delayer::globalComponentId = 0;

  //
  ComponentId Delayer::getComponentId(){
    return this->componentId;
  }

  const char* AbstractComponent::getName() const {
    return this->basename();
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
