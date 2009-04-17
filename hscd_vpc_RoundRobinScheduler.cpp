#include "systemcvpc/hscd_vpc_RoundRobinScheduler.h"
#include "systemcvpc/hscd_vpc_Director.h"
#include "systemcvpc/hscd_vpc_Component.h"

namespace SystemC_VPC{
  RoundRobinScheduler::RoundRobinScheduler(const char *schedulername){
    TIMESLICE=5;
    lastassign=0;
    this->remainingSlice = 0;
  }

  void RoundRobinScheduler::setProperty(const char* key, const char* value){
    if(0==strncmp(key,"timeslice",strlen("timeslice"))){
      char *domain;
      domain=strstr(value,"ns");
      if(domain!=NULL){
        domain[0]='\0';
        sscanf(value,"%lf",&TIMESLICE);
      }

    }
  }

  bool RoundRobinScheduler::getSchedulerTimeSlice(
    sc_time& time,
    const TaskMap &ready_tasks,
    const  TaskMap &running_tasks )
  {
    if(rr_fifo.size()==0 && running_tasks.size()==0) return 0;
    time=sc_time(TIMESLICE,SC_NS);
    return true;
  }
  void RoundRobinScheduler::addedNewTask(Task *task){
    rr_fifo.push_back(task->getInstanceId());
  }
  void RoundRobinScheduler::removedTask(Task *task){
    std::deque<int>::iterator iter;
    for(iter=rr_fifo.begin();iter!=rr_fifo.end();iter++){
      if( *iter == task->getInstanceId()){
        rr_fifo.erase(iter);
        break;
      }
    }
  }
  scheduling_decision RoundRobinScheduler::schedulingDecision(
    int& task_to_resign,
    int& task_to_assign,
    const  TaskMap &ready_tasks,
    const  TaskMap &running_tasks )
  {

    scheduling_decision ret_decision=NOCHANGE;

    this->remainingSlice = this->remainingSlice -
      (sc_time_stamp().to_default_time_units() - this->lastassign);
    this->lastassign = sc_time_stamp().to_default_time_units();

    if(this->remainingSlice <= 0){//Zeitscheibe wirklich abgelaufen!
      if(rr_fifo.size()>0){    // neuen Task bestimmen
        task_to_assign = rr_fifo.front();
        rr_fifo.pop_front();
        
        //alter wurde schon entfernt (freiwillige abgabe "BLOCK")
        // -> kein preemption!
        ret_decision= ONLY_ASSIGN;
        if(running_tasks.size()!=0){  // alten Task entfernen
          TaskMap::const_iterator iter;
          iter=running_tasks.begin();
          Task *task=iter->second;
          task_to_resign=task->getInstanceId();
          rr_fifo.push_back(task->getInstanceId());
          ret_decision= PREEMPT;  
        }
        // else{}    ->
        //kein laufender Task (wurde wohl gleichzeitig beendet "BLOCK")
      }    
    }else{//neuer Task hinzugefügt -> nichts tun 
      //oder alter entfernt    -> neuen setzen

      //neuen setzen:
      if(running_tasks.size()==0){       //alter entfernt  -> neuen setzen
        if(rr_fifo.size()>0){            // ist da auch ein neuer da?
          task_to_assign = rr_fifo.front();
          rr_fifo.pop_front();

          //alter wurde schon entfernt (freiwillige abgabe "BLOCK")
          // -> kein preemption!
          ret_decision= ONLY_ASSIGN;
        }
      }

      //nichts tun:
      //     ret_decision=NOCHANGE;
      //neuer Task hinzugefügt -> nichts tun
    } 

    /*
       if(ret_decision==ONLY_ASSIGN || ret_decision==PREEMPT){
       LASTASSIGN=sc_time_stamp().to_default_time_units();
       }
       */

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


  /**
   *
   */
  sc_time* RoundRobinScheduler::schedulingOverhead(){
    return NULL; //new sc_time(1,SC_NS);
  }
}
