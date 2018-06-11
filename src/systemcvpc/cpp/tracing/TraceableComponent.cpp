// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2018-2018 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#include "TraceableComponent.hpp"

namespace SystemC_VPC { namespace Tracing {

TraceableComponent::TraceableComponent()
  : noTasks(true) {}

void TraceableComponent::addTracer(TracerIf *tracer) {
  assert(noTasks);
  assert(tracer);
  tracers.push_back(tracer);
}

/// Called once per actor
void TraceableComponent::registerTask(ProcessControlBlock *task) {
  noTasks = false;
  for (TTask *ttask : task->ttasks)
    delete ttask;
  task->ttasks.clear();
  for (TracerIf *tracer : tracers)
    task->ttasks.push_back(tracer->registerTask(task->getName()));
}

/// Called once per actor firing to create a trace task instance in the task instance.
void TraceableComponent::releaseTask(TaskInstance *taskInstance) {
  for (TTaskInstance *ttaskInstance : taskInstance->ttaskInstances)
    delete ttaskInstance;
  taskInstance->ttaskInstances.clear();
  assert(taskInstance->getPCB()->ttasks.size() == tracers.size());
  for (int i = 0; i < tracers.size(); ++i)
    taskInstance->ttaskInstances.push_back(
        tracers[i]->release(taskInstance->getPCB()->ttasks[i]));
}

/// Called possibly multiple times to assign the task instance to the resource.
void TraceableComponent::assignTaskInstance(TaskInstance *taskInstance) {
  assert(taskInstance->ttaskInstances.size() == tracers.size());
  for (int i = 0; i < tracers.size(); ++i)
    tracers[i]->assign(taskInstance->ttaskInstances[i]);
}

/// Called possibly multiple times to resign the task instance from the resource.
void TraceableComponent::resignTaskInstance(TaskInstance *taskInstance) {
  assert(taskInstance->ttaskInstances.size() == tracers.size());
  for (int i = 0; i < tracers.size(); ++i)
    tracers[i]->resign(taskInstance->ttaskInstances[i]);
}

/// Called possibly multiple times to indicate that the task is blocked waiting for something.
void TraceableComponent::blockTaskInstance(TaskInstance *taskInstance) {
  assert(taskInstance->ttaskInstances.size() == tracers.size());
  for (int i = 0; i < tracers.size(); ++i)
    tracers[i]->block(taskInstance->ttaskInstances[i]);
}

/// Called once per actor firing to indicate that the DII of the task instance is over.
void TraceableComponent::finishDiiTaskInstance(TaskInstance *taskInstance) {
  assert(taskInstance->ttaskInstances.size() == tracers.size());
  for (int i = 0; i < tracers.size(); ++i)
    tracers[i]->finishDii(taskInstance->ttaskInstances[i]);
}

/// Called once per actor firing to indicate that the latency of the task instance is over.
void TraceableComponent::finishLatencyTaskInstance(TaskInstance *taskInstance) {
  assert(taskInstance->ttaskInstances.size() == tracers.size());
  for (int i = 0; i < tracers.size(); ++i)
    tracers[i]->finishLatency(taskInstance->ttaskInstances[i]);
}

TraceableComponent::~TraceableComponent() {
  for (TracerIf *tracer : tracers)
    delete tracer;
  tracers.clear();
}

} } // namespace SystemC_VPC::Tracing
