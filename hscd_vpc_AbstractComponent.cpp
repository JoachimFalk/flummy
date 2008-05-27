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
#include <systemcvpc/hscd_vpc_Component.h>

namespace SystemC_VPC{

  //
  ComponentId AbstractComponent::globalComponentId = 0;

  //
  ComponentId AbstractComponent::getComponentId(){
    return this->componentId;
  }


} //namespace SystemC_VPC
