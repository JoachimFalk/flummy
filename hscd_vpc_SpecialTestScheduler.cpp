#include "hscd_vpc_SpecialTestScheduler.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_Component.h"



int SpecialTestScheduler::getSchedulerTimeSlice(sc_time& time ){
  if(rr_fifo.size()==0 && running_tasks.size()==0) return 0;
  time=sc_time(TIMESLICE,SC_NS);
  return 1;
}
void SpecialTestScheduler::addedNewTask(int pid){
  rr_fifo.push_back(pid);
}
void SpecialTestScheduler::removedTask(int pid){
  deque<int>::iterator iter;
  for(iter=rr_fifo.begin();iter!=rr_fifo.end();iter++){
    if( *iter == pid){
      rr_fifo.erase(iter);
      break;
    }
  }
}
scheduling_decision SpecialTestScheduler::schedulingDecision(int& task_to_resign, int& task_to_assign){
  if(rr_fifo.size()>0){
    task_to_assign = rr_fifo.front();//int rr_new_task = rr_fifo.front();
    rr_fifo.pop_front();
    if(running_tasks.size()>0){
      map<int,p_struct>::iterator iter2;
      iter2=running_tasks.begin();
      p_struct pcb=iter2->second;
      task_to_resign=pcb.pid;

      rr_fifo.push_back(task_to_resign);

      return PREEMPT;
    }else{  //kein Task auf running!
      
      return ONLY_ASSIGN;
    }
  }else{
    return RESIGNED;
  }
}
