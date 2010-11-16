/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * AbstractComponent.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef HSCD_VPC_ABSTRACTCOMPONENT_H
#define HSCD_VPC_ABSTRACTCOMPONENT_H

#include <vector>
#include <map>

#include <systemc.h>

#include <string.h>

#include <CoSupport/SystemC/systemc_support.hpp>

#include "datatypes.hpp"
#include "Delayer.hpp"
#include "ProcessControlBlock.hpp"
#include "FunctionTimingPool.hpp"
#include "PCBPool.hpp"
#include "Task.hpp"
#include "PowerSumming.hpp"
#include "PowerMode.hpp"
#include "PluggablePowerGovernor.hpp"
#include "ComponentInfo.hpp"
#include "ComponentModel.hpp"
#include "Attribute.hpp"
#include "FunctionTimingPool.hpp"

namespace SystemC_VPC{

class ComponentObserver;

  using CoSupport::SystemC::Event;
  using CoSupport::SystemC::RefCountEventPtr;

  typedef std::map<ComponentState, double> PowerTable;
  typedef std::map<const PowerMode*, PowerTable>  PowerTables;


  /**
   * \brief The interface of a Virtual-Processing-Component (VPC).
   */
  class AbstractComponent:
    public sc_module, public Delayer, public ComponentModel {
  
  public:

    virtual ~AbstractComponent(){
#ifndef NO_VCD_TRACES
      for(std::map<std::string, Tracing* >::iterator iter
          = trace_map_by_name.begin();
          iter != trace_map_by_name.end();
          ++iter){
        delete iter->second;
      }
      trace_map_by_name.clear();
      sc_close_vcd_trace_file(traceFile);
#endif //NO_VCD_TRACES

      this->timingPools.clear();
    }

    /**
     * \brief Set parameter for Component and Scheduler.
     */
    virtual bool setAttribute(AttributePtr attributePtr);

    const char* getName() const;

    /**
     * \brief Create the process control block.
     */
    ProcessControlBlock& createPCB(ProcessId pid){
      PCBPool& pool = this->getPCBPool();
      if(pool.find(pid) == pool.end()){
        pool[pid].reset(new ProcessControlBlock( this ));
        pool[pid]->setPid(pid);
      }
      return *(pool[pid]);
    }

    /**
     *
     */
    PCBPool& getPCBPool(){
      return this->pcbPool;
    }
  protected:

    std::map<const PowerMode*, sc_time> transactionDelays;

#ifndef NO_VCD_TRACES
    sc_trace_file *traceFile;
    std::map<std::string, Tracing* > trace_map_by_name;
    sc_signal<trace_value> schedulerTrace;
#endif //NO_VCD_TRACES
  
  public:
  
    AbstractComponent(sc_module_name name)
      : sc_module(name),
        Delayer(),
#ifndef NO_VCD_TRACES
        traceFile(NULL),
#endif //NO_VCD_TRACES
        powerMode(NULL),
        localGovernorFactory(NULL),
        midPowerGov(NULL),
        powerAttribute(new Attribute("",""))
    {
      if(powerTables.find(getPowerMode()) == powerTables.end()){
        powerTables[getPowerMode()] = PowerTable();
      }

      PowerTable &powerTable=powerTables[getPowerMode()];
      powerTable[ComponentState::IDLE]    = 0.0;
      powerTable[ComponentState::RUNNING] = 1.0;
    }
            
    /**
       * \brief Simulate an execution on this "Virtual Component".
       *
       * While this simulation is running SystemC simulation time is consumed.
       */
    virtual void compute(Task* task)=0;

    /**
     *
     */
    virtual void requestBlockingCompute(Task* task, RefCountEventPtr blocker)=0;

    /**
     *
     */
    virtual void execBlockingCompute(Task* task, RefCountEventPtr blocker)=0;

    /**
     *
     */
    virtual void abortBlockingCompute(Task* task, RefCountEventPtr blocker)=0;

    /**
     * 
     */
    virtual void updatePowerConsumption() = 0;

    /**
     * 
     */
    void setPowerMode(const PowerMode* mode){
      this->powerMode = translatePowerMode(mode->getName());
      this->updatePowerConsumption();

      if(timingPools.find(powerMode) == timingPools.end()){
        timingPools[powerMode].reset(new FunctionTimingPool());
      }
      this->timingPool = timingPools[powerMode];
    }

    const PowerMode* getPowerMode() const {
      return this->powerMode;
    }

    /**
     *
     */
    FunctionTimingPtr getTiming(const PowerMode *mode, ProcessId pid){
      if(timingPools.find(mode) == timingPools.end()){
        timingPools[mode].reset(new FunctionTimingPool());
      }
      FunctionTimingPoolPtr pool = this->timingPools[mode];
      if(pool->find(pid) == pool->end()){
        (*pool)[pid].reset(new FunctionTiming());
        (*pool)[pid]->setBaseDelay(this->transactionDelays[mode]);
      }
      return (*pool)[pid];
    }

#ifndef NO_VCD_TRACES
    Tracing * addToTraceFile(std::string name){
      if (this->traceFile == NULL){
        std::string tracefilename=this->getName(); //componentName;

        char* traceprefix= getenv("VPCTRACEFILEPREFIX");
        if(0!=traceprefix){
          tracefilename.insert(0,traceprefix);
        }

        this->traceFile = sc_create_vcd_trace_file(tracefilename.c_str());
        this->traceFile->set_time_unit(1, SC_NS);
      }
      Tracing *newsignal = new Tracing(name, this->getName());

      this->trace_map_by_name.insert(std::pair<std::string, Tracing*>(
          this->getName(), newsignal));
      sc_trace(this->traceFile, *newsignal->traceSignal, name);
      newsignal->traceSleeping();
      return newsignal;
    }
#endif //NO_VCD_TRACES

  private:

    bool processPower(AttributePtr att);

    void loadLocalGovernorPlugin(std::string plugin);


    /**
     *
     */
    PCBPool pcbPool;
    FunctionTimingPoolPtr timingPool;
    std::map<const PowerMode*, FunctionTimingPoolPtr> timingPools;
    const PowerMode *powerMode;

  protected:
    PlugInFactory<PluggableLocalPowerGovernor> *localGovernorFactory;
    PluggableLocalPowerGovernor *midPowerGov;
    AttributePtr powerAttribute;
    typedef std::map<std::string,
                     DLLFactory<PlugInFactory<PluggableLocalPowerGovernor> >* >
      Factories;
    static Factories factories;
    PowerTables powerTables;
    };
}

#endif //HSCD_VPC_ABSTRACTCOMPONENT_H
