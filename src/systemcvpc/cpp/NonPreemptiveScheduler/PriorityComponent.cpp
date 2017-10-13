/*
 * PriorityComponent.hpp
 *
 *  Created on: 16.05.2017
 *      Author: muellersi
 */

#include "PriorityComponent.hpp"

namespace SystemC_VPC {

PriorityComponent::PriorityComponent(
    Config::Component::Ptr component, Director *director)
  : NonPreemptiveComponent(component, director)
  , fcfsOrder(0)
{}

PriorityComponent::~PriorityComponent() {}

void PriorityComponent::addTask(Task *newTask) {
  DBG_OUT(this->getName() << " add Task: " << newTask->getName()
          << " @ " << sc_core::sc_time_stamp() << std::endl);
  p_queue_entry entry(fcfsOrder++, newTask);
  readyQueue.push(entry);
}

int PriorityComponent::getPriority(const ScheduledTask * scheduledTask) {
  ProcessId pid = scheduledTask->getPid();
  ProcessControlBlockPtr pcb = this->getPCB(pid);
  return pcb->getPriority();
}

/*
void PriorityComponent::notifyActivation(
    ScheduledTask * scheduledTask, bool active) {
  DBG_OUT(this->name() << " notifyActivation " << scheduledTask
      << " " << active << std::endl);
  if (active) {
    int priority = getPriority(scheduledTask);
    DBG_OUT("  priority is: "<< priority << std::endl);
    releaseQueue.push(QueueElem(priority, fcfsOrder++, scheduledTask));
    if (this->runningTask == NULL) {
      this->notify_scheduler_thread.notify(sc_core::SC_ZERO_TIME);
    }
  }
}
 */

void PriorityComponent::notifyActivation(ScheduledTask * scheduledTask, bool active) {
  DBG_OUT(this->name() << " notifyActivation " << scheduledTask
      << " " << active << std::endl);
  if (active) {
    if (scheduledTask->getNextReleaseTime() > sc_core::sc_time_stamp()) {
      ttReleaseQueue.push(
          TT::TimeNodePair(scheduledTask->getNextReleaseTime(), scheduledTask));
    } else {
      int priority = this->getPriority(scheduledTask);
      DBG_OUT("  priority is: "<< priority << std::endl);
      this->releaseQueue.push(QueueElem(priority, this->fcfsOrder++,
          scheduledTask));
    }
    if (this->runningTask == NULL) {
      this->notify_scheduler_thread.notify(sc_core::SC_ZERO_TIME);
    }
  }
}

bool PriorityComponent::releaseActor() {
  //move active TT actors to fcfsQueue
  while(!ttReleaseQueue.empty()
      && ttReleaseQueue.top().time<=sc_core::sc_time_stamp()){
    ScheduledTask * scheduledTask = ttReleaseQueue.top().node;
    int priority = this->getPriority(scheduledTask);
    this->releaseQueue.push(QueueElem(priority, this->fcfsOrder++,
        scheduledTask));
    ttReleaseQueue.pop();
  }
  bool released = false;
  while (!releaseQueue.empty()) {
    ScheduledTask * scheduledTask = releaseQueue.top().payload;
    releaseQueue.pop();

    bool canExec = scheduledTask->canFire();
//    bool canExec = Director::canExecute(scheduledTask);
    DBG_OUT("PS test task: " << scheduledTask
        << " -> " << canExec << std::endl);
    if (canExec) {
      scheduledTask->scheduleLegacyWithCommState();
//      Director::execute(scheduledTask);
      released = true;
      break;
    }
  }
  if(!ttReleaseQueue.empty() && !released){
    sc_core::sc_time delta = ttReleaseQueue.top().time-sc_core::sc_time_stamp();
    this->notify_scheduler_thread.notify(delta);
  }
  return released;
}

Task * PriorityComponent::scheduleTask() {
  assert(!readyQueue.empty());
  Task* task = readyQueue.top().task;
  readyQueue.pop();
  this->startTime = sc_core::sc_time_stamp();
  DBG_OUT(this->getName() << " schedule Task: " << task->getName()
      << " @ " << sc_core::sc_time_stamp() << std::endl);

  /*
    * Assuming PSM actors are assigned to the same component they model, the executing state of the component should be IDLE
    */
  if (task->isPSM() == true){
   this->fireStateChanged(ComponentState::IDLE);
  }else{
   this->fireStateChanged(ComponentState::RUNNING);
  }
  if (task->isBlocking() /* && !assignedTask->isExec() */) {
    //TODO
  }
  return task;
}

} // namespace SystemC_VPC
