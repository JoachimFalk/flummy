/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * hscd_vpc_AbstractComponent.cpp
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#include "hscd_vpc_AbstractComponent.h"
#include "ComponentObserver.h"
#include "ComponentInfo.h"

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
