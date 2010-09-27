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
#include "Tracing.hpp"
#include "datatypes.hpp"
#include "FastLink.hpp"
#include "PowerMode.hpp"
#include "Timing.hpp"
#include "FunctionTimingPool.hpp"

namespace SystemC_VPC {

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
      void setName(std::string name);

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

      void setTraceSignal(Tracing* signal);

      Tracing* getTraceSignal();

      void setTiming(const Timing& timing);
      void setBaseDelay(sc_time delay);
      void setBaseLatency(sc_time latency);
      void addDelay(FunctionId fid, sc_time delay);
      void addLatency(FunctionId fid, sc_time latency);

    private:

      typedef std::map<PowerMode, FunctionTiming*> FunctionTimings;
      FunctionTimings functionTimings;

      std::string name;
      ProcessId   pid;
      FunctionId  fid;

      int priority;
      sc_time period;
      sc_time deadline;
      Tracing * traceSignal;
      AbstractComponent * component;
      CoSupport::Tracing::TaskTracer::Ptr taskTracer;
      /**
       * \brief Initialize a newly created instance of ProcessControlBlock
       */
      void init();
 
  };
typedef boost::shared_ptr<ProcessControlBlock> ProcessControlBlockPtr;
}
#endif // HSCD_VPC_PROCESSCONTROLBLOCK_H_
