/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Director.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef HSCD_VPC_DIRECTOR_H
#define HSCD_VPC_DIRECTOR_H

#include "hscd_vpc_AbstractDirector.h"
#include "hscd_vpc_AbstractComponent.h"
#include "hscd_vpc_TaskEventListener.h"

#include <string>
#include <map.h>
#include <vector.h>


namespace SystemC_VPC{

  struct p_struct;
  
  class Constraint;

  /**
   * \brief Director knowes all (Abstract-)Components, all mappings (task -> component).
   *
   * Direktor reads allokation and binding from file.
   */
  class Director : public AbstractDirector, public TaskEventListener{
  private:

    /**
     * Singleton design pattern
     */
    static std::auto_ptr<Director> singleton; 

    /**
     * \brief Reads allokation and binding from file.
     */
    Director();
    map<std::string,AbstractComponent*> component_map_by_name;
    //map<int,AbstractComponent*> component_map_by_pid;
    map<std::string,AbstractComponent*> mapping_map_by_name;
    //map<int,AbstractComponent*> mapping_map_by_pid;
    map<std::string,p_struct*> p_struct_map_by_name;
    //map<int,p_struct> p_struct_map_by_pid;

    vector<Constraint*> constraints;

  public:
    bool FALLBACKMODE;

    /**
     * \brief A task (identifikation by name) calling this Funktion gets the 
     * AbstractComponent where he is binded to.
     */
    //AbstractComponent& getResource( const char *name );
    //  AbstractComponent& getResource(int process);

    /**
     * \brief Get the process controll block used within SystemC-VPC Modell.
     */
    p_struct* getProcessControlBlock( const char *name );
    //  p_struct& getProcessControlBlock(int process);

    /**
     *
     */
    map<std::string,p_struct*>& getPcbMap(){
      return p_struct_map_by_name;
    }

    /**
     *
     */
    void checkConstraints();

    /**
     *
     */
    void getReport();

    /**
     * \brief Access to singleton Director. 
     */
    static Director& getInstance(){
      return *singleton;
    }

    virtual ~Director();

    /**
     * \brief Simulates computation of a given task
     * 
     * Determines for a given task the component to run on and delegates it.
     * \param name of task
     * \param name of function
     * \param event to signal finished request
     */
    virtual void compute(const char* name, const char* funcname, VPC_Event* end=NULL);
   
    /**
     * \brief Simulates computation of a given task
     *
     * Determines for a given task the component to run on and delegates it.
     * \param name of task
     * \param event to signal finished request
     */
    virtual void compute(const char *name, VPC_Event *end=NULL);
 
    /**
     * \brief Adds new constraint to Director
     * \param cons points to the constraint to be added
     */
    void addConstraint(Constraint* constraint);
    
    /**
     * \brief Register component to Director
     * Used to register a component to the Director for
     * later computation of task on it. The components name
     * is used as identifier for it.
     * \param comp points to component instance to be registered
     */
    void registerComponent(AbstractComponent* comp);
    
    /**
     * \brief Registers mapping between task and component to Director
     * \param taskName specifies name of task
     * \param compName specifies name of component
     */
    void registerMapping(const char* taskName, const char* compName);
    
    /**
     * \brief Generates and registers new PCB to Director
     * Generates a new PCB or returns a already registered one
     * form the Director.
     * \param name specifies name of actor/task/process for PCB
     * \return p_struct representing default initialized 
     * PCB for given task;
     */
    p_struct* generatePCB(const char* name);
    
    void signalTaskEvent(p_struct* pcb);

  };

}
#endif
