#ifndef HSCD_VPC_ABSTRACTDIRECTOR_H_
#define HSCD_VPC_ABSTRACTDIRECTOR_H_

//#include <cosupport/systemc_support.hpp>
#include <systemc_support.hpp>

#include <hscd_vpc_AbstractComponent.h>

namespace SystemC_VPC{

	class AbstractDirector{
	
	public:
    
    virtual ~AbstractDirector(){}
    
    /**
     * \brief Delegate task to mapped component.
     *
     * Determines for a given task the component to run on.
     * \param name of task
     * \param name of function
     * \param event to signal finished request
     */
    virtual void compute(const char* name, const char* funcname, CoSupport::SystemC::Event* end=NULL)=0;
   
    /**
     * \brief Delegate task to mapped component.
     *
     * Determines for a given task the component to run on.
     * \param name of task
     * \param event to signal finished request
     */
    virtual void compute(const char *name, CoSupport::SystemC::Event *end=NULL)=0;
    
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
