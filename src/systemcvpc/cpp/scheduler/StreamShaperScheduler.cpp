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
#include <systemcvpc/Director.hpp>
#include "ComponentImpl.hpp"
#include "StreamShaperScheduler.hpp"

#include <CoSupport/SystemC/algorithm.hpp>

#include <utility>

namespace SystemC_VPC{

  StreamShaperScheduler::StreamShaperScheduler()
    : _properties() {
    lastassign=SC_ZERO_TIME;
    this->remainingSlice = SC_ZERO_TIME;
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
    cout << "------------ END Initialize ---------"<<endl;
#endif //VPC_DEBUG      
    this->_properties.clear();
  }
  
 
  void StreamShaperScheduler::_setProperty(const char* key, const char* value){
    //nothing to do yet
  }

  void StreamShaperScheduler::setAttribute(AttributePtr attributePtr){
    std::string value = attributePtr->getType();
    if(value=="shapePeriod"){
        shapeCycle = Director::createSC_Time(attributePtr->getValue());
    }
  }

  
  bool StreamShaperScheduler::getSchedulerTimeSlice( sc_time& time,
                                             const TaskMap &ready_tasks,
                                             const TaskMap &running_tasks )
  {
    if(firstrun) return false;

    sc_time nextRelTime = lastassign + shapeCycle;
    if(nextRelTime <= sc_time_stamp()){
        return false;
    }
    time = nextRelTime - sc_time_stamp();
    return true;
  }
  
  
  void StreamShaperScheduler::addedNewTask(Task *task){
    stream_fifo.push_back(task->getInstanceId());
  }
  
  
  void StreamShaperScheduler::removedTask(Task *task){
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
    if(firstrun || sc_time_stamp() >= lastassign + shapeCycle){
        firstrun=false;
        if(running_tasks.size()==0){
           if(stream_fifo.size()>0){
              task_to_assign = stream_fifo.front();
              stream_fifo.pop_front();
              lastassign = sc_time_stamp();
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
  sc_time* StreamShaperScheduler::schedulingOverhead(){
    return NULL; //new sc_time(1,SC_NS);
  }
}
