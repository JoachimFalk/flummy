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
      RRBinder(Controller* controller, MIMapper* miMapper);

      ~RRBinder();
      
      /**
       * \brief Resolves binding for a given task
       * \param task specifies the task
       * \param comp refers to the component requesting resolving, which is ignored in this implementation
       * \sa AbstractBinder
       */
      std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock& task, AbstractComponent* comp) throw(UnknownBindingException);
     
      /**
       * \brief Implementation of TaskEventListener::signalTaskEvent
       * Dummy implementation as strategy is independent of task events
       */
      void signalTaskEvent(ProcessControlBlock* pcb); 
  };

}


#endif //HSCD_VPC_RRBINDER_H_
