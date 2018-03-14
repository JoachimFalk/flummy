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

#ifndef _INCLUDED_SYSTEMCVPC_TASK_HPP
#define _INCLUDED_SYSTEMCVPC_TASK_HPP

#include <sstream>

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TaskTracer.hpp>


#include <systemcvpc/vpc_config.h>

#include <systemcvpc/FastLink.hpp>
#include "ProcessControlBlock.hpp"
#include <systemcvpc/ScheduledTask.hpp>
#include "TaskPool.hpp"

namespace SystemC_VPC {

  namespace Trace {
    class Tracing;
    class VcdTracer;
    class PajeTracer;
  }

  using CoSupport::SystemC::Event;

  class Task{
  public:
    friend class Trace::VcdTracer;
    friend class Trace::PajeTracer;
    friend class NonPreemptiveComponent;
    friend class RoundRobinComponent;

    Task(TaskPool * pool)
      : pid(-1)
      , fid()
      , gid()
      , blockEvent()
      , blockingCompute(NULL)
      , blockAck(false)
      , exec(false)
      , write(false)
      , factorOverhead(0)
      , timing()
      , pcb()
      , pool(pool)
      , name("NN")
      , timingScale(1)
      , taskTracerTicket()
      , scheduledTask(NULL)
    {
          this->instanceId = Task::globalInstanceId++;
    }

    // getter, setter
    std::string getName() const                          {return name;}
    void        setName(std::string name)                {this->name = name;}
    ProcessId   getProcessId()                           {return pid;}
    void        setProcessId(ProcessId pid)              {this->pid = pid;}
    FunctionIds getFunctionIds()                         {return fid;}
    void        setFunctionIds(FunctionIds fid)          {this->fid = fid;}
    FunctionIds getGuardIds()                            {return gid;}
    void        setGuardIds(FunctionIds gid)             {this->gid = gid;}
    EventPair   getBlockEvent()                          {return blockEvent;}
    void        setBlockEvent(EventPair p)               {this->blockEvent = p;}
    void        setPCB(ProcessControlBlock *pcb)         {this->pcb = pcb;}
    void        setTiming(FunctionTimingPtr timing)      {this->timing = timing;}

    void       ackBlockingCompute(){
      blockAck = true;
      blockingCompute->notify();
    }
    void       abortBlockingCompute(){
      blockAck = false;
      blockingCompute->notify();
    }

    void       resetBlockingCompute(){this->setBlockingCompute(NULL);}
    void       setBlockingCompute(Coupling::VPCEvent::Ptr blocker)
      { blockingCompute = blocker; }
    bool       isBlocking()
      { return blockingCompute != NULL; }
    bool       isAckedBlocking()
      { return blockAck; }
    void       setExec( bool exec ) {this->exec=exec;}
    bool       isExec(  ) { return this->exec;}
    void       setWrite( bool write ) {this->write=write;}
    bool       isWrite(  ) { return this->write;}

    //////////////////////////////
    // Begin from Simone Mueller //
    //////////////////////////////

    //getter/setter of states that have to executed before the actual task can be.
    void setPreState(std::string state)        {this->preState = state;}
    std::string getPreState()                  {return this->preState;}

    //not used right now
    void setRuntime(const sc_core::sc_time& runtime)     {this->runtime = runtime;}
    sc_core::sc_time getRuntime()const                   {return this->runtime;}


    void setFactorOverhead(int complexity)      {this->factorOverhead = complexity;}
    int getFactorOverhead()                     {return this->factorOverhead;}
    void setOverhead(const sc_core::sc_time& overhead)   {this->overhead = overhead;}
    sc_core::sc_time getOverhead()const                  {return this->overhead;}
    //////////////////////////////
    // End from Simone Mueller //
    //////////////////////////////

    void setDelay(const sc_core::sc_time& delay)         {this->delay = delay;}
    sc_core::sc_time getDelay() const                    {return this->delay;}
    void setLatency(const sc_core::sc_time& latency)     {this->latency = latency;}
    sc_core::sc_time getLatency() const                  {return this->latency;}
    void setRemainingDelay(const sc_core::sc_time& delay){this->remainingDelay = delay;}
    sc_core::sc_time getRemainingDelay() const           {return this->remainingDelay;}
    int getInstanceId() const                   {return this->instanceId;}
    void setTimingScale( double scale )         {this->timingScale = scale;}
    double getTimingScale()                     {return this->timingScale;}

    void setScheduledTask(TaskInterface * st)
      {this->scheduledTask = st;}
    TaskInterface * getScheduledTask()
      {return this->scheduledTask;}
    bool hasScheduledTask() const
      {return this->scheduledTask != NULL;}

    /**
     * 
     */
    void initDelays();

    // Adaptor setter / getter for ProcessControlBlock
    int getPriority()
      {assert(pcb != NULL); return pcb->getPriority();}
    sc_core::sc_time getPeriod()
      {assert(pcb != NULL); return pcb->getPeriod();}

    void release() {
      //this->setBlockEvent(EventPair(NULL, NULL));
      this->getBlockEvent().dii     = NULL;
      this->getBlockEvent().latency = NULL;
      pool->free(this->getProcessId(), this);
    }

    bool isPSM()
    {
      return pcb->isPSM();
    }

  private:
    void traceReleaseTask(){
      taskTracerTicket = pcb->taskTracer->releaseTask();
    }

    void traceFinishTaskLatency(){
      pcb->taskTracer->finishTaskLatency(taskTracerTicket);
    }

    Trace::Tracing* getTraceSignal() const
      {assert(pcb != NULL); return pcb->getTraceSignal();}

    friend class AssociativePrototypedPool<ProcessId, Task>;
    friend class PrototypedPool<Task>;

    Task(const Task &task) :
      pid(task.pid),
      fid(task.fid),
      blockEvent(task.blockEvent),
      blockingCompute(task.blockingCompute),
      blockAck(task.blockAck),
      exec(task.exec),
      write(task.write),
      delay(task.delay),
      latency(task.latency),
      remainingDelay(task.remainingDelay),
      overhead(task.overhead),
      runtime(task.runtime),
      factorOverhead(task.factorOverhead),
      timing(task.timing),
      pcb(task.pcb),
      pool(task.pool),
      name(task.name),
      timingScale(task.timingScale),
      taskTracerTicket(task.taskTracerTicket),
      scheduledTask(task.scheduledTask)
    {
          this->instanceId = Task::globalInstanceId++;
    }
        

    ProcessId        pid;
    FunctionIds      fid;
    FunctionIds      gid;
    EventPair        blockEvent;

    Coupling::VPCEvent::Ptr blockingCompute;
    bool       blockAck;
    bool       exec;
    bool       write;

    sc_core::sc_time startTime;
    sc_core::sc_time endTime;
    sc_core::sc_time blockingTime;
    sc_core::sc_time delay;
    sc_core::sc_time latency;
    sc_core::sc_time remainingDelay;
    sc_core::sc_time overhead;
    sc_core::sc_time runtime;
    
    int factorOverhead;

    FunctionTimingPtr    timing;
    ProcessControlBlock *pcb;
    TaskPool            *pool;

    static int globalInstanceId;
    int instanceId;
    std::string name;
    double timingScale;
    CoSupport::Tracing::TaskTracer::Ticket taskTracerTicket;
    TaskInterface * scheduledTask;
    std::string preState;
    std::string destState;
  };

  static inline
  Task *getTaskOfTaskInterface(TaskInterface const *task)
    { return reinterpret_cast<Task *>(task->getSchedulerInfo()); }

  typedef std::map<int, Task*>  TaskMap;

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
    Task *task;
    p_queue_entry(int fifo_order, Task *task) :
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
    Task *task;

    bool operator<(const timePcbPair& right) const
    {
      if (time > right.time)
        return true;
      else
        return false;
    }

  };

}
#endif /* _INCLUDED_SYSTEMCVPC_TASK_HPP */
