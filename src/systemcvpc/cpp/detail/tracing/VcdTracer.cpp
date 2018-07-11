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

#include <systemcvpc/Component.hpp>

#include "VcdTracer.hpp"
#include "../TaskInstance.hpp"

#include <systemcvpc/vpc_config.h>

namespace SystemC_VPC { namespace Detail { namespace Tracing {

typedef char trace_value;

/** ASCII lower case bit is  2^5 */
const unsigned int LOWER_CASE = 32;
const unsigned int UPPER_CASE = ~LOWER_CASE;

/**
 * tiny little helper: toggle ASCII characters used for VCD tracing
 */
class VcdTracer::VcdTask: public TTask {
public:
  static const trace_value S_SLEEP;
  static const trace_value S_BLOCKED;
  static const trace_value S_READY;
  static const trace_value S_RUNNING;

   VcdTask(
       sc_core::sc_trace_file *traceFile,
       std::string      const &resource,
       std::string      const &task)
    : resource(resource), task(task)
    , lastChange(sc_core::SC_ZERO_TIME)
    , lastValue(0), currentValue(0)
  {
    sc_core::sc_trace(traceFile, currentValue, task);
  }

  void traceRunning(){
    this->tracePlain("RUN");
    this->setValueWithCaseCorrection(S_RUNNING);
  }

  void traceBlocking(){
    this->tracePlain("BLOCK");
    this->setValueWithCaseCorrection(S_BLOCKED);
  }

  void traceSleeping(){
    this->tracePlain("SLEEP");
    this->writeValue(S_SLEEP);
  }

  void traceReady(){
    this->tracePlain("WAIT");
    this->setValueWithCaseCorrection(S_READY);
  }

  void tracePlain(std::string traceValue) {
#ifdef VPC_ENABLE_PLAIN_TRACING
    if(plainTrace != NULL){
      *plainTrace << sc_core::sc_time_stamp().value()
                  << "\t" << resource
                  << "\t" << task
                  << "\t" << traceValue
                  <<  std::endl;
    }
#endif // VPC_ENABLE_PLAIN_TRACING
  }
private:

  /**
   * remember last value and time stamp of change
   */
  trace_value getLastValue() {
    if (sc_core::sc_time_stamp() > lastChange) {
      // remember value from last real changing (ignore delta cycle changing)
      lastValue  = currentValue;
    }
    return lastValue;
  }

  /**
   * write value to signal
   */
  void writeValue(trace_value value) {
    getLastValue(); // To update lastValue
    lastChange   = sc_core::sc_time_stamp();
    currentValue = value;
  }

  /**
   * Set trace value.
   * If the signal is identical to lastValue then the ASCII bit for
   * lower case is toggled
   */
  void setValueWithCaseCorrection(trace_value value){
    if(getLastValue() == value){
      // if value does not change toggle between upper and lower case
      if(value & LOWER_CASE){
        writeValue(value & UPPER_CASE);
      } else {
        writeValue(value | LOWER_CASE);
      }
    }else{
      writeValue(value);
    }
  }

#ifdef VPC_ENABLE_PLAIN_TRACING
  static std::ostream * plainTrace;
#endif // VPC_ENABLE_PLAIN_TRACING

  /// name of traced resource
  std::string      resource;

  /// name of traced task
  std::string      task;

  /** remeber last time of signal changing */
  sc_core::sc_time lastChange;

  /** rember last signal value */
  trace_value      lastValue;

  /** rember current signal value */
  trace_value      currentValue;

}; // struct Tracing

const trace_value VcdTracer::VcdTask::S_SLEEP   = ' ';
const trace_value VcdTracer::VcdTask::S_BLOCKED = 'b';
const trace_value VcdTracer::VcdTask::S_READY   = 'w';
const trace_value VcdTracer::VcdTask::S_RUNNING = 'R';

class VcdTracer::VcdTaskInstance: public TTaskInstance {
public:
  VcdTaskInstance(VcdTask *vcdTask)
    : vcdTask(vcdTask) {}

  VcdTask *vcdTask;

  ~VcdTaskInstance() {}
};

#ifdef VPC_ENABLE_PLAIN_TRACING
std::ostream * Tracing::plainTrace = new CoSupport::Streams::AOStream(std::cout, "vpc.trace", "-");
#endif // VPC_ENABLE_PLAIN_TRACING

VcdTracer::VcdTracer(SystemC_VPC::Component::Ptr component)
  : traceFile_(NULL)
  , name_(component->getName())
{}

VcdTracer::~VcdTracer() {
  if (traceFile_) {
    sc_core::sc_close_vcd_trace_file(traceFile_);
  }
}

std::string VcdTracer::getName() const {
  return name_;
}

TTask         *VcdTracer::registerTask(std::string const &name) {
  if (this->traceFile_ == NULL) {
    std::string tracefilename = this->getName(); //componentName;

    char* traceprefix = getenv("VPCTRACEFILEPREFIX");
    if (0 != traceprefix) {
      tracefilename.insert(0, traceprefix);
    }

    this->traceFile_ = sc_core::sc_create_vcd_trace_file(tracefilename.c_str());
    this->traceFile_->set_time_unit(1, sc_core::SC_NS);
  }
  VcdTask *newsignal = new VcdTask(this->traceFile_, getName(), name);
  newsignal->traceSleeping();
  return newsignal;
}

TTaskInstance *VcdTracer::release(TTask *ttask) {
  VcdTaskInstance *ttaskInstance = new VcdTaskInstance(static_cast<VcdTask *>(ttask));
  ttaskInstance->vcdTask->traceReady();
//// FIXME: This should become its own tracer!
//const_cast<Task *>(task)->traceReleaseTask();
  return ttaskInstance;
}

void           VcdTracer::assign(TTaskInstance *ttaskInstance) {
  static_cast<VcdTaskInstance *>(ttaskInstance)->vcdTask->traceRunning();
}

void           VcdTracer::resign(TTaskInstance *ttaskInstance) {
  static_cast<VcdTaskInstance *>(ttaskInstance)->vcdTask->traceReady();
}

void           VcdTracer::block(TTaskInstance *ttaskInstance) {
  static_cast<VcdTaskInstance *>(ttaskInstance)->vcdTask->traceBlocking();
}

void           VcdTracer::finishDii(TTaskInstance *ttaskInstance) {
  static_cast<VcdTaskInstance *>(ttaskInstance)->vcdTask->traceSleeping();
}

void           VcdTracer::finishLatency(TTaskInstance *ttaskInstance) {
//// FIXME: This should become its own tracer!
//const_cast<Task *>(task)->traceFinishTaskLatency();
}

} } } // namespace SystemC_VPC::Detail::Tracing
