#ifndef HSCD_VPC_PROCESSCONTROLBLOCK_H_
#define HSCD_VPC_PROCESSCONTROLBLOCK_H_

#include <systemc.h>
#include <float.h>
#include <map>
#include <vector>
#include <string>

#include <CoSupport/SystemC/systemc_support.hpp>

#include "hscd_vpc_EventPair.h"
#include "hscd_vpc_Tracing.h"
#include "FastLink.h"

namespace SystemC_VPC {

  class Director;

  class PCBPool;

  typedef size_t ComponentId;

  /**
   * Internal helper class to manage delays of components which an
   * associated PCB can run on.
   */
  class DelayMapper{
  public:

    ~DelayMapper();

    explicit DelayMapper(const DelayMapper& dm);

    DelayMapper();

    /**
     * Internal helper class to enable management of component specific
     * delays of a task, additional function specific delays on an associated
     * component are managed.
     */
    class ComponentDelay{

    public:

      /**
       * \brief Default constuctor of an ComponentDealy instance
       * \param name specifies the name of the associated component
       * simulation
       */
      ComponentDelay( );

      /**
       * copy constructor
       */
      ComponentDelay( const ComponentDelay &delay );

      /**
       * \brief Adds a new function delay to the instance
       * \param funcname specifies the associated function
       * \param delay is the corresponding delay for the function execution
       */
      void addDelay(FunctionId fid, sc_time delay);

      /**
       * \brief Set the base delay to the instance
       * \param funcname specifies the associated function
       * \param delay is the corresponding delay for the function execution
       */
      void setBaseDelay(sc_time delay);
      sc_time getBaseDelay( ) const;

      /**
       * \brief Used to access delay
       * \param funcname specifies a possible function if given
       * \return delay required for a function execution  on the associated
       * component of the process. If no function name is given or there is
       * no corresponding entry registered the default delay is returned.
       */
      sc_time getDelay(FunctionId fid) const;

      /**
       * \brief Adds a new function latency to the instance
       * \param funcname specifies the associated function
       * \param latency is the corresponding latency for the function
       * execution
       */
      void addLatency(FunctionId fid, sc_time latency);

      /**
       * \brief Set the base latency to the instance
       * \param latency is the corresponding latency for the function
       * execution
       */
      void setBaseLatency(sc_time latency);
      sc_time getBaseLatency( ) const;

      /**
       * \brief Used to access latency
       * \param funcname specifies a possible function if given
       * \return latency required for a function execution  on the
       * associated component of the process. If no function name is given
       * or there is no corresponding entry registered the default latency
       * is returned.
       */
      sc_time getLatency(FunctionId fid) const;

    private:

      typedef std::vector<sc_time> FunctionTimes;
      // map of possible special delay depending on functions
      FunctionTimes funcDelays;

      // map of possible function specific latencies
      FunctionTimes funcLatencies;
    };

  };

 /**
  * This class represents all necessary data of a simulated process within VPC
  * and provides necessary access methods for its data.
  */
  class ProcessControlBlock :
    //    public DelayMapper,
    public DelayMapper::ComponentDelay {

    public:
      
      /**
       * \brief Default constructor of an PCB instance
       */
      ProcessControlBlock( PCBPool *parent );
      
      /**
       * \brief Default constructor of an PCB instance
       * \param name specifies the identifying name of the instance
       */
      ProcessControlBlock(std::string name);

      ~ProcessControlBlock();

      /**
       * \brief Copy Contstructor
       */
      ProcessControlBlock(const ProcessControlBlock& pcb);
    
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

      /**
       * \brief Sets interrupt event of PCB instance
       */
      void setInterrupt(sc_event* interrupt);

      /**
       * \brief Gets interrupt event of PCB instance
       */
      sc_event* getInterrupt() const;

      /**
       * \brief Sets block event of PCB instance
       */
      void setBlockEvent(EventPair blockEvent);

      /**
       * \brief Gets block event of PCB instance
       */
      EventPair getBlockEvent() const;

      /**
       * \brief Sets current associated delay of instance
       */
      void setDelay(sc_time delay);

      /**
       * \brief Gets currently associated delay of PCB instance
       */
      sc_time getDelay() const;

      /**
       * \brief Sets current associated latency of instance
       */
      void setLatency(sc_time latency);

      /**
       * \brief Gets currently associated latency of PCB instance
       */
      sc_time getLatency() const;

      /**
       * \brief Sets currently remaining delay of PCB instance
       */
      void setRemainingDelay(sc_time delay);

      /**
       * \brief Gets currently remaining delay of PCB instance
       */
      sc_time getRemainingDelay() const;
      
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

      void release();

    private:

      /**
       * Internal helper class to count the activation of a
       * ProcessControlBlock instance,
       * more precisely used with PCBPool instance type.
       */
      class ActivationCounter{
        public:

          ActivationCounter();

          /**
           * \brief increments activation count
           */
          void increment();

          /**
           * \brief Used to access activation count
           */
          unsigned int getActivationCount();

        private:

          unsigned int activation_count;

      };

      static int globalInstanceId;

      std::string name;
      ProcessId   pid;
      FunctionId  fid;

      int instanceId;
      sc_event* interrupt;
      EventPair blockEvent;
      sc_time delay;
      
      /**
       * \brief For pipelining usage!
       * This is not exact the latency, but it is "real_latency - delay",
       * in fact.
       * While real_latency is given within configuration file.
       */
      sc_time latency;
      sc_time remainingDelay;
      int priority;
      sc_time period;
      sc_time deadline;
      Tracing * traceSignal;


      /**
       * variables common to pcb type
       */
      
      ActivationCounter* activationCount; //activation_count;
      int* copyCount;

      // reference to "parent" PCBPool
      PCBPool *parentPool;

    private:

      /**
       * \brief Initializes a newly created intance of ProcessControlBlock
       */
      void init();
 
  };

  struct p_queue_entry{
    int fifo_order;  // sekundärstrategie
    ProcessControlBlock *pcb;
  };

  struct p_queue_compare{
    bool operator()(const p_queue_entry& pqe1,
        const p_queue_entry& pqe2) const
    {
      int p1=pqe1.pcb->getPriority();
      int p2=pqe2.pcb->getPriority();
      if (p1 > p2)
        return true;
      else if(p1 == p2)
        return (pqe1.fifo_order>pqe2.fifo_order);
      else
        return false;
    }

  };

  struct rm_queue_compare{
    bool operator()(const p_queue_entry& pqe1,
        const p_queue_entry& pqe2) const
    {
      double p1 = sc_time(1,SC_NS)/pqe1.pcb->getPeriod();
      double p2 = sc_time(1,SC_NS)/pqe2.pcb->getPeriod();
      //double p1=pqe1.pcb->getPriority()/pqe1.pcb->getPeriod();
      //double p2=pqe2.pcb->getPriority()/pqe2.pcb->getPeriod();
      if (p1 > p2)
        return true;
      else if(p1 == p2)
        return (pqe1.fifo_order>pqe2.fifo_order);
      else
        return false;
    }

  };

  typedef std::map<int,ProcessControlBlock*>  PCBMap;
}
#endif // HSCD_VPC_PROCESSCONTROLBLOCK_H_
