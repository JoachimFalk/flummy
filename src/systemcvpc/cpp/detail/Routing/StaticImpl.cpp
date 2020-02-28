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

#include "StaticImpl.hpp"
#include "../DebugOStream.hpp"

#include <CoSupport/Tracing/TracingFactory.hpp>
#include <CoSupport/Tracing/PtpTracer.hpp>

namespace SystemC_VPC { namespace Detail { namespace Routing {

  StaticImpl::StaticImpl(std::string const &name)
    : AbstractRoute(name,
        reinterpret_cast<char *>(static_cast<Route         *>(this)) -
        reinterpret_cast<char *>(static_cast<AbstractRoute *>(this)))
    , Static(
        reinterpret_cast<char *>(static_cast<AbstractRoute *>(this)) -
        reinterpret_cast<char *>(static_cast<Route         *>(this)))
    , firstHopImpl(nullptr)
  {
  }

  StaticImpl::Hop  *StaticImpl::addHop(Component::Ptr component, Hop *parent) {
    AbstractComponent::Ptr comp(
        static_cast<AbstractComponent *>(component.get()));
    std::pair<HopImpls::iterator, bool> status =
        hopImpls.insert(std::make_pair(comp, HopImpl(component)));
    assert(status.second);
    HopImpl &hopImpl = status.first->second;
    if (!parent) {
      assert(!firstHopImpl);
      firstHopImpl = &hopImpl;
    } else {
      HopImpl &parentImpl = *static_cast<HopImpl *>(parent);
#ifndef NDEBUG
      HopImpls::iterator iter = hopImpls.find(SystemC_VPC::getImpl(parent->getComponent()));
      assert(iter != hopImpls.end());
      assert(&iter->second == &parentImpl);
#endif //NDEBUG
      parentImpl.getChildHops().push_back(&hopImpl);
    }
    return &hopImpl;
  }

  void              StaticImpl::addDest(std::string const &chan, Hop *parent) {
    HopImpl &parentImpl = *static_cast<HopImpl *>(parent);
#ifndef NDEBUG
    HopImpls::iterator iter = hopImpls.find(SystemC_VPC::getImpl(parent->getComponent()));
    assert(iter != hopImpls.end());
    assert(&iter->second == &parentImpl);
#endif //NDEBUG
    acquireChannelInterface(chan, parentImpl.destinations);
  }

  StaticImpl::Hop  *StaticImpl::getFirstHop() {
    return firstHopImpl;
  }

  std::map<Component::Ptr, StaticImpl::Hop> const &StaticImpl::getHops() const {
    return reinterpret_cast<std::map<Component::Ptr, Hop> const &>(hopImpls);
  }

  void StaticImpl::start(size_t quantitiy, void *userData, CallBack completed) {
    // This will auto delete.
    size_t bytes = sizeof(MessageInstance) + sizeof(HopImpl *)*(getDestinations().size()-1);

    new(allocate(bytes)) MessageInstance(this, quantitiy, userData, completed);
  }

  void StaticImpl::recurseHop(HopImpl *hopImpl) {
    hopImpl->taskImpl = hopImpl->getComponent()->createTask(getName());
    hopImpl->taskImpl->setPriority(hopImpl->getPriority());
    for (HopImpl *childHop : hopImpl->getChildHops())
      recurseHop(childHop);
  }

  void StaticImpl::finalize() {
    recurseHop(firstHopImpl);
  }

  void StaticImpl::MessageInstance::startHop(size_t hop) {
    assert(currHop[hop] != nullptr);
    currHop[hop]->getComponent()
        ->executeHop(currHop[hop]->taskImpl, currHop[hop]->getTransferTiming(), quantitiy, [this, hop](TaskInstanceImpl *) {
          this->finishHop(hop);
      });
  }

  void StaticImpl::MessageInstance::finishHop(size_t hop) {
    for (ChannelInterface *ci : currHop[hop]->destinations) {
      (*completed)(userData, quantitiy, ci);
    }
    if (!currHop[hop]->getChildHops().empty()) {
      for (HopImpl *nextHop : currHop[hop]->getChildHops()) {
        currHop[hop] = nextHop;
        startHop(hop);
        hop = nextFreeCurrHop++;
      }
    } else {
      currHop[hop] = nullptr;
      for (size_t i = 0; i < nextFreeCurrHop; ++i) {
        if (currHop[i] != nullptr)
          // Still some stuff running
          return;
      }
      // Its all over
      static_cast<StaticImpl *>(route)->release(this);
    }
  }

  bool StaticImpl::closeStream(){
    for (HopImpls::value_type const &v : hopImpls) {
      v.second.getComponent()->closeStream(getRouteId());
    }
    return true;
  }

  bool StaticImpl::addStream(){
    for (HopImpls::value_type const &v : hopImpls) {
      v.second.getComponent()->addStream(getRouteId());
    }
    return true;
  }

  StaticImpl::~StaticImpl( ){
    DBG_OUT("Static::~Static( )" << std::endl);
  }


} } } // namespace SystemC_VPC::Detail::Routing
