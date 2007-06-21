#ifndef HSCD_VPC_PROCESSCONTROLBLOCK_H_
#define HSCD_VPC_PROCESSCONTROLBLOCK_H_

#include <systemc.h>
#include <float.h>
#include <map>
#include <string>

#include <cosupport/systemc_support.hpp>

#include "hscd_vpc_EventPair.h"
#include "hscd_vpc_Tracing.h"

namespace SystemC_VPC {
 
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

      /**
       * \brief Used to register component specific delay to PCB instance
       * \param comp specifies associated component
       * \param funcname refers to an function name or may be null for common
       * association only to a component
       * \param delay represents the execution delay needed for execution  on
       * specified component
       */
      void addFuncDelay(std::string comp, const char* funcname, sc_time delay);

      /**
       * \brief Used to access delay of a PCB instance on a given component
       * \param comp specifies the associated component
       * \param funcname refers to an optional function name
       */
      sc_time getFuncDelay(std::string comp, const char* funcname=NULL) const;

      /**
       * \brief Used to register component specific latency to PCB instance
       * \param comp specifies associated component
       * \param funcname refers to an function name or may be null for common
       * association only to a component
       * \param latency represents the execution latency needed for execution
       * on specified component
       */
      void addFuncLatency( std::string comp, const char* funcname,
                           sc_time latency);

      /**
       * \brief Used to access latency of a PCB instance on a given component
       * \param comp specifies the associated component
       * \param funcname refers to an optional function name
       */
      sc_time getFuncLatency( std::string comp,
                              const char* funcname=NULL) const;

    private:
      
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
           * \param base_delay specifies the standard delay used for execution
           * simulation
           */
          ComponentDelay(std::string name);

          /**
           * \brief Adds a new function delay to the instance
           * \param funcname specifies the associated function
           * \param delay is the corresponding delay for the function execution
           */
          void addDelay(const char* funcname, sc_time delay);
          
          /**
           * \brief Used to access delay
           * \param funcname specifies a possible function if given
           * \return delay required for a function execution  on the associated
           * component of the process. If no function name is given or there is
           * no corresponding entry registered the default delay is returned.
           */
          sc_time getDelay(const char* funcname=NULL) const;

          /**
           * \brief Adds a new function latency to the instance
           * \param funcname specifies the associated function
           * \param latency is the corresponding latency for the function
           * execution
           */
          void addLatency(const char* funcname, sc_time latency);
          
          /**
           * \brief Used to access latency
           * \param funcname specifies a possible function if given
           * \return latency required for a function execution  on the
           * associated component of the process. If no function name is given
           * or there is no corresponding entry registered the default latency
           * is returned.
           */
          sc_time getLatency(const char* funcname=NULL) const;

        private:
         
          // name of the associated component 
          std::string name;
          // base delay used for task running on this component
          sc_time base_delay;
          // map of possible special delay depending on functions
          std::map<std::string, sc_time> funcDelays;
          // base latency used for tasks running on this component
          sc_time base_latency;
          // map of possible function specific latencies
          std::map<std::string, sc_time> funcLatencies;      
      };
      
      /**
       * Internal helper class to manage delays of components which an
       * associated PCB can run on.
       */
      class DelayMapper{
        public:
        
        ~DelayMapper();
        
        /**
         * \brief Registers new special function delay to the mapping instance
         * This method registers a new function delay to the mapping instance.
         * if there is no currently associated management entry for the given
         * component, a new entry is created with the given delay additionally
         * as standard delay.
         * \param comp specifies the associated component
         * \param funcname represents the function name
         * \param delay is the given function delay
         */
        void addFuncDelay(std::string comp,
                          const char* funcname,
                          sc_time delay);

        /**
         * \brief Used to access the delay
         * \param comp specifies the requested component
         * \param funcname optionally refers to the function name
         * \return delay for a requested component and the optionally given
         *  function name.
         * If there is no value found 0 is returned as default.
         */
        sc_time getFuncDelay(std::string comp,
                             const char* funcname=NULL) const;
        
        /**
         * \brief Registers special function latency to the mapping instance
         * This method registers a function latency to the mapping instance.
         * if there is no currently associated management entry for the given
         * component, a new entry is created with the given latency
         * additionally as standard latency.
         * \param comp specifies the associated component
         * \param funcname represents the function name
         * \param latency is the given function latency
         */
        void addFuncLatency( std::string comp,
                             const char* funcname,
                             sc_time latency );

        /**
         * \brief Used to access the latency
         * \param comp specifies the requested component
         * \param funcname optionally refers to the function name
         * \return latency for a requested component and the optionally given
         * function name.
         * If there is no value found 0 is returned as default.
         */
        sc_time getFuncLatency(std::string comp,
                               const char* funcname=NULL) const;
        
      private:
        /*
         * Replaced by new version to enable multiple
         * delays for different components on different 
         * functions
         */
        std::map<std::string, ComponentDelay*> compDelays;

      };

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

      static int global_pid;

      std::string name;
      const char* funcname;
      int pid;
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
      activation_state state;
      Tracing * traceSignal;


      /**
       * variables common to pcb type
       */
      
      ActivationCounter* activationCount; //activation_count;
      DelayMapper* dmapper;
      int* copyCount;

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
