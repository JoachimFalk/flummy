// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_TRACING_TRACERIF_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_TRACING_TRACERIF_HPP

#include <systemcvpc/Component.hpp>

#include <string>
#include <vector>
#include <functional>

namespace SystemC_VPC { namespace Detail { namespace Tracing {

// Custom data for a tracer in the PCB.
class TTask {
public:
  virtual ~TTask();
};

class TTaskHolder {
  friend class TraceableComponent;
private:
  // Custom data for a tracer in the PCB.
  // This can only be managed by TraceableComponent.
  std::vector<TTask *> ttasks;
protected:
  ~TTaskHolder();
};

// Custom data for a tracer in the Task instance.
class TTaskInstance {
public:
  virtual ~TTaskInstance();
};

class TTaskInstanceHolder {
  friend class TraceableComponent;
private:
  // Custom data for a tracer in the PCB.
  // This can only be managed by TraceableComponent.
  std::vector<TTaskInstance *> ttaskInstances;
protected:
  ~TTaskInstanceHolder();
};

class ComponentTracerIf {
public:
  /// Called once per actor
  virtual TTask         *registerTask(std::string const &name) = 0;

  /// Called once per actor firing to create a task instance.
  virtual TTaskInstance *release(TTask *ttask) = 0;

  /// Called possibly multiple times to assign the task instance to the resource.
  virtual void           assign(TTaskInstance *ttaskInstance) = 0;

  /// Called possibly multiple times to resign the task instance from the resource.
  virtual void           resign(TTaskInstance *ttaskInstance) = 0;

  /// Called possibly multiple times to indicate that the task is blocked waiting for something.
  virtual void           block(TTaskInstance *ttaskInstance) = 0;

  /// Called once per actor firing to indicate that the DII of the task instance is over.
  virtual void           finishDii(TTaskInstance *ttaskInstance) = 0;

  /// Called once per actor firing to indicate that the latency of the task instance is over.
  virtual void           finishLatency(TTaskInstance *ttaskInstance) = 0;

  virtual ~ComponentTracerIf();
protected:
  static void registerTracer(
      const char                                   *tracerName,
      std::function<ComponentTracerIf *(Component const *)>  tracerFactory);
};

} } } // namespace SystemC_VPC::Detail::Tracing

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_TRACING_TRACERIF_HPP */