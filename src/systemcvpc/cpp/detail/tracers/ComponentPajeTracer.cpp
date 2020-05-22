// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
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

#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/ComponentTracer.hpp>
#include <systemcvpc/Extending/ComponentTracerIf.hpp>

#include <CoSupport/String/color.hpp>
#include <CoSupport/String/DoubleQuotedString.hpp>
#include <CoSupport/Tracing/PajeTracer.hpp>

#include <iomanip>
#include <memory>
#include <boost/algorithm/string.hpp>

//using CoSupport::Tracing::PajeTracer;

//namespace {
//
//  std::unique_ptr<PajeTracer> myPajeTracer;
//
//}

namespace SystemC_VPC {

  const char *Component::Tracer::PAJE = "PAJE";

} // namespace SystemC_VPC

namespace SystemC_VPC { namespace Detail { namespace Tracers {

  class ComponentPajeTracerBase {
  protected:
    ComponentPajeTracerBase(Attributes const &attrs);

    std::string const &getTraceFileName() const
      { return traceFileName; }

  private:
    std::string traceFileName;
  };

  ComponentPajeTracerBase::ComponentPajeTracerBase(Attributes const &attrs)
  {
    bool haveTraceFileName = false;

    for (Attribute const &attr : attrs) {
      if (attr.getType() == "traceFileName") {
        if (haveTraceFileName) {
          throw ConfigException(
              "Duplicate attribute traceFileName in PAJE component tracer!");
        }
        traceFileName = attr.getValue();
        haveTraceFileName = true;
      } else {
        std::stringstream msg;

        msg << "Component PAJE tracers do not support the "+attr.getType()+" attribute! ";
        msg << "Only the traceFileName attribute is supported.";
        throw ConfigException(msg.str());
      }
    }
    if (!haveTraceFileName) {
      if (char *traceprefix = getenv("VPCTRACEFILEPREFIX"))
        traceFileName = traceprefix;
      traceFileName += "PAJE.paje";
    }
  }

  class ComponentPajeTracer
    : public Extending::ComponentTracerIf
    , public ComponentTracer
    , public ComponentPajeTracerBase
    , public CoSupport::Tracing::PajeTracer
  {
  public:
    ComponentPajeTracer(Attributes const &attrs);

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
      : res_(parent->getOrCreateContainer(c.getName().c_str(), parent->arch))
      , power_(parent->getOrCreateGauge("power", res_))
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
    PajeTask(ComponentPajeTracer *parent
      , std::string const &name
      , CoSupport::Tracing::PajeTracer::Container *c
    ) : task(parent->getOrCreateActivity(name.c_str(), c, "action"))
      , guard(parent->getOrCreateActivity(name.c_str()
          , CoSupport::String::Color(
                parent->getColor(task).r() / 2
              , parent->getColor(task).g() / 2
              , parent->getColor(task).b() / 2
            )
          , c, "guard"))
      , releaseEvent(parent->getOrCreateEvent(
          name.c_str(), parent->getColor(task), c, "release"))
      , latencyEvent(parent->getOrCreateEvent(
          name.c_str(), parent->getColor(task), c, "latency"))
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
        [](Attributes const &attrs) { return new ComponentPajeTracer(attrs); });
    }
  } ComponentPajeTracer::registerMe;

  ComponentPajeTracer::ComponentPajeTracer(Attributes const &attrs)
    : Extending::ComponentTracerIf(
          reinterpret_cast<char *>(static_cast<ComponentTracer              *>(this)) -
          reinterpret_cast<char *>(static_cast<Extending::ComponentTracerIf *>(this))
        , sizeof(PajeComponent), sizeof(PajeTask), sizeof(PajeTaskInstance))
    , ComponentTracer(
         reinterpret_cast<char *>(static_cast<Extending::ComponentTracerIf *>(this)) -
         reinterpret_cast<char *>(static_cast<ComponentTracer              *>(this))
       , "PAJE")
    , ComponentPajeTracerBase(attrs)
    , PajeTracer(getTraceFileName())
    , arch(getOrCreateContainer("Architecture"))
  {}

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
            traceGauge(pajeComponent.res_, pajeComponent.power_,
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
      new (&pajeTask) PajeTask(this, t.getName(), pajeComponent.res_);
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
        traceGauge(pajeComponent.res_, pajeComponent.power_,
            pajeComponent.powerTime, pajeComponent.powerValue);
        pajeComponent.powerValueOld = pajeComponent.powerValue;
    }
    pajeComponent.powerTime  = now;
    pajeComponent.powerValue = c.getPowerConsumption().value();

    switch (TaskInstanceOperation((int) tio & ~ (int) TaskInstanceOperation::MEMOP_MASK)) {
      case TaskInstanceOperation::RELEASE:
        traceEvent(pajeComponent.res_,
            pajeTask.releaseEvent,
            sc_core::sc_time_stamp());
        break;
      case TaskInstanceOperation::ASSIGN:
        pajeComponent.startTime = sc_core::sc_time_stamp();
        break;
      case TaskInstanceOperation::RESIGN:
        traceActivity(pajeComponent.res_
          , ti.getType() == TaskInstance::Type::GUARD
              ? pajeTask.guard
              : pajeTask.task
          , pajeComponent.startTime, sc_core::sc_time_stamp());
        break;
      case TaskInstanceOperation::BLOCK:
        traceActivity(pajeComponent.res_
          , ti.getType() == TaskInstance::Type::GUARD
              ? pajeTask.guard
              : pajeTask.task
          , pajeComponent.startTime, sc_core::sc_time_stamp());
        break;
      case TaskInstanceOperation::FINISHDII:
        traceActivity(pajeComponent.res_
          , ti.getType() == TaskInstance::Type::GUARD
              ? pajeTask.guard
              : pajeTask.task
          , pajeComponent.startTime, sc_core::sc_time_stamp());
        break;
      case TaskInstanceOperation::FINISHLAT:
        traceEvent(pajeComponent.res_,
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

  bool ComponentPajeTracer::addAttribute(Attribute const &attr) {
    throw ConfigException("The PAJE tracer does not support attributes!");
  }

} } } // namespace SystemC_VPC::Detail::Tracers
