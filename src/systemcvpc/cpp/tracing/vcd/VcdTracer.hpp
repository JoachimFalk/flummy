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

#ifndef _INCLUDED_SYSTEMCVPC_TRACING_VCD_VCDTRACER_HPP
#define _INCLUDED_SYSTEMCVPC_TRACING_VCD_VCDTRACER_HPP

#include <systemcvpc/ProcessControlBlock.hpp>
#include <systemcvpc/Task.hpp>
#include <systemcvpc/config/Component.hpp>

#include "Tracing.hpp"

namespace SystemC_VPC { namespace Trace {

class VcdTracer {
public:

  //
  VcdTracer(Config::Component::Ptr component)
    : traceFile_(NULL), name_(component->getName())
  {}

  virtual ~VcdTracer() {
    for (std::map<std::string, Tracing*>::iterator iter =
        trace_map_by_name_.begin(); iter != trace_map_by_name_.end(); ++iter) {
      delete iter->second;
    }
    trace_map_by_name_.clear();
    if (traceFile_) {
      sc_close_vcd_trace_file(traceFile_);
    }
  }

  std::string getName() const
  {
    return name_;
  }

  void release(Task * task) const
  {
    task->getTraceSignal()->traceReady();
    task->traceReleaseTask();
  }

  void finishDii(Task * task) const
  {
    task->getTraceSignal()->traceSleeping();
  }

  void finishLatency(Task * task) const
  {
    task->traceFinishTaskLatency();
  }

  void assign(Task * task) const
  {
    task->getTraceSignal()->traceRunning();
  }

  void resign(Task * task) const
  {
    task->getTraceSignal()->traceReady();
  }

  void block(Task * task) const
  {
    task->getTraceSignal()->traceBlocking();
  }

  Tracing * getOrCreateTraceSignal(std::string name)
  {
    if (this->traceFile_ == NULL) {
      std::string tracefilename = this->getName(); //componentName;

      char* traceprefix = getenv("VPCTRACEFILEPREFIX");
      if (0 != traceprefix) {
        tracefilename.insert(0, traceprefix);
      }

      this->traceFile_ = sc_create_vcd_trace_file(tracefilename.c_str());
      this->traceFile_->set_time_unit(1, SC_NS);
    }
    Tracing *newsignal = new Tracing(name, this->getName());

    this->trace_map_by_name_.insert(
        std::pair<std::string, Tracing*>(this->getName(), newsignal));
    sc_trace(this->traceFile_, *newsignal->traceSignal, name);
    newsignal->traceSleeping();
    return newsignal;
  }
private:
  sc_trace_file *traceFile_;
  std::string name_;
  std::map<std::string, Trace::Tracing*> trace_map_by_name_;

};

} } // namespace SystemC_VPC::Trace

#endif /* _INCLUDED_SYSTEMCVPC_TRACING_VCD_VCDTRACER_HPP */
