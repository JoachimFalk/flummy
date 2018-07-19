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
    if (portInterface.which() == 0) {
      portInterface = port;
      smoc::SimulatorAPI::ChannelSourceInterface *csi = port->getSource();
      assert(csi != nullptr);
      if (channelLinks.empty()) {
        /// setPortInterface has been called before acquireChannelInterface
        sassert(channelLinks.insert(std::make_pair(csi->name(),
            reinterpret_cast<ChannelInterface *>(csi))).second);
      } else {
        assert(channelLinks.size() == 1);
        ChannelLinks::iterator iter = channelLinks.begin();
        assert(iter->first == csi->name());
        assert(iter->second.cil);
        assert(*iter->second.cil == nullptr);
        iter->second.ci   = reinterpret_cast<ChannelInterface *>(csi);
        *iter->second.cil = reinterpret_cast<ChannelInterface *>(csi);
      }
    } else {
      assert(portInterface.which() == 1);
      assert(boost::get<PortInInterface *>(portInterface) == port);
    }
  }

  void AbstractRoute::setPortInterface(PortOutInterface *port) {
    if (portInterface.which() == 0) {
      portInterface = port;
      if (channelLinks.empty()) {
        /// setPortInterface has been called before acquireChannelInterface
        for (smoc::SimulatorAPI::ChannelSinkInterface *csi : port->getSinks()) {
          assert(csi != nullptr);
          sassert(channelLinks.insert(std::make_pair(csi->name(),
              reinterpret_cast<ChannelInterface *>(csi))).second);
        }
        assert(!channelLinks.empty());
      } else {
        size_t missing = channelLinks.size();
        for (smoc::SimulatorAPI::ChannelSinkInterface *csi : port->getSinks()) {
          assert(csi != nullptr);
          ChannelLinks::iterator iter = channelLinks.find(csi->name());
          assert(iter != channelLinks.end());
          assert(iter->second.cil);
          assert(*iter->second.cil == nullptr);
          iter->second.ci   = reinterpret_cast<ChannelInterface *>(csi);
          *iter->second.cil = reinterpret_cast<ChannelInterface *>(csi);
          --missing;
        }
        assert(missing == 0);
      }
    } else {
      assert(portInterface.which() == 2);
      assert(boost::get<PortOutInterface *>(portInterface) == port);
    }
  }

} } // namespace SystemC_VPC::Detail
