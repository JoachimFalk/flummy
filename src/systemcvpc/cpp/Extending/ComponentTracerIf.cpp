// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2010 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
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

    ComponentTracer *tracer = nullptr;

    if (name) {
      TracerByName &tracerByName = getTracerByName();
      std::pair<TracerByName::iterator, bool> status =
          tracerByName.insert(TracerByName::value_type(name, nullptr));
      if (!status.second)
        throw ConfigException("Duplicate name "+std::string(name)+ " for creating component tracer of type "+std::string(type)+"!");
      try {
        tracer = iter->second(attrs)->getComponentTracer();
        status.first->second = tracer;
      } catch (...) {
        tracerByName.erase(status.first);
        throw;
      }
    } else {
      tracer = iter->second(attrs)->getComponentTracer();
    }
    assert(tracer != nullptr);
    assert(!strcmp(tracer->getType(), type) && "Oops, type of tracer does not match request!");
    return tracer;
  }

  ComponentTracer::Ptr getComponentTracer(const char *name) {
    TracerByName &tracerByName = getTracerByName();

    TracerByName::const_iterator iter = tracerByName.find(name);
    if (iter == tracerByName.end())
      throw TracerNameUnknown(name);
    return iter->second;
  }

} // namespace SystemC_VPC
