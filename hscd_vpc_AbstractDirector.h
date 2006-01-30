#ifndef HSCD_VPC_ABSTRACTDIRECTOR_H_
#define HSCD_VPC_ABSTRACTDIRECTOR_H_

//#include <cosupport/systemc_support.hpp>
#include <hscd_vpc_datatypes.h>

#include <hscd_vpc_AbstractComponent.h>

namespace SystemC_VPC{

  class AbstractDirector{
  
  public:
    
    virtual ~AbstractDirector(){}
     
    /**
     * \brief Register component to Director
     * Used to register a component to the Director for
     * later computation of task on it. The components name
     * is used as identifier for it.
     * \param comp points to component instance to be registered
     */
    virtual void registerComponent(AbstractComponent* comp)=0;
    
    /**
     * \brief Registers mapping between task and component to Director
     * \param taskName specifies name of task
     * \param compName specifies name of component
     */
    virtual void registerMapping(const char* taskName, const char* compName)=0;
    
  };
  
}
#endif /*HSCD_VPC_ABSTRACTDIRECTOR_H_*/
