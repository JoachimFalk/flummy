#include <hscd_vpc_RoundRobinScheduler.h>
#include <hscd_vpc_Director.h>
#include <hscd_vpc_Component.h>

namespace SystemC_VPC{
  RoundRobinScheduler::RoundRobinScheduler(const char *schedulername){
    TIMESLICE=5;
    LASTASSIGN=0;
    char rest[VPC_MAX_STRING_LENGTH];
    //      char muell[VPC_MAX_STRING_LENGTH];
    /*
      if(0==strncmp(schedulername,STR_ROUNDROBIN,strlen(STR_ROUNDROBIN))){
      cerr << "Scheduler: "<< STR_ROUNDROBIN <<" - "<< schedulername <<endl;
      sscanf(schedulername,"%s-%s",muell,rest);
      cerr << "----- Rest: "<<rest<< " muell: "<<muell<<endl;
      }else if(0==strncmp(schedulername,STR_RR,strlen(STR_RR))){
      cerr << "Scheduler: "<< STR_RR << endl;
      }
    */
    int sublength;
    char *secondindex;
    char *firstindex=strchr(schedulername,':');    //':' finden -> ':' trennt key-value Paare 
    while(firstindex!=NULL){
      secondindex=strchr(firstindex+1,':');        //':' überspringen und nächste ':' finden
      if(secondindex!=NULL)
	sublength=secondindex-firstindex;          //Länge bestimmen
      else
	sublength=strlen(firstindex);              
      strncpy(rest,firstindex+1,sublength-1);      //key-value extrahieren
      rest[sublength-1]='\0';
      firstindex=secondindex;                     
    
    
      char *key, *value;               // key und value trennen und Property setzen
      value=strstr(rest,"-");
      if(value!=NULL){
	value[0]='\0';
	value++;
	key=rest;
	setProperty(key,value);
      }
    
    }
  }

  void RoundRobinScheduler::setProperty(char* key, char* value){
    if(0==strncmp(key,"timeslice",strlen("timeslice"))){
      char *domain;
      domain=strstr(value,"ns");
      if(domain!=NULL){
	domain[0]='\0';
	sscanf(value,"%lf",&TIMESLICE);
      }
    
    }
  }

  bool RoundRobinScheduler::getSchedulerTimeSlice(sc_time& time,const map<int,p_struct*> &ready_tasks,const  map<int,p_struct*> &running_tasks){
    if(rr_fifo.size()==0 && running_tasks.size()==0) return 0;
    time=sc_time(TIMESLICE,SC_NS);
    return true;
  }
  void RoundRobinScheduler::addedNewTask(p_struct *pcb){
    rr_fifo.push_back(pcb->pid);
  }
  void RoundRobinScheduler::removedTask(p_struct *pcb){
    deque<int>::iterator iter;
    for(iter=rr_fifo.begin();iter!=rr_fifo.end();iter++){
      if( *iter == pcb->pid){
	rr_fifo.erase(iter);
	break;
      }
    }
  }
  scheduling_decision RoundRobinScheduler::schedulingDecision(int& task_to_resign, int& task_to_assign,const  map<int,p_struct*> &ready_tasks,const  map<int,p_struct*> &running_tasks){
    /*
      if(running_tasks.size()==0){
      if(rr_fifo.size()>0){
      //      cerr << "only_assign" << endl;
      task_to_assign = rr_fifo.front();
      rr_fifo.pop_front();
      return ONLY_ASSIGN;
      }
      }
      return NOCHANGE;
    */
    scheduling_decision ret_decision=NOCHANGE;
    //  cerr << LASTASSIGN+TIMESLICE << " : "<<sc_simulation_time()<< " : " << LASTASSIGN << " : " << TIMESLICE<< " : " <<rr_fifo.size()<<endl;
    if(LASTASSIGN+TIMESLICE==sc_simulation_time()){//Zeitscheibe wirklich abgelaufen!
      if(rr_fifo.size()>0){    // neuen Task bestimmen
	task_to_assign = rr_fifo.front();
	rr_fifo.pop_front();
	ret_decision= ONLY_ASSIGN;    //alter wurde schon entfernt (freiwillige abgabe "BLOCK") -> kein preemption!
	if(running_tasks.size()!=0){  // alten Task entfernen
	  map<int,p_struct*>::const_iterator iter;
	  iter=running_tasks.begin();
	  p_struct *pcb=iter->second;
	  task_to_resign=pcb->pid;
	  rr_fifo.push_back(pcb->pid);
	  ret_decision= PREEMPT;	
	}// else{}    -> //kein laufender Task (wurde wohl gleichzeitig beendet "BLOCK")
      }    
    }else{//neuer Task hinzugefügt -> nichts tun 
      //oder alter entfernt    -> neuen setzen
    
      //neuen setzen:
      if(running_tasks.size()==0){       //alter entfernt  -> neuen setzen
	if(rr_fifo.size()>0){            // ist da auch ein neuer da?
	  task_to_assign = rr_fifo.front();
	  rr_fifo.pop_front();
	  ret_decision= ONLY_ASSIGN;    //alter wurde schon entfernt (freiwillige abgabe "BLOCK") -> kein preemption!
	}
      }
    
      //nichts tun:
      //     ret_decision=NOCHANGE;         //neuer Task hinzugefügt -> nichts tun
    } 


    if(ret_decision==ONLY_ASSIGN || ret_decision==PREEMPT){
      LASTASSIGN=sc_simulation_time();
    }


    /*if(ret_decision==ONLY_ASSIGN){
      cerr << "ONLY_ASSIGN" <<endl;
      }else if(ret_decision==PREEMPT){
      cerr << "PREEMPT" <<endl;
      }else if(ret_decision==NOCHANGE){
      cerr << "NOCHANGE " <<endl;
      }else if(ret_decision==RESIGNED){
      cerr << "RESIGNED " <<endl;
      }*/

    return ret_decision;
  }
}
