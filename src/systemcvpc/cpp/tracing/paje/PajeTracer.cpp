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

    if(!task->hasScheduledTask()){
      string name = task->getName();
      string msg_name;
      int msg_cf = name.find("msg_cf_");
      int begin = name.find("_cf_");
      int end = name.find("_1_");
      if (msg_cf == -1)
        msg_name = name.substr(begin+4, name.length()-begin-6);
      else
        msg_name = name.substr(7, end-7);
      int n = msg_name.find("_");
      string from = msg_name.substr(0,n);
      string to = msg_name.substr(n+1,msg_name.length()-n-1);

      TaskToPreTask::const_iterator iterTask = taskToPreTask.find(to);

      if(iterTask == taskToPreTask.end()) {
        taskToPreTask[to] = from;
        taskToDestTask[from] = to;
        myPajeTracer->registerLink(msg_name.c_str());
      } //else if(taskToPreTask[to] != from) {
//        taskToPreTask[to] = from;
//        myPajeTracer->registerLink(msg_name.c_str());
      //}
    }

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

//    taskToEndTime[task->getName()] = sc_time_stamp();
//    taskToResource[task->getName()] = this->res_;

    TaskToDestTask::const_iterator iterDestTask = taskToDestTask.find(task->getName());
    if (iterDestTask != taskToDestTask.end()){
      std::string destTask = iterDestTask->second;
      std::string link_ = task->getName().append("_" + destTask);

      myPajeTracer->traceLinkBegin(link_.c_str(), this->res_, sc_time_stamp());
    }

    TaskToPreTask::const_iterator iterTask = taskToPreTask.find(task->getName());
    if (iterTask != taskToPreTask.end()){
      std::string preTask = iterTask->second;
      std::string link_ = preTask.append("_" + task->getName());

//      TaskToEndTime::const_iterator iterPre = taskToEndTime.find(preTask.c_str());
//      assert(iterPre == taskToEndTime.end());

      myPajeTracer->traceLinkEnd(link_.c_str(), this->res_, sc_time_stamp());

    }


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
