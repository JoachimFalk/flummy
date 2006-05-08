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

#include "hscd_vpc_PCBPool.h"

namespace SystemC_VPC{

  class AbstractBinder;
  class StaticBinder;
  class MIMapper;
  class ProcessControlBlock;
  class Constraint;

  /**
   * \brief Director knowes all (Abstract-)Components, all mappings (task -> component).
   *
   * Direktor reads allokation and binding from file.
   */
  class Director : public AbstractDirector{
  private:

    /**
     * Singleton design pattern
     */
    static std::auto_ptr<Director> singleton; 

    /**
     * \brief Reads allokation and binding from file.
     */
    Director();
    
    map<std::string, AbstractComponent*> component_map_by_name;
    
    map<std::string, AbstractComponent*> mapping_map_by_name;
    PCBPool pcbPool;

    vector<Constraint*> constraints;

    // output file to write result to
    std::string vpc_result_file;
    
    // time of latest acknowledge simulated task
    double end;
 
    // binder instance to resolve bindings
    StaticBinder* binder;

    // mapper for MappingInformation
    MIMapper* miMapper;
    
  public:
    bool FALLBACKMODE;

    /**
     * \brief A task (identifikation by name) calling this Funktion gets the 
     * AbstractComponent where he is binded to.
     * \note Re-added for downward compatibility
     */
    //AbstractComponent& getResource( const char *name );
    //  AbstractComponent& getResource(int process);
     Director& Director::getResource( const char* name);
     
    /**
     * \brief Get the process control block used within SystemC-VPC Modell.
     */
    ProcessControlBlock* getProcessControlBlock( const char *name );
    //  ProcessControlBlock& getProcessControlBlock(int process);

    /**
     *
     */
    /*
    map<std::string,ProcessControlBlock*>& getPcbMap(){
      return pcb_map_by_name;
    }*/

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
     * \param mInfo specifies additional mapping information
     * \sa AbstractDirector
     */
    void registerMapping(const char* taskName, const char* compName, MappingInformation* mInfo);
    
    /**
     * \brief Generates and registers new PCB to Director
     * Generates a new PCB or returns a already registered one
     * form the Director.
     * \param name specifies name of actor/task/process for PCB
     * \return ProcessControlBlock representing default initialized 
     * PCB for given task or if PCB already exists the initialized one;
     */
    ProcessControlBlock& generatePCB(const char* name);
   
    PCBPool& getPCBPool();

    MIMapper* getMIMapper();

    //void registerPCB(const char* name, ProcessControlBlock* pcb);

    void signalTaskEvent(ProcessControlBlock* pcb);

    void setResultFile(std::string vpc_result_file){
      this->vpc_result_file = vpc_result_file;
    }
    
    string getResultFile(){
      return this->vpc_result_file;
    }
    
  };

}
#endif
