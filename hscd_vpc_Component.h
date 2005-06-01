/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Component.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef HSCD_VPC_COMPONENT_H
#define HSCD_VPC_COMPONENT_H
#include "systemc.h"
#include "hscd_vpc_datatypes.h"
//#include "hscd_vpc_SchedulerProxy.h"
#include <vector.h>
#include <map.h>
namespace SystemC_VPC{
  class SchedulerProxy;


  /**
   * \brief The interface definition to a Virtual-Processing-Component (VPC).
   * 
   * An application using this Framework should call the AbstractComponent::compute(const char *) Funktion.
   */
  class AbstractComponent{
  public:

    /**
     * \brief Simulate an execution on this "Virtual Component".
     *
     * While this simulation is running SystemC simulation time is consumed.
     */
    virtual void compute( const char *name )=0;
    //    virtual void compute(int iprocess)=0;
    virtual ~AbstractComponent(){};
  };

  /**
   * \brief An implementation of AbstractComponent.
   * 
   * This is an "Virtual Component" used to simulate execution time and scheduling (also preemptive scheduling).
   * An event based communication to a Scheduler is realised, using the SchedulerProxy as counterpart.
   */
  class Component : public AbstractComponent{
  public:
    map<int,p_struct> &getNewTasks();
    vector<action_struct> &getNewCommands();
    virtual void compute( const char *name );
    //  virtual void compute(int process);
    Component();
    Component(const char *name,const char *schedulername);
    virtual ~Component();
    virtual void informAboutMapping(string component);

  private:
    void compute(p_struct actualTask);
    map<string,sc_signal<trace_value>*> trace_map_by_name;
    map<int,p_struct>      new_tasks;
    vector<action_struct>  open_commands;
    sc_trace_file *trace;   ////////////////////////                              
    //  sc_trace_file *trace_wif;   ////////////////////////                              
    SchedulerProxy *schedulerproxy;
    //  int id;
    char name [VPC_MAX_STRING_LENGTH];
  };
}
#endif
