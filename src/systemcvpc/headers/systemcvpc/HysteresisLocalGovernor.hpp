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

#ifndef __INCLUDED__HYSTERESISLOCALGOVERNOR_IMPL_H_
#define __INCLUDED__HYSTERESISLOCALGOVERNOR_IMPL_H_

#include <map>
#include <deque>
#include <systemc.h>

#include <CoSupport/SystemC/systemc_time.hpp>

#include <systemcvpc/PowerMode.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/PluggablePowerGovernor.hpp>

namespace SystemC_VPC{

  class InternalLoadHysteresisGovernor : public PluggableLocalPowerGovernor,
                                 public sc_module {
  public:
    InternalLoadHysteresisGovernor(const sc_time& windowTime,
                           const sc_time& fastTime,
                           const sc_time& slowTime);

    ~InternalLoadHysteresisGovernor();

    SC_HAS_PROCESS(InternalLoadHysteresisGovernor);

    virtual void notify(ComponentInfo *ci);

  private:
    sc_time                                         m_windowTime;
    sc_time                                         m_fastTime;
    sc_time                                         m_slowTime;
    PowerModeParameter                              m_mode;
    ComponentInfo                                  *m_ci;
    ComponentState                                  m_lastState;
    std::deque<std::pair<ComponentState, sc_time> > m_stateHistory;
    sc_event                                        m_wakeup_ev;

    void process();
  };

  /**
   * creates an InternalLoadHysteresisGovernor
   */
  class InternalLoadHysteresisGovernorFactory
    : public PlugInFactory<PluggableLocalPowerGovernor>{

  public:
    InternalLoadHysteresisGovernorFactory()
      : windowTime( SC_ZERO_TIME ),
        fastTime(   sc_time(12.1, SC_MS) ),
        slowTime(   sc_time( 4.0, SC_MS) )
    {
    }

    PluggableLocalPowerGovernor * createPlugIn(){
      return new InternalLoadHysteresisGovernor(windowTime, fastTime, slowTime);
    }

    virtual void processAttributes(AttributePtr powerAtt){
      //std::cerr << "InternalLoadHysteresisGovernorFactory::processAttributes" << std::endl;
      if(powerAtt->isType("governor")){
        if(powerAtt->hasParameter("sliding_window")){
          std::string v = powerAtt->getParameter("sliding_window");
          windowTime = CoSupport::SystemC::createSCTime(v.c_str());
        }
        if(powerAtt->hasParameter("upper_threshold")){
          std::string v = powerAtt->getParameter("upper_threshold");
          fastTime = windowTime * atof(v.c_str());
        }
        if(powerAtt->hasParameter("lower_threshold")){
          std::string v = powerAtt->getParameter("lower_threshold");
          slowTime = windowTime * atof(v.c_str());
        }
      }
    }
  private:
    sc_time windowTime;
    sc_time fastTime;
    sc_time slowTime;
  };
}

#endif // __INCLUDED__HYSTERESISLOCALGOVERNOR_IMPL_H_