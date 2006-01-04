#ifndef HSCD_VPC_NONPREEMPTIVCONTROLLER_H_
#define HSCD_VPC_NONPREEMPTIVCONTROLLER_H_

#include <systemc.h>

#include <string>
#include <map>
#include <queue>
#include <vector>

#include <hscd_vpc_AbstractController.h>
#include <hscd_vpc_Configuration.h>
#include <hscd_vpc_ReconfigurableComponent.h>
#include <hscd_vpc_Director.h>

namespace SystemC_VPC{

	class NonPreemptivController : public AbstractController {
	
	private:
	 
		// queue of waiting tasks to be executed
		std::queue<std::pair<p_struct*, const char* > > waitingTasks;
		// queue of tasks ready to be processed
		std::queue<std::pair<p_struct*, const char* > > tasksToProcess;
		
		// queue containing order of configuration to be loaded in next "rounds"
		std::queue<Configuration* > nextConfigurations;
		
	public:
	
		NonPreemptivController(const char* name);
		      
		virtual ~NonPreemptivController();
		
		/**
		 * \brief Delegate task to mapped component.
		 *
		 * Determines for a given task the component to run on.
		 */
		virtual void compute(const char* name, const char* funcname, CoSupport::SystemC::Event* end=NULL);
		   
		/**
		 * \brief Delegate task to mapped component.
		 *
		 * Determines for a given task the component to run on.
		 */
		virtual void compute(const char* name, CoSupport::SystemC::Event* end=NULL);
		
		/**
		 * \brief Register component to Controller
		 * Used to register a component to the Controller for
		 * later computation of task on it. The components name
		 * is used as identifier for it.
		 * \param comp points to component instance to be registered
		*/
		void registerComponent(AbstractComponent* comp);
		    
		/**
		 * \brief Registers mapping between task and component to Controller
		 * \param taskName specifies name of task
		 * \param compName specifies name of component
		 */
		void registerMapping(const char* taskName, const char* compName);
	
		/**
		 * \brief Used to set controller specific values
		 * \param key specifies the identy of the property
		 * \param value specifies the actual value
		 */
		void setProperty(char* key, char* value);
		
		/**
	 	 * \brief Realizes scheduling decision for tasks to be forwarded to configurations
	 	 * This method is used to perform scheduling decision for tasks and within this context
	 	 * their corresponding configurationgs depending on the strategie of the different
	 	 * controller. It is used to initialize and set up all necessary data for a new "round" of
	 	 * scheduling. 
	 	 */
		virtual void addTasksToSchedule(std::deque<std::pair<p_struct* , const char* > >& newTasks);
	 		 	
	 	/**
	 	 * \brief Returns next configuration to be loaded
	 	 * Used to indicate if a new configuration should be loaded by the controller
	 	 * component.
	 	 * \return pointer to next configuration to be loaded or NULL if no configuration
	 	 * is selected up to now.
	 	 */
	 	virtual Configuration* getNextConfiguration();
	 	 
	 	/**
	 	 * \brief Returns mapped component for a given task
	 	 * \param task specifies the task to get component for
	 	 * \return pointer to AbstractComponent refering to mapped component
	 	 */
	 	virtual AbstractComponent* getMappedComponent(p_struct* task);
	 	 
	 	/**
	 	 * \brief Indicates if controller still can forward tasks
	 	 * \return TRUE if there are still task to be forwarded else FALSE
	 	 */
	 	virtual bool hasTaskToProcess();
	 	 
	 	/**
	 	 * \brief Returns next task to be forwarded
	 	 * This method should only be called after calling hasTaskToProcess
	 	 * to ensure that there are still existing task to process.
	 	 * \return pair containing p_struct of task and requested function
	 	 * to be simulated.
	 	 */
	 	virtual std::pair<p_struct*, const char* > getNextTask();
	 	 
	 	/**
	 	 * \brief Signals if a configuration has to be reactived by controlled component
	 	 * \param config points to configuration which should be reactivated.
	 	 */ 
	 	bool needToReactivateConfiguration(Configuration* config);
		
		/**
		 * \brief Sets needed loadtime for configuration
		 */
		void setLoadTime(Configuration* oldConfig, Configuration* newConfig, bool killOld);
		
	};

}
#endif /*HSCD_VPC_NONPREEMPTIVCONTROLLER_H_*/
