/******************************************************************************
 *                        Copyright 2008
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * RoutePool.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#ifndef HSCD_VPC_ROUTEPOOL_H
#define HSCD_VPC_ROUTEPOOL_H

#include <CoSupport/SystemC/systemc_support.hpp>

#include "Route.h"
#include "Pool.h"

namespace SystemC_VPC{

  /**
   * \brief a memory pool for routes
   */
  template<class ROUTE>
  class RoutePool
    : public PrototypedPool<ROUTE>,
      public Route
  {
  public:

    void addHop(std::string name, AbstractComponent * hop, FunctionId *layer = 0)
    {
      return this->getPrototype().addHop(name, hop, layer);
    }

    const ComponentList& getHops() const
    {
      return this->getPrototype().getHops();
    }


    void compute( Task* task )
    {
      ROUTE* route = this->allocate();
      route->setPool(this);
      route->compute( task );
    }

    const char* getName() const
    {
      return this->getPrototype().getName();
    }

    RoutePool( std::string source, std::string dest )
      : PrototypedPool<ROUTE>(source, dest), Route()
    {
    }
  };

}

#endif // HSCD_VPC_ROUTEPOOL_H
