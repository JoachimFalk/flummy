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

#include "hscd_vpc_AbstractComponent.h"
#include "Route.h"
#include "hscd_vpc_ProcessEventListener.h"
#include "hscd_vpc_EventPair.h"
#include "FastLink.h"
#include "TaskPool.h"
#include "SelectFastestPowerModeGlobalGovernor.hpp"
#include "HysteresisLocalGovernor.hpp"
#include "hscd_vpc_InvalidArgumentException.h"
#include "PluggablePowerGovernor.hpp"

// provide compatibility with other compilers then gcc, hopefully
//#include <ansidecl.h>

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <stdio.h>

namespace SystemC_VPC{

  class PowerSumming;
 
  /**
   * \brief Director knowes all (Abstract-)Components, all mappings (task -> component).
   *
   * Direktor reads allokation and binding from file.
   */
  class Director : public ProcessEventListener{
  public:
    bool FALLBACKMODE;

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
     * \brief Simulates communication delay of a given task
     * 
     * Determines the component from FastLink.
     * Supports pipelining!
     * \param fLink FastLink for task and function to execute.
     * \param quantum Number of read data tokens.
     * \param endPair EventPair to signal finishing of data introduction 
     * intervall (dii) and lateny.
     * If dii == latency no pipelinig is assumed and both events are notified
     * at same time!
     * \sa EventPair
     */
    void read( FastLink fLink,
               size_t quantum,
               EventPair endPair = EventPair(NULL, NULL) );

    /**
     * \brief Simulates communication delay of a given task
     * 
     * Determines the component from FastLink.
     * Supports pipelining!
     * \param fLink FastLink for task and function to execute.
     * \param quantum Number of written data tokens.
     * \param endPair EventPair to signal finishing of data introduction 
     * intervall (dii) and lateny.
     * If dii == latency no pipelinig is assumed and both events are notified
     * at same time!
     * \sa EventPair
     */
    void write( FastLink fLink,
                size_t quantum,
                EventPair endPair = EventPair(NULL, NULL) );


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
     * \brief Register component to Director
     * Used to register a component to the Director for
     * later computation of task on it. The components name
     * is used as identifier for it.
     * \param comp points to component instance to be registered
     */
    void registerComponent(Delayer* comp);
    
    /**
     * \brief Register mapping between task and component to Director
     * \param taskName specifies name of task
     * \param compName specifies name of component
     */
    void registerMapping(const char* taskName, const char* compName);

    /**
     * \brief Register a communication route.
     * \param route the route
     */
    void registerRoute(Route* route);
    
    void signalProcessEvent(Task* task);

    void setResultFile(std::string vpc_result_file){
      this->vpc_result_file = vpc_result_file;
      remove(vpc_result_file.c_str());
    }
    
    std::string getResultFile(){
      return this->vpc_result_file;
    }

    ProcessId uniqueProcessId();

    ProcessId getProcessId(std::string process);
    ProcessId getProcessId(std::string source, std::string destination);

    ComponentId getComponentId(std::string component);

    FastLink getFastLink(std::string process, std::string function);

    FastLink getFastLink(std::string source,
                         std::string destination,
                         std::string function);

    /**
     * \brief Takes a string representation of a time (e.g. a delay) and constructs a sc_time object.
     */
    static sc_time createSC_Time(const char* timeString) throw(InvalidArgumentException);

    /**
     * \brief Takes a string representation of a time (e.g. a delay) and constructs a sc_time object.
     */
    static sc_time createSC_Time(std::string timeString) throw(InvalidArgumentException);
    
    std::vector<ProcessId> * getTaskAnnotation(std::string compName);

    FunctionId getFunctionId(std::string function);
    FunctionId createFunctionId(std::string function);

    Task* allocateTask(ProcessId pid){
      return this->taskPool.allocate( pid );
    }

    // FIXME !!!
    PluggableGlobalPowerGovernor   *topPowerGov;
    DLLFactory<PlugInFactory<PluggableGlobalPowerGovernor> >
                                   *topPowerGovFactory;
    void loadGlobalGovernorPlugin(std::string plugin, Attribute att);

    std::string getTaskName(ProcessId id) {
      if(debugProcessNames.find(id) != debugProcessNames.end()){
        return debugProcessNames[id];
      }else{
        return "Route from " + debugRouteNames[id].first +
          " to: " + debugRouteNames[id].second;
      }
    }
    
    sc_time getEnd() const {
      return end;
    }
  private:

    Task * preCompute( FastLink fLink,
                       EventPair endPair );

    void postCompute( Task *task,
                      EventPair endPair );

    void debugUnknownNames( );

    /**
     * Singleton design pattern
     */
    static std::auto_ptr<Director> singleton; 

    /**
     * \brief Reads allocation and binding from file.
     */
    Director();

    typedef std::map<std::string, FunctionId>  FunctionIdMap;
    FunctionId uniqueFunctionId();

    FunctionIdMap   functionIdMap;
    FunctionId      globalFunctionId;

    typedef std::vector<Delayer* >  Components;
    Components                           components;
    
    typedef std::vector<Delayer* >  Mappings;
    Mappings                             mappings;

    typedef std::vector<ProcessId>                ProcessList;  
    typedef std::map<ComponentId, ProcessList* >  ReverseMapping;
    ReverseMapping reverseMapping;

    // output file to write result to
    std::string vpc_result_file;
    
    // time of latest acknowledge simulated task
    sc_time end;

#ifndef NO_POWER_SUM
    std::ofstream powerConsStream;
#endif // NO_POWER_SUM

    typedef std::map<std::string, ProcessId>   ProcessIdMap;
    typedef std::map<std::string, ComponentId> ComponentIdMap;

    ProcessIdMap    processIdMap;
    ComponentIdMap  componentIdMap;
    std::map<ProcessId, std::string> debugProcessNames;
    std::map<ProcessId, std::pair<std::string, std::string> > debugRouteNames;

    ProcessId       globalProcessId;

    PowerSumming    *powerSumming;

    TaskPool        taskPool;
  };

}
#endif
