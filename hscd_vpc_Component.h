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
#include "hscd_vpc_SchedulerProxy.h"
#include <vector.h>
#include <map.h>

/**
 * \brief The interface definition to a Virtual-Processing-Component (VPC).
 */
class AbstractComponent{
 public:
    virtual void compute( const char *name )=0;
    //    virtual void compute(int iprocess)=0;
    virtual ~AbstractComponent(){};
};

/**
 * \brief An implementation of AbstractComponent.
 * An event based kommunikation to an Scheduler is realised by using the SchedulerProxy.
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

 protected:
  void compute(p_struct actualTask);
  map<string,sc_signal<trace_value>*> trace_map_by_name;
  map<int,p_struct>      new_tasks;
  vector<action_struct>  open_commands;
  sc_trace_file *trace;   ////////////////////////                              
  //  sc_trace_file *trace_wif;   ////////////////////////                              
  SchedulerProxy *scheduler;
  int id;
  char name [VPC_MAX_STRING_LENGTH];
};
#endif
