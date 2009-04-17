#include "hscd_vpc_FCFSScheduler.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_Component.h"

namespace SystemC_VPC{

  bool FCFSScheduler::getSchedulerTimeSlice(
    sc_time& time,
    const  TaskMap &ready_tasks,
    const  TaskMap &running_tasks ){
    return false;
  }
  void FCFSScheduler::addedNewTask(Task *task){
    fcfs_fifo.push_back(task->getInstanceId());
  }
  void FCFSScheduler::removedTask(Task *task){
    std::deque<int>::iterator iter;
    for(iter=fcfs_fifo.begin();iter!=fcfs_fifo.end();iter++){
      if( *iter == task->getInstanceId()){
  fcfs_fifo.erase(iter);
  break;
      }
    }
  }
  scheduling_decision FCFSScheduler::schedulingDecision(
    int& task_to_resign,
    int& task_to_assign,
    const  TaskMap &ready_tasks,
    const  TaskMap &running_tasks ){
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
