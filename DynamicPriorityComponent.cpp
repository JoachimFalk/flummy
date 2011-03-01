/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 *
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#include "DynamicPriorityComponent.hpp"

#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/VpcApi.hpp>

namespace SystemC_VPC
{

void DynamicPriorityComponent::buildInitialPriorityList(
    Config::Component::Ptr component)
{
  std::priority_queue<PriorityFcfsElement<ScheduledTask*> > pQueue;
  size_t fcfsOrder = 0;

  // put every task in a priority queue
  Config::Component::MappedTasks mp = component->getMappedTasks();
  for (Config::Component::MappedTasks::iterator iter = mp.begin(); iter
      != mp.end(); ++iter) {
    ScheduledTask *actor = *iter;
    size_t priority = Config::getCachedTask(*actor)->getPriority();
    pQueue.push(PriorityFcfsElement<ScheduledTask*> (priority, fcfsOrder++,
        actor));
  }

  // pop tasks (in order of priority) from queue and build priority list
  while (!pQueue.empty()) {
    ScheduledTask *actor = pQueue.top().payload;
    priorities_.push_back(actor);
    pQueue.pop();
  }
}

DynamicPriorityComponent::DynamicPriorityComponent(
    Config::Component::Ptr component, Director *director) :
  NonPreemptiveComponent(component, director), priorities_(),
      mustYield_(false), lastTask_(NULL), releasedTask_(NULL)
{
  buildInitialPriorityList(component);
}

void DynamicPriorityComponent::addTask(Task *newTask)
{
  if (newTask->hasScheduledTask()) {
    assert(releasedTask_ == newTask->getScheduledTask());
    lastTask_ = newTask;
  } else {
    throw Config::ConfigException(std::string("DynamicPriorityComponent ")
        + this->getName() + " doesn't support messages! "
        + "Please do not include it in any Route!");
  }

}

Task * DynamicPriorityComponent::scheduleTask()
{
  assert(runningTask == NULL);
  assert(lastTask_ != NULL);
  assert(releasedTask_ != NULL);
  startTime = sc_time_stamp();

  releasedTask_ = NULL;

  fireStateChanged(ComponentState::RUNNING);
  return lastTask_;
}

void DynamicPriorityComponent::notifyActivation(ScheduledTask * scheduledTask,
    bool active)
{
  if (active && (runningTask == NULL)) {
    notify_scheduler_thread.notify(SC_ZERO_TIME);
  }
}

bool DynamicPriorityComponent::releaseActor()
{
  bool yield = mustYield_;
  mustYield_ = false;
  if (yield || (lastTask_ == NULL)) {
    for (PriorityList::const_iterator iter = priorities_.begin(); iter
        != priorities_.end(); ++iter) {
      ScheduledTask * scheduledTask = *iter;
      bool canExec = Director::canExecute(scheduledTask);
      if (canExec) {
        releasedTask_ = scheduledTask;
        Director::execute(scheduledTask);
        return true;
      }
    }
    return false;
  } else {
    bool canExec = Director::canExecute(lastTask_->getProcessId());
    if (canExec) {
      releasedTask_ = lastTask_->getScheduledTask();
      Director::execute(lastTask_->getProcessId());
      return true;
    }
    return false;
  }
}

bool DynamicPriorityComponent::hasReadyTask()
{
  return releasedTask_ != NULL;
}

void DynamicPriorityComponent::setDynamicPriority(PriorityList priorityList)
{

  priorities_ = priorityList;
  for (PriorityList::const_iterator iter = priorityList.begin(); iter
      != priorityList.end(); ++iter) {
  }
}

void DynamicPriorityComponent::schedulerAfterTransition()
{
  mustYield_ = true;
}

} //namespace SystemC_VPC