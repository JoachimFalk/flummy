// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2011 Tim Grocki <grocki@codesign.informatik.uni-erlangen.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_BLOCKINGTRANSPORT_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_BLOCKINGTRANSPORT_HPP

#include <systemcvpc/Route.hpp>
#include <systemcvpc/EventPair.hpp>

#include "../AbstractRoute.hpp"
#include "../AbstractComponent.hpp"

#include <systemc>

#include <list>
#include <utility>

namespace SystemC_VPC { namespace Detail { namespace Routing {

  /**
   *
   */
  class BlockingTransport
    : public AbstractRoute
    , public Route
//  , public CoSupport::SystemC::Event
//  , protected CoSupport::SystemC::EventListener
  {
  public:
    typedef std::list<AbstractComponent *> ComponentList;

    static const char *Type;

    BlockingTransport(std::string const &name);

    ~BlockingTransport();

    ///
    /// Handle interfaces for SystemC_VPC::Route
    ///

    // For resolving ambiguity
    using AbstractRoute::getName;
    using AbstractRoute::getRouteId;

  private:
    ///
    /// Handle interfaces for AbstractRoute
    ///

    void start(size_t quantitiy, void *userData, CallBack completed);

    void finalize();

    ///
    /// Other stuff
    ///

//  void compute( TaskInstanceImpl* task );
//
//  void route( EventPair np );
//
//  void signaled(EventWaiter *e);
//
//  void eventDestroyed(EventWaiter *e);

    void addHop(std::string name, AbstractComponent * hop);

    const ComponentList& getHops() const;

    void resetHops();
    void resetLists();

  private:
    enum Phase {
      LOCK_ROUTE,
      COMPUTE_ROUTE
    };
    typedef std::list<std::pair<AbstractComponent *, TaskInstanceImpl *> > Components;

    Components                             hopList;
    Components                             lockList;
    Components::iterator                   nextHop;
    ComponentList                          components;

    // a rout is either input (read) or output (write)
    bool                                   isWrite;

    TaskInstanceImpl*                          task;
    EventPair                              taskEvents;
    VPCEvent::Ptr                          dummyDii;
    VPCEvent::Ptr                          routeLat;

    // blocking transport has two phases:
    // - lock the route
    // - apply the route
    Phase                                  phase;

    
  };

} } } // namespace SystemC_VPC::Detail::Routing

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_BLOCKINGTRANSPORT_HPP */
