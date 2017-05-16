/*
 * FcfsComponent.hpp
 *
 *  Created on: 16.05.2017
 *      Author: muellersi
 */

#ifndef FCFSCOMPONENT_HPP_
#define FCFSCOMPONENT_HPP_

#include "NonPreemptiveComponent.hpp"

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
      std::list<ScheduledTask *>       fcfsQueue;
      std::deque<Task*>                readyTasks;

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


#endif /* FCFSCOMPONENT_HPP_ */
