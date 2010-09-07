/******************************************************************************
 *                        Copyright 2008
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * AbstractComponent.h
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

#include <CoSupport/Tracing/TracingFactory.hpp>

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

    Route() : Delayer(), instanceId(createRouteId()), enableTracer(false){}

    Route(const Route & orig) : Delayer(orig), instanceId(createRouteId()),
        enableTracer(orig.enableTracer), ptpTracer(orig.ptpTracer) {}

    virtual ~Route(){}

    int getInstanceId() const
    {
      return instanceId;
    }

    virtual void enableTracing(bool enable){
      enableTracer = enable && ptpTracer != NULL;
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
