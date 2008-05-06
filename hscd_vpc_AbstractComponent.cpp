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
#include <hscd_vpc_Component.h>

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
} //namespace SystemC_VPC
