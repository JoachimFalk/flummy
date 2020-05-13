// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include "AbstractRoute.hpp"
#include "Director.hpp"

#include <CoSupport/sassert.h>

namespace SystemC_VPC { namespace Detail {

  AbstractRoute::AbstractRoute(std::string const &name, int facadeAdj)
    : routeName(name)
    , routeId(Director::getInstance().getProcessId(name))
    , facadeAdj(facadeAdj)
    , ticket(-1)
  {
//  if (configuredRoute->getTracing()) {
//    this->ptpTracer
//      = CoSupport::Tracing::TracingFactory::getInstance().createPtpTracer(
//          this->getName());
//  }
  }

  void AbstractRoute::setPortInterface(PortInInterface  *port) {
    assert(portInterface.which() == 0);
    portInterface = port;
    smoc::SimulatorAPI::ChannelSourceInterface *csi = port->getSource();
    assert(csi != nullptr);
    /// FIXME: This is the case for IgnoreImpl routes where
    /// acquireChannelInterface is not used.
    if (channelLinks.empty())
      return;
    assert(channelLinks.size() == 1);
    ChannelLinks::iterator iter = channelLinks.begin();
    assert(iter->first == csi->name() || iter->first == "DEFAULT");
    assert(iter->second.link);
    iter->second.link->push_back(reinterpret_cast<ChannelInterface *>(csi));
    destinations.push_back(Destination(csi->name(), reinterpret_cast<ChannelInterface *>(csi)));
  }

  void AbstractRoute::setPortInterface(PortOutInterface *port) {
    assert(portInterface.which() == 0);
    portInterface = port;
    size_t provided  = port->getSinks().size();
    assert(provided != 0);
    /// FIXME: This is the case for IgnoreImpl routes where
    /// acquireChannelInterface is not used.
    if (channelLinks.empty())
      return;
    size_t requested = channelLinks.size();
    assert(requested == provided);
    if (requested == 1) {
      smoc::SimulatorAPI::ChannelSinkInterface *csi = *port->getSinks().begin();
      ChannelLinks::iterator iter = channelLinks.begin();
      assert(iter->first == csi->name() || iter->first == "DEFAULT");
      assert(iter->second.link);
      iter->second.link->push_back(reinterpret_cast<ChannelInterface *>(csi));
      destinations.push_back(Destination(csi->name(), reinterpret_cast<ChannelInterface *>(csi)));
    } else {
      for (smoc::SimulatorAPI::ChannelSinkInterface *csi : port->getSinks()) {
        assert(csi != nullptr);
        ChannelLinks::iterator iter = channelLinks.find(csi->name());
        assert(iter != channelLinks.end());
        assert(iter->second.link);
        iter->second.link->push_back(reinterpret_cast<ChannelInterface *>(csi));
        destinations.push_back(Destination(csi->name(), reinterpret_cast<ChannelInterface *>(csi)));
      }
    }
  }

  void AbstractRoute::acquireChannelInterface(
      std::string               const &chan,
      std::vector<ChannelInterface *> &link)
  {
    // acquireChannelInterface must be called before setPortInterface.
    assert(portInterface.which() == 0);
    sassert(channelLinks.insert(
        ChannelLinks::value_type(chan, link)).second);
  }

} } // namespace SystemC_VPC::Detail
