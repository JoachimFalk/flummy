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

#include <assert.h>

namespace SystemC_VPC { namespace Tracing {

TraceableComponent::TraceableComponent()
  : noTasks(true) {}

void TraceableComponent::addTracer(TracerIf *tracer) {
  assert(noTasks);
  assert(tracer);
  tracers.push_back(tracer);
}

/// Called once per actor
void TraceableComponent::registerTask(TTaskHolder *ttaskHolder, std::string const &name) {
  noTasks = false;
  for (TTask *ttask : ttaskHolder->ttasks)
    delete ttask;
  ttaskHolder->ttasks.clear();
  for (TracerIf *tracer : tracers)
    ttaskHolder->ttasks.push_back(tracer->registerTask(name));
}

/// Called once per actor firing to create a trace task instance in the task instance.
void TraceableComponent::releaseTask(TTaskHolder *ttaskHolder, TTaskInstanceHolder *ttaskInstanceHolder) {
  for (TTaskInstance *ttaskInstance : ttaskInstanceHolder->ttaskInstances)
    delete ttaskInstance;
  ttaskInstanceHolder->ttaskInstances.clear();
  assert(ttaskHolder->ttasks.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    ttaskInstanceHolder->ttaskInstances.push_back(
        tracers[i]->release(ttaskHolder->ttasks[i]));
}

/// Called possibly multiple times to assign the task instance to the resource.
void TraceableComponent::assignTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder) {
  assert(ttaskInstanceHolder->ttaskInstances.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    tracers[i]->assign(ttaskInstanceHolder->ttaskInstances[i]);
}

/// Called possibly multiple times to resign the task instance from the resource.
void TraceableComponent::resignTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder) {
  assert(ttaskInstanceHolder->ttaskInstances.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    tracers[i]->resign(ttaskInstanceHolder->ttaskInstances[i]);
}

/// Called possibly multiple times to indicate that the task is blocked waiting for something.
void TraceableComponent::blockTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder) {
  assert(ttaskInstanceHolder->ttaskInstances.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    tracers[i]->block(ttaskInstanceHolder->ttaskInstances[i]);
}

/// Called once per actor firing to indicate that the DII of the task instance is over.
void TraceableComponent::finishDiiTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder) {
  assert(ttaskInstanceHolder->ttaskInstances.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    tracers[i]->finishDii(ttaskInstanceHolder->ttaskInstances[i]);
}

/// Called once per actor firing to indicate that the latency of the task instance is over.
void TraceableComponent::finishLatencyTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder) {
  assert(ttaskInstanceHolder->ttaskInstances.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    tracers[i]->finishLatency(ttaskInstanceHolder->ttaskInstances[i]);
}

TraceableComponent::~TraceableComponent() {
  for (TracerIf *tracer : tracers)
    delete tracer;
  tracers.clear();
}

} } // namespace SystemC_VPC::Tracing
