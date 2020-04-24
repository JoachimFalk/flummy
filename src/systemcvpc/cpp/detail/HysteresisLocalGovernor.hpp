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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_HYSTERESISLOCALGOVERNOR_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_HYSTERESISLOCALGOVERNOR_HPP

#include <systemcvpc/PowerMode.hpp>

#include "PluggablePowerGovernor.hpp"

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

    virtual void processAttributes(Attribute::Ptr powerAtt){
      //std::cerr << "InternalLoadHysteresisGovernorFactory::processAttributes" << std::endl;
      if(powerAtt->isType("governor")){
        if(powerAtt->hasAttribute("sliding_window")){
          std::string v = powerAtt->getAttribute("sliding_window")->getValue();
          windowTime = CoSupport::SystemC::createSCTime(v.c_str());
        }
        if(powerAtt->hasAttribute("upper_threshold")){
          std::string v = powerAtt->getAttribute("upper_threshold")->getValue();
          fastTime = windowTime * atof(v.c_str());
        }
        if(powerAtt->hasAttribute("lower_threshold")){
          std::string v = powerAtt->getAttribute("lower_threshold")->getValue();
          slowTime = windowTime * atof(v.c_str());
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
