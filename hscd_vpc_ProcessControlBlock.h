#ifndef HSCD_VPC_PROCESSCONTROLBLOCK_H_
#define HSCD_VPC_PROCESSCONTROLBLOCK_H_

#include <systemc.h>
#include <float.h>
#include <map>
#include <set>
#include <string>

#include <cosupport/systemc_support.hpp>

#include "hscd_vpc_EventPair.h"
#include "hscd_vpc_Tracing.h"

namespace SystemC_VPC {
 
  class BindingGraph; //MappingInformation;

  enum activation_state {
    inaktiv,
    starting,
    aktiv,
    ending,
    aborted
  };

 /**
  * This class represents all necessary data of a simulated process within VPC
  * and provides necessary access methods for its data.
  */
  class ProcessControlBlock{

    private:
    
      /**
       * Internal helper class to count the activation of a ProcessControlBlock instance,
       * more precisely used with PCBPool instance type.
       */
      class ActivationCounter{

        private:

          unsigned int activation_count;

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

      };

      static int global_pid;

      std::string name;
      const char* funcname;
      int pid;
      sc_event* interrupt;
      EventPair blockEvent;
      sc_time delay;
      
      /**
       * \brief For pipelining usage!
       * This is not exact the latency, but it is "real_latency - delay", in fact.
       * While real_latency is given within configuration file.
       */
      sc_time latency;
      sc_time remainingDelay;
      int priority;
      sc_time period;
      sc_time deadline;
      activation_state state;
      Tracing * traceSignal;


      /**
       * variables common to pcb type
       */
      
      ActivationCounter* activationCount; 
			BindingGraph* bGraph;
      int* copyCount;

    public:
      
      /**
       * \brief Default constructor of an PCB instance
       */
      ProcessControlBlock();
      
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

      /**
       * \brief Set process id of instance
       */
      void setPID(int pid);

      /**
       * \brief Used to access process id
       */
      int getPID() const;
      
      /**
       * \brief Sets currently associated function name of process
       */
      void setFuncName(const char* funcname);

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

      void setState(activation_state state);

      activation_state getState() const;
      
      void setTraceSignal(Tracing* signal);

      Tracing* getTraceSignal();

			BindingGraph& getBindingGraph();

    private:

      /**
       * \brief Initializes a newly created intance of ProcessControlBlock
       */
      void init();
 
  };

  struct p_queue_entry{
    int fifo_order;  // sekund?rstrategie
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

}
#endif // HSCD_VPC_PROCESSCONTROLBLOCK_H_
