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

#ifndef SCHEDULEDTASK_HPP_
#define SCHEDULEDTASK_HPP_

#include <cstddef>
#include <systemc.h>

namespace SystemC_VPC
{

typedef size_t ProcessId;

class Delayer;


class SchedulingInterface {
  template<class TASKTRACER> friend class ComponentImpl;
  template<class TASKTRACER> friend class RoundRobinComponent;
  template<class TASKTRACER> friend class FcfsComponent;
  template<class TASKTRACER> friend class PriorityComponent;
  template<class TASKTRACER> friend class DynamicPriorityComponent;
  template<class DEBUG_OUT, class TASKTRACER> friend class DynamicPriorityComponentImpl;
  template<class TASKTRACER> friend class NonPreemptiveComponent;

protected:
  // This will execute the actor. The actor must be fireable if this method is called.
  // This will be implemented by the SysteMoC actor and called by the scheduler.
  virtual void schedule() = 0;
  // This will test if the actor is fireable.
  // This will be implemented by the SysteMoC actor and called by the scheduler.
  virtual bool canFire()  = 0;

  // This will add the Number of Guards to the ScheduledTask.
  // This will be implemented by the SysteMoc actor and called by the scheduler.
  //virtual void addGuardsNr() = 0;

  // If this method returns true, then SysteMoC will call
  // setActivation to notify the SchedulingInterface
  // that an actor is fireable. Otherwise, this has
  // to be check via calls to canFire.
  virtual bool useActivationCallback() const = 0;

  // This will be called by SysteMoC if useActivationCallback()
  // return true.
  virtual void setActivation(bool activation) = 0;
  // This must return a time until the actor is suspended
  // due to some timing behavior. If no timing behavior is
  // given, then this should return sc_core::sc_time_stamp().
  // This will be implemented by the SysteMoC actor and called by the scheduler.
  virtual sc_core::sc_time const &getNextReleaseTime() const = 0;

  // setActive(false) will disable the scheduling of this actor
  // setActive(true) will enable the scheduling of this actor
  virtual void setActive(bool) = 0;
  virtual bool getActive() const = 0;

  virtual ~SchedulingInterface() {}
};

class ScheduledTask
  : public sc_core::sc_module
  , public SchedulingInterface
{
  friend                     class Component;
  template<class TASKTRACER> class TtPriorityComponent;
  template<class TASKTRACER> class TtFcfsComponent;
  template<class DEBUG_OUT,class TASKTRACER> class DynamicPriorityComponentImpl;
  friend                     class smoc_root_node;

  SC_HAS_PROCESS(ScheduledTask);
public:
  ScheduledTask(sc_core::sc_module_name name);
  virtual ~ScheduledTask();
  void setDelayer(Delayer *component);
  Delayer* getDelayer();
  void setPid(ProcessId pid);
  ProcessId getPid() const;

  virtual sc_core::sc_time const &getNextReleaseTime() const = 0;

  bool useActivationCallback() const
    { return true; }

  void setActivation(bool active);

  void setActive(bool a);

  bool getActive() const {
    return active;
  }

private:
  Delayer *component;
  ProcessId pid;
  bool active;
//  const smoc::Expr::Ex<bool>::type NrGuards;

  /// The following member variable are for the fallback
  /// case if VPC scheduling is not enabled due to missing
  /// configuration.
  sc_core::sc_event scheduleRequest;
  void scheduleRequestMethod();
};

}

#endif /* SCHEDULEDTASK_HPP_ */
