/*
 * RoundRobinComponent.hpp
 *
 *  Created on: 17.05.2017
 *      Author: muellersi
 */

#ifndef NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP_
#define NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP_

#include "memory.h"

#include <vector>
#include <deque>

#include <systemcvpc/AbstractComponent.hpp>

namespace SystemC_VPC{

  template<class TASKTRACER>
  class RoundRobinComponent : public AbstractComponent {
    SC_HAS_PROCESS(RoundRobinComponent);
  public:
    RoundRobinComponent(Config::Component::Ptr component,
        Director *director = &Director::getInstance())
      : AbstractComponent(component)
      , actualTask(NULL)
      , taskTracer(component)
    {
      /// FIXME: WTF?! SLOW hardcoded?
      this->setPowerMode(this->translatePowerMode("SLOW"));

      SC_THREAD(scheduleThread);
    }

  protected:
    void end_of_elaboration() {
      PCBPool const &pcbPool = getPCBPool();
      for (PCBPool::const_iterator it=pcbPool.begin(); it!=pcbPool.end(); ++it) {
        std::cout << "\t " << it->second->getName() << std::endl;
        Task &task = Director::getInstance().taskPool.getPrototype(it->first);
        task.setPCB(it->second);
//      task.setTiming(this->getTiming(this->getPowerMode(), it->first));
        if (task.hasScheduledTask()) {
          ScheduledTask *scheduledTask = task.getScheduledTask();
          scheduledTask->setUseActivationCallback(false);
          taskList.push_back(scheduledTask);
        }
      }
    }

    void compute(Task *actualTask) {
      std::cout << "\t " << sc_time_stamp() << " : task PID " << actualTask->getProcessId() << std::endl;
      ProcessId pid = actualTask->getProcessId();
//      taskTracer.release(actualTask);
      /// Note that actualTask is not the task prototype, i.e.,
      /// Director::getInstance().taskPool.getPrototype(pid),
      /// but an instance allocated with PrototypedPool<Task>::allocate().
      actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));
      actualTask->initDelays();
      scheduleMessageTasks(true);
      if (actualTask->hasScheduledTask()) {
        assert(!this->actualTask);
        this->actualTask = actualTask;
      } else
        readyMsgTasks.push_back(actualTask);

      readyEvent.notify();
    }

    void check(Task *actualTask) {
      ProcessId pid = actualTask->getProcessId();
      actualTask->setTiming(this->getTiming(this->getPowerMode(), pid));
      actualTask->initDelays();

//      scheduleMessageTasks(true);
      this->taskTracer.assign(actualTask);
      wait(actualTask->getOverhead());//Director::getInstance().getOverhead() +
      this->taskTracer.finishDii(actualTask);
      this->taskTracer.finishLatency(actualTask);

      std::cout << "check: " <<  actualTask->getName() << std::endl;
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
      return taskTracer.getOrCreateTraceSignal(name);
    }

    void scheduleMessageTasks(bool success) {
      while (!readyMsgTasks.empty()) {
        Task *messageTask = readyMsgTasks.front();
        readyMsgTasks.pop_front();
        assert(!messageTask->hasScheduledTask());
        this->taskTracer.assign(messageTask);
        /// This will setup the trigger for schedule_method to be called
        /// again when the task execution time is over.
        wait(messageTask->getDelay());
//        wait(0.9 * messageTask->getDelay());
        Director::getInstance().signalLatencyEvent(messageTask);
        this->taskTracer.finishDii(messageTask);
        this->taskTracer.finishLatency(messageTask);
        /// The scheduledTask, i.e., the SysteMoC actor, should now be in the comm state.
        /// Enable transition out of comm state by notifying the dii event.
        messageTask->getBlockEvent().dii->notify();
        /// FIXME: What about dii != latency
        success = true;
      }
      if (!success)
        wait(1, SC_NS);
    }

    std::vector<ScheduledTask *> taskList;

    void scheduleThread() {
      while (true) {
        bool success = false;
        if(taskList.empty())
          scheduleMessageTasks(success);
        for (ScheduledTask *scheduledTask: taskList) {
          scheduleMessageTasks(true);
          while (scheduledTask->canFire()) {
            // This will invoke our compute callback and setup actualTask.
            assert(readyMsgTasks.empty());
            std::cout << "schedule Aufruf at : " << sc_time_stamp() << std::endl;
            scheduledTask->schedule();
            scheduleMessageTasks(true);
            while (!actualTask) {
              wait(readyEvent);
              scheduleMessageTasks(true);
            }
            assert(actualTask);
            assert(actualTask->hasScheduledTask());
            assert(actualTask->getScheduledTask() == scheduledTask);

            actualTask->destState = scheduledTask->getDestStateName();
            this->taskTracer.assign(actualTask);
            wait(actualTask->getOverhead());//Director::getInstance().getOverhead() +
            this->taskTracer.finishDii(actualTask);
            this->taskTracer.finishLatency(actualTask);

            /// The scheduledTask, i.e., the SysteMoC actor, should now be in the comm state.
            /// Enable transition out of comm state by notifying the dii event.
            actualTask->getBlockEvent().dii->notify();
            /// Thus, the transition out of the comm state should now be enabled.
            assert(scheduledTask->canFire());
            /// Finally, take the transition out of the comm state.
            scheduledTask->schedule();
            /// FIXME: What about dii != latency
            Director::getInstance().signalLatencyEvent(actualTask);
            actualTask = nullptr;
            success = true;
            scheduleMessageTasks(success);
          }
        }
      }
    }

    virtual ~RoundRobinComponent() {}

    std::deque<Task *>  readyMsgTasks;
    Task               *actualTask;
    sc_core::sc_event   readyEvent;
    TASKTRACER          taskTracer;

  };

} // namespace SystemC_VPC


#endif /* NONPREEMPTIVESCHEDULER_ROUNDROBINCOMPONENT_HPP_ */
