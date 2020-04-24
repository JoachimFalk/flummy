// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2020 Hardware-Software-CoDesign, University of
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

#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/ComponentTracer.hpp>
#include <systemcvpc/Extending/ComponentTracerIf.hpp>

#include <CoSupport/String/color.hpp>
#include <CoSupport/String/DoubleQuotedString.hpp>
#include <CoSupport/Tracing/PajeTracer.hpp>

#include <iomanip>
#include <memory>
#include <boost/algorithm/string.hpp>

using CoSupport::Tracing::PajeTracer;

namespace {

  std::unique_ptr<PajeTracer> myPajeTracer;

}

namespace SystemC_VPC {

  const char *Component::Tracer::PAJE = "PAJE";

} // namespace SystemC_VPC

namespace SystemC_VPC { namespace Detail { namespace Tracing {

  class ComponentPajeTracer
    : public Extending::ComponentTracerIf
    , public ComponentTracer
  {
  public:
    ComponentPajeTracer(Attribute::Ptr attr);

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

    bool addAttribute(Attribute::Ptr attr);
  private:
    class PajeComponent;
    class PajeTask;
    class PajeTaskInstance;
    class RegisterMe;

    static RegisterMe registerMe;

    PajeTracer::Container *arch;
  };

  class ComponentPajeTracer::PajeComponent: public OComponent {
  public:
    PajeComponent(ComponentPajeTracer *parent, Component const &c)
      : res_(myPajeTracer->getOrCreateContainer(c.getName().c_str(), parent->arch))
      , power_(myPajeTracer->getOrCreateGauge("power", res_))
      , powerValue(0), powerValueOld(-1)
      {}

    PajeTracer::Container *res_;
    PajeTracer::Gauge     *power_;

    sc_core::sc_time startTime;

    sc_core::sc_time powerTime;
    double           powerValue;
    double           powerValueOld;
  };

  class ComponentPajeTracer::PajeTask: public OTask {
  public:
    PajeTask(std::string const &name, CoSupport::Tracing::PajeTracer::Container *c)
      : task(myPajeTracer->getOrCreateActivity(name.c_str(), c, "action"))
      , guard(myPajeTracer->getOrCreateActivity(name.c_str()
          , CoSupport::String::Color(
                myPajeTracer->getColor(task).r() / 2
              , myPajeTracer->getColor(task).g() / 2
              , myPajeTracer->getColor(task).b() / 2
            )
          , c, "guard"))
      , releaseEvent(myPajeTracer->getOrCreateEvent(
          name.c_str(), myPajeTracer->getColor(task), c, "release"))
      , latencyEvent(myPajeTracer->getOrCreateEvent(
          name.c_str(), myPajeTracer->getColor(task), c, "latency"))
      {}

    CoSupport::Tracing::PajeTracer::Activity *task;
    CoSupport::Tracing::PajeTracer::Activity *guard;
    CoSupport::Tracing::PajeTracer::Event    *releaseEvent;
    CoSupport::Tracing::PajeTracer::Event    *latencyEvent;

    ~PajeTask() {}
  };

  class ComponentPajeTracer::PajeTaskInstance: public OTaskInstance {
  public:
    PajeTaskInstance(PajeTask *pajeTask)
      : pajeTask(pajeTask) {}

    PajeTask *pajeTask;

    ~PajeTaskInstance() {}
  };

  class ComponentPajeTracer::RegisterMe {
  public:
    RegisterMe() {
      ComponentPajeTracer::registerTracer("PAJE",
        [](Attribute::Ptr attr) { return new ComponentPajeTracer(attr); });
    }
  } ComponentPajeTracer::registerMe;

  ComponentPajeTracer::ComponentPajeTracer(Attribute::Ptr attr)
    : Extending::ComponentTracerIf(
          reinterpret_cast<char *>(static_cast<ComponentTracer              *>(this)) -
          reinterpret_cast<char *>(static_cast<Extending::ComponentTracerIf *>(this))
        , sizeof(PajeComponent), sizeof(PajeTask), sizeof(PajeTaskInstance))
    , ComponentTracer(
         reinterpret_cast<char *>(static_cast<Extending::ComponentTracerIf *>(this)) -
         reinterpret_cast<char *>(static_cast<ComponentTracer              *>(this))
       , "PAJE")
  {
    if (!myPajeTracer){
      std::string traceFilename;

      if (char *traceprefix = getenv("VPCTRACEFILEPREFIX"))
        traceFilename = traceprefix;
      traceFilename += "paje.trace";
      myPajeTracer.reset(new CoSupport::Tracing::PajeTracer(traceFilename));
    }

    this->arch = myPajeTracer->getOrCreateContainer("Architecture");
//  myPajeTracer->traceGauge(this->res_, this->power_, sc_core::SC_ZERO_TIME, 0);
  }

  void ComponentPajeTracer::componentOperation(ComponentOperation co
    , Component const &c
    , OComponent      &oc)
  {
    PajeComponent &pajeComponent = static_cast<PajeComponent &>(oc);

    if (ComponentOperation((int) co & (int) ComponentOperation::MEMOP_MASK) ==
        ComponentOperation::ALLOCATE) {
      new (&pajeComponent) PajeComponent(this, c);
    }
    switch (ComponentOperation((int) co & ~ (int) ComponentOperation::MEMOP_MASK)) {
      case ComponentOperation::PWRCHANGE: {
        sc_core::sc_time const &now = sc_core::sc_time_stamp();

        if (now > pajeComponent.powerTime &&
            pajeComponent.powerValue != pajeComponent.powerValueOld) {
            myPajeTracer->traceGauge(pajeComponent.res_, pajeComponent.power_,
                pajeComponent.powerTime, pajeComponent.powerValue);
            pajeComponent.powerValueOld = pajeComponent.powerValue;
        }
        pajeComponent.powerTime  = now;
        pajeComponent.powerValue = c.getPowerConsumption().value();
        break;
      }
      default:
        break;
    }
    if (ComponentOperation((int) co & (int) ComponentOperation::MEMOP_MASK) ==
        ComponentOperation::DEALLOCATE) {
      pajeComponent.~PajeComponent();
    }
  }

  void ComponentPajeTracer::taskOperation(TaskOperation to
    , Component const &c
    , OComponent      &oc
    , Task      const &t
    , OTask           &ot)
  {
    PajeComponent &pajeComponent = static_cast<PajeComponent &>(oc);
    PajeTask      &pajeTask      = static_cast<PajeTask &>(ot);

    if (TaskOperation((int) to & (int) TaskOperation::MEMOP_MASK) ==
        TaskOperation::ALLOCATE) {
      new (&pajeTask) PajeTask(t.getName(), pajeComponent.res_);
    } else
    if (TaskOperation((int) to & (int) TaskOperation::MEMOP_MASK) ==
        TaskOperation::DEALLOCATE) {
      pajeTask.~PajeTask();
    }
  }

  void ComponentPajeTracer::taskInstanceOperation(TaskInstanceOperation tio
    , Component    const &c
    , OComponent         &oc
    , OTask              &ot
    , TaskInstance const &ti
    , OTaskInstance      &oti)
  {
    PajeComponent    &pajeComponent    = static_cast<PajeComponent &>(oc);
    PajeTask         &pajeTask         = static_cast<PajeTask &>(ot);
    PajeTaskInstance &pajeTaskInstance = static_cast<PajeTaskInstance &>(oti);

    if (TaskInstanceOperation((int) tio & (int) TaskInstanceOperation::MEMOP_MASK) ==
        TaskInstanceOperation::ALLOCATE) {
      new (&pajeTaskInstance) PajeTaskInstance(&pajeTask);
    }

    sc_core::sc_time const &now = sc_core::sc_time_stamp();

    if (now > pajeComponent.powerTime &&
        pajeComponent.powerValue != pajeComponent.powerValueOld) {
        myPajeTracer->traceGauge(pajeComponent.res_, pajeComponent.power_,
            pajeComponent.powerTime, pajeComponent.powerValue);
        pajeComponent.powerValueOld = pajeComponent.powerValue;
    }
    pajeComponent.powerTime  = now;
    pajeComponent.powerValue = c.getPowerConsumption().value();

    switch (TaskInstanceOperation((int) tio & ~ (int) TaskInstanceOperation::MEMOP_MASK)) {
      case TaskInstanceOperation::RELEASE:
        myPajeTracer->traceEvent(pajeComponent.res_,
            pajeTask.releaseEvent,
            sc_core::sc_time_stamp());
        break;
      case TaskInstanceOperation::ASSIGN:
        pajeComponent.startTime = sc_core::sc_time_stamp();
        break;
      case TaskInstanceOperation::RESIGN:
        myPajeTracer->traceActivity(pajeComponent.res_
          , ti.getType() == TaskInstance::Type::GUARD
              ? pajeTask.guard
              : pajeTask.task
          , pajeComponent.startTime, sc_core::sc_time_stamp());
        break;
      case TaskInstanceOperation::BLOCK:
        myPajeTracer->traceActivity(pajeComponent.res_
          , ti.getType() == TaskInstance::Type::GUARD
              ? pajeTask.guard
              : pajeTask.task
          , pajeComponent.startTime, sc_core::sc_time_stamp());
        break;
      case TaskInstanceOperation::FINISHDII:
        myPajeTracer->traceActivity(pajeComponent.res_
          , ti.getType() == TaskInstance::Type::GUARD
              ? pajeTask.guard
              : pajeTask.task
          , pajeComponent.startTime, sc_core::sc_time_stamp());
        break;
      case TaskInstanceOperation::FINISHLAT:
        myPajeTracer->traceEvent(pajeComponent.res_,
            pajeTask.latencyEvent,
            sc_core::sc_time_stamp());
        break;
      default:
        break;
    }

    if (TaskInstanceOperation((int) tio & (int) TaskInstanceOperation::MEMOP_MASK) ==
        TaskInstanceOperation::DEALLOCATE) {
      pajeTaskInstance.~PajeTaskInstance();
    }
  }

  bool ComponentPajeTracer::addAttribute(Attribute::Ptr attr) {
    throw ConfigException("The PAJE tracer does not support attributes!");
  }

} } } // namespace SystemC_VPC::Detail::Tracing
