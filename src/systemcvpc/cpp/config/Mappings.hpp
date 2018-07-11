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

#ifndef _INCLUDED_SYSTEMCVPC_CONFIG_MAPPINGS_HPP
#define _INCLUDED_SYSTEMCVPC_CONFIG_MAPPINGS_HPP

#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/Route.hpp>
#include <systemcvpc/config/VpcTask.hpp>

#include <map>

#include <systemc>

namespace SystemC_VPC {

class AbstractComponent;
class Route;

} // namespace SystemC_VPC

namespace SystemC_VPC { namespace Config {

typedef std::map<const ProcessId, Route::Ptr> Routes;
typedef std::map<const sc_core::sc_port_base *, Route::Ptr> RoutesByPort;

namespace Mappings
{
  // TODO: replace other mapping maps
  // TODO: provide getter, setter, etc.
  std::map<VpcTask::Ptr, Component::Ptr > & getConfiguredMappings();
  std::map<Component::Ptr, AbstractComponent * > & getComponents();
  bool isMapped(VpcTask::Ptr task, Component::Ptr component);

} // namespace Mappings

namespace Routing
{
  void add(const ProcessId pid, Route::Ptr route);
  void add(const sc_core::sc_port_base * leafPort, Route::Ptr route);

  void set(const ProcessId pid, Route::Ptr route);
  void set(const sc_core::sc_port_base * leafPort, Route::Ptr route);

  bool has(const ProcessId pid);
  bool has(const sc_core::sc_port_base * leafPort);

  Route::Ptr get(const ProcessId pid);
  Route::Ptr get(const sc_core::sc_port_base * leafPort);
  SystemC_VPC::Route * create(Config::Route::Ptr configuredRoute);

} // namespace Routing

} } // namespace SystemC_VPC::Config

#endif /* _INCLUDED_SYSTEMCVPC_CONFIG_MAPPINGS_HPP */
