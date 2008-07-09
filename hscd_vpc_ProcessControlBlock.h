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
#include "hscd_vpc_datatypes.h"
#include "FastLink.h"
#include "PCBPool.h"
#include "Pool.h"

namespace SystemC_VPC {

  class Director;

  typedef size_t ComponentId;

  /**
   * Internal helper class to manage  function specific delays.
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

 /**
  * This class represents all necessary data of a simulated process within VPC
  * and provides necessary access methods for its data.
  */
  class ProcessControlBlock :
    public ComponentDelay {

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

  typedef std::map<int,ProcessControlBlock*>  PCBMap;
}
#endif // HSCD_VPC_PROCESSCONTROLBLOCK_H_
