// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2020 Hardware-Software-CoDesign, University of
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

#include "LegacyComponentObserver.hpp"

#include <systemcvpc/ConfigException.hpp>

namespace SystemC_VPC { namespace Detail {

  LegacyComponentObserver::LegacyComponentObserver()
    : Extending::ComponentObserverIf(
          reinterpret_cast<char *>(static_cast<ComponentObserver              *>(this)) -
          reinterpret_cast<char *>(static_cast<Extending::ComponentObserverIf *>(this))
        , 0, 0)
    , ComponentObserver(
         reinterpret_cast<char *>(static_cast<Extending::ComponentObserverIf *>(this)) -
         reinterpret_cast<char *>(static_cast<ComponentObserver              *>(this))
       , "legacy")
    {}

  void LegacyComponentObserver::componentOperation(ComponentOperation co
    , Component       const &c)
    { this->notify(const_cast<Component *>(&c)); }

  void LegacyComponentObserver::taskOperation(TaskOperation to
    , Component       const &c
    , Extending::Task const &t
    , OTask                 &ot)
    { this->notify(const_cast<Component *>(&c)); }

  void LegacyComponentObserver::taskInstanceOperation(TaskInstanceOperation tio
    , Component               const &c
    , Extending::TaskInstance const &ti
    , OTask                         &ot
    , OTaskInstance                 &oti)
    { this->notify(const_cast<Component *>(&c)); }

  bool LegacyComponentObserver::addAttribute(AttributePtr attr) {
    throw ConfigException("Legacy component observers do not support attributes!");
  }

} } // namespace SystemC_VPC::Detail
