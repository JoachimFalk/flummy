/******************************************************************************
 *                        Copyright 2008
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * hscd_vpc_AbstractComponent.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#ifndef HSCD_VPC_ROUTE_H
#define HSCD_VPC_ROUTE_H

#include <vector>

#include "hscd_vpc_AbstractComponent.h"

namespace SystemC_VPC{

  /**
   * \brief Interface for classes implementing routing simulation.
   */
  class Route : public Delayer {
  public:
    virtual void addHop(std::string name, AbstractComponent * hop) = 0;

    Route() : Delayer() {}

    virtual ~Route(){}
  };

}

#endif // HSCD_VPC_ROUTE_H
