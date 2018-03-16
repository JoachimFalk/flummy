// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SYSTEMCVPC_DIRECTOR_HPP
#define _INCLUDED_SYSTEMCVPC_DIRECTOR_HPP

#include <systemcvpc/vpc_config.h>
#include "FastLink.hpp"
#include "EventPair.hpp"
#include "ScheduledTask.hpp"
#include "InvalidArgumentException.hpp"
#include "Attribute.hpp"

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <memory>

#include <stdio.h>

#include <boost/function.hpp>

template <class T> class DLLFactory;

namespace SystemC_VPC{

  typedef std::vector<std::string> FunctionNames;

  class PowerSumming;
  class Delayer;
  class Route;
  class PluggableGlobalPowerGovernor;

  template <class T> class PlugInFactory;

  template<typename KEY, class OBJECT>
  class AssociativePrototypedPool;

  class Task;
  typedef AssociativePrototypedPool<ProcessId, Task> TaskPool;

  /**
   * \brief Director knows all (Abstract-)Components, all mappings (task -> component).
   *
   * Director reads allocation and binding from file.
   */
  class Director {
    friend class RoundRobinComponent;
    friend class NonPreemptiveComponent;
  public:
    bool FALLBACKMODE;
    bool defaultRoute;
    bool checkVpcConfig;

    /**
     * \brief Access to singleton Director. 
     */
    static Director &getInstance() {
      if (!singleton.get())
        singleton.reset(new Director());
      return *singleton;
    }

    /**
     * end_of_elaboration call back
     * called from SysteMoC in order to cleanup/delete VPC objects
     */
    static void endOfSystemcSimulation(){
      delete singleton.release();
    }

    ~Director();

    /**
     * \brief Simulates timing simulations of guard checks.
     *
     * Determines the component from FastLink.
     * \param fLink FastLink for transition to evaluate its guard.
     */
    void check(FastLink const *fLink);

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
    void compute(FastLink const *fLink,
                 EventPair endPair = EventPair(NULL, NULL));

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
    void read(FastLink const *fLink,
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
    void write(FastLink const *fLink,
               size_t quantum,
               EventPair endPair = EventPair(NULL, NULL) );

    /**
     * \brief Register component to Director
     * Used to register a component to the Director for
     * later computation of task on it. The components name
     * is used as identifier for it.
     * \param comp points to component instance to be registered
     */
    void registerComponent(Delayer *comp);
    
    /**
     * \brief Register mapping between task and component to Director
     * \param taskName specifies name of task
     * \param compName specifies name of component
     */
    void registerMapping(
        const std::string &taskName,
        const std::string &compName);

    /**
     * \brief Register a communication route.
     * \param route the route
     */
    void registerRoute(Route *route);

    /**
     * \brief resolve mapping
     */
    const Delayer *getComponent(FastLink const *vpcLink) const ;
    
    void signalLatencyEvent(Task* task);

    void setResultFile(std::string vpc_result_file){
      this->vpc_result_file = vpc_result_file;
      remove(vpc_result_file.c_str());
    }
    
    std::string getResultFile(){
      return this->vpc_result_file;
    }

    static ProcessId getProcessId(std::string process_or_source,
        std::string destination = "");

    ComponentId getComponentId(std::string component);

    FastLink registerActor(TaskInterface * actor,
                             std::string actorName,
                             const FunctionNames& actionNames,
                             const FunctionNames& guardNames,
                             const int complexity);

    FastLink registerRoute(std::string source, std::string destination,
        sc_core::sc_port_base * leafPort);

    /**
     * \brief Takes a string representation of a time (e.g. a delay) and constructs a sc_core::sc_time object.
     */
    static sc_core::sc_time createSC_Time(const char* timeString) throw(InvalidArgumentException);

    /**
     * \brief Takes a string representation of a time (e.g. a delay) and constructs a sc_core::sc_time object.
     */
    static sc_core::sc_time createSC_Time(std::string timeString) throw(InvalidArgumentException);
    
    std::vector<ProcessId> * getTaskAnnotation(std::string compName);

    static bool hasFunctionId(const std::string& function);
    static FunctionId getFunctionId(const std::string& function);
    static FunctionId createFunctionId(const std::string& function);

    Task *allocateTask(ProcessId pid);

    // FIXME !!!
    PluggableGlobalPowerGovernor   *topPowerGov;
    DLLFactory<PlugInFactory<PluggableGlobalPowerGovernor> >
                                   *topPowerGovFactory;
    void loadGlobalGovernorPlugin(std::string plugin, AttributePtr attPtr);

    std::string getTaskName(ProcessId id);
    
    static sc_core::sc_time getEnd() {
      return end;
    }

    void beforeVpcFinalize();
    void endOfVpcFinalize();
    bool hasValidConfig() const;
  private:

    Task *preCompute(FastLink const *fLink);

    void  postCompute(Task *task, EventPair endPair);

    void debugUnknownNames( ) const;

    void assertMapping(ProcessId const pid);

    void finalizeMapping(
         TaskInterface       *actor,
         std::string   const &actorName,
         FunctionNames const &actionNames,
         FunctionNames const &guardNames);

    /**
     * Singleton design pattern
     */
    static std::unique_ptr<Director> singleton;

    /**
     * \brief Reads allocation and binding from file.
     */
    Director();

    typedef std::vector<Delayer* >  Components;
    Components                      components;
    
    typedef std::vector<Delayer* >  Mappings;
    Mappings                        mappings;

    typedef std::vector<ProcessId>                ProcessList;  
    typedef std::map<ComponentId, ProcessList *>  ReverseMapping;
    ReverseMapping                                reverseMapping;

    // output file to write result to
    std::string vpc_result_file;
    
    // time of latest acknowledge simulated task
    static sc_core::sc_time end;


    typedef std::map<std::string, ComponentId> ComponentIdMap;
    ComponentIdMap                             componentIdMap;

    std::map<ProcessId, std::set<std::string> > debugFunctionNames;

#ifndef NO_POWER_SUM
    std::ofstream  powerConsStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM

    TaskPool        *taskPool;
  };

}
#endif /* _INCLUDED_SYSTEMCVPC_DIRECTOR_HPP */
