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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_STATICROUTE_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_STATICROUTE_HPP

#include <systemcvpc/config/Route.hpp>
#include <systemcvpc/EventPair.hpp>

#include "RouteImpl.hpp"
#include "Director.hpp"

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TracingFactory.hpp>

#include <systemc>

#include <list>

namespace SystemC_VPC { namespace Detail {

  template<class ROUTE>
  class RoutePool;

  /**
   *
   */
  class StaticRoute :
    public Route,
    public CoSupport::SystemC::Event,
    protected CoSupport::SystemC::EventListener {
  public:

    void compute( TaskInstance* task );

    FunctionId getFunctionId(ProcessId pid, std::string function);

    void route( EventPair np );

    void signaled(EventWaiter *e);

    void eventDestroyed(EventWaiter *e);

    void addHop(std::string name, AbstractComponent * hop);

    void setPool(RoutePool<StaticRoute> * pool);

    const ComponentList& getHops() const;

    StaticRoute( Config::Route::Ptr configuredRoute );

    StaticRoute( const StaticRoute & route );

    ~StaticRoute( );

    virtual bool closeStream(){
      std::list<AbstractComponent *>::iterator it;
       for ( it=components.begin() ; it != components.end(); it++ ){
         ProcessId pid= Director::getInstance().getProcessId(this->getName());
         (*it)->closeStream(pid);
       }
       return true;
    }

    virtual bool addStream(){
      std::list<AbstractComponent *>::iterator it;
        for ( it=components.begin() ; it != components.end(); it++ ){
          ProcessId pid= Director::getInstance().getProcessId(this->getName());
          (*it)->addStream(pid);
        }
        return true;
    }

  private:
    typedef std::list<AbstractComponent *> Components;

    Components                             components;
    TaskInstance                          *task;
    EventPair                              taskEvents;
    Coupling::VPCEvent::Ptr                dummyDii;
    Coupling::VPCEvent::Ptr                routeLat;
    Components::iterator                   nextHop;
    RoutePool<StaticRoute>                *pool;
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_STATICROUTE_HPP */
