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
      : keyCounter(0)
      , name_(component->getName()) {
    if (!myPajeTracer){
      myPajeTracer.reset(new CoSupport::Tracing::PajeTracer("paje.trace"));
    }
    this->res_ = myPajeTracer->registerResource(component->getName().c_str());
  }

  PajeTracer::~PajeTracer() {

  }

  std::string PajeTracer::getName() const {
    return name_;
  }

  void PajeTracer::release(Task * task) {
//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "release Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::finishDii(Task * task) {
//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "finishDii Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();

    TaskToActivity::const_iterator iterAction = taskToActivity.find(task->getName());
    assert(iterAction != taskToActivity.end());
    myPajeTracer->traceActivity(this->res_, iterAction->second, this->startTime, sc_time_stamp()); //

  }

  void PajeTracer::finishLatency(Task * task) {

    taskToEndTime[task->getName()] = sc_time_stamp();
    taskToResource[task->getName()] = this->res_;
//    if(!task->destState.empty())
//      taskToPreTask[task->destState] = task->getName();
//
//    TaskToPreTask::const_iterator iterTask = taskToPreTask.find(task->getName());
//    if (iterTask != taskToPreTask.end())
//      //    std::string link = task->getName().append("_to_").append(task->getDestState());
//      std::string link_ = taskToPreTask[task->getName()].append("_to_" + task->getName());
//      myPajeTracer->registerLink(link_.c_str());
//      int key = getNextKey();
//
//      TaskToEndTime::const_iterator iterPre = taskToEndTime.find(task->getPreState().c_str());
//      assert(iterPre == taskToEndTime.end());
//
//      myPajeTracer->traceLinkBegin(link_.c_str(), taskToResource[task->getPreState()], key, taskToEndTime[task->getPreState()]);
//      myPajeTracer->traceLinkEnd(link_.c_str(), this->res_, key, sc_time_stamp());
//
//    }


//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "finishLatency Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::assign(Task * task) {

    this->startTime = sc_time_stamp();

    TaskToActivity::const_iterator iterActivity = taskToActivity.find(task->getName());
    if (iterActivity == taskToActivity.end()) {
      taskToActivity[task->getName()] = myPajeTracer->registerActivity(task->getName().c_str(),true);
    }

//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "assign Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::resign(Task * task) {

    this->startTime = sc_time_stamp();

    std::string event = task->getName().append(" resigned!");

    TaskToEvent::const_iterator iterEvent = taskToEvent.find(event);
    if (iterEvent == taskToEvent.end())
      taskToEvent[event] = myPajeTracer->registerEvent(task->getName().c_str(),true);

    myPajeTracer->traceEvent(this->res_, taskToEvent.find(event)->second, sc_time_stamp());
//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "resign Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();
  }

  void PajeTracer::block(Task * task) {

    std::string event = task->getName().append(" blocked!");

    TaskToEvent::const_iterator iterEvent = taskToEvent.find(event);
    if (iterEvent == taskToEvent.end())
      taskToEvent[event] = myPajeTracer->registerEvent(task->getName().c_str(),true);

    myPajeTracer->traceEvent(this->res_, taskToEvent.find(event)->second, sc_time_stamp());

    myPajeTracer->traceActivity(this->res_, taskToActivity.find(task->getName())->second, this->startTime, sc_time_stamp());

//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "block Task " << task->getName() << " "<< task->pid << " on " << this->res_ << " at: " << t1 << "\n";
//    logfile.close();
  }

  Tracing * PajeTracer::getOrCreateTraceSignal(std::string name) {
    Tracing *newsignal = new Tracing(name, this->getName()); // resource, task

    // not relevant for paje tracese, as all components are displayed in one trace.

//    ofstream logfile;
//    logfile.open("logfile.txt", std::ios_base::app);
//    sc_time t1 = sc_time_stamp();
//    logfile << "getOrCreateTraceSignal at: " << t1 << "\n";
//    logfile.close();
    return newsignal;
  }

  int PajeTracer::getNextKey() {
    return (keyCounter++);
  }


} } // namespace SystemC_VPC::Trace
