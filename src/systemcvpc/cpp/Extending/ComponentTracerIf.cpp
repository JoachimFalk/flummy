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

#include <systemcvpc/Extending/ComponentTracerIf.hpp>
#include <systemcvpc/ConfigException.hpp>

#include <string>
#include <map>
#include <functional>
#include <stdexcept>

namespace SystemC_VPC { namespace {

  typedef std::map<
      std::string,
      std::function<Extending::ComponentTracerIf *(Attributes)>
    > TracerByType;

  /// We need this to be independent from the global variable initialization order.
  static TracerByType &getTracerByType() {
    static TracerByType tracerByType;
    return tracerByType;
  }

  typedef std::map<
      std::string,
      ComponentTracer::Ptr
    > TracerByName;

  /// We need this to be independent from the global variable initialization order.
  static TracerByName &getTracerByName() {
    static TracerByName tracerByName;
    return tracerByName;
  }

} }

namespace SystemC_VPC { namespace Extending {

  void ComponentTracerIf::registerTracer(
      const char       *type,
      FactoryFunction  factory)
  {
    TracerByType &tracerByName = getTracerByType();
    std::pair<TracerByType::iterator, bool> status =
        tracerByName.insert(TracerByType::value_type(type, factory));
    if (!status.second)
      throw  std::runtime_error("Duplicate component tracer type " + std::string(type)+"!");
  }

} } // namespace SystemC_VPC::Extending

namespace SystemC_VPC {

  TracerTypeUnknown::TracerTypeUnknown(const char *type)
    : ConfigException("No component tracer of type "+std::string(type)+" registered!") {}

  TracerNameUnknown::TracerNameUnknown(const char *name)
    : ConfigException("No component tracer of name "+std::string(name)+" registered!") {}

  ComponentTracer::Ptr createComponentTracer(
      char const       *type
    , char const       *name
    , Attributes const &attrs)
  {
    TracerByType &tracerByType = getTracerByType();

    TracerByType::const_iterator iter = tracerByType.find(type);
    if (iter == tracerByType.end())
      throw TracerTypeUnknown(type);

    if (name) {
      TracerByName &tracerByName = getTracerByName();
      std::pair<TracerByName::iterator, bool> status =
          tracerByName.insert(TracerByName::value_type(name, nullptr));
      if (!status.second)
        throw ConfigException("Duplicate name "+std::string(name)+ " for creating component tracer of type "+std::string(type)+"!");
      try {
        return status.first->second = iter->second(attrs)->getComponentTracer();
      } catch (...) {
        tracerByName.erase(status.first);
        throw;
      }
    } else
      return iter->second(attrs)->getComponentTracer();
  }

  ComponentTracer::Ptr getComponentTracer(const char *name) {
    TracerByName &tracerByName = getTracerByName();

    TracerByName::const_iterator iter = tracerByName.find(name);
    if (iter == tracerByName.end())
      throw TracerNameUnknown(name);
    return iter->second;
  }

} // namespace SystemC_VPC
