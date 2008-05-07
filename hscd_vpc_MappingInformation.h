#ifndef HSCD_VPC_MAPPINGINFORMATION_H_
#define HSCD_VPC_MAPPINGINFORMATION_H_

#include <string>

#include "hscd_vpc_DelayMapper.h"

namespace SystemC_VPC {

  class MappingInformation  {

    private:

      int priority;
      sc_time period;
      sc_time deadline;
      sc_time RCWaitInterval;
      DelayMapper dmapper;

    public:

      MappingInformation();

      ~MappingInformation();

      /**
       * \brief Gets period of instance
       */
      void setPeriod(sc_time period);

      sc_time getPeriod() const;

      void setPriority(int priority);

      int getPriority() const;

      void setDeadline(sc_time deadline);

      sc_time getDeadline() const;

      void setRCWaitInterval(sc_time);
      
      sc_time getRCWaitInterval();
      
      /**
       * \brief Used to register component specific delay to MappingInformation instance
       * \param funcname refers to an function name or may be null for common
       * association only to a component
       * \param delay represents the execution delay needed for execution  on
       * current component
       */
      void addDelay(const char* funcname, sc_time delay);

      /**
       * \brief Used to access delay for a PCB instance on a given component
       * \param funcname refers to an optional function name
       */
      sc_time getDelay(const char* funcname=NULL) const;

      /**
       * \brief Test if an delay is specified for a given component
       * \param funcname refers to an optional function name
       * \return true if a delay is registered else false
       */
      bool hasDelay(const char* funcname=NULL) const;

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
#endif //HSCD_VPC_MAPPINGINFORMATION_H_
