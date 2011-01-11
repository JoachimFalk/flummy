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

#include <iostream>
#include <sstream>

#include <CoSupport/SystemC/systemc_time.hpp>

#include <systemcvpc/Director.hpp>
#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/VPCBuilder.hpp>
#include <systemcvpc/InvalidArgumentException.hpp>
#include <systemcvpc/StaticRoute.hpp>
#include <systemcvpc/PowerSumming.hpp>
#include <systemcvpc/Task.hpp>
#include <systemcvpc/SelectFastestPowerModeGlobalGovernor.hpp>
#include <systemcvpc/HysteresisLocalGovernor.hpp>
#include <systemcvpc/PluggablePowerGovernor.hpp>
#include <systemcvpc/StaticRoute.hpp>
#include <systemcvpc/RoutePool.hpp>
#include "ConfigCheck.hpp"

#include <systemc.h>
#include <map>

#include <systemcvpc/debug_config.hpp>
// if compiled with DBG_DIRECTOR create stream and include debug macros
#ifdef DBG_DIRECTOR
#include <CoSupport/Streams/DebugOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include <systemcvpc/debug_on.hpp>
#else
  #include <systemcvpc/debug_off.hpp>
#endif

namespace SystemC_VPC{

  //
  std::auto_ptr<Director> Director::singleton(new Director());

  /**
   *
   */
  Director::Director()
    : FALLBACKMODE(false),
      defaultRoute(false),
      checkVpcConfig(true),
      topPowerGov(new InternalSelectFastestPowerModeGovernor),
      topPowerGovFactory(NULL),
      globalFunctionId(1),
      mappings(),
      reverseMapping(),
      end(SC_ZERO_TIME),
#ifndef NO_POWER_SUM
      powerConsStream("powerconsumption.dat"),
#endif // NO_POWER_SUM
      componentIdMap()
  {
     sc_report_handler::set_actions(SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_,
                                    SC_DO_NOTHING);
    //sc_report_handler::set_actions(SC_ID_OBJECT_EXISTS_,
    //                               SC_DO_NOTHING);
    sc_report_handler::set_actions(SC_WARNING, SC_DO_NOTHING);

    try{
      VPCBuilder builder((Director*)this);
      builder.buildVPC();
    }catch(InvalidArgumentException& e){
      std::cerr << "Director> Got exception while setting up VPC:\n"
                << e.what() << std::endl;
      exit(-1);
    }catch(const std::exception& e){
      std::cerr << "Director> Got exception while setting up VPC:\n"
                << e.what() << std::endl;
      exit(-1);
    }

#ifndef NO_POWER_SUM
    powerSumming = new PowerSumming(powerConsStream);
#endif // NO_POWER_SUM
    for( Components::iterator it = components.begin();
         it != components.end();
         ++it )
    {
      if(*it != NULL) {
#ifndef NO_POWER_SUM
        (*it)->addObserver(powerSumming);
#endif // NO_POWER_SUM
        (*it)->initialize(this);

      }
    }

  }

  /**
   *
   */
  Director::~Director(){

    sc_time start = SC_ZERO_TIME;
    sc_time end = this->end;
#ifdef DBG_DIRECTOR
    std::cerr << "start: " << start << " end: " << end  << std::endl;
#endif //DBG_DIRECTOR

    if(0 != this->vpc_result_file.compare("")) {
#ifdef DBG_DIRECTOR
      std::cerr << "Director> result_file: "
                << this->vpc_result_file << std::endl;
#endif //DBG_DIRECTOR
      ofstream resultFile;
      resultFile.open(this->vpc_result_file.c_str());
      if(resultFile){
        resultFile << (end-start).to_default_time_units();
      }
      resultFile.flush();
      resultFile.close();
    }else{
      std::cerr << "[VPC] overall simulated time: " << end - start << std::endl;
    }

#ifndef NO_POWER_SUM
    for( Components::iterator it = components.begin();
         it != components.end();
         ++it )
    {
      if(*it != NULL) {
        (*it)->removeObserver(powerSumming);
      }
    }

    delete powerSumming;
#endif // NO_POWER_SUM

    // clear components
    for( Components::iterator it = components.begin();
         it != components.end();
         ++it ){
      // FIXME: find the bug
      if(*it != NULL) {
        delete *it;
      }
    }

    componentIdMap.clear();
  }

  //
  Task* Director::preCompute( FastLink fLink,
                              EventPair endPair ){
    if(FALLBACKMODE){
      // create Fallback behavior for active and passive mode!
      if( endPair.dii != NULL )
        endPair.dii->notify();      // passive mode: notify end
      if( endPair.latency != NULL )
        endPair.latency->notify();  // passive mode: notify end

      // do nothing, just return
      return NULL;
    }

//    EventPair blockEvent = endPair;
//    if( endPair.dii == NULL ){
//      // prepare active mode
//      blockEvent=EventPair(new VPC_Event(), new VPC_Event());
//      // we could use a pool of VPC_Events instead of new/delete
//    }

    try{
      //Task *task = new Task(fLink, endPair);
      Task *task = this->allocateTask( fLink.process );
      task->setFunctionIds( fLink.functions );
      task->setBlockEvent( endPair );
    
    
      //HINT: also treat mode!!
      //if( endPair.latency != NULL ) endPair.latency->notify();
    
      assertMapping(fLink.process);
      
      return task;
    } catch (NotAllocatedException e){
      cerr << "Unknown Task: ID = " << fLink.process
           << " name = " << this->getTaskName(fLink.process)  << std::endl;

      debugUnknownNames();
    }
    throw NotAllocatedException(this->getTaskName(fLink.process));
  }

  //
  void Director::postCompute( Task * task,
                              EventPair endPair ){

    if( endPair.dii == NULL){
      // active mode -> waits until simulated delay time has expired
      
      EventPair blockEvent =  task->getBlockEvent();

      CoSupport::SystemC::wait(*blockEvent.dii);
      blockEvent.dii = NULL;
      blockEvent.latency = NULL;
    }
  }

  //
  void Director::compute( FastLink fLink, EventPair endPair,
      const sc_time & extraDelay ){
    Task * task = preCompute(fLink, endPair);
    if(task == NULL) return;
    task->setTimingScale(1);
    task->setExtraDelay(extraDelay);
    assert(!FALLBACKMODE);

    Delayer* comp = mappings[fLink.process];
    comp->compute(task);
    postCompute(task, endPair);
  }

  //
  void Director::read( FastLink fLink,
                       size_t quantum,
                       EventPair endPair ) {
    // FIXME: treat quantum
    Task * task = preCompute(fLink, endPair);
    if(task == NULL) return;
    task->setWrite(false);
    task->setTimingScale(quantum);
    assert(!FALLBACKMODE);

    Delayer* comp = mappings[fLink.process];
    comp->compute(task);
    postCompute(task, endPair);
  }

  //
  void Director::write( FastLink fLink,
                        size_t quantum,
                        EventPair endPair ) {
    // FIXME: treat quantum
    Task * task = preCompute(fLink, endPair);
    if(task == NULL) return;
    task->setWrite(true);
    task->setTimingScale(quantum);
    assert(!FALLBACKMODE);

    Delayer* comp = mappings[fLink.process];
    comp->compute(task);
    postCompute(task, endPair);
  }

  /**
   * \brief Implementation of Director::registerComponent
   */
  void Director::registerComponent(Delayer* comp){
    ComponentId cid = comp->getComponentId();
    if(cid >= components.size())
      components.resize(cid+100, NULL);

    this->componentIdMap[comp->getName()] = cid;

    this->components[cid] = comp;

    DBG_OUT(" Director::registerComponent(" << comp->getName()
            << ") [" << comp->getComponentId() << "] # " << components.size()
            << endl);
  }
    
  /**
   * \brief Implementation of Director::registerMapping
   */
  void Director::registerMapping(const char* taskName, const char* compName){
    assert(!FALLBACKMODE);
    DBG_OUT("registerMapping( " << taskName<< ", " << compName << " )"<< endl);
    ProcessId       pid = getProcessId( taskName );
    if( pid >= mappings.size() ){
      mappings.resize( pid + 100, NULL );
    }

    if( !taskPool.contains( pid ) ){
      Task &task = taskPool.createObject( pid );
      task.setProcessId( pid );
      task.setName( taskName );
    }

    assert(pid <= mappings.size());
    
    ComponentId cid = this->getComponentId(compName);

    Delayer * comp = components[cid];

    assert( comp != NULL );
    mappings[pid] = comp;
    if(reverseMapping[cid] == NULL) reverseMapping[cid] = new ProcessList();
    reverseMapping[cid]->push_back(pid);
  }
   
  /**
   *
   */ 
  void Director::registerRoute(Route* route){
    assert(!FALLBACKMODE);
    this->registerComponent(route);
    const char * taskName = route->getName();
    const char * compName = route->getName();

    ProcessId       pid = getProcessId( taskName );
    if( pid >= mappings.size() ){
      mappings.resize( pid + 100, NULL );
    }
    DBG_OUT("registerRoute( " << taskName << " " << pid << " )"<< endl);

    if( !taskPool.contains( pid ) ){
      Task &task = taskPool.createObject( pid );
      task.setProcessId( pid );
      task.setName( taskName );
    }

    assert(pid <= mappings.size());
    
    ComponentId cid = this->getComponentId(compName);

    Delayer * comp = components[cid];

    assert( comp != NULL );
    mappings[pid] = comp;
    const ComponentList& hops = route->getHops();
    for(ComponentList::const_iterator iter = hops.begin();
        iter != hops.end();
        ++iter){
      ComponentId hid = this->getComponentId((*iter)->getName());
      if(reverseMapping[hid] == NULL) reverseMapping[hid] = new ProcessList();
      DBG_OUT("register reverse Route-mapping: " << (*iter)->getName()
                << " " << hid << " -> "
                << pid << std::endl);
      reverseMapping[hid]->push_back(pid);
    }

  }

  //
  const Delayer * Director::getComponent(const FastLink vpcLink) const {
    if (mappings.size() < vpcLink.process ||
        mappings[vpcLink.process] == NULL) {
      std::map<ProcessId, std::string>::const_iterator iter =
        debugProcessNames.find(vpcLink.process);
      std::cerr << "Unknown mapping for task " << iter->second << std::endl;

      debugUnknownNames();
    }

    return mappings[vpcLink.process];
  }

  //
  void Director::signalLatencyEvent(Task* task){
    assert(!FALLBACKMODE);

#ifdef DBG_DIRECTOR
    std::cerr << "Director> got notified from: " << task->getName()
              << std::endl;
    std::cerr << "Director> task successful finished: " << task->getName()
              << std::endl;
#endif //DBG_DIRECTOR
    if(NULL != task->getBlockEvent().latency)
      task->getBlockEvent().latency->notify();
    // remember last acknowledged task time
    this->end = sc_time_stamp();
    
    // free allocated task
    task->release();
  }


  ProcessId Director::uniqueProcessId() {
    static ProcessId       globalProcessId = 0;
    return globalProcessId++;
  }

  ProcessId Director::getProcessId(std::string process) {
    ProcessIdMap::const_iterator iter = processIdMap.find(process);
    if( iter == processIdMap.end() ) {
      ProcessId id = this->uniqueProcessId();
      processIdMap[process] = id;
      debugProcessNames[id] = process;
    }
    iter = processIdMap.find(process);
    return iter->second;
  }

  ProcessId Director::getProcessId(std::string source, std::string dest) {
    std::string name_hack = "msg_" + source + "_2_" + dest;
    ProcessIdMap::const_iterator iter = processIdMap.find(name_hack);
    if( iter == processIdMap.end() ) {
      ProcessId id = this->uniqueProcessId();
      processIdMap[name_hack] = id;
      debugRouteNames[id] = make_pair(source, dest);
    }
    iter = processIdMap.find(name_hack);
    return iter->second;
  }

  ComponentId Director::getComponentId(std::string component) {
#ifdef DBG_DIRECTOR
    cerr << " Director::getComponentId(" << component
         << ") # " << componentIdMap.size()
         << endl;
#endif //DBG_DIRECTOR

    ComponentIdMap::const_iterator iter = componentIdMap.find(component);
    assert( iter != componentIdMap.end() );
    return iter->second;
      
  }

  // FunctionIds are created by VPC during XML parsing.
  FunctionId Director::createFunctionId(const std::string& function) {
    FunctionIdMap::const_iterator iter = functionIdMap.find(function);
    if( iter == functionIdMap.end() ) {
      functionIdMap[function] = this->uniqueFunctionId();
    }
    iter = functionIdMap.find(function);
    return iter->second;
  }

  bool Director::hasFunctionId(const std::string& function) const {
    FunctionIdMap::const_iterator iter = functionIdMap.find(function);
    return (iter != functionIdMap.end());
  }

  // FunctionIds are used (get) by SysteMoC.
  // The default ID (and a default timing) is used if no ID was created during
  // parsing. (The function name was not given in the XM.L)
  FunctionId Director::getFunctionId(const std::string& function) const {
    FunctionIdMap::const_iterator iter = functionIdMap.find(function);

    // the function name was not set in configuration
    // -> we have to use the default delay
    if( iter == functionIdMap.end() ) {
      return defaultFunctionId;
    }
    return iter->second;
    
  }

  //
void Director::endOfVpcFinalize()
{
  if (checkVpcConfig) {
    ConfigCheck::check();
  }
}

  FunctionId Director::uniqueFunctionId() {
    return globalFunctionId++;
  }

  FastLink Director::registerActor(ScheduledTask * actor,
      std::string actorName,
      const FunctionNames &functionNames)
  {
    //TODO: check if this is really required.
    if(FALLBACKMODE) return FastLink();

    ProcessId       pid = getProcessId(  actorName  );
    FunctionIds     functionIds;
    for(FunctionNames::const_iterator iter = functionNames.begin();
        iter != functionNames .end();
        ++iter){
      //check if we have timing data for this function in the XML configuration
      if(hasFunctionId(*iter)){
        functionIds.push_back( getFunctionId(*iter) );
      }
      debugFunctionNames[pid].insert(*iter);
      ConfigCheck::modelTiming(pid, *iter);
    }

    if (!taskPool.contains( pid )){
      cerr << "Unknown Task: name = " << actorName  << std::endl;
      return FastLink(pid, functionIds);

      //debugUnknownNames();
      //throw NotAllocatedException(actorName);
    }
    Task &task = taskPool.getPrototype(pid);
    task.setScheduledTask(actor);

    assertMapping(pid);
    Delayer* delayer = mappings[pid];
    AbstractComponent * component = dynamic_cast<AbstractComponent*>(delayer);
    if (component != NULL){
      component->addScheduledTask(pid);
    }
    actor->setDelayer(delayer);
    actor->setPid(pid);

    return FastLink(pid, functionIds);
  }

  FastLink Director::registerRoute(std::string source,
                                      std::string destination){
    //TODO: check if this is really required.
    if(FALLBACKMODE) return FastLink();

    ProcessId       pid = getProcessId( source, destination );
    FunctionIds     fids; // empty functionIds
    fids.push_back( getFunctionId("1") );

    // change default behavior: add empty route
    if (defaultRoute){
      if ( !taskPool.contains(pid) ){
        Route * route = new RoutePool<StaticRoute>(source, destination);
        route->enableTracing(false);
        this->registerRoute(route);
      }
    }

    return FastLink(pid, fids);
  }

  FastLink Director::getFastLink(std::string source,
                                 std::string destination,
                                 std::string function){
    return registerRoute(source, destination);
  }

  
  sc_time Director::createSC_Time(const char* timeString)
    throw(InvalidArgumentException)
  {
    try{
      return CoSupport::SystemC::createSCTime(timeString);
    } catch(std::string msg){
      throw InvalidArgumentException(msg);
    }
  }

  sc_time Director::createSC_Time(std::string timeString)
    throw(InvalidArgumentException)
  {
    return Director::createSC_Time( timeString.c_str());
  }

  void Director::loadGlobalGovernorPlugin(std::string plugin,
                                          AttributePtr attPtr){
    //std::cerr << "Director::loadGlobalGovernorPlugin" << std::endl;
    topPowerGovFactory =
      new DLLFactory<PlugInFactory<PluggableGlobalPowerGovernor> >
        (plugin.c_str());
    if( topPowerGovFactory->factory){
      delete topPowerGov;

      topPowerGovFactory->factory->processAttributes(attPtr);
      topPowerGov = topPowerGovFactory->factory->createPlugIn();
    }
  }

std::vector<ProcessId> * Director::getTaskAnnotation(std::string compName){
  ComponentId cid=getComponentId(compName);
  return reverseMapping[cid];
}  
  
  void Director::debugUnknownNames( ) const {
    bool route = false;
    bool mappings = false;
    for(std::map<ProcessId, std::pair<std::string, std::string> >::const_iterator
          iter = debugRouteNames.begin();
        iter != debugRouteNames.end();
        ++iter){
      if( !taskPool.contains( iter->first ) ){
           if(!route){
             std::cout << "Found unknown routes.\n"
                       <<" Please add the following routes to the config file:"
                       << std::endl;
             route = true;
           }

        std::cout << "  <route  source=\""
                  << iter->second.first
                  << "\" destination=\""
                  << iter->second.second
                  << "\">\n   <hop name=\"?\">  </hop>\n  </route>"
                  << std::endl;
      }
    }
    for(std::map<ProcessId, std::string>::const_iterator iter =
          debugProcessNames.begin();
        iter != debugProcessNames.end();
        ++iter){
      if( !taskPool.contains( iter->first ) ){
        if(!mappings){
          std::cout << "\n" << std::endl;
          std::cout << "Unknown mapping for tasks.\n"
                    <<" Please add the following mapping to the config file:"
                    << std::endl;
          mappings = true;
        }        
        std::cout << "  <mapping source=\""
                  << iter->second
                  << "\" target=\"?\">\n"
                  << "    <!-- we may use a default delay: "
                  << "    <timing dii=\"? us\""
                  << " latency=\"? us\" />"
                  << " --> "
                  << std::endl;

        const std::set<std::string>& functionNames =
          debugFunctionNames.find(iter->first)->second;
        
        for(std::set<std::string>::const_iterator fiter =
            functionNames.begin();
            fiter != functionNames.end();
            ++fiter){
          if(0 == fiter->compare("???")){
            std::cout << "    <!-- the \"???\" is caused by a SysteMoC"
                      << " transition without function CALL  -->" << std::endl;
          }
          std::cout << "    <timing fname=\""
                    << *fiter
                    << "\" dii=\"? us\""
                    << " latency=\"? us\" />"
                    << std::endl;
        }
        std::cout <<"  </mapping>" << std::endl;
      }
    }
    if(route || mappings){
      std::cout << "\n" << std::endl;
    }
    exit(-1);
  }

}

