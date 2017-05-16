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

#include <tracing/paje/PajeTracer.hpp>

#include <CoSupport/Tracing/PajeTracer.hpp>
#include <CoSupport/String/DoubleQuotedString.hpp>

#include <iomanip>

#include <memory>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <map>
using namespace std;

namespace {

  std::unique_ptr<CoSupport::Tracing::PajeTracer> myPajeTracer;

}

namespace SystemC_VPC { namespace Trace {

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
//           sc_core::sc_time(10, sc_core::SC_NS), sc_core::sc_time(3, sc_core::SC_US));
//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "release Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::finishDii(Task * task) const {
//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "finishDii Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::finishLatency(Task * task) const {
//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "finishLatency Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::assign(Task * task) {

    std::string taskOverhead = task->getName().append("_overhead");
    std::string timeLeft = task->getName().append("_time_left");
    std::map<string,CoSupport::Tracing::PajeTracer::Activity*>::iterator it_1;
    std::map<string,CoSupport::Tracing::PajeTracer::Activity*>::iterator it_2;

    int done =0;
    if (!(this->my_map_.empty())){
      it_1 = this->my_map_.find(timeLeft);
      if (it_1 != this->my_map_.end()){
        myPajeTracer->traceActivity(this->res_, this->my_map_.find(timeLeft)->second, sc_time_stamp(),sc_time_stamp()+task->getRuntime());
        done =1;
      }

      it_2 = this->my_map_.find(task->getName());
      if (it_2 != this->my_map_.end()){
        myPajeTracer->traceActivity(this->res_, this->my_map_.find(task->getName())->second, sc_time_stamp()+task->getRuntime(),sc_time_stamp()+task->getDelay());
        done =1;
      }
    }

    if (done ==0) {
      this->my_map_[timeLeft] = myPajeTracer->registerActivity(task->getName().c_str());
      myPajeTracer->traceActivity(this->res_, this->my_map_.find(timeLeft)->second,sc_time_stamp(),sc_time_stamp()+task->getRuntime());

      this->my_map_[task->getName()] = myPajeTracer->registerActivity(task->getName().c_str());
      myPajeTracer->traceActivity(this->res_, this->my_map_.find(task->getName())->second,sc_time_stamp()+task->getRuntime(), sc_time_stamp()+task->getDelay());

    }

//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "assign Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::resign(Task * task) const {
//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "resign Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::block(Task * task) const {
//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "block Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();
  }

  Tracing * PajeTracer::getOrCreateTraceSignal(std::string name) {
    Tracing *newsignal = new Tracing(name, this->getName()); // ressource, task

//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "getOrCreateTraceSignal at: " << t1 << "\n";
//    logfile.close();
    return newsignal;
  }

} } // namespace SystemC_VPC::Trace
