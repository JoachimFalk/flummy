// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
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

#include "IgnoreImpl.hpp"
#include "../DebugOStream.hpp"

#include <CoSupport/Tracing/TracingFactory.hpp>
#include <CoSupport/Tracing/PtpTracer.hpp>

namespace SystemC_VPC { namespace Detail { namespace Routing {

  IgnoreImpl::IgnoreImpl(std::string const &name)
    : AbstractRoute(name,
        reinterpret_cast<char *>(static_cast<Route         *>(this)) -
        reinterpret_cast<char *>(static_cast<AbstractRoute *>(this)))
    , Ignore(
         reinterpret_cast<char *>(static_cast<AbstractRoute *>(this)) -
         reinterpret_cast<char *>(static_cast<Route         *>(this)))
  {
  }

  class IgnoreImpl::Visitor: public boost::static_visitor<void> {
  public:
    Visitor(int quantity, void *userData, IgnoreImpl::CallBack callBack)
      : quantity(quantity), userData(userData), callBack(callBack) {}

    result_type operator()(smoc::SimulatorAPI::PortInInterface *in) const {
      (*callBack)(userData, quantity, reinterpret_cast<IgnoreImpl::ChannelInterface *>(in->getSource()));
    }

    result_type operator()(smoc::SimulatorAPI::PortOutInterface *out) const {
      for (smoc::SimulatorAPI::ChannelSinkInterface *sink : out->getSinks()) {
        (*callBack)(userData, quantity, reinterpret_cast<IgnoreImpl::ChannelInterface *>(sink));
      }
    }

    result_type operator()(boost::blank &) const {
      assert(!"WTF?!");
    }
  private:
    int                   quantity;
    void                 *userData;
    IgnoreImpl::CallBack  callBack;
  };

  void IgnoreImpl::start(size_t quantitiy, void *userData, CallBack completed) {
    boost::apply_visitor(Visitor(quantitiy, userData, completed), portInterface);
  }

  void IgnoreImpl::finalize() {
  }

  IgnoreImpl::~IgnoreImpl( ){
    DBG_OUT("Ignore::~Ignore( )" << std::endl);
  }


} } } // namespace SystemC_VPC::Detail::Routing
