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

#ifndef _INCLUDED_SYSTEMCVPC_NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP

#include "memory.h"

#include <vector>
#include <deque>

#include "../AbstractComponent.hpp"

namespace SystemC_VPC{

  class RoundRobinComponent : public AbstractComponent {
    SC_HAS_PROCESS(RoundRobinComponent);
  public:
    RoundRobinComponent(Config::Component::Ptr component,
        Director *director = &Director::getInstance())
      : AbstractComponent(component)
      , useActivationCallback(false)
      , actualTask(NULL)
    {
      /// FIXME: WTF?! SLOW hardcoded?
      this->setPowerMode(this->translatePowerMode("SLOW"));

      SC_THREAD(scheduleThread);
    }

  protected:
    bool useActivationCallback;

    void setActivationCallback(bool flag) {
      if (useActivationCallback == flag)
        return;
      useActivationCallback = flag;
      if (!flag) {
        for (ScheduledTask *scheduledTask: taskList)
          scheduledTask->setUseActivationCallback(flag);
      } else {
        for (std::vector<ScheduledTask *>::iterator iter = taskList.begin();
             iter != taskList.end();
             ++iter) {
          (*iter)->setUseActivationCallback(true);
          if ((*iter)->canFire()) {
            /// Oops, undo it
            useActivationCallback = false;
            (*iter)->setUseActivationCallback(false);
            while (iter != taskList.begin()) {
              --iter;
              (*iter)->setUseActivationCallback(false);
            }
            return;
          }
        }
      }
    }

    void end_of_elaboration() {
      PCBPool const &pcbPool = getPCBPool();
      for (PCBPool::const_iterator it=pcbPool.begin(); it!=pcbPool.end(); ++it) {
        std::cout << "\t " << it->second->getName() << std::endl;
        Task &task = Director::getInstance().taskPool->getPrototype(it->first);
        task.setPCB(it->second);
        if (task.hasScheduledTask()) {
          ScheduledTask *scheduledTask = task.getScheduledTask();
          scheduledTask->setUseActivationCallback(false);
          taskList.push_back(scheduledTask);
        }
      }
    }

    void notifyActivation(ScheduledTask * scheduledTask, bool active) {
      if (active) {
        setActivationCallback(false);
        readyEvent.notify();
      }
    }

    void compute(Task *actualTask) {
      std::cout << "\t " << sc_core::sc_time_stamp() << " : task PID " << actualTask->getProcessId() << std::endl;
      ProcessId pid = actualTask->getProcessId();
      /// Note that actualTask is not the task prototype, i.e.,
      /// Director::getInstance().taskPool.getPrototype(pid),
      /// but an instance allocated with PrototypedPool<Task>::allocate().
      actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));
      actualTask->initDelays();
      if (actualTask->hasScheduledTask()) {
        assert(!useActivationCallback);
        assert(!this->actualTask);
        this->actualTask = actualTask;
      } else
        readyMsgTasks.push_back(actualTask);
      readyEvent.notify();
    }

    void check(Task *actualTask) {
      if (!useActivationCallback) {
        ProcessId pid = actualTask->getProcessId();
        actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));
        actualTask->initDelays();
        this->taskTracer_->release(actualTask);
        this->taskTracer_->assign(actualTask);
        wait(actualTask->getOverhead());//Director::getInstance().getOverhead() +
        this->taskTracer_->finishDii(actualTask);
        this->taskTracer_->finishLatency(actualTask);

        std::cout << "check: " <<  actualTask->getName() << std::endl;
      }
    }

    /**
     *
     */
    virtual void requestBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker) {


    }

    /**
     *
     */
    virtual void execBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker) {


    }

    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, Coupling::VPCEvent::Ptr blocker) {


    }

    /**
     *
     */
    virtual void updatePowerConsumption() {
    }

    /*
     * from ComponentInterface
     */
    bool hasWaitingOrRunningTasks()
    {
      /// FIXME: Implement this;
      assert(false);
    }

    virtual Trace::Tracing * getOrCreateTraceSignal(std::string name) {
      return taskTracer_->getOrCreateTraceSignal(name);
    }

    bool scheduleMessageTasks() {
      bool progress = !readyMsgTasks.empty();
      while (!readyMsgTasks.empty()) {
        Task *messageTask = readyMsgTasks.front();
        readyMsgTasks.pop_front();
        assert(!messageTask->hasScheduledTask());
        this->taskTracer_->release(messageTask);
        this->taskTracer_->assign(messageTask);
        /// This will setup the trigger for schedule_method to be called
        /// again when the task execution time is over.
        wait(messageTask->getDelay());
//        wait(0.9 * messageTask->getDelay());
        Director::getInstance().signalLatencyEvent(messageTask);
        this->taskTracer_->finishDii(messageTask);
        this->taskTracer_->finishLatency(messageTask);
        /// The scheduledTask, i.e., the SysteMoC actor, should now be in the comm state.
        /// Enable transition out of comm state by notifying the dii event.
        messageTask->getBlockEvent().dii->notify();
        /// FIXME: What about dii != latency
      }
      return progress;
    }

    void scheduleThread() {
      while (true) {
        bool progress = scheduleMessageTasks();
        for (ScheduledTask *scheduledTask: taskList) {
          std::cout << "Checking " << scheduledTask->name() << "@" << sc_core::sc_time_stamp() << std::endl;
          while (scheduledTask->canFire()) {
            progress = true;
            // This will invoke our compute callback and setup actualTask.
            assert(readyMsgTasks.empty());
            std::cout << "Scheduling " << scheduledTask->name() << "@" << sc_core::sc_time_stamp() << std::endl;
            assert(!this->actualTask);
            scheduledTask->schedule();
            while (!actualTask) {
              scheduleMessageTasks();
              if (!actualTask)
                wait(readyEvent);
            }
            assert(actualTask);
            assert(actualTask->hasScheduledTask());
            assert(actualTask->getScheduledTask() == scheduledTask);
            this->taskTracer_->release(actualTask);
            this->taskTracer_->assign(actualTask);
            wait(actualTask->getOverhead());
            this->taskTracer_->finishDii(actualTask);
            this->taskTracer_->finishLatency(actualTask);

            /// FIXME: What about DII != latency
            Director::getInstance().signalLatencyEvent(actualTask);
            actualTask = nullptr;
            scheduleMessageTasks();
          }
        }
        if (!progress) {
          setActivationCallback(true);
          while (useActivationCallback) {
            wait(readyEvent);
            scheduleMessageTasks();
          }
        }
      }
    }

    virtual ~RoundRobinComponent() {}

    /// This list contains the message tasks that will appear
    /// via compute calls.
    std::deque<Task *>            readyMsgTasks;
    /// This list represent all the SysteMoC actors that
    /// are mapped to this component. The list will be
    /// filled by the the end_of_elaboration method.
    std::vector<ScheduledTask *>  taskList;
    /// This is the actual running task that will
    /// be assigned by the compute method if
    /// on of the SysteMoC actors of the component
    /// is currently running.
    Task                         *actualTask;
    sc_core::sc_event             readyEvent;
  };

} // namespace SystemC_VPC


#endif /* _INCLUDED_SYSTEMCVPC_NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP */
