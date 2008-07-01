#ifndef HSCD_VPC_TASK_H_
#define HSCD_VPC_TASK_H_

#include <sstream>

#include "FastLink.h"
#include "hscd_vpc_ProcessControlBlock.h"

namespace SystemC_VPC {
  class Task{
  public:
    Task(){}
    
    Task(FastLink link, EventPair pair)
      :  pid(link.process),
         fid (link.func),
         blockEvent(pair) {}
    
    
    // getter, setter
    std::string getName(){return pcb->getName();}
    ProcessId  getProcessId()                    {return pid;}
    FunctionId getFunctionId()                   {return fid;}
    EventPair  getBlockEvent()                   {return blockEvent;}
    void       setBlockEvent(EventPair p)        {blockEvent = p;}
    void       setPCB(ProcessControlBlock* pcb)  {this->pcb = pcb;}

    void setDelay(const sc_time& delay)         {this->delay = delay;}
    sc_time getDelay() const                    {return this->delay;}
    void setLatency(const sc_time& latency)     {this->latency = latency;}
    sc_time getLatency() const                  {return this->latency;}
    void setRemainingDelay(const sc_time& delay){this->remainingDelay = delay;}
    sc_time getRemainingDelay() const           {return this->remainingDelay;}


    /**
     * 
     */
    void initDelays(){
      FunctionId fid = this->getFunctionId();
      this->setRemainingDelay( pcb->getDelay(fid));
      this->setDelay(          pcb->getDelay(fid));
      this->setLatency(        pcb->getLatency(fid));
    }

    // Adaptor setter / getter for ProcessControlBlock
    Tracing* getTraceSignal()        {return pcb->getTraceSignal();}
    void setTraceSignal(Tracing* t)  {return pcb->setTraceSignal(t);}
    int getInstanceId()              {return pcb->getInstanceId();}
    void release()                   {return pcb->release(); delete this;}
    int getPriority()                {return pcb->getPriority();}
    sc_time getPeriod()              {return pcb->getPeriod();}

  private:
    ProcessId  pid;
    FunctionId fid;
    EventPair  blockEvent;

    sc_time delay;
    sc_time latency;
    sc_time remainingDelay;

    
    ProcessControlBlock *pcb;
  };

  typedef std::map<int, Task*>  TaskMap;

}
#endif // HSCD_VPC_TASK_H_
