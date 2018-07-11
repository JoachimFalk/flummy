// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_ROUTEIMPL_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_ROUTEIMPL_HPP

#include <vector>

#include <CoSupport/Tracing/TracingFactory.hpp>

#include <systemcvpc/Route.hpp>
#include "AbstractComponent.hpp"

namespace SystemC_VPC { namespace Detail {

  typedef std::list<AbstractComponent *> ComponentList;

  /**
   * \brief Interface for classes implementing routing simulation.
   */
  class Route : public Delayer, public Config::RouteInterface {
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
      configuredRoute->routeInterface_ = this;
    }

    Route(const Route & orig) : Delayer(orig), instanceId(createRouteId()),
        ptpTracer(orig.ptpTracer) {}

    virtual ~Route(){}

    int getInstanceId() const
    {
      return instanceId;
    }

    virtual void notifyActivation(smoc::SimulatorAPI::TaskInterface *scheduledTask,
        bool active);
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

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_ROUTEIMPL_HPP */
