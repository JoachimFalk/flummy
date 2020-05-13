// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
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

/* 
   TDMA-Scheduler mit variabler Minislot-Groesze!

   CMX-Format:
   Slotkennzeichen: type="slotxxxxx"
   Funktionszuordnung: value="slotxxxxx"

   z.B.
   <component name="Component1" type="threaded" scheduler="TDMA">
   <attribute type="slot0" value="20ns"/>
   <attribute type="periodic.task1" value="slot0"/>
   <attribute type="slot1" value="20ns"/>
   <attribute type="periodic.task2" value="slot1"/>
   </component>

*/
#include "StreamShaperScheduler.hpp"
#include "PreemptiveComponent.hpp"
#include "../common.hpp"

#include <CoSupport/SystemC/algorithm.hpp>

#include <utility>

namespace SystemC_VPC { namespace Detail {

  StreamShaperScheduler::StreamShaperScheduler()
    : _properties() {
    lastassign=sc_core::SC_ZERO_TIME;
    this->remainingSlice = sc_core::SC_ZERO_TIME;
    firstrun = true;
  }
  
  void StreamShaperScheduler::setProperty(const char* key, const char* value){
    _properties.push_back( std::make_pair(key, value) );
  }
  
  void StreamShaperScheduler::initialize(){
    for(Properties::const_iterator iter = this->_properties.begin();
        iter != this->_properties.end();
        ++iter){
      this->_setProperty(iter->first.c_str(), iter->second.c_str());
    }
#ifdef VPC_DEBUG
    std::cout << "------------ END Initialize ---------"<<std::endl;
#endif //VPC_DEBUG      
    this->_properties.clear();
  }
  
 
  void StreamShaperScheduler::_setProperty(const char* key, const char* value){
    //nothing to do yet
  }

  void StreamShaperScheduler::setAttribute(Attribute const &attribute){
    if(attribute.getType() == "shapePeriod"){
        shapeCycle = createSC_Time(attribute.getValue().c_str());
    }
  }

  
  bool StreamShaperScheduler::getSchedulerTimeSlice( sc_core::sc_time& time,
                                             const TaskMap &ready_tasks,
                                             const TaskMap &running_tasks )
  {
    if(firstrun) return false;

    sc_core::sc_time nextRelTime = lastassign + shapeCycle;
    if(nextRelTime <= sc_core::sc_time_stamp()){
        return false;
    }
    time = nextRelTime - sc_core::sc_time_stamp();
    return true;
  }
  
  
  void StreamShaperScheduler::addedNewTask(TaskInstanceImpl *task){
    stream_fifo.push_back(task->getInstanceId());
  }
  
  
  void StreamShaperScheduler::removedTask(TaskInstanceImpl *task){
    std::deque<int>::iterator iter;
       for(iter=stream_fifo.begin();iter!=stream_fifo.end();iter++){
         if( *iter == task->getInstanceId()){
             stream_fifo.erase(iter);
             break;
         }
       }
  }
  
  
  scheduling_decision
  StreamShaperScheduler::schedulingDecision(int& task_to_resign,
                                    int& task_to_assign,
                                    const TaskMap &ready_tasks,
                                    const TaskMap &running_tasks )
  {
    scheduling_decision ret = NOCHANGE;
    if(firstrun || sc_core::sc_time_stamp() >= lastassign + shapeCycle){
        firstrun=false;
        if(running_tasks.size()==0){
           if(stream_fifo.size()>0){
              task_to_assign = stream_fifo.front();
              stream_fifo.pop_front();
              lastassign = sc_core::sc_time_stamp();
              ret = ONLY_ASSIGN;
            }
          }
    }else{
    }
    return ret;
  }


  /**
   *
   */
  sc_core::sc_time* StreamShaperScheduler::schedulingOverhead(){
    return NULL; //new sc_core::sc_time(1,sc_core::SC_NS);
  }

} } // namespace SystemC_VPC::Detail
