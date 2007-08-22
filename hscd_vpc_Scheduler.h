#include <systemc.h>
#ifndef HSCD_VPC_SCHEDULER_H
#define HSCD_VPC_SCHEDULER_H
#include <hscd_vpc_datatypes.h>
#include <map>

namespace SystemC_VPC{
  
  enum scheduling_decision {ONLY_ASSIGN // neuer Task keine alten
          ,PREEMPT    // neuer Task verdrängt alten
          ,RESIGNED   // alter Task beendet/verdraengt, kein neuer
          ,NOCHANGE}; //keine änderung 
  class Component;

  /**
   * \brief A callback class called from Component to do Scheduling.
   *
   * Main part is virtual funktion scheduling_decision schedulingDecision(int&, int&, std::map<int,ProcessControlBlock>, std::map<int,ProcessControlBlock>)
   */
  class Scheduler{
  public:
    virtual ~Scheduler() {};

    /**
     * /brief Called from Component to determine a "time slice" used as time out.
     * 
     */
    virtual bool getSchedulerTimeSlice(sc_time &time,const std::map<int,ProcessControlBlock*> &ready_tasks,const std::map<int,ProcessControlBlock*> &running_tasks)=0;

    /**
     * \brief Inform Scheduler about new tasks.
     */
    virtual void addedNewTask(ProcessControlBlock *pcb)=0;
    
    /**
     * \brief Inform Scheduler about removed tasks.
     */
    virtual void removedTask(ProcessControlBlock *pcb)=0;

    /**
     * \brief Call the Scheduler to do a scheduling decision.
     *
     * The tasks to ressign and to assign have to be calculated. 
     * \param [out] task_to_resign The task that have to be resigned.
     * \param [out] task_to_assign The task that have to be assigned.
     * \param [in] ready_tasks A map of ready tasks! Component knowes this map.
     * \param [in] running_tasks A map of running tasks! Usualy only one! Component knowes this map.
     * \return Returns a scheduling_decision enum. So Component knows what he has to do.
     */
    virtual scheduling_decision schedulingDecision(int& task_to_resign, int& task_to_assign,const std::map<int,ProcessControlBlock*> &ready_tasks,const std::map<int,ProcessControlBlock*> &running_tasks)=0;

    /**
     *\brief The overhead needed to determine the scheduling descission.
     */
    virtual sc_time* schedulingOverhead()=0;

    /**
     *\brief Customize scheduler options, like timeslice or scheduling overhead.
     *
     * Does nothing by default.
     */
    virtual void setProperty(const char* key, const char* value){}

    /**************************/
    /*   EXTENSION SECTION    */
    /**************************/
    
    virtual const char* getName(){
      return this->name;
    }
    
    virtual void setName(const char* name){
      assert(name != NULL);
      this->name = name;
    }

    virtual void signalDeallocation(bool kill){}
    
    virtual void signalAllocation(){}
    
    virtual void initialize(){}
            
  private:
      const char* name;
    /**************************/
    /*  END OF EXTENSION      */
    /**************************/
  };

}
#endif
