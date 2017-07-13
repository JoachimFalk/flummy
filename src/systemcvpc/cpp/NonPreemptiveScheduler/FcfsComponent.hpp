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

#ifndef NONPREEMPTIVESCHEUDLER_FCFSCOMPONENT_HPP_
#define NONPREEMPTIVESCHEUDLER_FCFSCOMPONENT_HPP_

#include <NonPreemptiveScheduler/NonPreemptiveComponent.hpp>

namespace SystemC_VPC{

  template<class TASKTRACER>
    class FcfsComponent : public NonPreemptiveComponent<TASKTRACER> {
    public:
      FcfsComponent(Config::Component::Ptr component, Director *director =
        &Director::getInstance()) :
        NonPreemptiveComponent<TASKTRACER>(component, director)
      {
      }

      virtual ~FcfsComponent() {}

      void addTask(Task *newTask)
      {
        DBG_OUT(this->getName() << " add Task: " << newTask->getName()
                << " @ " << sc_time_stamp() << std::endl);
        readyTasks.push_back(newTask);
      }


      Task * scheduleTask();

      virtual void notifyActivation(ScheduledTask * scheduledTask,
          bool active);

      virtual bool releaseActor();

      bool hasReadyTask(){
        return !readyTasks.empty();
      }
    protected:
      std::deque<Task*>                readyTasks;
      std::list<ScheduledTask *>       fcfsQueue;

    };

    template<class TASKTRACER>
    class TtFcfsComponent : public FcfsComponent<TASKTRACER> {
    public:
      TtFcfsComponent(Config::Component::Ptr component, Director *director =
        &Director::getInstance()) :
        FcfsComponent<TASKTRACER>(component, director)
      {
      }

      virtual ~TtFcfsComponent() {}


      virtual void notifyActivation(ScheduledTask * scheduledTask, bool active)
      {
        DBG_OUT(this->name() << " notifyActivation " << scheduledTask
            << " " << active << std::endl);
        if (active) {
          if (scheduledTask->getNextReleaseTime() > sc_time_stamp()) {
            ttReleaseQueue.push(
                TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask));
          } else {
            this->fcfsQueue.push_back(scheduledTask);
          }
          if (this->runningTask == NULL) {
            this->notify_scheduler_thread.notify(SC_ZERO_TIME);
          }
        }
      }

      virtual bool releaseActor()
      {
        //move active TT actors to fcfsQueue
        while(!ttReleaseQueue.empty()
            && ttReleaseQueue.top().time<=sc_time_stamp()){
          this->fcfsQueue.push_back(ttReleaseQueue.top().node);
          ttReleaseQueue.pop();
        }
        bool released = FcfsComponent<TASKTRACER>::releaseActor();
        if(!ttReleaseQueue.empty() && !released){
          sc_time delta = ttReleaseQueue.top().time-sc_time_stamp();
          this->notify_scheduler_thread.notify(delta);
        }
        return released;
      }
    private:
      TT::TimedQueue ttReleaseQueue;
    };


    template<class TASKTRACER>
    void FcfsComponent<TASKTRACER>::notifyActivation(ScheduledTask * scheduledTask,
        bool active)
    {
      DBG_OUT(this->name() << " notifyActivation " << scheduledTask
          << " " << active << std::endl);
      if (active) {
        fcfsQueue.push_back(scheduledTask);
        if (this->runningTask == NULL) {
          this->notify_scheduler_thread.notify(SC_ZERO_TIME);
        }
      }
    }

    template<class TASKTRACER>
    bool FcfsComponent<TASKTRACER>::releaseActor()
    {
      while (!fcfsQueue.empty()) {
        ScheduledTask * scheduledTask = fcfsQueue.front();
        fcfsQueue.pop_front();

        bool canExec = scheduledTask->canFire();
    //    bool canExec = Director::canExecute(scheduledTask);
        DBG_OUT("FCFS test task: " << scheduledTask
            << " -> " << canExec << std::endl);
        if (canExec) {
          scheduledTask->schedule();
    //      Director::execute(scheduledTask);
          return true;
        }
      }

      return false;
    }

    template<class TASKTRACER>
    Task * FcfsComponent<TASKTRACER>::scheduleTask()
    {
      assert(!readyTasks.empty());
      Task* task = readyTasks.front();
      readyTasks.pop_front();
      this->startTime = sc_time_stamp();
      DBG_OUT(this->getName() << " schedule Task: " << task->getName()
          << " @ " << sc_time_stamp() << std::endl);

      /*
       * Assuming PSM actors are assigned to the same component they model, the executing state of the component should be IDLE
       */
      if (task != NULL and task->isPSM() == true)
        this->fireStateChanged(ComponentState::IDLE);
      else
        this->fireStateChanged(ComponentState::RUNNING);

      if (task->isBlocking() /* && !assignedTask->isExec() */) {
        //TODO
      }
      return task;
    }






} // namespace SystemC_VPC


#endif /* NONPREEMPTIVESCHEUDLER_FCFSCOMPONENT_HPP_ */
