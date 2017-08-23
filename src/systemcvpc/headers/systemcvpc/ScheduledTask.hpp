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

#ifndef _INCLUDED_SYSTEMCVPC_SCHEDULEDTASK_HPP
#define _INCLUDED_SYSTEMCVPC_SCHEDULEDTASK_HPP

#include <list>
#include <cstddef>
#include <systemc>

#include "../smoc/EvalAPI/SchedulingInterface.hpp"

namespace SystemC_VPC {

typedef size_t ProcessId;

class Delayer;

class ScheduledTask
  : public sc_core::sc_module
  , protected smoc::EvalAPI::SchedulingInterface
{
  friend class PajeTracer;
  friend class PreemptiveComponent;
  friend class ComponentImpl;
  friend class RoundRobinComponent;
  friend class FcfsComponent;
  friend class PriorityComponent;
  friend class DynamicPriorityComponent;
  template<class DEBUG_OUT>
  friend class DynamicPriorityComponentImpl;
  friend class NonPreemptiveComponent;

  SC_HAS_PROCESS(ScheduledTask);
public:
  ScheduledTask(sc_core::sc_module_name name);

  void      setDelayer(Delayer *component);
  Delayer  *getDelayer();

  void      setPid(ProcessId pid);
  ProcessId getPid() const;

  // This is here to give access to the friends of this class.
  virtual sc_core::sc_time const &getNextReleaseTime() const = 0;

  void setActivation(bool active);

  virtual ~ScheduledTask();
private:
  Delayer *component;
  ProcessId pid;

  /// The following member variable are for the fallback
  /// case if VPC scheduling is not enabled due to missing
  /// configuration.
  sc_core::sc_event scheduleRequest;
  void scheduleRequestMethod();
};

}

#endif /* _INCLUDED_SYSTEMCVPC_SCHEDULEDTASK_HPP */
