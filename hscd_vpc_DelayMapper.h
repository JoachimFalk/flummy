#ifndef HSCD_VPC_DELAYMAPPER_H_
#define HSCD_VPC_DELAYMAPPER_H_

#include <map>
#include <string>

#include <systemc.h>

namespace SystemC_VPC {

  /**
   * Internal helper class to manage delays of components which an associated
   * PCB can run on.
   */
  class DelayMapper{

    private:

      // base delay used for task running on this component
      sc_time base_delay;
      // map of possible special delay depending on functions
      std::map<std::string, sc_time> funcDelays;
      // base latency used for tasks running on this component
      sc_time base_latency;
      // map of possible function specific latencies
      std::map<std::string, sc_time> funcLatencies;

    public:

      /**
       * \brief Default constructor
       * \param base defines standard delay for mapping
       */
      DelayMapper();
      DelayMapper(sc_time base);

      ~DelayMapper();

      /**
       * \brief Adds a new function delay to the instance
       * Basic function enabling adding of delays to DelayMapper, if there is a functionname specified
       * a new entry for the function delay is created or if a entry already exist the old value is replaced by
       * the new one. If no function is specified the base delay value of the DelayMapper is
       * set to a given value. 
       * \param funcname optional specifies the associated function else base delay is changed
       * \param delay is the corresponding delay for the function execution
       */
      void addDelay(const char* funcname, sc_time delay);

      /**
       * \brief Used to access delay
       * \param funcname specifies a possible function if given
       * \return delay required for a function execution  on the associated component
       * of the process. If no function name is given or there is no corresponding
       * entry registered the default delay is returned.
       */
      sc_time getDelay(const char* funcname=NULL) const;

      /**
       * \brief Tests if an specific function delay exisits
       * \param funcname specifies name of the requested function delay
       * \returns true if a specific function delay has been found
       * else false
       */
      bool hasDelay(const char* funcname) const;

      /**
       * \brief Used to register component specific latency to PCB instance
       * \param funcname refers to an function name or may be null for common
       * association only to a component
       * \param latency represents the execution latency needed for execution  on
       * specified component
       */
      void addFuncLatency(const char* funcname, sc_time latency);

      /**
       * \brief Used to access latency of a PCB instance on a given component
       * \param funcname refers to an optional function name
       */
      sc_time getFuncLatency(const char* funcname=NULL) const;

      /**
       * \brief Test if a latency is specified for a given component
       * \param funcname refers to an optional function name
       * \return true if a delay is registered else false
       */
      bool hasLatency(const char* funcname=NULL) const;

  }; 
}

#endif //HSCD_VPC_DELAYMAPPER_H_
