/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
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

#include <src/systemcvpc/cpp/tracing/paje/PajeTracer.hpp>

#include <CoSupport/Tracing/PajeTracer.hpp>
#include <CoSupport/String/DoubleQuotedString.hpp>

#include <iomanip>

#include <memory>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
using namespace std;

namespace {

  std::unique_ptr<CoSupport::Tracing::PajeTracer> myPajeTracer;

}

namespace SystemC_VPC { namespace Trace {


  struct PajeTracer::TaskAct {
    std::string                               name;
    CoSupport::Tracing::PajeTracer::Activity  *activity;
  };

  PajeTracer::PajeTracer(Config::Component::Ptr component)
      : traceFile_(NULL)
      , name_(component->getName()) {
    if (!myPajeTracer)
      myPajeTracer.reset(new CoSupport::Tracing::PajeTracer("paje.trace"));
    this->res_ = myPajeTracer->registerResource(component->getName().c_str());
  }

  PajeTracer::~PajeTracer() {

  }

  std::string PajeTracer::getName() const {
    return name_;
  }

  void PajeTracer::release(Task * task) {
    task->getTraceSignal()->traceReady();
    task->traceReleaseTask();

//           sc_core::sc_time(10, sc_core::SC_NS), sc_core::sc_time(3, sc_core::SC_US));
//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "release Task " << task->getName() << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::finishDii(Task * task) const {
    task->getTraceSignal()->traceSleeping();

//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "finishDii Task " << task->getName().c_str() << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::finishLatency(Task * task) const {
    task->traceFinishTaskLatency();

//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "finishLatency Task " << task->getName() << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::assign(Task * task) {
    task->getTraceSignal()->traceRunning();

    int done =0;
     if (!(this->taskActList.empty())){
       for (TaskAct pair: this->taskActList) {
        if (pair.name == task->getName()){
          myPajeTracer->traceActivity(this->res_, pair.activity, sc_time_stamp(),sc_time_stamp()+task->getLatency());
          done =1;
        }
       }
     }


     if (done ==0) {
       this->taskActList.push_back(TaskAct());
       TaskAct &newPair = this->taskActList.back();
       newPair.name = task->getName();
       newPair.activity = myPajeTracer->registerActivity(task->getName().c_str());

       myPajeTracer->traceActivity(this->res_,newPair.activity,sc_time_stamp(),sc_time_stamp()+task->getLatency());
     }

//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "assign Task " << task->getName() << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::resign(Task * task) const {
    task->getTraceSignal()->traceReady();

//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "resign Task " << task->getName() << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::block(Task * task) const {
    task->getTraceSignal()->traceBlocking();

//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "block Task " << task->getName() << " at: " << t1 << "\n";
//    logfile.close();
  }

  Tracing * PajeTracer::getOrCreateTraceSignal(std::string name) {
    if (this->traceFile_ == NULL) {
      std::string tracefilename = this->getName(); //componentName;

      char* traceprefix = getenv("VPCTRACEFILEPREFIX");
      if (0 != traceprefix) {
        tracefilename.insert(0, traceprefix);
      }

      this->traceFile_ = sc_create_vcd_trace_file(tracefilename.c_str());
      this->traceFile_->set_time_unit(1, SC_NS);
    }
    Tracing *newsignal = new Tracing(name, this->getName()); // ressource, task

    this->trace_map_by_name_.insert(
        std::pair<std::string, Tracing*>(this->getName(), newsignal));
    sc_trace(this->traceFile_, *newsignal->traceSignal, name);
    newsignal->traceSleeping();

//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "getOrCreateTraceSignal at: " << t1 << "\n";
//    logfile.close();
    return newsignal;
  }

} } // namespace SystemC_VPC::Trace
