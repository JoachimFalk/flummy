/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 *
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#ifndef TASKTRACER_HPP_
#define TASKTRACER_HPP_

#include <systemcvpc/ProcessControlBlock.hpp>
#include <systemcvpc/Task.hpp>
#include <systemcvpc/config/Component.hpp>

#include "Tracing.hpp"

namespace SystemC_VPC
{
namespace Trace
{

class DiscardTrace
{
public:
  //
  DiscardTrace(Config::Component::Ptr component)
  {
  }

  void release(const Task * task) const
  {
  }

  void finishDii(const Task * task) const
  {
  }

  void finishLatency(const Task * task) const
  {
  }

  void assign(const Task * task) const
  {
  }

  void resign(const Task * task) const
  {
  }

  void block(const Task * task) const
  {
  }

  Tracing * getOrCreateTraceSignal(const std::string name) const
  {
    return NULL;
  }

};

class VcdTrace
{
public:

  //
  VcdTrace(Config::Component::Ptr component) :
    traceFile_(NULL), name_(component->getName())
  {
  }

  virtual ~VcdTrace()
  {
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

  void release(Task * task)
  {
    task->getTraceSignal()->traceReady();
    task->traceReleaseTask();
  }

  void finishDii(Task * task)
  {
    task->getTraceSignal()->traceSleeping();
  }

  void finishLatency(Task * task)
  {
    task->traceFinishTaskLatency();
  }

  void assign(Task * task)
  {
    task->getTraceSignal()->traceRunning();
  }

  void resign(Task * task)
  {
    task->getTraceSignal()->traceReady();
  }

  void block(Task * task)
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

} // namespace Trace
} // namespace SystemC_VPC
#endif /* TASKTRACER_HPP_ */
