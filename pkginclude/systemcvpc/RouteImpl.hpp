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
        configuredRoute->getComponentId()),
        instanceId(createRouteId()),
        enableTracer(false) {}

    Route(const Route & orig) : Delayer(orig), instanceId(createRouteId()),
        enableTracer(orig.enableTracer), ptpTracer(orig.ptpTracer) {}

    virtual ~Route(){}

    int getInstanceId() const
    {
      return instanceId;
    }

    virtual void enableTracing(bool enable){
      enableTracer = enable && ptpTracer;
    }

  protected:
    void traceStart() {
      assert(ptpTracer != NULL);
      if (enableTracer) ticket = ptpTracer->startOoo();
    }

    void traceStop() {
      assert(ptpTracer != NULL);
      if (enableTracer) ptpTracer->stopOoo(ticket);
    }

    void setPtpTracer(CoSupport::Tracing::PtpTracer::Ptr     ptpTracer){
      this->ptpTracer = ptpTracer;
    }
  private:
    size_t createRouteId();

    const int instanceId;
    bool enableTracer;
    CoSupport::Tracing::PtpTracer::Ptr     ptpTracer;
    CoSupport::Tracing::PtpTracer::Ticket  ticket;
  };

}

#endif // HSCD_VPC_ROUTE_H
