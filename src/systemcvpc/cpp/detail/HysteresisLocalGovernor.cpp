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

#include "HysteresisLocalGovernor.hpp"

namespace SystemC_VPC { namespace Detail {

  InternalLoadHysteresisGovernor::InternalLoadHysteresisGovernor(const sc_core::sc_time& windowTime,
                                                 const sc_core::sc_time& fastTime,
                                                 const sc_core::sc_time& slowTime) :
    sc_core::sc_module(sc_core::sc_module_name("InternalLoadHysteresisGovernor")),
    m_windowTime(windowTime),
    m_fastTime(fastTime),
    m_slowTime(slowTime),
    m_mode(PowerMode::DEFAULT),
    m_ci(NULL),
    m_lastState(ComponentState::IDLE)
  {
    //std::cout << "InternalLoadHysteresisGovernor" << std::endl;
    if(m_windowTime > sc_core::SC_ZERO_TIME){
      SC_METHOD(process);
      sensitive << m_wakeup_ev;
      dont_initialize();
    }
  }


  InternalLoadHysteresisGovernor::~InternalLoadHysteresisGovernor()
  {}

  void InternalLoadHysteresisGovernor::notify(ComponentInfo *ci)
  {
    //    std::cerr << "InternalLoadHysteresisGovernor::notify() @ " << sc_core::sc_time_stamp() << std::endl;

    const ComponentState newState = ci->getComponentState();

    if(m_ci == NULL) {
      m_ci = ci;
      m_mode.powerMode = m_ci->getPowerMode();
      m_tpg->notify_top(m_ci, &m_mode);
      m_wakeup_ev.notify(sc_core::SC_ZERO_TIME);
    }

    assert(m_ci == ci);

    if(newState != m_lastState) {
      m_stateHistory.push_back(std::pair<ComponentState, sc_core::sc_time>(m_lastState, sc_core::sc_time_stamp()));
      m_lastState = newState;
      m_wakeup_ev.notify(sc_core::SC_ZERO_TIME);
    }
  }

  void InternalLoadHysteresisGovernor::process()
  {
    //    std::cerr << "InternalLoadHysteresisGovernor::process() @ " << sc_core::sc_time_stamp() << std::endl;

    const PowerMode SLOW("SLOW");
    const PowerMode FAST("FAST");

    sc_core::sc_time execTime(sc_core::SC_ZERO_TIME);
    sc_core::sc_time startTime(sc_core::SC_ZERO_TIME);

    // calculate start time for sliding window
    if(sc_core::sc_time_stamp() > m_windowTime)
      startTime = sc_core::sc_time_stamp() - m_windowTime;

    // drop old state history
    while(!m_stateHistory.empty() &&
          (m_stateHistory.front().second) < startTime)
      {
        m_stateHistory.pop_front();
      }

    // sum up execution times
    for(std::deque<std::pair<ComponentState, sc_core::sc_time> >::const_iterator
          iter  = m_stateHistory.begin();
        iter != m_stateHistory.end();
        iter++)
      {
        assert(iter->second >= startTime);
        if(iter->first != ComponentState::IDLE)
          execTime += iter->second - startTime;
        startTime = iter->second;
      }
    if(m_lastState != ComponentState::IDLE)
      execTime += sc_core::sc_time_stamp() - startTime;

    //    std::cerr << "execTime = " << execTime << std::endl;

    // notify power mode suggestions
    if((m_mode.powerMode == SLOW) && (execTime >= m_fastTime)) {
      m_mode.powerMode = FAST;
      m_tpg->notify_top(m_ci, &m_mode);
    } else if((m_mode.powerMode == FAST) && (execTime <= m_slowTime)) {
      m_mode.powerMode = SLOW;
      m_tpg->notify_top(m_ci, &m_mode);
    }

    // wake up process when reaching load boundary
    if((m_mode.powerMode == SLOW) && (m_lastState != ComponentState::IDLE)) {
      m_wakeup_ev.notify(m_fastTime - execTime);
    } else if((m_mode.powerMode == FAST) && (m_lastState == ComponentState::IDLE)) {
      m_wakeup_ev.notify(execTime - m_slowTime);
    }
  }

} } // namespace SystemC_VPC::Detail
