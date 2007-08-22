
/* 
	TDMA-Scheduler mit variabler Minislot-Größe!
	
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

#include <TDMAScheduler.h>
#include <hscd_vpc_Director.h>
#include <hscd_vpc_Component.h>

namespace SystemC_VPC{

  TDMAScheduler::TDMAScheduler(const char *schedulername)
    : _properties() {
    processcount=0;
    lastassign=sc_time(0,SC_NS);
    this->remainingSlice = sc_time(0,SC_NS);
    slicecount=0;
    curr_slicecount=-1;  
    char rest[VPC_MAX_STRING_LENGTH];
    int sublength;
    char *secondindex;
     //':' finden -> ':' trennt key-value Paare 
    char *firstindex=strchr(schedulername,':');
    while(firstindex!=NULL){

      //':' überspringen und nächste ':' finden
      secondindex=strchr(firstindex+1,':');
      if(secondindex!=NULL)
        sublength=secondindex-firstindex;          //Länge bestimmen
      else
        sublength=strlen(firstindex);              
      strncpy(rest,firstindex+1,sublength-1);      //key-value extrahieren
      rest[sublength-1]='\0';
      firstindex=secondindex;                     
     
      // key und value trennen und Property setzen
      char *key, *value;
      value=strstr(rest,"-");
      if(value!=NULL){
        value[0]='\0';
        value++;
        key=rest;
        setProperty(key,value);
      }
    }
  }
  
  void TDMAScheduler::setProperty(const char* key, const char* value){
  	std::pair<std::string, std::string> toadd(key,value);
  	_properties.push_back(toadd);
  }
  
   void TDMAScheduler::initialize(){   
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
			//Erstellen der TDMA-Struktur
				TDMASlot newSlot;
				newSlot.length = Director::createSC_Time(value);	
				newSlot.name = key;
				TDMA_slots.insert(TDMA_slots.end(), newSlot);
				slicecount++;
				/*Erzeugen einer Info-Ausgabe		
				domain[0]='\0';
				sscanf(value,"%lf",&slottime);
				std::cout << "Datensatz für Slot Nr." << slot 
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
		}while(TDMA_slots[++i].name != value && (i+1)<TDMA_slots.size());
    	 
		//auch wirklich etwas passendes gefunden?		
 		assert(i<TDMA_slots.size());
		//Beziehung PId - SlotID herstellen
  		PIDmap[Director::getInstance().getProcessId(key)]=i;   
// 		cout<<"add Function " <<  key << " to " << value<<endl;
    	}	
  }

  
  bool TDMAScheduler::getSchedulerTimeSlice( sc_time& time,
  	 	const std::map<int,ProcessControlBlock*> &ready_tasks,
    		const  std::map<int,ProcessControlBlock*> &running_tasks )
  {      
    // keine wartenden + keine aktiven Threads -> ende!
    if(processcount==0 && running_tasks.size()==0) return 0;   
    //ansonsten: Restlaufzeit der Zeitscheibe
    time=TDMA_slots[curr_slicecount].length -(sc_time_stamp() - this->lastassign);  
    return true;   
  }
  
  
  void TDMAScheduler::addedNewTask(ProcessControlBlock *pcb){    
     //Neu für TDMA: Task der entsprechenden Liste des passenden TDMA-Slots hinzufügen
     TDMA_slots[ PIDmap[pcb->getPid()] ].pid_fifo.push_back(pcb->getInstanceId());
#ifdef VPC_DEBUG     
     cout<<"added Process " <<  pcb->getInstanceId() << " to Slot " << PIDmap[pcb->getPid()]  <<endl;
#endif //VPC_DEBUG
     processcount++;
  }
  
  
  void TDMAScheduler::removedTask(ProcessControlBlock *pcb){  
    std::deque<ProcessId>::iterator iter;
    for(iter = TDMA_slots[ PIDmap[pcb->getPid()] ].pid_fifo.begin(); iter!=TDMA_slots[PIDmap[pcb->getPid()]].pid_fifo.end() ;iter++){
      if( *iter == pcb->getInstanceId()){
        TDMA_slots[PIDmap[pcb->getPid()]].pid_fifo.erase(iter);
        break;
      }
    }
#ifdef VPC_DEBUG    
    cout<<"removed Task: " << pcb->getInstanceId()<<endl;
#endif //VPC_DEBUG   
    processcount--;  
  }
  
  
  // Eigentlicher Scheduler
  scheduling_decision TDMAScheduler::schedulingDecision(
    		int& task_to_resign,
    		int& task_to_assign,
    		const  std::map<int,ProcessControlBlock*> &ready_tasks,
    		const  std::map<int,ProcessControlBlock*> &running_tasks )
  {
    scheduling_decision ret_decision=NOCHANGE;
    //Zeitscheibe abgelaufen?
    if(this->remainingSlice < (sc_time_stamp() - this->lastassign)) this->remainingSlice=SC_ZERO_TIME;
    else{
    	this->remainingSlice = this->remainingSlice - (sc_time_stamp() - this->lastassign);  
    }
    this->lastassign = sc_time_stamp();
    
    if(this->remainingSlice <= sc_time(0,SC_NS)){//Zeitscheibe wirklich abgelaufen!
    	curr_slicecount = (curr_slicecount + 1)%slicecount; // Wechsel auf die nächste Zeitscheibe nötig!
	//neue Timeslice laden
    	this->remainingSlice = TDMA_slots[curr_slicecount].length;

      if(TDMA_slots[curr_slicecount].pid_fifo.size()>0){    // neuer Task da?
        task_to_assign = TDMA_slots[curr_slicecount].pid_fifo.front();
// 	cout<<"Scheduler:new task: " << task_to_assign << "..." <<endl;
        
	//alter wurde schon entfernt (freiwillige abgabe "BLOCK")
        // -> kein preemption!
        ret_decision= ONLY_ASSIGN;
	
        if(running_tasks.size()!=0){  // alten Task entfernen
          std::map<int,ProcessControlBlock*>::const_iterator iter;
          iter=running_tasks.begin();
          ProcessControlBlock *pcb=iter->second;
          task_to_resign=pcb->getInstanceId();
          ret_decision= PREEMPT;  
        }
        // else{}    ->
        //kein laufender Task (wurde wohl gleichzeitig beendet "BLOCK")
      }else{
      //kein neuer Task da.. aber Zeitscheibe trotzdem abgelaufen = Prozess verdrängen und "idle" werden!
      if(running_tasks.size()!=0){  // alten Task entfernen
          std::map<int,ProcessControlBlock*>::const_iterator iter;
          iter=running_tasks.begin();
          ProcessControlBlock *pcb=iter->second;
          task_to_resign=pcb->getInstanceId();
          ret_decision=RESIGNED;
        }else{
	//war keiner da... und ist auch kein Neuer da -> keine Änderung	
      	ret_decision=NOCHANGE;
	}      	
      }    
    }else{
     	//neuer Task hinzugefügt -> nichts tun 
      	//oder alter entfernt    -> neuen setzen
      	//neuen setzen:
      if(running_tasks.size()==0){       //alter entfernt  -> neuen setzen
        if(TDMA_slots[curr_slicecount].pid_fifo.size()>0){            // ist da auch ein neuer da?
          task_to_assign = TDMA_slots[curr_slicecount].pid_fifo.front();

          //alter wurde schon entfernt (freiwillige abgabe "BLOCK")
          // -> kein preemption!
          ret_decision= ONLY_ASSIGN;
        }
      }
      //neuer Task hinzugefügt, aber ein anderer laeuft noch -> nichts tun
    } 

#ifdef VPC_DEBUG  
      cout << "Decision: " << ret_decision << "newTask: " << task_to_assign 
      << " old task: " << task_to_resign <<  "Timeslice: " << this->remainingSlice << endl;
#endif //VPC_DEBUG  
    return ret_decision;
  }


  /**
   *
   */
  sc_time* TDMAScheduler::schedulingOverhead(){
    return NULL; //new sc_time(1,SC_NS);
  }

  /**
   * \brief Implementation of TDMAScheduler::signalDeallocation
   */
  void TDMAScheduler::signalDeallocation(bool kill){
  
  if(!kill){
    this->remainingSlice =
      this->remainingSlice - (sc_time_stamp() - this->lastassign);
      }else{
       
      //alle Prozesse aus den pid_fifos loeschen
          std::vector<TDMASlot>::iterator iter;
    		for(iter = TDMA_slots.begin(); iter!=TDMA_slots.end() ;iter++){
      		iter->pid_fifo.clear();        
      		}
      }
  }
  
  /**
   * \brief Implementation of TDMAScheduler::signalAllocation
   */  
  void TDMAScheduler::signalAllocation(){
    this->lastassign = sc_time_stamp();
  }
}
