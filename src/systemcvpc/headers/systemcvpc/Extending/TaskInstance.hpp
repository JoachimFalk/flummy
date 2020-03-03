// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_SYSTEMCVPC_EXTENDING_TASKINSTANCE_HPP
#define _INCLUDED_SYSTEMCVPC_EXTENDING_TASKINSTANCE_HPP

#include "../Power.hpp"

#include <systemc>

namespace SystemC_VPC { namespace Extending {


  /**
   * This class represents the publicly visible information of a
   * task instance, i.e., one execution of a task on a component.
   */
  class TaskInstance {
  public:

    sc_core::sc_time getDelay() const
      { return this->delay; }
    sc_core::sc_time getLatency() const
      { return this->latency; }
    sc_core::sc_time getRemainingDelay() const
      { return this->remainingDelay; }

    Power            getPower() const
      { return pwr; }

  protected:
    void setDelay(sc_core::sc_time delay)
      { this->delay = delay; }
    void setLatency(sc_core::sc_time latency)
      { this->latency = latency; }
    void setRemainingDelay(sc_core::sc_time delay)
      { this->remainingDelay = delay; }

    void setPower(Power pwr)
      { this->pwr = pwr; }
  private:

    // Timings
    sc_core::sc_time delay;
    sc_core::sc_time latency;
    sc_core::sc_time remainingDelay;

    Power pwr; ///< Power consumption when this task instance is running on a component.
  };

} } // namespace SystemC_VPC::Extending

#endif /* _INCLUDED_SYSTEMCVPC_EXTENDING_TASKINSTANCE_HPP */
