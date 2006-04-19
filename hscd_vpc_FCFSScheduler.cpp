#include <hscd_vpc_FCFSScheduler.h>
#include <hscd_vpc_Director.h>
#include <hscd_vpc_Component.h>

namespace SystemC_VPC{

  bool FCFSScheduler::getSchedulerTimeSlice(sc_time& time,const  map<int,ProcessControlBlock*> &ready_tasks,const  map<int,ProcessControlBlock*> &running_tasks){
    return false;
  }
  void FCFSScheduler::addedNewTask(ProcessControlBlock *pcb){
    fcfs_fifo.push_back(pcb->getPID());
  }
  void FCFSScheduler::removedTask(ProcessControlBlock *pcb){
    deque<int>::iterator iter;
    for(iter=fcfs_fifo.begin();iter!=fcfs_fifo.end();iter++){
      if( *iter == pcb->getPID()){
  fcfs_fifo.erase(iter);
  break;
      }
    }
  }
  scheduling_decision FCFSScheduler::schedulingDecision(int& task_to_resign, int& task_to_assign,const  map<int,ProcessControlBlock*> &ready_tasks,const  map<int,ProcessControlBlock*> &running_tasks){
    if(running_tasks.size()==0){
      if(fcfs_fifo.size()>0){
        //      cerr << "only_assign" << endl;
        task_to_assign = fcfs_fifo.front();
        fcfs_fifo.pop_front();
        return ONLY_ASSIGN;
      }
    }
    return NOCHANGE;

  }
}
