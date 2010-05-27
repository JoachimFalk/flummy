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
#include "hscd_vpc_ProcessEventListener.h"
#include "hscd_vpc_EventPair.h"
#include "FastLink.h"
#include "hscd_vpc_InvalidArgumentException.h"

#include <string>
#include <map>
#include <vector>

#include "hscd_vpc_PCBPool.h"

namespace SystemC_VPC{

  class ProcessControlBlock;
//  class PCBPool;  
  class Constraint;

  /**
   * \brief Director knowes all (Abstract-)Components, all mappings (task -> component).
   *
   * Direktor reads allokation and binding from file.
   */
  class Director : public AbstractDirector{
  public:
    bool FALLBACKMODE;

    /**
     * \brief A task (identifikation by name) calling this Funktion gets the 
     * AbstractComponent where he is binded to.
     * \note Re-added for downward compatibility
     */
    //AbstractComponent& getResource( const char *name );
    //  AbstractComponent& getResource(int process);
     Director& getResource( const char* name)
       __attribute__((deprecated));
     
    /**
     * \brief Get the process control block used within SystemC-VPC Modell.
     */
    ProcessControlBlock* getProcessControlBlock( const char *name );

    /**
     * \brief Get the process control block used within SystemC-VPC Modell.
     */
    ProcessControlBlock* getProcessControlBlock(  ProcessId pid );

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
      //return *singleton;
      static Director d; return d;
    }

    virtual ~Director();

    /**
     * \brief Simulates computation of a given task
     * 
     * Determines the component from FastLink.
     * Supports pipelining!
     * \param fLink FastLink for task and function to execute.
     * \param endPair EventPair to signal finishing of data introduction 
     * intervall (dii) and lateny.
     * If dii == latency no pipelinig is assumed and both events are notified
     * at same time!
     * \sa EventPair
     */
    void compute(FastLink fLink,
                 EventPair endPair = EventPair(NULL, NULL));

    /**
     * \brief Simulates computation of a given task
     * 
     * Determines the component for a given task and delegates it for execution time simulation.
     * Supports pipelining!
     * \param name Name of task to execute.
     * \param funcname Name of executed function within task.
     * \param endPair EventPair to signal finishing of data introduction intervall (dii) and lateny.
     * If dii == latency no pipelinig is assumed and both events are notified at same time!
     * \sa EventPair
     */
    void compute(const char* name, const char* funcname, EventPair endPair = EventPair(NULL, NULL));
   
    /**
     * \brief Simulates computation of a given task
     *
     * Determines the component for a given task and delegates it for execution time simulation.
     * Supports pipelining!
     * \param name Name of task to execute.
     * \param endPair EventPair to signal finishing of data introduction intervall (dii) and lateny.
     * If dii == latency no pipelinig is assumed and both events are notified at same time!
     * \sa EventPair
     */
    void compute(const char *name, EventPair endPair = EventPair(NULL, NULL));

    /**
     * \brief Simulates computation of a given task
     * 
     * Determines for a given task the component to run on and delegates it.
     * \param name of task
     * \param funcname of function
     * \param end to signal finished request
     */
    void compute(const char* name, const char* funcname, VPC_Event* end) 
      __attribute__((__deprecated__));
   
    /**
     * \brief Simulates computation of a given task
     *
     * Determines for a given task the component to run on and delegates it.
     * \param name of task
     * \param end to signal finished request
     */
    void compute(const char *name, VPC_Event *end)
      __attribute__((__deprecated__));
 
    /**
     * \brief Adds new constraint to Director
     * \param constraint points to the constraint to be added
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
     * \return ProcessControlBlock representing default initialized 
     * PCB for given task or if PCB already exists the initialized one;
     */
    ProcessControlBlock& generatePCB(const char* name);
    
    PCBPool& getPCBPool(); 

    void signalProcessEvent(ProcessControlBlock* pcb);

    void setResultFile(std::string vpc_result_file){
      this->vpc_result_file = vpc_result_file;
    }
    
    string getResultFile(){
      return this->vpc_result_file;
    }

    ProcessId uniqueProcessId();

    ProcessId getProcessId(std::string process);

    ComponentId getComponentId(std::string component);

    FastLink getFastLink(std::string process, std::string function);

    /**
     * \brief Takes a string representation of a time (e.g. a delay) and constructs a sc_time object.
     */
    static sc_time createSC_Time(const char* timeString) throw(InvalidArgumentException);
    
    std::vector<ProcessId> * getTaskAnnotation(std::string compName);

  private:

    /**
     * Singleton design pattern
     */
    //static std::auto_ptr<Director> singleton; 

    /**
     * \brief Reads allokation and binding from file.
     */
    Director();

    typedef std::vector<AbstractComponent* >  Components;
    Components                           components;
    
    typedef std::vector<AbstractComponent* >  Mappings;
    Mappings                             mappings;

    typedef std::vector<ProcessId>                ProcessList;  
    typedef std::map<ComponentId, ProcessList* >  ReverseMapping;
    ReverseMapping reverseMapping;

    PCBPool pcbPool;

    std::vector<Constraint*> constraints;

    // output file to write result to
    std::string vpc_result_file;
    
    // time of latest acknowledge simulated task
    double end;

    typedef std::map<std::string, ProcessId>   ProcessIdMap;
    typedef std::map<std::string, ComponentId> ComponentIdMap;

    ProcessIdMap    processIdMap;
    ComponentIdMap  componentIdMap;

    ProcessId       globalProcessId;
  };

}
#endif
