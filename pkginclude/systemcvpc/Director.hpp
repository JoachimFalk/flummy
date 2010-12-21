/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * Director.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef HSCD_VPC_DIRECTOR_H
#define HSCD_VPC_DIRECTOR_H

#include <systemcvpc/vpc_config.h>
#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/Route.hpp>
#include <systemcvpc/EventPair.hpp>
#include <systemcvpc/FastLink.hpp>
#include <systemcvpc/TaskPool.hpp>
#include <systemcvpc/InvalidArgumentException.hpp>
#include <systemcvpc/PluggablePowerGovernor.hpp>
#include <systemcvpc/ScheduledTask.hpp>

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <boost/function.hpp>

namespace SystemC_VPC{

  typedef std::vector<std::string> FunctionNames;

  class PowerSumming;
 
  /**
   * \brief Director knows all (Abstract-)Components, all mappings (task -> component).
   *
   * Director reads allocation and binding from file.
   */
  class Director {
  public:
    bool FALLBACKMODE;
    bool defaultRoute;
    bool checkVpcConfig;

    /**
     * \brief Access to singleton Director. 
     */
    static Director& getInstance(){
      return *singleton;
    }

    /**
     *
     */
    static bool canExecute(ScheduledTask * scheduledTask) {
      return getInstance().callCanExecute(scheduledTask);
    }

    /**
     *
     */
    static void execute(ScheduledTask * scheduledTask) {
      getInstance().callExecute(scheduledTask);
    }
    /**
     *
     */
    static bool canExecute(ProcessId pid){
      Task & task = getInstance().taskPool.getPrototype(pid);
      if (task.hasScheduledTask()){
        return canExecute(task.getScheduledTask());
      }
      return false;
    }

    /**
     *
     */
    static void execute(ProcessId pid){
      Task & task = getInstance().taskPool.getPrototype(pid);
      assert (task.hasScheduledTask());
      execute(task.getScheduledTask());
    }

    ~Director();

    /**
     * \brief Simulates computation of a given task
     * 
     * Determines the component from FastLink.
     * Supports pipelining!
     * \param fLink FastLink for task and function to execute.
     * \param endPair EventPair to signal finishing of data introduction 
     * interval (dii) and latency.
     * If dii == latency no pipelining is assumed and both events are notified
     * at same time!
     * \sa EventPair
     */
    void compute(FastLink fLink,
                 EventPair endPair = EventPair(NULL, NULL),
                 const sc_time & extraDelay = SC_ZERO_TIME);

    /**
     * \brief Simulates communication delay of a given task
     * 
     * Determines the component from FastLink.
     * Supports pipelining!
     * \param fLink FastLink for task and function to execute.
     * \param quantum Number of read data tokens.
     * \param endPair EventPair to signal finishing of data introduction 
     * interval (dii) and latency.
     * If dii == latency no pipelining is assumed and both events are notified
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
     * interval (dii) and latency.
     * If dii == latency no pipelining is assumed and both events are notified
     * at same time!
     * \sa EventPair
     */
    void write( FastLink fLink,
                size_t quantum,
                EventPair endPair = EventPair(NULL, NULL) );

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

    /**
     * \brief resolve mapping
     */
    const Delayer * getComponent(const FastLink vpcLink) const ;
    
    void signalLatencyEvent(Task* task);

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

    FastLink registerActor(ScheduledTask * actor,
                             std::string actorName,
                             const FunctionNames& functionNames);

    FastLink registerRoute(std::string source, std::string destination);

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

    bool hasFunctionId(const std::string& function) const;
    FunctionId getFunctionId(const std::string& function) const;
    FunctionId createFunctionId(const std::string& function);

    Task* allocateTask(ProcessId pid){
      return this->taskPool.allocate( pid );
    }

    // FIXME !!!
    PluggableGlobalPowerGovernor   *topPowerGov;
    DLLFactory<PlugInFactory<PluggableGlobalPowerGovernor> >
                                   *topPowerGovFactory;
    void loadGlobalGovernorPlugin(std::string plugin, AttributePtr attPtr);

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

    void registerSysteMoCCallBacks(
      boost::function<void (SystemC_VPC::ScheduledTask* actor)> execute,
      boost::function<bool (SystemC_VPC::ScheduledTask* actor)> testExecute){

      callExecute = execute;
      callCanExecute = testExecute;
    }

    void endOfVpcFinalize();
  private:

    Task * preCompute( FastLink fLink,
                       EventPair endPair );

    void postCompute( Task *task,
                      EventPair endPair );

    void debugUnknownNames( ) const;

    void assertMapping(ProcessId & pid){
      if (mappings.size() < pid ||
          mappings[pid] == NULL) {

        Task &task = this->taskPool.getPrototype( pid );

        std::cerr << "Unknown mapping <"
            << task.getName() << "> to ??" << std::endl;

        assert(mappings.size() >= pid &&
               mappings[pid] != NULL);
        exit(-1);
      }
    }

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
    std::map<ProcessId, std::set<std::string> > debugFunctionNames;

    PowerSumming    *powerSumming;

    TaskPool        taskPool;

    // callback to SysteMoC
    boost::function<void (SystemC_VPC::ScheduledTask* actor)> callExecute;
    boost::function<bool (SystemC_VPC::ScheduledTask* actor)> callCanExecute;

  };

}
#endif
