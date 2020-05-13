// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#include <systemcvpc/Component.hpp>
#include <systemcvpc/ConfigException.hpp>

#include <systemcvpc/Extending/ComponentTracerIf.hpp>
#include <systemcvpc/ComponentTracer.hpp>

#include <systemc>

#include <string>
#include <sstream>

#include <systemcvpc/vpc_config.h>

namespace SystemC_VPC {

  const char *Component::Tracer::VCD = "VCD";

} // namespace SystemC_VPC

namespace SystemC_VPC { namespace Detail { namespace Tracing {

  class ComponentVCDTracer
    : public Extending::ComponentTracerIf
    , public ComponentTracer
  {
  public:
    ComponentVCDTracer(Attributes const &attrs);

    ~ComponentVCDTracer();

    ///
    /// Implement interface for ComponentTracerIf
    ///

    void componentOperation(ComponentOperation co
      , Component const &c
      , OComponent      &oc);

    void taskOperation(TaskOperation to
      , Component const &c
      , OComponent      &oc
      , Task      const &t
      , OTask           &ot);

    void taskInstanceOperation(TaskInstanceOperation tio
      , Component    const &c
      , OComponent         &oc
      , OTask              &ot
      , TaskInstance const &ti
      , OTaskInstance      &oti);

    ///
    /// Implement interface for ComponentTracer
    ///

    bool addAttribute(Attribute const &attr);

  private:
    class VcdTask;
    class VcdTaskInstance;
    class RegisterMe;

    static RegisterMe registerMe;

    sc_core::sc_trace_file *traceFile_;
  //std::map<std::string, Trace::Tracing*> trace_map_by_name_;
  };

  typedef char trace_value;

  /** ASCII lower case bit is  2^5 */
  const unsigned int LOWER_CASE = 32;
  const unsigned int UPPER_CASE = ~LOWER_CASE;

  /**
   * tiny little helper: toggle ASCII characters used for VCD tracing
   */
  class ComponentVCDTracer::VcdTask: public OTask {
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
      traceSleeping();
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

  const trace_value ComponentVCDTracer::VcdTask::S_SLEEP   = ' ';
  const trace_value ComponentVCDTracer::VcdTask::S_BLOCKED = 'b';
  const trace_value ComponentVCDTracer::VcdTask::S_READY   = 'w';
  const trace_value ComponentVCDTracer::VcdTask::S_RUNNING = 'R';

  class ComponentVCDTracer::VcdTaskInstance: public OTaskInstance {
  public:
    VcdTaskInstance(VcdTask *vcdTask)
      : vcdTask(vcdTask) {}

    VcdTask *vcdTask;

    ~VcdTaskInstance() {}
  };

  class ComponentVCDTracer::RegisterMe {
  public:
    RegisterMe() {
      ComponentVCDTracer::registerTracer("VCD",
        [](Attributes const &attrs) { return new ComponentVCDTracer(attrs); });
    }
  } ComponentVCDTracer::registerMe;

  #ifdef VPC_ENABLE_PLAIN_TRACING
  std::ostream * Tracing::plainTrace = new CoSupport::Streams::AOStream(std::cout, "vpc.trace", "-");
  #endif // VPC_ENABLE_PLAIN_TRACING

  ComponentVCDTracer::ComponentVCDTracer(Attributes const &attrs)
    : Extending::ComponentTracerIf(
          reinterpret_cast<char *>(static_cast<ComponentTracer              *>(this)) -
          reinterpret_cast<char *>(static_cast<Extending::ComponentTracerIf *>(this))
        , 0, sizeof(VcdTask), sizeof(VcdTaskInstance))
    , ComponentTracer(
         reinterpret_cast<char *>(static_cast<Extending::ComponentTracerIf *>(this)) -
         reinterpret_cast<char *>(static_cast<ComponentTracer              *>(this))
       , "VCD")
    , traceFile_(nullptr)
  {
    for (Attribute const &attr : attrs) {
      if (attr.getType() == "traceFileName") {
        if (traceFile_) {
          throw ConfigException(
              "Duplicate attribute traceFileName in VCD component tracer!");
        }
        std::string const &traceFileName = attr.getValue();
        if (traceFileName.size() < 4 ||
            traceFileName.substr(traceFileName.size()-4,4) != ".vcd") {
          throw ConfigException(
              "The traceFileName "+traceFileName+" must end in .vcd for VCD component tracers!");
        }

        traceFile_ = sc_core::sc_create_vcd_trace_file(
            traceFileName.substr(0, traceFileName.size()-4).c_str());
        traceFile_->set_time_unit(1, sc_core::SC_NS);
      } else {
        std::stringstream msg;

        msg << "Component VCD tracers do not support the "+attr.getType()+" attribute! ";
        msg << "Only the traceFileName attribute is supported.";
        throw ConfigException(msg.str());
      }
    }
  }

  ComponentVCDTracer::~ComponentVCDTracer() {
    if (traceFile_) {
      sc_core::sc_close_vcd_trace_file(traceFile_);
    }
  }

  void ComponentVCDTracer::componentOperation(ComponentOperation co
    , Component const &c
    , OComponent      &oc)
  {
    // Ignore
  }

  void ComponentVCDTracer::taskOperation(TaskOperation to
    , Component const &c
    , OComponent      &oc
    , Task      const &t
    , OTask           &ot)
  {
    if (this->traceFile_ == NULL) {
      std::string tracefilename = c.getName(); //componentName;

      char* traceprefix = getenv("VPCTRACEFILEPREFIX");
      if (0 != traceprefix) {
        tracefilename.insert(0, traceprefix);
      }

      this->traceFile_ = sc_core::sc_create_vcd_trace_file(tracefilename.c_str());
      this->traceFile_->set_time_unit(1, sc_core::SC_NS);
    }

    VcdTask &vcdTask = static_cast<VcdTask &>(ot);

    if (TaskOperation((int) to & (int) TaskOperation::MEMOP_MASK) ==
        TaskOperation::ALLOCATE) {
      new (&vcdTask) VcdTask(this->traceFile_, c.getName(), t.getName());
    }
    if (TaskOperation((int) to & (int) TaskOperation::MEMOP_MASK) ==
        TaskOperation::DEALLOCATE) {
      vcdTask.~VcdTask();
    }
  }

  void ComponentVCDTracer::taskInstanceOperation(TaskInstanceOperation tio
    , Component    const &c
    , OComponent         &oc
    , OTask              &ot
    , TaskInstance const &ti
    , OTaskInstance      &oti)
  {
    VcdTask         &vcdTask         = static_cast<VcdTask &>(ot);
    VcdTaskInstance &vcdTaskInstance = static_cast<VcdTaskInstance &>(oti);

    if (TaskInstanceOperation((int) tio & (int) TaskInstanceOperation::MEMOP_MASK) ==
        TaskInstanceOperation::ALLOCATE) {
      new (&vcdTaskInstance) VcdTaskInstance(&vcdTask);
    }

    switch (TaskInstanceOperation((int) tio & ~ (int) TaskInstanceOperation::MEMOP_MASK)) {
      case TaskInstanceOperation::RELEASE:
        vcdTaskInstance.vcdTask->traceReady();
        break;
      case TaskInstanceOperation::ASSIGN:
        vcdTaskInstance.vcdTask->traceRunning();
        break;
      case TaskInstanceOperation::RESIGN:
        vcdTaskInstance.vcdTask->traceReady();
        break;
      case TaskInstanceOperation::BLOCK:
        vcdTaskInstance.vcdTask->traceBlocking();
        break;
      case TaskInstanceOperation::FINISHDII:
        vcdTaskInstance.vcdTask->traceSleeping();
        break;
      case TaskInstanceOperation::FINISHLAT:
        // Ignore this. Use PAJE tracer to visualize this.
        break;
      default:
        break;
    }

    if (TaskInstanceOperation((int) tio & (int) TaskInstanceOperation::MEMOP_MASK) ==
        TaskInstanceOperation::DEALLOCATE) {
      vcdTaskInstance.~VcdTaskInstance();
    }
  }

  bool ComponentVCDTracer::addAttribute(Attribute const &attr) {
    throw ConfigException("The VCD tracer does not support attributes!");
  }

} } } // namespace SystemC_VPC::Detail::Tracing
