// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
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

#include "LegacyComponentObserver.hpp"

#include <systemcvpc/ConfigException.hpp>

namespace SystemC_VPC { namespace Detail {

  LegacyComponentObserver::LegacyComponentObserver()
    : Extending::ComponentObserverIf(
          reinterpret_cast<char *>(static_cast<ComponentObserver              *>(this)) -
          reinterpret_cast<char *>(static_cast<Extending::ComponentObserverIf *>(this))
        , 0, 0, 0)
    , ComponentObserver(
         reinterpret_cast<char *>(static_cast<Extending::ComponentObserverIf *>(this)) -
         reinterpret_cast<char *>(static_cast<ComponentObserver              *>(this))
       , "legacy")
    {}

  void LegacyComponentObserver::componentOperation(ComponentOperation co
    , Component const &c
    , OComponent      &oc)
    { this->notify(const_cast<Component *>(&c)); }

  void LegacyComponentObserver::taskOperation(TaskOperation to
    , Component const &c
    , OComponent      &oc
    , Task      const &t
    , OTask           &ot)
    { this->notify(const_cast<Component *>(&c)); }

  void LegacyComponentObserver::taskInstanceOperation(TaskInstanceOperation tio
    , Component    const &c
    , OComponent         &oc
    , OTask              &ot
    , TaskInstance const &ti
    , OTaskInstance      &oti)
    { this->notify(const_cast<Component *>(&c)); }

  bool LegacyComponentObserver::addAttribute(Attribute const &attr) {
    throw ConfigException("Legacy component observers do not support attributes!");
  }

} } // namespace SystemC_VPC::Detail
