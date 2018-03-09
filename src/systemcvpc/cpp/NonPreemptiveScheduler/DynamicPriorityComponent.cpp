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

#include "config.h"
#include "DynamicPriorityComponent.hpp"

namespace SystemC_VPC {

DynamicPriorityComponent::DynamicPriorityComponent(
    Config::Component::Ptr  component, Director *director)
  : NonPreemptiveComponent(component, director)
  , yieldTask(nullptr)
  , selectedTask(nullptr)
  , debugOut(component->hasDebugFile()
      ? new Diagnostics::PrintDebug(component->getDebugFileName())
      : nullptr)
{
  // build initial priority list
  std::priority_queue<PriorityFcfsElement<TaskInterface*> > pQueue;
  size_t fcfsOrder = 0;

  // put every task in a priority queue
  Config::Component::MappedTasks mp = component->getMappedTasks();
  for (Config::Component::MappedTasks::iterator iter = mp.begin(); iter
      != mp.end(); ++iter) {
    TaskInterface *actor = *iter;
    size_t priority = Config::getCachedTask(*actor)->getPriority();
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

// Implement ComponentInterface
void DynamicPriorityComponent::setDynamicPriority(std::list<ScheduledTask *> priorityList)
  { priorities_ = reinterpret_cast<PriorityList &>(priorityList); }

// Implement ComponentInterface
std::list<ScheduledTask *> DynamicPriorityComponent::getDynamicPriority()
  { return reinterpret_cast<std::list<ScheduledTask *> &>(priorities_); }

// Implement ComponentInterface
void DynamicPriorityComponent::scheduleAfterTransition() {
  yieldTask = selectedTask->getScheduledTask();
  assert(yieldTask);
}

DynamicPriorityComponent::~DynamicPriorityComponent() {
  if (debugOut)
    delete debugOut;
  debugOut = nullptr;
}

void DynamicPriorityComponent::newReadyTask(Task *newTask) {
  readyTasks.push_back(newTask);
}

Task *DynamicPriorityComponent::selectReadyTask() {
  assert(!readyTasks.empty());
  for (TaskInterface *priorityTask : priorities_) {
    if (yieldTask == priorityTask)
      continue;
    for (std::list<Task *>::iterator readyTaskIter = readyTasks.begin();
         readyTaskIter != readyTasks.end();
         ++readyTaskIter) {
      if ((*readyTaskIter)->getScheduledTask() == priorityTask) {
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

} //namespace SystemC_VPC
