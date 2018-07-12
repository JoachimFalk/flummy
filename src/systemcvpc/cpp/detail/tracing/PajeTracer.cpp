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

#include "TracerIf.hpp"

#include <CoSupport/String/color.hpp>
#include <CoSupport/String/DoubleQuotedString.hpp>
#include <CoSupport/Tracing/PajeTracer.hpp>

#include <iomanip>
#include <memory>
#include <boost/algorithm/string.hpp>

namespace {

  std::unique_ptr<CoSupport::Tracing::PajeTracer> myPajeTracer;

}

namespace SystemC_VPC {

  const char *Component::Tracer::PAJE = "PAJE";

} // namespace SystemC_VPC

namespace SystemC_VPC { namespace Detail { namespace Tracing {

  class PajeTracer: public TracerIf {
  public:
    PajeTracer(Component const *component);

    TTask         *registerTask(std::string const &name);

    TTaskInstance *release(TTask *ttask);

    void           assign(TTaskInstance *ttaskInstance);

    void           resign(TTaskInstance *ttaskInstance);

    void           block(TTaskInstance *ttaskInstance);

    void           finishDii(TTaskInstance *ttaskInstance);

    void           finishLatency(TTaskInstance *ttaskInstance);

    ~PajeTracer();
  private:
    class PajeTask;
    class PajeTaskInstance;
    class RegisterMe;

    static RegisterMe registerMe;

    std::string  name_;

    CoSupport::Tracing::PajeTracer::Resource const *res_;
    sc_core::sc_time startTime;
  };

  class PajeTracer::PajeTask: public TTask {
  public:
    PajeTask(std::string const &name)
      : task(myPajeTracer->registerActivity(name.c_str(),true))
      , releaseEvent(myPajeTracer->registerEvent((name+" released").c_str(),true))
      , latencyEvent(myPajeTracer->registerEvent((name+" latency").c_str(),true))
      {}

    CoSupport::Tracing::PajeTracer::Activity *task;
    CoSupport::Tracing::PajeTracer::Event    *releaseEvent;
    CoSupport::Tracing::PajeTracer::Event    *latencyEvent;

    ~PajeTask() {}
  };

  class PajeTracer::PajeTaskInstance: public TTaskInstance {
  public:
    PajeTaskInstance(PajeTask *pajeTask)
      : pajeTask(pajeTask) {}

    PajeTask *pajeTask;

    ~PajeTaskInstance() {}
  };

  class PajeTracer::RegisterMe {
  public:
    RegisterMe() {
      PajeTracer::registerTracer("PAJE",
        [](Component const *comp) { return new PajeTracer(comp); });
    }
  } PajeTracer::registerMe;

  PajeTracer::PajeTracer(Component const *component)
      : name_(component->getName()) {
    if (!myPajeTracer){
      myPajeTracer.reset(new CoSupport::Tracing::PajeTracer("paje.trace"));
    }
    this->res_ = myPajeTracer->registerResource(component->getName().c_str());
  }

  PajeTracer::~PajeTracer() {

  }

  TTask         *PajeTracer::registerTask(std::string const &name) {
    return new PajeTask(name);
  }

  TTaskInstance *PajeTracer::release(TTask *ttask) {
    PajeTaskInstance *ttaskInstance = new PajeTaskInstance(static_cast<PajeTask *>(ttask));
    myPajeTracer->traceEvent(this->res_,
        ttaskInstance->pajeTask->releaseEvent,
        sc_core::sc_time_stamp());
    return  ttaskInstance;
  }

  void           PajeTracer::assign(TTaskInstance *ttaskInstance) {
    this->startTime = sc_core::sc_time_stamp();
  }

  void           PajeTracer::resign(TTaskInstance *ttaskInstance) {
    myPajeTracer->traceActivity(this->res_,
        static_cast<PajeTaskInstance *>(ttaskInstance)->pajeTask->task,
        this->startTime, sc_core::sc_time_stamp());
  }

  void           PajeTracer::block(TTaskInstance *ttaskInstance) {
    myPajeTracer->traceActivity(this->res_,
        static_cast<PajeTaskInstance *>(ttaskInstance)->pajeTask->task,
        this->startTime, sc_core::sc_time_stamp());
  }

  void           PajeTracer::finishDii(TTaskInstance *ttaskInstance) {
    myPajeTracer->traceActivity(this->res_,
        static_cast<PajeTaskInstance *>(ttaskInstance)->pajeTask->task,
        this->startTime, sc_core::sc_time_stamp());
  }

  void           PajeTracer::finishLatency(TTaskInstance *ttaskInstance) {
    myPajeTracer->traceEvent(this->res_,
        static_cast<PajeTaskInstance *>(ttaskInstance)->pajeTask->latencyEvent,
        sc_core::sc_time_stamp());
  }

} } } // namespace SystemC_VPC::Detail::Tracing
