// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2018 Hardware-Software-CoDesign, University of
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

#include "TraceableComponent.hpp"

#include <stdexcept>
#include <map>
#include <sstream>

#include <assert.h>

namespace SystemC_VPC { namespace Detail { namespace Tracing {

TraceableComponent::TraceableComponent()
  : tracingStarted(false) {}

typedef std::map<std::string, std::function<TracerIf *(Component const *)> > TracerByName;

/// We need this to be independent from the global variable initialization order.
TracerByName &getTracerByName() {
  static TracerByName tracerByName;
  return tracerByName;
}

void TraceableComponent::registerTracer(
    const char                                   *tracerName,
    std::function<TracerIf *(Component const *)>  tracerFactory)
{
  sassert(getTracerByName().insert(std::make_pair(tracerName, tracerFactory)).second);
}

/// Add a tracer by its name.
/// This must no longer be called after the first task has been
/// registered via registerTask.
void TraceableComponent::addTracer(
    const char      *tracerName,
    Component const *componentConfig) {
  TracerByName::const_iterator iter = getTracerByName().find(tracerName);
  if (iter == getTracerByName().end()) {
    std::stringstream msg;
    msg << "Unknown component tracer " << tracerName << "!";
    throw std::runtime_error(msg.str().c_str());
  }
  addTracer(iter->second(componentConfig));
}

/// Add a tracer. This must no longer be called after the first task
/// has been registered via registerTask.
void TraceableComponent::addTracer(TracerIf *tracer) {
  assert(registerdTasks.empty());
  assert(tracer);
  tracers.push_back(tracer);
}

/// Called once per actor. Must not be called after startTracing was called.
void TraceableComponent::registerTask(TTaskHolder *ttaskHolder, std::string const &name) {
  for (TTask *ttask : ttaskHolder->ttasks)
    delete ttask;
  ttaskHolder->ttasks.clear();
  // We use this map to sort the tasks by name. This is required for a
  // deterministic generation of tracing logs. Otherwise, the tracing log
  // might depend on the order the tasks are registered.
  registerdTasks[name].push_back(ttaskHolder);
}

/// Called once to initialize tracing
void TraceableComponent::startTracing() {
  for (RegisterdTasks::value_type const &entry : registerdTasks) {
    std::string const &name = entry.first;
    for (TTaskHolder *ttaskHolder : entry.second) {
      for (TracerIf *tracer : tracers)
        ttaskHolder->ttasks.push_back(tracer->registerTask(name));
    }
  }
  tracingStarted = true;
}

/// Called once per actor firing to create a trace task instance in the task instance.
/// The startTracing method has to be called first.
void TraceableComponent::releaseTask(TTaskHolder *ttaskHolder, TTaskInstanceHolder *ttaskInstanceHolder) {
  assert(tracingStarted);
  for (TTaskInstance *ttaskInstance : ttaskInstanceHolder->ttaskInstances)
    delete ttaskInstance;
  ttaskInstanceHolder->ttaskInstances.clear();
  assert(ttaskHolder->ttasks.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    ttaskInstanceHolder->ttaskInstances.push_back(
        tracers[i]->release(ttaskHolder->ttasks[i]));
}

/// Called possibly multiple times to assign the task instance to the resource.
/// The startTracing method has to be called first.
void TraceableComponent::assignTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder) {
  assert(tracingStarted);
  assert(ttaskInstanceHolder->ttaskInstances.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    tracers[i]->assign(ttaskInstanceHolder->ttaskInstances[i]);
}

/// Called possibly multiple times to resign the task instance from the resource.
/// The startTracing method has to be called first.
void TraceableComponent::resignTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder) {
  assert(tracingStarted);
  assert(ttaskInstanceHolder->ttaskInstances.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    tracers[i]->resign(ttaskInstanceHolder->ttaskInstances[i]);
}

/// Called possibly multiple times to indicate that the task is blocked waiting for something.
/// The startTracing method has to be called first.
void TraceableComponent::blockTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder) {
  assert(tracingStarted);
  assert(ttaskInstanceHolder->ttaskInstances.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    tracers[i]->block(ttaskInstanceHolder->ttaskInstances[i]);
}

/// Called once per actor firing to indicate that the DII of the task instance is over.
/// The startTracing method has to be called first.
void TraceableComponent::finishDiiTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder) {
  assert(tracingStarted);
  assert(ttaskInstanceHolder->ttaskInstances.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    tracers[i]->finishDii(ttaskInstanceHolder->ttaskInstances[i]);
}

/// Called once per actor firing to indicate that the latency of the task instance is over.
/// The startTracing method has to be called first.
void TraceableComponent::finishLatencyTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder) {
  assert(tracingStarted);
  assert(ttaskInstanceHolder->ttaskInstances.size() == tracers.size());
  for (unsigned int i = 0; i < tracers.size(); ++i)
    tracers[i]->finishLatency(ttaskInstanceHolder->ttaskInstances[i]);
}

TraceableComponent::~TraceableComponent() {
  for (TracerIf *tracer : tracers)
    delete tracer;
  tracers.clear();
}

} } } // namespace SystemC_VPC::Detail::Tracing
