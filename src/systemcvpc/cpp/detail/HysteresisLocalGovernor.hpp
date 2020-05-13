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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_HYSTERESISLOCALGOVERNOR_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_HYSTERESISLOCALGOVERNOR_HPP

#include <systemcvpc/PowerMode.hpp>

#include "PluggablePowerGovernor.hpp"
#include "common.hpp"

#include <CoSupport/SystemC/systemc_time.hpp>

#include <map>
#include <deque>
#include <systemc>

namespace SystemC_VPC { namespace Detail {

  class InternalLoadHysteresisGovernor : public PluggableLocalPowerGovernor,
                                 public sc_core::sc_module {
  public:
    InternalLoadHysteresisGovernor(const sc_core::sc_time& windowTime,
                           const sc_core::sc_time& fastTime,
                           const sc_core::sc_time& slowTime);

    ~InternalLoadHysteresisGovernor();

    SC_HAS_PROCESS(InternalLoadHysteresisGovernor);

    virtual void notify(Component *ci);

  private:
    sc_core::sc_time                                         m_windowTime;
    sc_core::sc_time                                         m_fastTime;
    sc_core::sc_time                                         m_slowTime;
    PowerModeParameter                              m_mode;
    Component                                      *m_ci;
    ComponentState                                  m_lastState;
    std::deque<std::pair<ComponentState, sc_core::sc_time> > m_stateHistory;
    sc_core::sc_event                                        m_wakeup_ev;

    void process();
  };

  /**
   * creates an InternalLoadHysteresisGovernor
   */
  class InternalLoadHysteresisGovernorFactory
    : public PlugInFactory<PluggableLocalPowerGovernor>{

  public:
    InternalLoadHysteresisGovernorFactory()
      : windowTime( sc_core::SC_ZERO_TIME ),
        fastTime(   sc_core::sc_time(12.1, sc_core::SC_MS) ),
        slowTime(   sc_core::sc_time( 4.0, sc_core::SC_MS) )
    {
    }

    PluggableLocalPowerGovernor * createPlugIn(){
      return new InternalLoadHysteresisGovernor(windowTime, fastTime, slowTime);
    }

    virtual void processAttributes(Attribute const &powerAtt){
      //std::cerr << "InternalLoadHysteresisGovernorFactory::processAttributes" << std::endl;
      if(powerAtt.isType("governor")){
        Attributes const           &attrs = powerAtt.getAttributes();
        Attributes::const_iterator  iter;
        if ((iter = attrs.find("sliding_window")) != attrs.end()) {
          windowTime = createSC_Time(iter->getValue().c_str());
        }
        if ((iter = attrs.find("upper_threshold")) != attrs.end()) {
          fastTime = windowTime * atof(iter->getValue().c_str());
        }
        if ((iter = attrs.find("lower_threshold")) != attrs.end()) {
          slowTime = windowTime * atof(iter->getValue().c_str());
        }
      }
    }
  private:
    sc_core::sc_time windowTime;
    sc_core::sc_time fastTime;
    sc_core::sc_time slowTime;
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_HYSTERESISLOCALGOVERNOR_HPP */
