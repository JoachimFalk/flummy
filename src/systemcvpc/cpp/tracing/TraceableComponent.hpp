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

#ifndef _INCLUDED_SYSTEMCVPC_TRACING_TRACEABLECOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_TRACING_TRACEABLECOMPONENT_HPP

#include "TracerIf.hpp"

#include <vector>

namespace SystemC_VPC { namespace Tracing {

/// Base class for all components that can have tracers.
class TraceableComponent {
public:
  /// Add a tracer. This must no longer be called after the first task
  /// has been registered via registerTask.
  void addTracer(TracerIf *tracer);
protected:
  TraceableComponent();

  /// Called once per actor
  void registerTask(TTaskHolder *ttaskHolder, std::string const &name);

  /// Called once per actor firing to create a trace task instance in the task instance.
  void releaseTask(TTaskHolder *ttaskHolder, TTaskInstanceHolder *ttaskInstanceHolder);

  /// Called possibly multiple times to assign the task instance to the resource.
  void assignTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder);

  /// Called possibly multiple times to resign the task instance from the resource.
  void resignTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder);

  /// Called possibly multiple times to indicate that the task is blocked waiting for something.
  void blockTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder);

  /// Called once per actor firing to indicate that the DII of the task instance is over.
  void finishDiiTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder);

  /// Called once per actor firing to indicate that the latency of the task instance is over.
  void finishLatencyTaskInstance(TTaskInstanceHolder *ttaskInstanceHolder);

  ~TraceableComponent();
private:
  /// This flag is true as long as not tasks have been registered by registerTask.
  bool noTasks;

  /// List of all traces.
  std::vector<TracerIf *> tracers;
};

} } // namespace SystemC_VPC::Tracing

#endif /* _INCLUDED_SYSTEMCVPC_TRACING_TRACEABLECOMPONENT_HPP */
