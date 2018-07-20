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
#include "../Director.hpp"
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
    hopImpl.pcb = hopImpl.getComponent()->createPCB(getName());
    hopImpl.pcb->setTiming(hopImpl.getTransferTiming());
    hopImpl.pcb->setPriority(hopImpl.getPriority());
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

  void StaticImpl::start(size_t quantitiy, std::function<void ()> completed) {
    // This will auto delete.
    new MessageInstance(this, quantitiy, completed);
  }

  StaticImpl::MessageInstance::MessageInstance(
      StaticImpl             *staticImpl,
      size_t                  quantitiy,
      std::function<void ()>  completed)
    : staticImpl(staticImpl)
    , quantitiy(quantitiy)
    , completed(completed)
    , currHop(staticImpl->firstHopImpl)
  {
    startHop();
  }

  class Visitor: public boost::static_visitor<void> {
  public:
    Visitor(int n) : n(n) {}

    result_type operator()(smoc::SimulatorAPI::PortInInterface *in) const {
//    in->getSource()->commFinish(n);
    }

    result_type operator()(smoc::SimulatorAPI::PortOutInterface *out) const {
      for (smoc::SimulatorAPI::ChannelSinkInterface *sink : out->getSinks())
        sink->commFinish(n);
    }

    result_type operator()(boost::blank &) const {
      assert(!"WTF?!");
    }


  private:
    int n;
  };

  void StaticImpl::MessageInstance::startHop() {
    if (!currHop) {
      boost::apply_visitor(Visitor(quantitiy), staticImpl->portInterface);
      completed();
      delete this;
      return;
    }
    currHop->getComponent()
        ->executeHop(currHop->pcb, quantitiy, [this](TaskInstance *) {
          this->finishHop();
      });
  }

  void StaticImpl::MessageInstance::finishHop() {
    if (currHop->getChildHops().empty()) {
      currHop = nullptr;
    } else {
      currHop = currHop->getChildHops().front();
    }
    startHop();
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
