/* 
        AutosarScheduler-Scheduler
        
        CMX-Format:
        
        z.B.
         <component name="Component1" type="threaded" scheduler="Autosar">
         <attribute type="AutosarParams">
              <attribute type="static">
                        <attribute type="slot0" value="100us">
                        <!--send -->
                          <attribute type="mapping" value="msg_simulation.x1_2_simulation.x2...">
                            <parameter type="multiplex" value="0"/>
                            <parameter type="offset" value="0"/>
                          </attribute>
                        </attribute>
                        
                        <attribute type="slot1" value="100us">
                        <!--receive -->
                          <attribute type="mapping" value="msg_simulation.x2_2_simulation.x1...">
                            <parameter type="multiplex" value="0"/>
                            <parameter type="offset" value="0"/>
                          </attribute>
                        </attribute>
                        <attribute type="slot2" value="100us">
                        <!--BSW -->
                        </attribute>

              </attribute>
                      
              <attribute type="dynamic" value="4700us">
                                                
              </attribute>
        </attribute>
        </component>
        Sebastian Graf - September 2009

*/

#include "AutosarScheduler.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_Component.h"

namespace SystemC_VPC{

  AutosarScheduler::AutosarScheduler(const char *schedulername):
    _properties(),
    scheduler_dynamic("Autosar"){
    dualchannel=0;
    //alt (TDMA)
    processcount=0;
    cyclecount=0;
    lastassign=sc_time(0,SC_NS);
    cycle_length = sc_time(0,SC_NS);
    this->remainingSlice = sc_time(0,SC_NS);
    slicecount=0;
    to_init=0;
   // firstrun=true;
    curr_slicecount=-1; 
    curr_slicecountA=0; 
  }
  
  void AutosarScheduler::setProperty(const char* key, const char* value){
    std::pair<std::string, std::string> toadd(key,value);
    _properties.push_back(toadd);
  }
  
  void AutosarScheduler::initialize(){   
    for(std::deque< std::pair <std::string, std::string> >::const_iterator iter = this->_properties.begin();
        iter != this->_properties.end();
        ++iter){
      this->_setProperty(iter->first.c_str(), iter->second.c_str());
    }
#ifdef VPC_DEBUG
    cout << "------------ END Initialize ---------"<<endl;
#endif //VPC_DEBUG      
    this->_properties.clear();
  }
  
 
  void AutosarScheduler::_setProperty(const char* key, const char* value){
    int i=-1;
    //schon Slots geadded? oder cmx-Syntax/Reihenfolge falsch?
    assert(0<TDMA_slots.size());
    //Betreffende SlotID in der Slotliste suchen
    do{
      //nichts zu tun.. da lediglich durchiteriert wird!
    }while(TDMA_slots[++i].name != value && (i+1)<(int)TDMA_slots.size());
                
    if(TDMA_slots[i].name != value){
      i+=StartslotDynamic;
    }
    ProcessId id=Director::getInstance().getProcessId(key);
    PIDmap[id]=i+1;
    ProcessParams[id]=ProcessParams_string[key];
  }

  void AutosarScheduler::setAttribute(Attribute& fr_Attribute){
    std::string value = fr_Attribute.getType();
    if( value!="AutosarParams" )
      return;
    if(fr_Attribute.getParameterSize()!=0){
      //es kann folglich globale Parameter geben
    }
        
    if( fr_Attribute.hasAttribute("static") ){
      Attribute fr_static = fr_Attribute.getAttribute("static");
      StartslotDynamic=0;
      for(size_t k=0; k<fr_static.getAttributeSize(); k++){
        std::pair<std::string, Attribute >attribute2=fr_static.getNextAttribute(k);
        //Slot einrichten
        StartslotDynamic++;
        slicecount++;
        std::pair<std::string, std::string > param;
        param.first=attribute2.second.getType();
        param.second=attribute2.second.getValue();
        TDMASlot newSlot;
        //Werte aus dem Attribute auslesen und damit neuen Slot erzeugen
        newSlot.length = Director::createSC_Time(param.second.c_str() );
        cycle_length += newSlot.length;
        newSlot.name = param.first;
        TDMA_slots.insert(TDMA_slots.end(), newSlot);
                        
        //jetzt noch die Task-mappings!
        //für jeden Attribute-Eintrag Parameter verarbeiten
        for(size_t l=0; l<attribute2.second.getAttributeSize(); l++){
          std::pair<std::string, Attribute >attribute3=attribute2.second.getNextAttribute(l);
          std::pair<std::string, std::string > param3;
          if(attribute3.first=="mapping"){
            param3.first=attribute3.second.getValue();
            param3.second=param.first;
            this->_properties.push_back(param3);
            ProcessParams_string[param3.first]=(struct SlotParameters){0,0};
            if(attribute3.second.getParameterSize()==0){
              //we don't have further Parameters, so let them as they are
            }else{
              //parse parameters
              if(attribute3.second.hasParameter("offset")){
                ProcessParams_string[param3.first].offset
                  = atoi(
                         attribute3.second.getParameter("offset")
                         .c_str()
                         );
              }
              if(attribute3.second.hasParameter("multiplex")){
                ProcessParams_string[param3.first].multiplex
                  = atoi(
                         attribute3.second.getParameter("multiplex")
                         .c_str()
                         );
              }
            }
          }
        }
      }
    }   
    if( fr_Attribute.hasAttribute("dynamic") ){
    //Schedule-Parameter für SWCs bei PriorityScheduler sollte es hier eigentlich keine geben..
    
      Attribute fr_dynamic = fr_Attribute.getAttribute("dynamic");
      this->TimeDynamicSegment = Director::createSC_Time(fr_dynamic.getValue());
      cycle_length += this->TimeDynamicSegment; 
    }
  }
  
  bool AutosarScheduler::getSchedulerTimeSlice( sc_time& time,
                                                const TaskMap &ready_tasks,
                                                const TaskMap &running_tasks )
  {
    //Restlaufzeit der Zeitscheibe
    if(curr_slicecount<StartslotDynamic){ //statisch
      if(curr_slicecount == -1){
	//vor Beginn des stat. Bereichs
        time=sc_time(0,SC_US);
      }else{
        time=this->remainingSlice;
      }
    }else{
          //Dynamisch
	  //Bugfix bei Start des dyn. Segments
    if(curr_slicecount == StartslotDynamic) time=sc_time(0,SC_US);
    else time=this->remainingSlice;
    }
    return true;
  }


  void AutosarScheduler::addedNewTask(Task *task){
    int index = PIDmap[task->getProcessId()];
    if(index!=0){
      //TDMA-Task
      index--;
      TDMA_slots[ index ].pid_fifo.push_back(task->getInstanceId());
      ProcessParams[task->getInstanceId()]=ProcessParams[task->getProcessId()];
#ifdef VPC_DEBUG     
      cout<<"added Process " <<  task->getInstanceId() << " to Slot " << PIDmap[task->getProcessId()]-1 <<endl;
#endif //VPC_DEBUG
    }else{
      scheduler_dynamic.addedNewTask(task);
    }
    if(processcount==0){
	    cyclecount = (int) (sc_time_stamp() / cycle_length);
	    to_init=true;
     }
     processcount++;
  }
  
  void AutosarScheduler::removedTask(Task *task){ 
    int index = PIDmap[task->getProcessId()];

    std::deque<int>::iterator iter;
    if(index!=0){
      index--;
      for(iter = TDMA_slots[ index ].pid_fifo.begin(); iter!=TDMA_slots[index].pid_fifo.end() ;iter++){
        if( *iter == task->getInstanceId()){
          TDMA_slots[index].pid_fifo.erase(iter);
          break;
        }
      }
    }else{
    scheduler_dynamic.removedTask(task);
    }
#ifdef VPC_DEBUG    
    cout<<"removed Task: " << task->getInstanceId()<<endl;
#endif //VPC_DEBUG   
    processcount--;  
  }
  
  
  // Eigentlicher Scheduler
  scheduling_decision
  AutosarScheduler::schedulingDecision(int& task_to_resign,
                                       int& task_to_assign,
                                       const TaskMap &ready_tasks,
                                       const TaskMap &running_tasks )
  {
    scheduling_decision ret_decision = NOCHANGE;
    //statischer oder dynamischer Teil?
      if(this->remainingSlice < (sc_time_stamp() - this->lastassign)){
	this->remainingSlice=SC_ZERO_TIME;
      }else{
        this->remainingSlice = this->remainingSlice - (sc_time_stamp() - this->lastassign);
      }
      this->lastassign = sc_time_stamp();
    if(curr_slicecount+1 < StartslotDynamic){
      if(this->remainingSlice <= sc_time(0,SC_NS)){//Zeitscheibe wirklich abgelaufen!
        curr_slicecount++; // Wechsel auf die naechste Zeitscheibe noetig!
        //neue Timeslice laden
        this->remainingSlice = TDMA_slots[curr_slicecount].length;
        
        //Korrekturfaktor falls mitten im Slot
        if(to_init == true && running_tasks.size()==0) {
        	to_init=false;
		if(sc_time_stamp() < cycle_length * cyclecount){
			//Quick-FIX: Bug if last Slot is completely empty
			this->remainingSlice = sc_time_stamp() - cycle_length * (cyclecount-1);
		}else{
        		this->remainingSlice = sc_time_stamp() - cycle_length * cyclecount;
		}
        	for(int i_fix = 0; i_fix < StartslotDynamic; i_fix++){
        		if( this->remainingSlice > TDMA_slots[i_fix].length){
        			this->remainingSlice = this->remainingSlice  - TDMA_slots[i_fix].length;
        		}else{
       				this->remainingSlice = TDMA_slots[i_fix].length - this->remainingSlice;
       				curr_slicecount = i_fix;
       				break;
        		}
        	}
        }
        if(TDMA_slots[curr_slicecount].pid_fifo.size()>0){    // neuer Task da?
          unsigned int tempcount=0;
          bool found=false;
          //if not.. try the next one (if existing)
          while(!found && tempcount<TDMA_slots[curr_slicecount].pid_fifo.size()){
            task_to_assign = TDMA_slots[curr_slicecount].pid_fifo[tempcount];
            if(ProcessParams[task_to_assign].offset<=cyclecount){
              int mux_value = 1 << ProcessParams[task_to_assign].multiplex;
              if(ProcessParams[task_to_assign].multiplex==0 || (cyclecount % mux_value)==ProcessParams[task_to_assign].offset){
                found=true;
              }
            }
            tempcount++;
          }
        
          //alter wurde schon entfernt (freiwillige abgabe "BLOCK")
          // -> kein preemption!
          ret_decision= ONLY_ASSIGN;
        
          if(!found){ //keinen lauffaehigen gefunden! -> idle werden
            task_to_assign=0;
            if(running_tasks.size()!=0){  // alten Task entfernen, wenn noetig
              TaskMap::const_iterator iter;
              iter=running_tasks.begin();
              Task *task=iter->second;
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
              Task *task=iter->second;
              task_to_resign=task->getInstanceId();
              ret_decision= PREEMPT;  
            }
          }

          //kein laufender Task (wurde wohl gleichzeitig beendet "BLOCK")
        }else{
          //kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdraengen und "idle" werden!
          if(running_tasks.size()!=0){  // alten Task entfernen
            TaskMap::const_iterator iter;
            iter=running_tasks.begin();
            Task *task=iter->second;
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
        if(running_tasks.size()==0){       //alter entfernt  -> neuen setzen
          if(TDMA_slots[curr_slicecount].pid_fifo.size()>0){
            unsigned int tempcount=0;
            bool found=false;
            //if not.. try the next one (if existing)
            while(!found && tempcount<TDMA_slots[curr_slicecount].pid_fifo.size()){
              task_to_assign = TDMA_slots[curr_slicecount].pid_fifo[tempcount];

              //is this task allowed to run in this slot? or is an offset required?
              if(ProcessParams[task_to_assign].offset<cyclecount){
                //potenzieren des cycle-multiplex
                int mux_value = 1 << ProcessParams[task_to_assign].multiplex;
                //no multiplex - run it  -- 
                if(ProcessParams[task_to_assign].multiplex==0 || (cyclecount % mux_value)==ProcessParams[task_to_assign].offset){
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
      //Dynamischer Teil
      if((curr_slicecount+1)==StartslotDynamic && this->remainingSlice > sc_time(0,SC_NS) ){
        //Restzeit des statischen Abschnitts "verbraten"
        ret_decision=NOCHANGE;
	
        if(this->remainingSlice < (sc_time_stamp() - this->lastassign)) this->remainingSlice=SC_ZERO_TIME;
        else{ this->remainingSlice = this->remainingSlice - (sc_time_stamp() - this->lastassign);  }
        this->lastassign = sc_time_stamp();
      }else{
        if(curr_slicecount+1==StartslotDynamic){
          //Start des Dyn. Segments
          curr_slicecount++;
          this->remainingSlice=TimeDynamicSegment;
          //auf alle Faelle den letzten (periodischen) Task verdraengen!
          if(running_tasks.size()!=0){  // alten Task entfernen
            TaskMap::const_iterator iter;
            iter=running_tasks.begin();
            Task *task=iter->second;
            task_to_resign=task->getInstanceId();
            ret_decision=RESIGNED;
          }else{
            //war keiner da... und ist auch kein Neuer da -> keine Aenderung    
            ret_decision=NOCHANGE;
          }
          this->lastassign = sc_time_stamp();
        }else{  
	curr_slicecount++;
          //Zeitscheibe abgelaufen? -> ende des dyn. Teils.. und Wechsel zurueck zum statischen
          if(this->remainingSlice <=(sc_time_stamp() - this->lastassign)){
            if(running_tasks.size()!=0){  // alten Task entfernen
              TaskMap::const_iterator iter;
              iter=running_tasks.begin();
              Task *task=iter->second;
              task_to_resign=task->getInstanceId();
              ret_decision=RESIGNED;
            }else{
              //war keiner da... und ist auch kein Neuer da -> keine Aenderung  
              ret_decision=NOCHANGE;
            }
	    this->remainingSlice=SC_ZERO_TIME;
	    //dynamischer Teil "aufgeraeumt" -> statischer kann starten.
	    curr_slicecount=-1;
	    //naechster Zyklus beginnt
	    cyclecount++;
            this->lastassign=sc_time_stamp();
          }else{ //Normalfall im dynamischen Teil
            //if(running_tasks.size() ==0){ //es laeuft grad keiner -> dynamic-Scheduler fragen..
              ret_decision = scheduler_dynamic.schedulingDecision(task_to_resign, task_to_assign, ready_tasks, running_tasks);
          }
        }
      }
    } 
    
#ifdef VPC_DEBUG  
    cout << "Decision: " << ret_decision << "newTask: " << task_to_assign 
         << " old task: " << task_to_resign <<  "Timeslice: " << this->remainingSlice << " " << sc_time_stamp() <<  endl;
#endif //VPC_DEBUG  
    return ret_decision;
  }


  /**
   *
   */
  sc_time* AutosarScheduler::schedulingOverhead(){
    return NULL; //new sc_time(1,SC_NS);
  }
}
