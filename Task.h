#ifndef HSCD_VPC_TASK_H_
#define HSCD_VPC_TASK_H_

#include <sstream>

#include "FastLink.h"
#include "hscd_vpc_ProcessControlBlock.h"
#include "TaskPool.h"

namespace SystemC_VPC {
  class Task{
  public:
    Task(TaskPool * pool) : timing(NULL), pcb(NULL), pool(pool), name("NN"){
          this->instanceId = Task::globalInstanceId++;
    }

    // getter, setter
    std::string getName(){return name;}
    void setName(std::string name){this->name = name;}
    ProcessId  getProcessId()                    {return pid;}
    void       setProcessId(ProcessId pid)       {this->pid = pid;}
    FunctionId getFunctionId()                   {return fid;}
    void       setFunctionId(FunctionId fid)     {this->fid = fid;}
    EventPair  getBlockEvent()                   {return blockEvent;}
    void       setBlockEvent(EventPair p)        {blockEvent = p;}
    void       setPCB(ProcessControlBlock* pcb)  {this->pcb = pcb;}
    void       setTiming(FunctionTiming* timing) {this->timing = timing;}

    void setDelay(const sc_time& delay)         {this->delay = delay;}
    sc_time getDelay() const                    {return this->delay;}
    void setLatency(const sc_time& latency)     {this->latency = latency;}
    sc_time getLatency() const                  {return this->latency;}
    void setRemainingDelay(const sc_time& delay){this->remainingDelay = delay;}
    sc_time getRemainingDelay() const           {return this->remainingDelay;}
    int getInstanceId()                         {return this->instanceId;}


    /**
     * 
     */
    void initDelays(){
      assert(pcb != NULL);
      FunctionId fid = this->getFunctionId();
      this->setRemainingDelay( timing->getDelay(fid));
      this->setDelay(          timing->getDelay(fid));
      this->setLatency(        timing->getLatency(fid));
    }

    // Adaptor setter / getter for ProcessControlBlock
    Tracing* getTraceSignal()
      {assert(pcb != NULL); return pcb->getTraceSignal();}
    void setTraceSignal(Tracing* t)
      {assert(pcb != NULL); return pcb->setTraceSignal(t);}
    int getPriority()
      {assert(pcb != NULL); return pcb->getPriority();}
    sc_time getPeriod()
      {assert(pcb != NULL); return pcb->getPeriod();}

    void release() {
      pool->free(this->getProcessId(), this);
    }

  private:
    friend class AssociativePrototypedPool<ProcessId, Task>;
    friend class PrototypedPool<Task>;

    Task(const Task &task) :
      pid(task.pid),
      fid(task.fid),
      blockEvent(task.blockEvent),
      delay(task.delay),
      latency(task.latency),
      remainingDelay(task.remainingDelay),
      timing(task.timing),
      pcb(task.pcb),
      pool(task.pool),
      name(task.name)
    {
          this->instanceId = Task::globalInstanceId++;
    }
        

    ProcessId  pid;
    FunctionId fid;
    EventPair  blockEvent;

    sc_time delay;
    sc_time latency;
    sc_time remainingDelay;

    
    FunctionTiming      *timing;
    ProcessControlBlock *pcb;
    TaskPool            *pool;

    static int globalInstanceId;
    int instanceId;
    std::string name;
  };

  typedef std::map<int, Task*>  TaskMap;

}
#endif // HSCD_VPC_TASK_H_
