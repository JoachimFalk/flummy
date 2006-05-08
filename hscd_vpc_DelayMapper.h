#ifndef HSCD_VPC_DELAYMAPPER_H_
#define HSCD_VPC_DELAYMAPPER_H_

#include <map>
#include <string>

namespace SystemC_VPC {

  /**
   * Internal helper class to manage delays of components which an associated
   * PCB can run on.
   */
  class DelayMapper{
   
    private:

      // base delay used for task running on this component
      double base_delay;
      // map of possible special delay depending on functions
      std::map<std::string, double> funcDelays;

    public:
    
      /**
       * \brief Default constructor
       * \param base defines standard delay for mapping
       */
      DelayMapper(double base);
      
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
      void addDelay(const char* funcname, double delay);

      /**
       * \brief Used to access delay
       * \param funcname specifies a possible function if given
       * \return delay required for a function execution  on the associated component
       * of the process. If no function name is given or there is no corresponding
       * entry registered the default delay is returned.
       */
      double getDelay(const char* funcname=NULL) const;

      /**
       * \brief Tests if an specific function delay exisits
       * \param funcname specifies name of the requested function delay
       * \returns true if a specific function delay has been found
       * else false
       */
      bool hasDelay(const char* funcname) const;

  }; 
}

#endif //HSCD_VPC_DELAYMAPPER_H_
