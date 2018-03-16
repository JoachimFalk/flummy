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

#include "../Task.hpp"
#include <systemcvpc/config/Component.hpp>

#include "VcdTracer.hpp"

#include <systemcvpc/vpc_config.h>

namespace SystemC_VPC { namespace Trace {

typedef char trace_value;

/** ASCII lower case bit is  2^5 */
const unsigned int LOWER_CASE = 32;
const unsigned int UPPER_CASE = ~LOWER_CASE;

/**
 * tiny little helper: toggle ASCII characters used for VCD tracing
 */
class Tracing {
public:
  static const trace_value S_SLEEP;
  static const trace_value S_BLOCKED;
  static const trace_value S_READY;
  static const trace_value S_RUNNING;

  Tracing(std::string resource, std::string task)
    : traceSignal(new sc_core::sc_signal<trace_value>())
    , resource(resource)
    , task(task)
    , lastChange(sc_core::SC_ZERO_TIME)
    , lastValue(0) {}

  /** signal for VCD tracing */
  sc_core::sc_signal<trace_value>* traceSignal;

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
  void rememberLastValue(){
    if(lastChange != sc_core::sc_time_stamp()){
      // remember value from last real changing (ignore delta cycle changing)
      lastValue    = *traceSignal;
      lastChange   = sc_core::sc_time_stamp();
    }
  }

  /**
   * write value to signal
   */
  void writeValue(trace_value value){
    rememberLastValue();
    *traceSignal  = value;
  }

  /**
   * Set trace value.
   * If the signal is identical to lastValue then the ASCII bit for
   * lower case is toggled
   */
  void setValueWithCaseCorrection(trace_value value){
    rememberLastValue();

    if(lastValue == value){
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
  std::string resource;

  /// name of traced task
  std::string task;

  /** remeber last time of signal changing */
  sc_core::sc_time                 lastChange;

  /** rember last signal value */
  trace_value             lastValue;

}; // struct Tracing

const trace_value Tracing::S_SLEEP   = ' ';
const trace_value Tracing::S_BLOCKED = 'b';
const trace_value Tracing::S_READY   = 'w';
const trace_value Tracing::S_RUNNING = 'R';

#ifdef VPC_ENABLE_PLAIN_TRACING
std::ostream * Tracing::plainTrace = new CoSupport::Streams::AOStream(std::cout, "vpc.trace", "-");
#endif // VPC_ENABLE_PLAIN_TRACING

VcdTracer::VcdTracer(Config::Component::Ptr component)
  : traceFile_(NULL)
  , name_(component->getName())
{}

VcdTracer::~VcdTracer() {
  for (std::map<std::string, Tracing*>::iterator iter =
      trace_map_by_name_.begin(); iter != trace_map_by_name_.end(); ++iter) {
    delete iter->second;
  }
  trace_map_by_name_.clear();
  if (traceFile_) {
    sc_core::sc_close_vcd_trace_file(traceFile_);
  }
}

std::string VcdTracer::getName() const {
  return name_;
}

void VcdTracer::release(Task const *task) {
  task->getTraceSignal()->traceReady();
  // FIXME: This should become its own tracer!
  const_cast<Task *>(task)->traceReleaseTask();
}

void VcdTracer::finishDii(Task const *task) {
  task->getTraceSignal()->traceSleeping();
}

void VcdTracer::finishLatency(Task const *task) {
  // FIXME: This should become its own tracer!
  const_cast<Task *>(task)->traceFinishTaskLatency();
}

void VcdTracer::assign(Task const *task) {
  task->getTraceSignal()->traceRunning();
}

void VcdTracer::resign(Task const *task) {
  task->getTraceSignal()->traceReady();
}

void VcdTracer::block(Task const *task) {
  task->getTraceSignal()->traceBlocking();
}

Tracing *VcdTracer::getOrCreateTraceSignal(std::string const &name) {
  if (this->traceFile_ == NULL) {
    std::string tracefilename = this->getName(); //componentName;

    char* traceprefix = getenv("VPCTRACEFILEPREFIX");
    if (0 != traceprefix) {
      tracefilename.insert(0, traceprefix);
    }

    this->traceFile_ = sc_core::sc_create_vcd_trace_file(tracefilename.c_str());
    this->traceFile_->set_time_unit(1, sc_core::SC_NS);
  }
  Tracing *newsignal = new Tracing(name, this->getName());

  this->trace_map_by_name_.insert(
      std::pair<std::string, Tracing*>(this->getName(), newsignal));
  sc_trace(this->traceFile_, *newsignal->traceSignal, name);
  newsignal->traceSleeping();
  return newsignal;
}

} } // namespace SystemC_VPC::Trace
