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
#include "PowerMode.h"
#include "Timing.h"

namespace SystemC_VPC {

  class AbstractComponent;

  typedef size_t ComponentId;

  /**
   * Internal helper class to manage  function specific delays.
   */
  class FunctionTiming{

  public:

    /**
     * \brief Default constuctor of an ComponentDealy instance
     * \param name specifies the name of the associated component
     * simulation
     */
    FunctionTiming( );

    /**
     * copy constructor
     */
    FunctionTiming( const FunctionTiming &delay );

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


    /**
     *
     */
    void setTiming(const Timing& timing);

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
  class ProcessControlBlock {

    public:
      
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

      /**
       * \brief Initializes a newly created intance of ProcessControlBlock
       */
      void init();
 
  };
}
#endif // HSCD_VPC_PROCESSCONTROLBLOCK_H_
