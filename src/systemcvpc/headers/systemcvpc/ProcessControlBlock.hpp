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

#ifndef HSCD_VPC_PROCESSCONTROLBLOCK_H_
#define HSCD_VPC_PROCESSCONTROLBLOCK_H_

#include <systemc.h>
#include <float.h>
#include <map>
#include <vector>
#include <string>

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TaskTracer.hpp>

#include "EventPair.hpp"
#include "datatypes.hpp"
#include "FastLink.hpp"
#include "PowerMode.hpp"
#include "config/Timing.hpp"
#include "FunctionTimingPool.hpp"

namespace SystemC_VPC {

namespace Trace{
  class Tracing;
}

  class AbstractComponent;

  typedef size_t ComponentId;

 /**
  * This class represents all necessary data of a simulated process within VPC
  * and provides necessary access methods for its data.
  */
  class ProcessControlBlock {

    public:
    friend class Task;
      /**
       * \brief Default constructor of an PCB instance
       */
      ProcessControlBlock( AbstractComponent * component );


      /**
       * \brief Sets name of instance
       */
      void configure(std::string name, bool tracing);

      /**
       * \brief Used to access name of PCB
       */
      std::string const& getName() const;

      void setPid( ProcessId pid);
      ProcessId getPid( ) const;
      
      /**
       * \brief Sets currently associated function id of process
       */
      void setFunctionId( FunctionId fid);
      FunctionId getFunctionId( ) const;
      
      /**
       * \brief due to pipelining, there may be several instances of a process
       */
      int getInstanceId() const;
      

      /**
       * \brief Gets currently associated function name of PCB instance
       */
      const char* getFuncName() const;

      void setPeriod(sc_time period);

      sc_time getPeriod() const;
      
      void setPriority(int priority);

      int getPriority() const;

      void setDeadline(sc_time deadline);

      sc_time getDeadline() const;

      /**
       * \brief Used to increment activation count of this PCB instance
       */
      void incrementActivationCount();

      unsigned int getActivationCount() const;

      void setTraceSignal(Trace::Tracing* signal);

      Trace::Tracing* getTraceSignal();

      void setTiming(const Config::Timing& timing);
      void setBaseDelay(sc_time delay);
      void setBaseLatency(sc_time latency);
      void addDelay(FunctionId fid, sc_time delay);
      void addLatency(FunctionId fid, sc_time latency);

      void setActorAsPSM(bool psm);
      bool isPSM();

    private:

      std::string name;
      ProcessId   pid;
      FunctionId  fid;

      int priority;
      sc_time period;
      sc_time deadline;
      Trace::Tracing * traceSignal;
      AbstractComponent * component;
      CoSupport::Tracing::TaskTracer::Ptr taskTracer;
      bool psm;
      /**
       * \brief Initialize a newly created instance of ProcessControlBlock
       */
      void init();
 
  };
typedef boost::shared_ptr<ProcessControlBlock> ProcessControlBlockPtr;
}
#endif // HSCD_VPC_PROCESSCONTROLBLOCK_H_