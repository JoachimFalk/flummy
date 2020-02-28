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

#include <systemcvpc/Extending/ComponentObserverIf.hpp>
#include <systemcvpc/ConfigException.hpp>

#include <string>
#include <map>
#include <functional>
#include <stdexcept>

namespace {

  typedef std::map<
      std::string,
      std::function<SystemC_VPC::Extending::ComponentObserverIf *()>
    > ObserverByName;

  /// We need this to be independent from the global variable initialization order.
  static ObserverByName &getObserverByName() {
    static ObserverByName observerByName;
    return observerByName;
  }

}

namespace SystemC_VPC { namespace Extending {

  void ComponentObserverIf::registerObserver(
      const char                             *type,
      std::function<ComponentObserverIf *()>  factory) {

    ObserverByName &observerByName = getObserverByName();
    std::pair<ObserverByName::iterator, bool> status =
        observerByName.insert(ObserverByName::value_type(type, factory));
    if (!status.second)
      throw  std::runtime_error("Duplicate component observer type " + std::string(type)+"!");
  }

} } // namespace SystemC_VPC::Extending

namespace SystemC_VPC {

  ComponentObserver::Ptr createComponentObserver(const char *type) {
    ObserverByName &observerByName = getObserverByName();

    ObserverByName::const_iterator iter = observerByName.find(type);
    if (iter == observerByName.end())
      throw ConfigException("No component observer of type "+std::string(type)+" registered!");
    return iter->second()->getComponentObserver();
  }

} // namespace SystemC_VPC
