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

#ifndef HSCD_VPC_ROUTE_H
#define HSCD_VPC_ROUTE_H

#include <vector>

#include <CoSupport/Tracing/TracingFactory.hpp>

#include "config/Route.hpp"
#include "AbstractComponent.hpp"

namespace SystemC_VPC{

  typedef std::list<AbstractComponent *> ComponentList;

  /**
   * \brief Interface for classes implementing routing simulation.
   */
  class Route : public Delayer {
  public:
    virtual void addHop(std::string name, AbstractComponent * hop) = 0;

    virtual const ComponentList& getHops() const = 0;

    Route(Config::Route::Ptr configuredRoute) : Delayer(
        configuredRoute->getComponentId(), configuredRoute->getName()),
        instanceId(createRouteId()),
        ptpTracer()
    {
      if (configuredRoute->getTracing()) {
        this->ptpTracer
          = CoSupport::Tracing::TracingFactory::getInstance() .createPtpTracer(
              this->getName());
      }
    }

    Route(const Route & orig) : Delayer(orig), instanceId(createRouteId()),
        ptpTracer(orig.ptpTracer) {}

    virtual ~Route(){}

    int getInstanceId() const
    {
      return instanceId;
    }

  protected:
    void traceStart() {
      if (ptpTracer) ticket = ptpTracer->startOoo();
    }

    void traceStop() {
      if (ptpTracer) ptpTracer->stopOoo(ticket);
    }
  private:
    size_t createRouteId();

    const int instanceId;
    CoSupport::Tracing::PtpTracer::Ptr     ptpTracer;
    CoSupport::Tracing::PtpTracer::Ticket  ticket;
  };

}

#endif // HSCD_VPC_ROUTE_H
