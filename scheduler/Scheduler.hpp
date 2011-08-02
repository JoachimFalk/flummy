/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#ifndef HSCD_VPC_SCHEDULER_H
#define HSCD_VPC_SCHEDULER_H

#include <systemcvpc/Task.hpp>
#include <systemcvpc/ComponentModel.hpp>
#include <systemcvpc/Attribute.hpp>

#include <systemc.h>

namespace SystemC_VPC{
  
  enum scheduling_decision {ONLY_ASSIGN // no old task, assign new task only
          ,PREEMPT    // preempt running task
          ,RESIGNED   // finished running task, no new task
          ,NOCHANGE}; // keep unchanged

  /**
   * \brief A call-back class called from Component to do Scheduling.
   *
   * Main part is virtual function scheduling_decision schedulingDecision(int&, int&, std::map<int,ProcessControlBlock>, std::map<int,ProcessControlBlock>)
   */
  class Scheduler{
  public:
    virtual ~Scheduler() {};

    /**
     * /brief Called from Component to determine a "time slice" used as time out.
     * 
     */
    virtual bool
    getSchedulerTimeSlice(sc_time &time,
                          const TaskMap &ready_tasks,
                          const TaskMap &running_tasks)=0;

    /**
     * \brief Inform Scheduler about new tasks.
     */
    virtual void addedNewTask(Task *task)=0;
    
    /**
     * \brief Inform Scheduler about removed tasks.
     */
    virtual void removedTask(Task *task)=0;

    /**
     * \brief Call the Scheduler to do a scheduling decision.
     *
     * The tasks to resign and to assign have to be calculated.
     * \param [out] task_to_resign The task that have to be resigned.
     * \param [out] task_to_assign The task that have to be assigned.
     * \param [in] ready_tasks A map of ready tasks! Component knows this map.
     * \param [in] running_tasks A map of running tasks! Usually only one! Component knowes this map.
     * \return Returns a scheduling_decision enum. So Component knows what he has to do.
     */
    virtual scheduling_decision
    schedulingDecision(int& task_to_resign,
                       int& task_to_assign,
                       const TaskMap &ready_tasks,
                       const TaskMap &running_tasks)=0;

    /**
     *\brief The overhead needed to determine the scheduling decision.
     */
    virtual sc_time* schedulingOverhead()=0;

    /**
     *\brief Customize scheduler options, like time slice or scheduling overhead.
     *
     * Does nothing by default.
     */
    virtual void setProperty(const char* key, const char* value){}
    
    virtual void setAttribute(AttributePtr attPtr)
    {
      if(attPtr->getAttributeSize() != 0 || attPtr->getParameterSize() != 0){
        return;
      }
      this->setProperty(attPtr->getType().c_str(), attPtr->getValue().c_str());
    }

    virtual void initialize(){}
  };

}
#endif
