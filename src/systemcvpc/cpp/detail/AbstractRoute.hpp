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

#include <systemcvpc/Route.hpp>

#include "AbstractComponent.hpp"

#include <smoc/SimulatorAPI/PortInterfaces.hpp>

#include <CoSupport/Tracing/TracingFactory.hpp>

#include <vector>

namespace SystemC_VPC { namespace Detail {

  typedef std::list<AbstractComponent *> ComponentList;

  /**
   * \brief Interface for classes implementing routing simulation.
   */
  class AbstractRoute {
    typedef AbstractRoute this_type;
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type> const ConstPtr;

    AbstractRoute(std::string const &name);

    virtual void start(size_t quantitiy, VPCEvent::Ptr finishEvent) = 0;

    std::string const &getName() const
      { return routeName; }

    int getRouteId() const
      { return routeId; }

    virtual
    Route       *getRoute() = 0;
    Route const *getRoute() const
      { return const_cast<this_type *>(this)->getRoute(); }

    virtual ~AbstractRoute(){}
  protected:
    void traceStart() {
      if (ptpTracer) ticket = ptpTracer->startOoo();
    }

    void traceStop() {
      if (ptpTracer) ptpTracer->stopOoo(ticket);
    }
  private:
    std::string const   routeName;
    RouteId             routeId;

    CoSupport::Tracing::PtpTracer::Ptr     ptpTracer;
    CoSupport::Tracing::PtpTracer::Ticket  ticket;
  };

  inline
  void intrusive_ptr_add_ref(AbstractRoute *p) {
    p->getRoute()->add_ref();
  }

  inline
  void intrusive_ptr_release(AbstractRoute *p) {
    if (p->getRoute()->del_ref())
      // AbstractRoute has virtual destructor
      delete p;
  }

  static inline
  AbstractRoute *getAbstractRouteOfPort(smoc::SimulatorAPI::PortInInterface const &port)
    { return static_cast<AbstractRoute *>(port.getSchedulerInfo()); }
  static inline
  AbstractRoute *getAbstractRouteOfPort(smoc::SimulatorAPI::PortOutInterface const &port)
    { return static_cast<AbstractRoute *>(port.getSchedulerInfo()); }

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_ROUTEIMPL_HPP */
