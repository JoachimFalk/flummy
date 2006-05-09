#ifndef HSCD_VPC_MAPPINGINFORMATION_H_
#define HSCD_VPC_MAPPINGINFORMATION_H_

#include <string>

#include "hscd_vpc_DelayMapper.h"

namespace SystemC_VPC {

  class MappingInformation  {

    private:
      
      int priority;
      double period;
      double deadline;
      DelayMapper dmapper;

    public:

      MappingInformation();

      ~MappingInformation();

      /**
       * \brief Gets period of instance
       */
      void setPeriod(double period);

      double getPeriod() const;

      void setPriority(int priority);

      int getPriority() const;

      void setDeadline(double deadline);

      double getDeadline() const;

      /**
       * \brief Used to register component specific delay to MappingInformation instance
       * \param funcname refers to an function name or may be null for common
       * association only to a component
       * \param delay represents the execution delay needed for execution  on
       * current component
       */
      void addDelay(const char* funcname, double delay);

      /**
       * \brief Used to access delay for a PCB instance on a given component
       * \param funcname refers to an optional function name
       */
      double getDelay(const char* funcname=NULL) const;

      /**
       * \brief Test if an delay is specified for a given component
       * \param funcname refers to an optional function name
       * \return true if a delay is registered else false
       */
      bool hasDelay(const char* funcname=NULL) const;

  };

}
#endif //HSCD_VPC_MAPPINGINFORMATION_H_
