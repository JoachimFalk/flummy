#ifndef HSCD_VPC_PROCESSCONTROLBLOCK_H_
#define HSCD_VPC_PROCESSCONTROLBLOCK_H_

#include <systemc.h>
#include <float.h>
#include <map>
#include <string>

#include <systemc_support.hpp>

#include "hscd_vpc_EventPair.h"

namespace SystemC_VPC {
 
  enum activation_state {
    inaktiv,
    starting,
    aktiv,
    ending,
    aborted
  };

  typedef char trace_value;

 
 /**
  * This class represents all necessary data of a simulated process within VPC
  * and provides necessary access methods for its data.
  */
  class ProcessControlBlock{

    private:
      
      /**
       * Internal helper class to enable management of component specific
       * delays of a task, additional function specific delays on an associated component
       * are managed.
       */
      class ComponentDelay{

        private:
         
          // name of the associated component 
          std::string name;
          // base delay used for task running on this component
          double base_delay;
          // map of possible special delay depending on functions
          std::map<std::string, double> funcDelays;

        public:

          /**
           * \brief Default constuctor of an ComponentDealy instance
           * \param name specifies the name of the associated component
           * \param base_delay specifies the standard delay used for execution simulation
           */
          ComponentDelay(std::string name, double base_delay);

          /**
           * \brief Adds a new function delay to the instance
           * \param funcname specifies the associated function
           * \param delay is the corresponding delay for the function execution
           */
          void addDelay(const char* funcname, double delay);
          
          /**
           * \brief Used to access delay
           * \param funcname specifies a possible function if given
           * \return delay required for a function execution  on the associated component
           * of the process. If no function name is given or there is no corresponding 
           * entry registered the default delay is returned.
           */
          double getDelay(const char* funcname=NULL);

          /**
           * \brief Tests if an specific function delay exisits
           * \param funcname specifies name of the requested function delay
           * \returns true if a specific function delay has been found
           * else false
           */
          bool hasDelay(const char* funcname);
      
      };
      
      /**
       * Internal helper class to manage delays of components which an associated
       * PCB can run on.
       */
      class DelayMapper{
        /*
         * Replaced by new version to enable multiple
         * delays for different components on different 
         * functions
         map<string,double>  functionDelays;
         */
        std::map<std::string, ComponentDelay*> compDelays;

        public:
        
        ~DelayMapper();
        
        /**
         * \brief Registers new component with its base delay to the mapping instance
         * \param comp specifies the corresponding component id
         * \param delay is the standard delay of the component corresponding to
         * the PCB
         */
        void registerDelay(std::string comp, double delay);
        
        /**
         * \brief Registers new special function delay to the mapping instance
         * This method registers a new function delay to the mapping instance.
         * if there is no currently associated management entry for the given component,
         * a new entry is created with the given delay additionally as standard delay.
         * \param comp specifies the associated component
         * \param funcname represents the function name
         * \param delay is the given function delay
         */
        void addDelay(std::string comp, const char* funcname, double delay);

        /**
         * \brief Used to access the delay
         * \param comp specifies the requested component
         * \param funcname optionally refers to the function name
         * \return delay for a requested component and the optionally given function name.
         * If there is no value found 0 is returned as default.
         */
        double getDelay(std::string comp, const char* funcname=NULL);
        
        /**
         * \brief Test if delay for an given component and optional given function exists
         * \param comp specifies the component
         * \param funcname refers to the optional function name
         * \return true if the requested delay exists else false
         */
        bool hasDelay(std::string comp, const char* funcname=NULL);
        
      };

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
      double delay;
      double remainingDelay;
      int priority;
      double period;
      double deadline;
      activation_state state;
      sc_signal<trace_value>* traceSignal;


      /**
       * variables common to pcb type
       */
      
      ActivationCounter* activationCount; //activation_count;
      DelayMapper* dmapper;
      int* copyCount;

    public:
      
      /**
       * \brief Default constructor of an PCB instance
       * \param name specifies the identifying name of the instance
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
      void setDelay(double delay);

      /**
       * \brief Gets currently associated delay of PCB instance
       */
      double getDelay() const;

      /**
       * \brief Sets currently remaining delay of PCB instance
       */
      void setRemainingDelay(double delay);

      /**
       * \brief Gets currently remaining delay of PCB instance
       */
      double getRemainingDelay() const;
      
      void setPeriod(double period);

      double getPeriod() const;
      
      void setPriority(int priority);

      int getPriority() const;

      void setDeadline(double deadline);

      double getDeadline() const;

      /**
       * \brief Used to increment activation count of this PCB instance
       */
      void incrementActivationCount();

      unsigned int getActivationCount() const;

      void setState(activation_state state);

      activation_state getState() const;
      
      void setTraceSignal(sc_signal<trace_value>* signal);

      sc_signal<trace_value>* getTraceSignal();

      /**
       * \brief Used to register component specific delay to PCB instance
       * \param comp specifies associated component
       * \param funcname refers to an function name or may be null for common
       * association only to a component
       * \param delay represents the execution delay needed for execution  on
       * specified component
       */
      void addFuncDelay(std::string comp, const char* funcname, double delay);

      /**
       * \brief Used to access delay of a PCB instance on a given component
       * \param comp specifies the associated component
       * \param funcname refers to an optional function name
       */
      double getFuncDelay(std::string comp, const char* funcname=NULL) const;

      /**
       * \brief Test if an delay is specified for a given component
       * \param comp specifies the associated component
       * \param funcname refers to an optional function name
       * \return true if a delay is registered else false
       */
      bool hasDelay(std::string comp, const char* funcname=NULL) const;
 
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
      double p1=pqe1.pcb->getPriority()/pqe1.pcb->getPeriod();
      double p2=pqe2.pcb->getPriority()/pqe2.pcb->getPeriod();
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
