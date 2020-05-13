// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#include "config.h"
#include "DynamicPriorityComponent.hpp"

namespace SystemC_VPC { namespace Detail {

DynamicPriorityComponent::DynamicPriorityComponent(std::string const &name)
  : NonPreemptiveComponent(name)
  , yieldTask(nullptr)
  , selectedTask(nullptr)
  , debugOut(nullptr)
{}

//
bool DynamicPriorityComponent::hasDebugFile() const {
  return debugOut != nullptr;
}

//
std::string DynamicPriorityComponent::getDebugFileName() const {
  return debugFileName;
}

//
void DynamicPriorityComponent::setDebugFileName(std::string const &fileName) {
  delete debugOut;
  debugOut = nullptr;
  debugOut = new Diagnostics::PrintDebug(fileName);
  debugFileName = fileName;
}

// Implement ComponentInterface
void DynamicPriorityComponent::setDynamicPriority(std::list<ScheduledTask *> priorityList)
  { priorities_ = reinterpret_cast<PriorityList &>(priorityList); }

// Implement ComponentInterface
std::list<ScheduledTask *> DynamicPriorityComponent::getDynamicPriority()
  { return reinterpret_cast<std::list<ScheduledTask *> &>(priorities_); }

// Implement ComponentInterface
void DynamicPriorityComponent::scheduleAfterTransition() {
  yieldTask = selectedTask->getTask()->getScheduledTask();
  assert(yieldTask);
}

DynamicPriorityComponent::~DynamicPriorityComponent() {
  if (debugOut)
    delete debugOut;
  debugOut = nullptr;
}

void DynamicPriorityComponent::newReadyTask(TaskInstanceImpl *newTask) {
  readyTasks.push_back(newTask);
}

TaskInstanceImpl *DynamicPriorityComponent::selectReadyTask() {
  assert(!readyTasks.empty());
  for (TaskInterface *priorityTask : priorities_) {
    if (yieldTask == priorityTask)
      continue;
    for (std::list<TaskInstanceImpl *>::iterator readyTaskIter = readyTasks.begin();
         readyTaskIter != readyTasks.end();
         ++readyTaskIter) {
      if ((*readyTaskIter)->getTask()->getScheduledTask() == priorityTask) {
        yieldTask    = nullptr;
        selectedTask = *readyTaskIter;
        readyTasks.erase(readyTaskIter);
        debugDump(priorityTask);
        return *readyTaskIter;
      }
    }
  }
  // Fall back to FCFS
  yieldTask    = nullptr;
  selectedTask = readyTasks.front();
  readyTasks.pop_front();
  return selectedTask;
}

void DynamicPriorityComponent::start_of_simulation() {
  if (!priorities_.empty())
    return;

  // build initial priority list
  std::priority_queue<PriorityFcfsElement<TaskInterface*> > pQueue;
  size_t fcfsOrder = 0;

  // put every task in a priority queue
  SystemC_VPC::Component::MappedTasks mp = this->getMappedTasks();
  for (SystemC_VPC::Component::MappedTasks::iterator iter = mp.begin(); iter
      != mp.end(); ++iter) {
    TaskInterface *actor = *iter;
    size_t priority = SystemC_VPC::getTask(static_cast<ScheduledTask &>(*actor))->getPriority();
    pQueue.push(
        PriorityFcfsElement<TaskInterface*> (priority, fcfsOrder++, actor));
  }

  // pop tasks (in order of priority) from queue and build priority list
  while (!pQueue.empty()) {
    TaskInterface *actor = pQueue.top().payload;
    priorities_.push_back(actor);
    pQueue.pop();
  }
}

void DynamicPriorityComponent::debugDump(const TaskInterface * toBeExecuted) const
{
  if (debugOut) {
    std::stringstream canExec;

    *debugOut << "@" << sc_core::sc_time_stamp() << "\t" << "[VPC DynamicPriorityComponent: "
        << this->getName() << "] " << "priority list: (";
    for (PriorityList::const_iterator iter = this->priorities_.begin(); iter
        != this->priorities_.end(); ++iter) {
      *debugOut << (*iter)->name() << " ";

      if((*iter)->canFire()){
        canExec << (*iter)->name() << " ";
      }
    }
    *debugOut << ") executable: (" << canExec.str() << ") execute: " <<
        toBeExecuted->name() << std::endl;
  }
}

} } // namespace SystemC_VPC::Detail
