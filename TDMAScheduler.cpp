
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
#include <utility>

#include <CoSupport/SystemC/algorithm.hpp>

#include <TDMAScheduler.h>
#include <hscd_vpc_Director.h>
#include <hscd_vpc_Component.h>

namespace SystemC_VPC{

  TDMAScheduler::TDMAScheduler(const char *schedulername)
    : _properties() {
    processcount=0;
    lastassign=SC_ZERO_TIME;
    this->remainingSlice = SC_ZERO_TIME;
    slicecount=0;
    curr_slicecount=-1;  
  }
  
  void TDMAScheduler::setProperty(const char* key, const char* value){
    _properties.push_back( std::make_pair(key, value) );
  }
  
  void TDMAScheduler::initialize(){   
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
  
 
  void TDMAScheduler::_setProperty(const char* key, const char* value){
    char *domain;
    int slot;
    //Herausfinden, welcher Slot genannt ist + welche Zeit ihm zugeordnet wird
    if(0==strncmp(key,"slot",strlen("slot"))){
      domain=strstr(key,"slot");
      if(domain!=NULL){
        domain+=4*sizeof(char);
        sscanf(domain,"%d",&slot);
        domain=strstr(value,"ns");
        if(domain!=NULL){
          unsigned int slotId = TDMA_slots.size();
          // at this time tdmaCycle corresponds to the end of the last slot 
          slotOffsets[tdmaCycle] = slotId;//slot slotId starts at time tdmaCyle

          //Erstellen der TDMA-Struktur
          TDMASlot newSlot;
          newSlot.length = Director::createSC_Time(value);      
          newSlot.name = key;
          TDMA_slots.insert(TDMA_slots.end(), newSlot);
          slicecount++;

          tdmaCycle += newSlot.length;
          /*Erzeugen einer Info-Ausgabe         
            domain[0]='\0';
            sscanf(value,"%lf",&slottime);
            std::cout << "Datensatz fuer Slot Nr." << slot 
            <<"gefunden! TDMA-Slotdauer: " <<slottime << "ns"<<std::endl;
          */            
        }
      }
    }else if(0==strncmp(value,"slot",strlen("slot"))){
      int i=-1;
      //schon Slots geadded? oder cmx-Syntax/Reihenfolge falsch?
      assert(0<TDMA_slots.size());
      //Betreffende SlotID in der Slotliste suchen
      do{
        //nichts zu tun.. da lediglich durchiteriert wird!
      }while(TDMA_slots[++i].name != value && (i+1)<(int)TDMA_slots.size());
         
      //auch wirklich etwas passendes gefunden?         
      assert(i<(int)TDMA_slots.size());
      //Beziehung PId - SlotID herstellen
      PIDmap[Director::getInstance().getProcessId(key)]=i;   
      //                cout<<"add Function " <<  key << " to " << value<<endl;
    }   
  }

  
  bool TDMAScheduler::getSchedulerTimeSlice( sc_time& time,
                                             const TaskMap &ready_tasks,
                                             const TaskMap &running_tasks )
  {      
    // keine wartenden + keine aktiven Threads -> ende!
    if(processcount==0 && running_tasks.size()==0) return false;   


    sc_time cycleTime =
      CoSupport::SystemC::modulus(sc_time_stamp(), tdmaCycle);
    std::map<sc_time, unsigned int>::iterator it =
      slotOffsets.upper_bound(cycleTime);
    if(it == slotOffsets.end()){
      remainingSlice  = tdmaCycle - cycleTime;
    } else {
      // difference to start of next slot
      remainingSlice  = it->first - cycleTime;
    }

    time = remainingSlice;
    return true;
  }
  
  
  void TDMAScheduler::addedNewTask(Task *task){    
    //Neu fuer TDMA: Task der entsprechenden Liste des passenden
    //TDMA-Slots hinzufuegen
    if(PIDmap.find(task->getProcessId()) == PIDmap.end()){
      std::cerr << "No TDMA slot for " << task->getName()
                << " please add a TDMA slot maping e.g.:\n"
                << "    <attribute type=\"slot?\" value=\"?? ns\"/>\n"
                << "    <attribute type=\"" << task->getName()
                << "\" value=\"slot?\"/>\n"
                << std::endl;


      if(0==TDMA_slots.size()){
        this->_setProperty("slot0","10 ns");
      }

      this->_setProperty(task->getName().c_str(),"slot0");
    }

    TDMA_slots[ PIDmap[task->getProcessId()] ].pid_fifo.push_back(task->getInstanceId());
#ifdef VPC_DEBUG     
    cout<<"added Process " <<  task->getInstanceId() << " to Slot " << PIDmap[task->getProcessId()]  <<endl;
#endif //VPC_DEBUG
    processcount++;
  }
  
  
  void TDMAScheduler::removedTask(Task *task){  
    std::deque<int>::iterator iter;
    for(iter = TDMA_slots[ PIDmap[task->getProcessId()] ].pid_fifo.begin();
        iter!=TDMA_slots[PIDmap[task->getProcessId()]].pid_fifo.end();
        ++iter){
      if( *iter == task->getInstanceId()){
        TDMA_slots[PIDmap[task->getProcessId()]].pid_fifo.erase(iter);
        break;
      }
    }
#ifdef VPC_DEBUG    
    cout<<"removed Task: " << task->getInstanceId()<<endl;
#endif //VPC_DEBUG   
    processcount--;  
  }
  
  
  // Eigentlicher Scheduler
  scheduling_decision
  TDMAScheduler::schedulingDecision(int& task_to_resign,
                                    int& task_to_assign,
                                    const TaskMap &ready_tasks,
                                    const TaskMap &running_tasks )
  {

    assert(tdmaCycle != SC_ZERO_TIME);

    sc_time cycleTime =
      CoSupport::SystemC::modulus(sc_time_stamp(), tdmaCycle);
    std::map<sc_time, unsigned int>::iterator it =
      slotOffsets.upper_bound(cycleTime); // upper_bound is the very next slot
    if(it == slotOffsets.end()){ //
      remainingSlice  = tdmaCycle - cycleTime;
    } else {
      // difference to start of next slot (which is the end of current slot)
      remainingSlice  = it->first - cycleTime;
    }

    --it; // switch to actual slot
    assert(it != slotOffsets.end());
    curr_slicecount = it->second ; // retrieve actual slot count
    const TDMASlot &currentSlot = TDMA_slots[curr_slicecount];

    scheduling_decision ret_decision=NOCHANGE;

    // running task available ?
    if( running_tasks.begin() != running_tasks.end() ){
      int runningTask = running_tasks.begin()->first;

      // running task == next task ?
      if( TDMA_slots[curr_slicecount].pid_fifo.front() == runningTask){
        //cerr << "TDMA: NOCHANGE " << endl;
        return NOCHANGE; // no task switch needed
      }else{ //running task != next task 
        //resign running task
        task_to_resign = runningTask;
        //cerr << "TDMA: RESIGNED?? " << endl;
        ret_decision = RESIGNED;
        if( ! currentSlot.pid_fifo.empty() ){
          //assign next task if available
          task_to_assign = currentSlot.pid_fifo.front();
          //cerr << "TDMA: PREEMPT " << endl;
          ret_decision = PREEMPT;
        }
        return ret_decision;
      }
    } else { // no running task available !

      //assign next task if available
      if( ! currentSlot.pid_fifo.empty() ){
        task_to_assign = currentSlot.pid_fifo.front();
        //cerr << "TDMA: ASSIGN " << endl;
        return ONLY_ASSIGN;;
      }
    }
    //cerr << "TDMA: NOCHANGE " << endl;
    return ret_decision;
  }


  /**
   *
   */
  sc_time* TDMAScheduler::schedulingOverhead(){
    return NULL; //new sc_time(1,SC_NS);
  }
}
