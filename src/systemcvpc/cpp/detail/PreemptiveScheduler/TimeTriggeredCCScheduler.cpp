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

/* 
        TimeTriggeredCC-Scheduler
        
        CMX-Format:
        
        z.B.
        <component name="Component1" type="threaded" scheduler="TTCC">
          <attribute type="FlexRayParams">
            <parameter type="dualchannel" value="false"/>
            <attribute type="static">
              <attribute type="slot0" value="120ns">
                <!-- empty slot -->
              </attribute>
              <attribute type="slot1" value="20ns">
                <attribute type="mapping" value="periodic.task1">
                  <parameter type="multiplex" value="0"/>
                  <parameter type="offset" value="1"/>
                </attribute>
              </attribute>
              <attribute type="slot2" value="120ns">
                <!-- empty slot -->
              </attribute>
            </attribute>
          </attribute>
        </component>
Sebastian Graf - Mai 2008

*/

#include "TimeTriggeredCCScheduler.hpp"
#include "PreemptiveComponent.hpp"
#include "../common.hpp"


namespace SystemC_VPC { namespace Detail {

  TimeTriggeredCCScheduler::TimeTriggeredCCScheduler()
    : _properties() {
    //Standardmaessig beide Kanaele aktivieren!
    dualchannel=1; 
    //alt (TDMA)
    processcount=0;
    cyclecount=0;
    lastassign=sc_core::sc_time(0,sc_core::SC_NS);
    this->remainingSlice = sc_core::sc_time(0,sc_core::SC_NS);
    slicecount=0;
    curr_slicecount=-1; 
  }
  
  void TimeTriggeredCCScheduler::setProperty(const char* key, const char* value){
    std::pair<std::string, std::string> toadd(key,value);
    _properties.push_back(toadd);
  }
  
  void TimeTriggeredCCScheduler::initialize(){   
    for(std::deque< std::pair <std::string, std::string> >::const_iterator iter = this->_properties.begin();
        iter != this->_properties.end();
        ++iter){
      this->_setProperty(iter->first.c_str(), iter->second.c_str());
    }
#ifdef VPC_DEBUG
    std::cout << "------------ END Initialize ---------"<<std::endl;
#endif //VPC_DEBUG      
    this->_properties.clear();
  }
  
 
  void TimeTriggeredCCScheduler::_setProperty(const char* key, const char* value){
    int i=-1;
    //schon Slots geadded? oder cmx-Syntax/Reihenfolge falsch?
    assert(0<TDMA_slots.size());
    //Betreffende SlotID in der Slotliste suchen
    do{
      //nichts zu tun.. da lediglich durchiteriert wird!
    }while(TDMA_slots[++i].name != value && (i+1)<(int)TDMA_slots.size());
    //std::cout<<"found at i= " <<i<<std::endl;
        
    if(TDMA_slots[i].name != value){
      i=-1;
      assert(0<Dynamic_slots.size());
      //std::cout<< "DynSize: " << Dynamic_slots.size()<<std::endl;
      do{
        //nichts zu tun.. da lediglich durchiteriert wird!
      }while(Dynamic_slots[++i].name != value && (i+1)<(int)Dynamic_slots.size());
      //auch wirklich etwas passendes gefunden?
      assert(i<(int)Dynamic_slots.size());      
      i+=StartslotDynamic;      
    }
    //std::cout<<"PId-Map "<<i<< " to "<< Director::getInstance().getProcessId(key) <<std::endl;
    //Beziehung PId - SlotID herstellen
    ProcessId id=Director::getInstance().getProcessId(key);
    PIDmap[id]=i;   
    ProcessParams[id]=ProcessParams_string[key];
    //      std::cout<<"registered function: "<< key<<" with ID: "<<id<<std::endl;
    //  std::cout<<"add Function " <<  key << " to " << value<<std::endl; 
  }

  void TimeTriggeredCCScheduler::setAttribute(AttributePtr attributePtr){
    std::string value = attributePtr->getType();

    if( value!="FlexRayParams" )
      return;

    if(attributePtr->getParameterSize()!=0){
      //es gibt folglich globale FlexRay-Parameter!
      for(size_t j=0;j<attributePtr->getParameterSize();j++){
        std::pair<std::string, std::string > param2 =attributePtr->getNextParameter(j);
        if(param2.first == "dualchannel")
          dualchannel=(param2.second == "true");
      }
    }

    for(size_t i=0;i<attributePtr->getAttributeSize();i++){
      std::pair<std::string, AttributePtr >attribute=attributePtr->getNextAttribute(i);
        
      if(attribute.first=="static"){
        StartslotDynamic=0;
        for(size_t k=0;k<attribute.second->getAttributeSize();k++){
          std::pair<std::string, AttributePtr >attribute2=attribute.second->getNextAttribute(k);
          //Slot einrichten
          StartslotDynamic++;
          slicecount++;
          std::pair<std::string, std::string > param;
          param.first=attribute2.second->getType();
          param.second=attribute2.second->getValue();

          //                         std::cout<<"found static Slot: "<<param.first <<" with value: "<<param.second<<std::endl;
          TDMASlot newSlot;
          //Werte aus dem Attribute auslesen und damit neuen Slot erzeugen
          newSlot.length = createSC_Time(param.second.c_str() );
          newSlot.name = param.first;
          TDMA_slots.insert(TDMA_slots.end(), newSlot);

          //jetzt noch die Task-mappings!
          //für jeden Attribute-Eintrag Parameter verarbeiten
          for(size_t l=0;l<attribute2.second->getAttributeSize();l++){
            std::pair<std::string, AttributePtr >attribute3=attribute2.second->getNextAttribute(l);
            std::pair<std::string, std::string > param3;
            if(attribute3.first=="mapping"){

              param3.first=attribute3.second->getValue();
              param3.second=param.first;
              //                    std::cout<<"found static binding: "<<param3.second <<" with value: "<<param3.first<<std::endl;
                            
              this->_properties.push_back(param3);
              ProcessParams_string[param3.first]=SlotParameters(0,0);
              if(attribute3.second->getParameterSize()==0){
                //we don't have further Parameters, so let them as they are
              }else{
                //parse parameters
                for(size_t m=0;m<attribute3.second->getParameterSize();m++){
                  std::pair<std::string, std::string > param4 =attribute3.second->getNextParameter(m);
                  if(param4.first=="offset"){
                    ProcessParams_string[param3.first].offset=atoi(param4.second.c_str());
                    //                                  std::cout<<"found Offset-Setting for "<<param3.first<<" with value: "<<param4.second<<std::endl;
                  }
                  if(param4.first=="multiplex"){
                    ProcessParams_string[param3.first].multiplex=atoi(param4.second.c_str());
                    //                                   std::cout<<"found Multiplex-Setting for "<<param3.first<<" with value: "<<param4.second<<std::endl;
                  }
                }
              }
            }
          }
        }
      } 
    }
  }
  
  bool TimeTriggeredCCScheduler::getSchedulerTimeSlice( sc_core::sc_time& time,
                                                        const TaskMap &ready_tasks,
                                                        const  TaskMap &running_tasks )
  {   
    // keine wartenden + keine aktiven Threads -> ende!
    //   std::cout<<"getSchedulerTimeSlice "<< processcount<<" "<<running_tasks.size()<<std::endl;
    if(processcount==0 && running_tasks.size()==0) return 0;   
    //ansonsten: Restlaufzeit der Zeitscheibe
    if(curr_slicecount<StartslotDynamic){ //statisch
      if(curr_slicecount == -1){
        time=sc_core::sc_time(0,sc_core::SC_NS);
      }else{
        //      time=TDMA_slots[curr_slicecount].length -(sc_core::sc_time_stamp() - this->lastassign);  
        time=this->remainingSlice;
        //              std::cout<<"static-timeSlice-request "<< curr_slicecount << "  " << sc_core::sc_time_stamp() << "  " << time  << "running_tasks " << running_tasks.size() <<std::endl;
      }
    }else{
      std::cout<<"getSchedulerTimeSlice: Dynamic not implemented in TimeTriggered-CommunicationController"<<std::endl;
    }
    return true;   
  }
  
  
  void TimeTriggeredCCScheduler::addedNewTask(TaskInstance *task){    
    int index = PIDmap[task->getProcessId()];
    //       std::cout<<"addedNewTask- index: "<<index<<" PID: "<<task->getProcessId()<<" instanceID: "<<task->getInstanceId()<<std::endl;
    if(index<StartslotDynamic){
      //TDMA-Task
      TDMA_slots[ index ].pid_fifo.push_back(task->getInstanceId());
      ProcessParams[task->getInstanceId()]=ProcessParams[task->getProcessId()];
#ifdef VPC_DEBUG     
      std::cout<<"added Process " <<  task->getInstanceId() << " to Slot " << PIDmap[task->getProcessId()]  <<std::endl;
#endif //VPC_DEBUG
    
      //std::cout << "added static Task" <<std::endl;
    }else{
      std::cout<<"addedNewTask: Dynamic not implemented in TimeTriggered-CommunicationController"<<std::endl;
    }
    processcount++;
  }
  
  void TimeTriggeredCCScheduler::removedTask(TaskInstance *task){ 
    int index = PIDmap[task->getProcessId()];
    
    // std::cout<<"Task entfernt! @ "<< sc_core::sc_time_stamp() << "  " << index << std::endl;
      
    std::deque<int>::iterator iter;
    if(index<StartslotDynamic){
      for(iter = TDMA_slots[ index ].pid_fifo.begin(); iter!=TDMA_slots[index].pid_fifo.end() ;iter++){
        if( *iter == task->getInstanceId()){
          TDMA_slots[index].pid_fifo.erase(iter);
          break;
        }
      }
    }else{
      std::cout<<"removedTask: Dynamic not implemented in TimeTriggered-CommunicationController"<<std::endl;
    }
#ifdef VPC_DEBUG    
    std::cout<<"removed Task: " << task->getInstanceId()<<std::endl;
#endif //VPC_DEBUG   
    processcount--;  
  }
  
  
  // Eigentlicher Scheduler
  scheduling_decision TimeTriggeredCCScheduler::schedulingDecision(
                                                                   int& task_to_resign,
                                                                   int& task_to_assign,
                                                                   const  TaskMap &ready_tasks,
                                                                   const  TaskMap &running_tasks )
  {
    scheduling_decision ret_decision = NOCHANGE;;
    
    //statischer oder dynamischer Teil?
    if(curr_slicecount+1<StartslotDynamic){
      //      std::cout<<"Static! @ "<< sc_core::sc_time_stamp() << " curr slice: " << curr_slicecount+1 <<" cycle: "<< cyclecount<< std::endl;
      //TDMA-Scheduling: unveraendert aus TDMAScheduler verwendet.
      ret_decision=NOCHANGE;
      //Zeitscheibe abgelaufen?
      if(this->remainingSlice < (sc_core::sc_time_stamp() - this->lastassign)) this->remainingSlice=sc_core::SC_ZERO_TIME;
      else{
        this->remainingSlice = this->remainingSlice - (sc_core::sc_time_stamp() - this->lastassign);  
      }
      this->lastassign = sc_core::sc_time_stamp();
    
      if(this->remainingSlice <= sc_core::sc_time(0,sc_core::SC_NS)){//Zeitscheibe wirklich abgelaufen!
        curr_slicecount++; // Wechsel auf die naechste Zeitscheibe noetig!
        //neue Timeslice laden
        this->remainingSlice = TDMA_slots[curr_slicecount].length;

        if(TDMA_slots[curr_slicecount].pid_fifo.size()>0){    // neuer Task da?
          unsigned int tempcount=0;
          bool found=false;
          //if not.. try the next one (if existing)
          while(!found && tempcount<TDMA_slots[curr_slicecount].pid_fifo.size()){
            task_to_assign = TDMA_slots[curr_slicecount].pid_fifo[tempcount];
            //is this task allowed to run in this slot?
            //             std::cout<<"testing "<<tempcount<<" @ "<<curr_slicecount<<" bei "<<task_to_assign<<" von gesamt: "<<TDMA_slots[curr_slicecount].pid_fifo.size()<<std::endl;
            if(ProcessParams[task_to_assign].offset<=cyclecount){
              //          std::cout<<"found task_to_assign: "<<task_to_assign<<std::endl;
              int mux_value = 1 << ProcessParams[task_to_assign].multiplex;
              if(ProcessParams[task_to_assign].multiplex==0 || (cyclecount % mux_value)==ProcessParams[task_to_assign].offset){
                /* std::cout<<"Abfrage: mux= "<< ProcessParams[task_to_assign].multiplex << std::endl;
                   std::cout<<" count= "<<cyclecount <<" 2^ = "<< mux_value<<std::endl;
                   std::cout<< " MOD= " << cyclecount % mux_value <<std::endl;
                */
                found=true;
              }
            }
            tempcount++;
          }
        
          //alter wurde schon entfernt (freiwillige abgabe "BLOCK")
          // -> kein preemption!
          //         std::cout<<"here we are"<<std::endl;
          ret_decision= ONLY_ASSIGN;
        
          if(!found){ //keinen lauffaehigen gefunden! -> idle werden
            task_to_assign=0;
            if(running_tasks.size()!=0){  // alten Task entfernen, wenn noetig
              TaskMap::const_iterator iter;
              iter=running_tasks.begin();
              TaskInstance *task=iter->second;

              task_to_resign=task->getInstanceId();
              ret_decision=RESIGNED;
            }else{
              //war keiner da... und ist auch kein Neuer da -> keine Aenderung  
              ret_decision=NOCHANGE;
            }  
          }else{
        
            if(running_tasks.size()!=0){  // alten Task entfernen
              TaskMap::const_iterator iter;
              iter=running_tasks.begin();
              TaskInstance *task=iter->second;
              task_to_resign=task->getInstanceId();
              ret_decision= PREEMPT;  
            }
          }
          // else{}

          //kein laufender Task (wurde wohl gleichzeitig beendet "BLOCK")
        }else{
          //kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdraengen und "idle" werden!
          if(running_tasks.size()!=0){  // alten Task entfernen
            TaskMap::const_iterator iter;
            iter=running_tasks.begin();
            TaskInstance *task=iter->second;
            task_to_resign=task->getInstanceId();
            ret_decision=RESIGNED;
          }else{
            //war keiner da... und ist auch kein Neuer da -> keine Aenderung    
            ret_decision=NOCHANGE;
          }             
        }    
      }else{
        //neuer Task hinzugefuegt -> nichts tun 
        //oder alter entfernt    -> neuen setzen
        //neuen setzen:
        if(running_tasks.size()==0){       //alter entfernt  -> neuen setzen
          //   std::cout<<"Task fertig!"<<std::endl;

          if(TDMA_slots[curr_slicecount].pid_fifo.size()>0){            // ist da auch ein neuer da?        
            unsigned int tempcount=0;
            bool found=false;
            //if not.. try the next one (if existing)
            while(!found && tempcount < TDMA_slots[curr_slicecount].pid_fifo.size()){
              std::cout<<"task_to_assign =  TDMA_slots["<<curr_slicecount<<"].pid_fifo["<<tempcount<<"]"<<std::endl;
              task_to_assign = TDMA_slots[curr_slicecount].pid_fifo[tempcount];

              //is this task allowed to run in this slot? or is an offset required?
              if(ProcessParams[task_to_assign].offset<cyclecount){
                //potenzieren des cycle-multiplex
                int mux_value = 1 << ProcessParams[task_to_assign].multiplex;
                //no multiplex - run it  -- 
                if(ProcessParams[task_to_assign].multiplex==0 || (cyclecount % mux_value)==ProcessParams[task_to_assign].offset){
                  /*  std::cout<<"Abfrage: mux= "<< ProcessParams[task_to_assign].multiplex << std::endl;
                      std::cout<<" count= "<<cyclecount <<" 2^ = "<< mux_value<<std::endl;
                      std::cout<< " MOD= " << cyclecount % mux_value <<std::endl;
                  */
                  found=true;
                }
              }
              tempcount++;
            }
        
            if(!found){
              ret_decision=NOCHANGE;
            }else{
              //alter wurde schon entfernt (freiwillige abgabe "BLOCK")
              // -> kein preemption!
              ret_decision= ONLY_ASSIGN;
            }
          }     
        }
        //neuer Task hinzugefuegt, aber ein anderer laeuft noch -> nichts tun
      }
    }else{
      if((curr_slicecount+1)==StartslotDynamic && this->remainingSlice > sc_core::sc_time(0,sc_core::SC_NS) ){
        //Restzeit des statischen Abschnitts "verbraten"
        ret_decision=NOCHANGE;
        if(this->remainingSlice < (sc_core::sc_time_stamp() - this->lastassign)) this->remainingSlice=sc_core::SC_ZERO_TIME;
        else{ this->remainingSlice = this->remainingSlice - (sc_core::sc_time_stamp() - this->lastassign);  }
        this->lastassign = sc_core::sc_time_stamp();
        
      }else{
        curr_slicecount=-1;
        cyclecount++;
        ret_decision=schedulingDecision(task_to_resign, task_to_assign, ready_tasks, running_tasks );   
      }
    } 
    
    /*  if(ret_decision != NOCHANGE){
        std::cout << sc_core::sc_time_stamp() << " Decision: " << ret_decision << "newTask: " << task_to_assign  << " old task: " << task_to_resign << " Timeslice: " << this->remainingSlice << "  "<< remainingSliceA << "  " << remainingSliceB <<std::endl;
        }
    */  

#ifdef VPC_DEBUG  
    std::cout << "Decision: " << ret_decision << "newTask: " << task_to_assign 
         << " old task: " << task_to_resign <<  "Timeslice: " << this->remainingSlice << " " << sc_core::sc_time_stamp() <<  std::endl;
#endif //VPC_DEBUG  
    return ret_decision;
  }


  /**
   *
   */
  sc_core::sc_time* TimeTriggeredCCScheduler::schedulingOverhead(){
    return NULL; //new sc_core::sc_time(1,sc_core::SC_NS);
    
    //     return new sc_core::sc_time(1,sc_core::SC_NS);
  }

} } // namespace SystemC_VPC::Detail
