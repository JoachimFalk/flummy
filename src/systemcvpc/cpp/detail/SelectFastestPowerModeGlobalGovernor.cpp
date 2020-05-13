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

#include "SelectFastestPowerModeGlobalGovernor.hpp"

namespace SystemC_VPC { namespace Detail {

  InternalSelectFastestPowerModeGovernor::InternalSelectFastestPowerModeGovernor()
    : m_lastMode(PowerMode::DEFAULT)
  {
    //std::cout << "InternalSelectFastestPowerModeGovernor" << std::endl; 
  }

  void InternalSelectFastestPowerModeGovernor::notify_top(Component *ci,
                                                  GenericParameter *param)
  {
    /*
    // extract PowerMode from container "GenericParameter"
    PowerModeParameter * p = dynamic_cast<PowerModeParameter*>(param);
    PowerMode newMode = p->powerMode;

    if(m_components.find(ci) == m_components.end()) {
      //std::cerr << "@" << sc_core::sc_time_stamp() << ": setPowerMode(" << newMode->getName() << ");" << std::endl;
      m_lastMode = newMode;
      ci->getComponentInterface()->changePowerMode(m_lastMode);
    }

    m_components[ci] = newMode;

    // FIXME: This is alphabetical comparison and not speed. Fix this!!!
    if(newMode < m_lastMode) {
      for(std::map<Component *, PowerMode>::iterator
            iter  = m_components.begin();
          iter != m_components.end();
          iter++)
        {
          if(iter->second > newMode)
            newMode = iter->second;
        }
    }

    if(newMode != m_lastMode) {
      std::cerr << "@" << sc_core::sc_time_stamp() << ": for all components setPowerMode(" << newMode << ");" << std::endl;

      for(std::map<Component *, PowerMode>::iterator
            iter  = m_components.begin();
          iter != m_components.end();
          iter++)
        {
          iter->first->getComponentInterface()->changePowerMode(newMode);
        }
      m_lastMode = newMode;
    }
    */
  }

} } // namespace SystemC_VPC::Detail
