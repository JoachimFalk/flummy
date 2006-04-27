#ifndef HSCD_VPC_RRBINDER_H_
#define HSCD_VPC_RRBINDER_H_


#include "hscd_vpc_AbstractBinder.h"

namespace SystemC_VPC {

  /**
   * \brief Simple implementation  of dynamic binder using RoundRobin strategy
   * This class uses RoundRobin strategy to resolve binding request for a given
   * task instance. It is intended to be used as local binder, meaning it cannot perform
   * binding over more than one hierarchy
   */
  class RRBinder : public DynamicBinder {
  
    public:

      /**
       * \brief Default constructor
       */
      RRBinder();

      ~RRBinder();
      
      /**
       * \brief Resolves binding for a given task
       * \param task specifies the task
       * \param comp refers to the component requesting resolving, which is ignored in this implementation
       * \sa AbstractBinder
       */
      virtual std::string resolveBinding(std::string task, AbstractComponent* comp) throw(UnknownBindingException);
      
  };

}


#endif //HSCD_VPC_RRBINDER_H_
