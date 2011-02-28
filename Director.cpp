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

#include <boost/foreach.hpp>


#include <CoSupport/SystemC/systemc_time.hpp>

#include <systemcvpc/ComponentImpl.hpp>
#include <systemcvpc/NonPreemptiveComponent.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/VPCBuilder.hpp>
#include <systemcvpc/InvalidArgumentException.hpp>
#include <systemcvpc/PowerSumming.hpp>
#include <systemcvpc/Task.hpp>
#include <systemcvpc/SelectFastestPowerModeGlobalGovernor.hpp>
#include <systemcvpc/HysteresisLocalGovernor.hpp>
#include <systemcvpc/PluggablePowerGovernor.hpp>
#include <systemcvpc/StaticRoute.hpp>
#include <systemcvpc/RoutePool.hpp>
#include <systemcvpc/config/Timing.hpp>
#include <systemcvpc/config/VpcApi.hpp>

#include "ConfigCheck.hpp"
#include "config/Mappings.hpp"

#include <systemc.h>
#include <map>
#include <vector>

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
        //delete *it;
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
      std::string name = ConfigCheck::getProcessName(vpcLink.process);
      std::cerr << "Unknown mapping for task " << name << std::endl;

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


  ProcessId getNextProcessId() {
    static ProcessId       globalProcessId = 0;
    return globalProcessId++;
  }

ProcessId Director::getProcessId(std::string process_or_source,
    std::string destination)
{
  typedef std::map<std::string, ProcessId> ProcessIdMap;
  static ProcessIdMap processIdMap;

  if (destination == "") {
    std::string & process = process_or_source;
    ProcessIdMap::const_iterator iter = processIdMap.find(process);
    if (iter == processIdMap.end()) {
      ProcessId id = getNextProcessId();
      processIdMap[process] = id;
      ConfigCheck::setProcessName(id, process);
    }
    iter = processIdMap.find(process);
    return iter->second;

  } else {
    std::string & source = process_or_source;
    std::string name_hack = "msg_" + source + "_2_" + destination;
    ProcessIdMap::const_iterator iter = processIdMap.find(name_hack);
    if (iter == processIdMap.end()) {
      ProcessId id = getNextProcessId();
      processIdMap[name_hack] = id;
      ConfigCheck::setRouteName(id, source, destination);
    }
    iter = processIdMap.find(name_hack);
    return iter->second;
  }
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

  typedef std::map<std::string, FunctionId>  FunctionIdMap;

  FunctionId uniqueFunctionId() {
    static FunctionId globalFunctionId = 1;
    return globalFunctionId++;
  }


  FunctionIdMap& getFunctionIdMap(){
    static FunctionIdMap functionIdMap;
    return functionIdMap;
  }

  // FunctionIds are created by VPC during XML parsing.
  FunctionId Director::createFunctionId(const std::string& function) {
    FunctionIdMap::const_iterator iter = getFunctionIdMap().find(function);
    if( iter == getFunctionIdMap().end() ) {
      // map empty function names to default timing
      if(function == "") {
        getFunctionIdMap()[function] = defaultFunctionId;
      }else{
        getFunctionIdMap()[function] = uniqueFunctionId();
      }
    }
    iter = getFunctionIdMap().find(function);
    return iter->second;
  }

  bool Director::hasFunctionId(const std::string& function){
    FunctionIdMap::const_iterator iter = getFunctionIdMap().find(function);
    return (iter != getFunctionIdMap().end());
  }

  // FunctionIds are used (get) by SysteMoC.
  // The default ID (and a default timing) is used if no ID was created during
  // parsing. (The function name was not given in the XM.L)
  FunctionId Director::getFunctionId(const std::string& function){
    FunctionIdMap::const_iterator iter = getFunctionIdMap().find(function);

    // the function name was not set in configuration
    // -> we have to use the default delay
    if( iter == getFunctionIdMap().end() ) {
      return defaultFunctionId;
    }
    return iter->second;
    
  }

  /// begin section: VpcApi.hpp related stuff
  namespace VC = Config;

  //
  void injectTaskName(const ScheduledTask * actor,
      std::string actorName)
  {
    if (VC::hasTask(actorName) && VC::hasTask(*actor)) {
      if (VC::getCachedTask(actorName) != VC::getCachedTask(*actor)) {
        // TODO: Check if a merging strategy is required.
        throw VC::ConfigException(actorName +
            " has configuration data from XML and from configuration API.");
      }
    } else if (!VC::hasTask(actorName) && !VC::hasTask(*actor)) {
      throw VC::ConfigException(actorName + " has NO configuration data at all.");
    } else if (VC::hasTask(actorName)){
      VC::VpcTask::Ptr task = VC::getCachedTask(actorName);
      VC::setCachedTask(actor, task);
      task->inject(actor);
    } else if (VC::hasTask(*actor)){
      VC::VpcTask::Ptr task = VC::getCachedTask(*actor);
      VC::setCachedTask(actorName, task);
    }
    assert(VC::hasTask(actorName) && VC::hasTask(*actor));
    assert(VC::getCachedTask(actorName) == VC::getCachedTask(*actor));
  }

  //
  void injectRoute(std::string src, std::string dest, sc_port_base * leafPort)
  {
    ProcessId pid = Director::getProcessId(src, dest);
    if (VC::Routing::has(pid) && VC::Routing::has(leafPort)) {
      if(VC::Routing::get(pid) != VC::Routing::get(leafPort)) {
        throw VC::ConfigException("Route " + src + " -> " + dest +
            " has configuration data from XML and from configuration API.");
      }
    } else if (!VC::Routing::has(pid) && !VC::Routing::has(leafPort)) {
      throw VC::ConfigException("Route " + src + " -> " + dest +
          " has NO configuration data at all.");
    } else if (VC::Routing::has(pid)) {
      VC::Route::Ptr route = VC::Routing::get(pid);
      VC::Routing::set(leafPort, route);
    } else if (VC::Routing::has(leafPort)) {
      VC::Route::Ptr route = VC::Routing::get(leafPort);
      VC::Routing::set(pid, route);
      route->inject(src, dest);
    }

    assert(VC::Routing::has(pid) && VC::Routing::has(leafPort));
    assert(VC::Routing::get(pid) == VC::Routing::get(leafPort));
  }

  void finalizeMapping(std::string actorName,
      const FunctionNames &actionNames,
      const FunctionNames &guardNames)
  {
    VC::VpcTask::Ptr task = VC::getCachedTask(actorName);
    assert(VC::Mappings::getConfiguredMappings().find(task) != VC::Mappings::getConfiguredMappings().end());
    VC::Component::Ptr configComponent = VC::Mappings::getConfiguredMappings()[task];
    assert(VC::Mappings::getComponents().find(configComponent) != VC::Mappings::getComponents().end());
    AbstractComponent * comp = VC::Mappings::getComponents()[configComponent];
    Director::getInstance().registerMapping(actorName.c_str(),
        comp->getName());

    //generate new ProcessControlBlock or get existing one for
    // initialization
    ProcessControlBlock& pcb =
      comp->createPCB(Director::getInstance().getProcessId(actorName));
    pcb.configure(actorName.c_str(), true, true);

    //TODO: VC::Timing -> Timing
    const VC::Components & components = VC::getComponents();
    BOOST_FOREACH(VC::Components::value_type component_pair, components)
    {
      std::string componentName = component_pair.first;
      VC::Component::Ptr component = component_pair.second;

      if (VC::Mappings::isMapped(task, component)) {
        VC::TimingsProvider::Ptr provider = component->getTimingsProvider();
        pcb.setPriority(task->getPriority());  // GFR BUGFIX
        BOOST_FOREACH(std::string guard, guardNames)
        {
          if (provider->hasGuardTiming(guard)) {
            pcb.setTiming(provider->getGuardTiming(guard));
            ConfigCheck::configureTiming(pcb.getPid(), guard);
          }
        }
        BOOST_FOREACH(std::string action, actionNames)
        {
          if (provider->hasActionTiming(action)) {
            pcb.setTiming(provider->getActionTiming(action));
            ConfigCheck::configureTiming(pcb.getPid(), action);
          }
        }
      }
    }
//TODO:
//    p.setPeriod(1);
//    p.setBaseDelay();
//    p.setBaseLatency();

  }

  //
  AbstractComponent * createComponent(VC::Component::Ptr component)
  {
    AbstractComponent *comp = NULL;
    switch (component->getScheduler()) {
      case VC::Scheduler::FCFS:
        comp = new FcfsComponent(component);
        break;
      case VC::Scheduler::StaticPriority_NP:
        comp = new PriorityComponent(component);
        break;
      default:
        comp = new Component(component);
    }

    //TODO: process attributes
    //       - processPower()
    VC::Mappings::getComponents()[component] = comp;
    Director::getInstance().registerComponent(comp);
    std::vector<AttributePtr> atts = component->getAttributes();
    for(std::vector<AttributePtr>::const_iterator iter = atts.begin();
        iter != atts.end(); ++iter){
      comp->setAttribute(*iter);
    }
    return comp;
  }

  //
  void Director::beforeVpcFinalize()
  {
    // create AbstractComponents and configure mappings given from config API
    const VC::Components & components = VC::getComponents();

    BOOST_FOREACH(VC::Components::value_type component_pair, components) {
      std::string componentName = component_pair.first;
      VC::Component::Ptr component = component_pair.second;
      assert(componentName == component->getName());

      VC::Component::MappedTasks tasks = component->getMappedTasks();

      AbstractComponent *comp = createComponent(component);
      assert(comp != NULL);

      BOOST_FOREACH(const ScheduledTask* task, tasks) {
        VC::Mappings::getConfiguredMappings()[VC::getCachedTask(*task)] = component;
      }
    }
  }
  /// end section: VpcApi.hpp related stuff

//
void Director::endOfVpcFinalize()
{
  if (checkVpcConfig) {
    ConfigCheck::check();
  }
}

  FastLink Director::registerActor(ScheduledTask * actor,
      std::string actorName,
      const FunctionNames &actionNames,
      const FunctionNames &guardNames)
  {
    //TODO: check if this is really required.
    if(FALLBACKMODE) return FastLink();

    try {
      injectTaskName(actor, actorName);
      finalizeMapping(actorName, actionNames, guardNames);
    }catch(std::exception & e){
      std::cerr << "Actor registration failed for \"" << actorName <<
          "\". Got exception:\n" << e.what() << std::endl;
      exit(-1);
    }

    ProcessId       pid = getProcessId(  actorName  );
    FunctionIds     functionIds;

    //TODO: move this to finalizeMappings ??
    FunctionNames functionNames;
    functionNames.insert(functionNames.end(), actionNames.begin(), actionNames.end());
    functionNames.insert(functionNames.end(), guardNames.begin(), guardNames.end());

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
    std::string destination,
    sc_port_base * leafPort)
  {
    //TODO: check if this is really required.
    if(FALLBACKMODE) return FastLink();

    ProcessId       pid = getProcessId( source, destination );
    FunctionIds     fids; // empty functionIds
    fids.push_back( getFunctionId("1") );

    if (!VC::Routing::has(pid) && !VC::Routing::has(leafPort) && defaultRoute) {
      // default behavior: add empty route
      VC::Route::Ptr route = VC::createRoute(source, destination);
      route->setTracing(false);
    }

    try{
      injectRoute(source, destination, leafPort);
    }catch(std::exception & e){
      std::cerr << "Route registration failed for route\"" << source << " - " <<
          destination <<
          "\". Got exception:\n" << e.what() << std::endl;
      exit(-1);
    }

    assert( !taskPool.contains(pid) );

    VC::Route::Ptr configuredRoute = VC::Routing::get(pid);
    Route * route = VC::Routing::create(configuredRoute);

    route->enableTracing(false);
    Director::registerRoute(route);

    return FastLink(pid, fids);
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
          iter = ConfigCheck::routeNames().begin();
        iter != ConfigCheck::routeNames().end();
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
          ConfigCheck::processNames().begin();
        iter != ConfigCheck::processNames().end();
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

  //
  std::string Director::getTaskName(ProcessId id) {
    if(ConfigCheck::hasProcessName(id)){
      return ConfigCheck::getProcessName(id);
    }else{
      return "Route from " + ConfigCheck::getRouteName(id).first +
        " to: " + ConfigCheck::getRouteName(id).second;
    }
  }
}

