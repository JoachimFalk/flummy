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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_TASKINSTANCE_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_TASKINSTANCE_HPP

#include <systemcvpc/vpc_config.h>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/PossibleAction.hpp>
#include <systemcvpc/EventPair.hpp>

#include "tracing/ComponentTracerIf.hpp"

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TaskTracer.hpp>

#include <functional>

namespace SystemC_VPC { namespace Detail {

  class ProcessControlBlock;

  using CoSupport::SystemC::Event;

  // Class representing a task instance, i.e., one execution of the task represented by the PCB.
  class TaskInstance
    : public Tracing::TTaskInstanceHolder {
  public:
    TaskInstance(
        std::function<void (TaskInstance *)> const &diiCallback,
        std::function<void (TaskInstance *)> const &latCallback);

    // getter, setter
    std::string getName() const                          {return name;}
    void        setName(std::string name)                {this->name = name;}
    FunctionIds getFunctionIds()                         {return fid;}
    void        setFunctionIds(FunctionIds fid)          {this->fid = fid;}
    ProcessControlBlock *getPCB()                        {return this->pcb;}
    void        setPCB(ProcessControlBlock *pcb)         {this->pcb = pcb;}

    void       ackBlockingCompute(){
      blockAck = true;
      blockingCompute->notify();
    }
    void       abortBlockingCompute(){
      blockAck = false;
      blockingCompute->notify();
    }

    void       resetBlockingCompute(){this->setBlockingCompute(NULL);}
    void       setBlockingCompute(VPCEvent::Ptr blocker)
      { blockingCompute = blocker; }
    bool       isBlocking()
      { return blockingCompute != NULL; }
    bool       isAckedBlocking()
      { return blockAck; }
    void       setExec( bool exec ) {this->exec=exec;}
    bool       isExec(  ) { return this->exec;}
    void       setWrite( bool write ) {this->write=write;}
    bool       isWrite(  ) { return this->write;}

    void setDelay(const sc_core::sc_time& delay)         {this->delay = delay;}
    sc_core::sc_time getDelay() const                    {return this->delay;}
    void setLatency(const sc_core::sc_time& latency)     {this->latency = latency;}
    sc_core::sc_time getLatency() const                  {return this->latency;}
    void setRemainingDelay(const sc_core::sc_time& delay){this->remainingDelay = delay;}
    sc_core::sc_time getRemainingDelay() const           {return this->remainingDelay;}
    void setTimingScale( double scale )                  {this->timingScale = scale;}
    double getTimingScale()                              {return this->timingScale;}

    int getInstanceId() const                            {return this->instanceId;}

    void setFiringRule(PossibleAction *fr)
      { this->firingRuleInterface = fr; }
    PossibleAction *getFiringRule()
      { return this->firingRuleInterface; }

    // Adaptor getter for ProcessControlBlock
    int              getPriority() const;
    sc_core::sc_time getPeriod() const;
    ProcessId        getProcessId() const;
    bool             isPSM() const;

    void diiExpired() { diiCallback(this); }
    void latExpired() { latCallback(this); }

    ~TaskInstance();
  private:
    static int globalInstanceId;

    int instanceId;

    std::function<void (TaskInstance *)> const diiCallback;
    std::function<void (TaskInstance *)> const latCallback;

    FunctionIds      fid;

    VPCEvent::Ptr blockingCompute;
    bool       blockAck;
    bool       exec;
    bool       write;

    sc_core::sc_time delay;
    sc_core::sc_time latency;
    sc_core::sc_time remainingDelay;
    
    ProcessControlBlock *pcb;

    std::string name;
    double timingScale;

    PossibleAction *firingRuleInterface;
  };

  typedef std::map<int, TaskInstance*>  TaskMap;

  template<typename PAYLOAD>
  struct PriorityFcfsElement {
    int     priority;
    size_t  fcfsOrder;
    PAYLOAD payload;

    PriorityFcfsElement(int priority, size_t fcfsOrder, PAYLOAD payload) :
      priority(priority), fcfsOrder(fcfsOrder), payload(payload)
    {
    }

    bool operator<(const PriorityFcfsElement<PAYLOAD>& other) const
    {
      int p1=priority;
      int p2=other.priority;
      // lesser value means higher priority
      if (p1 > p2)
        return true;
      else if(p1 == p2)
        return (fcfsOrder>other.fcfsOrder);
      else
        return false;
    }
  };

  struct p_queue_entry{
    int fifo_order;  // secondary scheduling policy
    TaskInstance *task;
    p_queue_entry(int fifo_order, TaskInstance *task) :
      fifo_order(fifo_order), task(task)
    {
    }

    //Threads with lesser priority value have higher priority
    bool operator<(const p_queue_entry& other) const
    {
      int p1=task->getPriority();
      int p2=other.task->getPriority();
      if (p1 > p2)
        return true;
      else if(p1 == p2)
        return (fifo_order>other.fifo_order);
      else
        return false;
    }

  };

  struct timePcbPair{
    sc_core::sc_time time;
    TaskInstance *task;

    bool operator<(const timePcbPair& right) const
    {
      if (time > right.time)
        return true;
      else
        return false;
    }

  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_TASKINSTANCE_HPP */
