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

#include <systemcvpc/Extending/ComponentObserverIf.hpp>
#include <systemcvpc/ConfigException.hpp>

#include <string>
#include <map>
#include <functional>
#include <stdexcept>

namespace SystemC_VPC { namespace {

  typedef std::map<
      std::string,
      std::function<Extending::ComponentObserverIf *(Attributes)>
    > ObserverByType;

  /// We need this to be independent from the global variable initialization order.
  static ObserverByType &getObserverByType() {
    static ObserverByType observerByType;
    return observerByType;
  }

  typedef std::map<
      std::string,
      ComponentObserver::Ptr
    > ObserverByName;

  /// We need this to be independent from the global variable initialization order.
  static ObserverByName &getObserverByName() {
    static ObserverByName observerByName;
    return observerByName;
  }

} }

namespace SystemC_VPC { namespace Extending {

  void ComponentObserverIf::registerObserver(
      const char      *type,
      FactoryFunction  factory)
  {
    ObserverByType &observerByType = getObserverByType();
    std::pair<ObserverByType::iterator, bool> status =
        observerByType.insert(ObserverByType::value_type(type, factory));
    if (!status.second)
      throw  std::runtime_error("Duplicate component observer type " + std::string(type)+"!");
  }

} } // namespace SystemC_VPC::Extending

namespace SystemC_VPC {

  ObserverTypeUnknown::ObserverTypeUnknown(const char *type)
    : ConfigException("No component observer of type "+std::string(type)+" registered!") {}

  ObserverNameUnknown::ObserverNameUnknown(const char *name)
    : ConfigException("No component observer of name "+std::string(name)+" registered!") {}

  ComponentObserver::Ptr createComponentObserver(
      char const       *type
    , char const       *name
    , Attributes const &attrs)
  {
    ObserverByType &observerByType = getObserverByType();

    ObserverByType::const_iterator iter = observerByType.find(type);
    if (iter == observerByType.end())
      throw ObserverTypeUnknown(type);

    ComponentObserver *observer = nullptr;

    if (name) {
      ObserverByName &observerByName = getObserverByName();
      std::pair<ObserverByName::iterator, bool> status =
          observerByName.insert(ObserverByName::value_type(name, nullptr));
      if (!status.second)
        throw ConfigException("Duplicate name "+std::string(name)+ " for creating component observer of type "+std::string(type)+"!");
      try {
        observer = iter->second(attrs)->getComponentObserver();
        status.first->second = observer;
      } catch (...) {
        observerByName.erase(status.first);
        throw;
      }
    } else {
      observer = iter->second(attrs)->getComponentObserver();
    }
    assert(observer != nullptr);
    assert(!strcmp(observer->getType(), type) && "Oops, type of observer does not match request!");
    return observer;
  }

  ComponentObserver::Ptr getComponentObserver(const char *name) {
    ObserverByName &observerByName = getObserverByName();

    ObserverByName::const_iterator iter = observerByName.find(name);
    if (iter == observerByName.end())
      throw ObserverNameUnknown(name);
    return iter->second;

    return nullptr;
  }

} // namespace SystemC_VPC
