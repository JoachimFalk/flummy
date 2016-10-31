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

#ifndef HSCD_VPC_ROUTEPOOL_H
#define HSCD_VPC_ROUTEPOOL_H

#include <CoSupport/SystemC/systemc_support.hpp>

#include "config/Route.hpp"
#include <systemcvpc/RouteImpl.hpp>
#include <systemcvpc/PCBPool.hpp>

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

    void addHop(std::string name, AbstractComponent * hop)
    {
      return this->getPrototype().addHop(name, hop);
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

    std::string getName() const
    {
      return this->getPrototype().getName();
    }

    void setRouteInterface(Route* route){
      configuredRoute_->routeInterface_=route;
    }

    RoutePool( const Config::Route::Ptr configuredRoute )
      : PrototypedPool<ROUTE>(configuredRoute), Route(configuredRoute)
    {
      configuredRoute_=configuredRoute;
    }

    Config::Route::Ptr configuredRoute_;
  };

}

#endif // HSCD_VPC_ROUTEPOOL_H
