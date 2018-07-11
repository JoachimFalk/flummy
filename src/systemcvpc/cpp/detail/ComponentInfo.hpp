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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_COMPONENTINFO_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_COMPONENTINFO_HPP

#include "PowerMode.hpp"

#include <cstddef>
#include <string>
#include <iostream>
#include <map>
#include <string>

namespace SystemC_VPC { namespace Detail {

  class ComponentModel;

  typedef std::map<std::string, PowerMode> PowerModes;

  class ComponentState
  {
    public:
      ComponentState()
      {
        *this = IDLE;
      }

      ComponentState(const size_t &_state) : state(_state) {}

      bool operator==(const ComponentState &rhs) const
      {
        return state == rhs.state;
      }

      bool operator!=(const ComponentState &rhs) const
      {
        return state != rhs.state;
      }

      bool operator<(const ComponentState &rhs) const
      {
        return state < rhs.state;
      }

      static const ComponentState IDLE;
      static const ComponentState RUNNING;
      static const ComponentState STALLED;
      //Execution state on which component is not ready to perform any task
      static const ComponentState SLEEPING;

    private:
      size_t state;
  };

  class ComponentInfo
  {
    public:
      ComponentInfo(ComponentModel *model)
        : powerConsumption(0.0), model(model) {}

      virtual ~ComponentInfo(){};

      ComponentState getComponentState() const
      {
        return componentState;
      }

      double getPowerConsumption() const
      {
        return powerConsumption;
      }

      virtual const PowerMode* getPowerMode() const = 0;

      const PowerMode* translatePowerMode(std::string mode)
      {
        PowerModes::const_iterator i = powerModes.find(mode);
        if(i == powerModes.end()) {
          size_t id = powerModes.size();
          powerModes[mode] = PowerMode(id, mode);
        }
        return &powerModes[mode];
      }


      ComponentModel * getModel(){
        return model;
      }
    protected:
      ComponentState componentState;
      ComponentState previousComponentState;
      double         powerConsumption;
      PowerModes     powerModes;
      ComponentModel *model;
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_COMPONENTINFO_HPP */
