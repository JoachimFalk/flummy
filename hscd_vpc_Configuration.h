#ifndef HSCD_VPC_CONFIGURATION_H_
#define HSCD_VPC_CONFIGURATION_H_

#include <systemc.h>

#include <map>
#include <string>

// #include <cosupport/systemc_support.hpp>
#include <systemc_support.hpp>

#include <hscd_vpc_AbstractComponent.h>


namespace SystemC_VPC{
	
	/**
	 * \brief Interface definition of Configuration
	 * A Configuration represents a set of components which can
	 * be dynamically loaded by a reconfigurable Component represented
	 * by an instance of ReconfigurableComponent.
	 */
	class Configuration{

	private:
		
		// identifies instance of configuration
		char configName[VPC_MAX_STRING_LENGTH];

		// map associating AbstractComponents with their identifying names
		std::map<std::string, AbstractComponent* > component_map_by_name;

	public:
		
		/**
		 * \brief Creates instance of Configuration
		 */
		Configuration(const char* name);
		
		~Configuration();
		
		/**
		 * \brief Getter to access name of Configuration
		 */
		char* getName();
		
		/**
		 * \brief Adds a given component to current configuration
		 * \param name specifies the identifying name of the component
		 * \param comp points to the component to be added
		 * \return TRUE if component could be added else FALSE
		 */
		bool addComponent(const char* name, AbstractComponent* comp);
		
		/**
		 * \brief Enables access to components of configuration
		 * Used to enable acces to components of given configuration.
		 * \param name specifies the requested component
		 * \return requested component or NULL if component is no
		 *         member of the configuration
		 */
		AbstractComponent* getComponent(const char* name);
		  
		/**
		 * \brief Preempts execution of components within current configuration
		 * Used to preempt the current execution of components.
		 * Actual executed tasks are "stored" for later execution
		 */
		void preemptComponents();
		
		/**
		 * \brief Resumes preempted execution
		 * Used to resume execution of preempted configuration which
		 * mean the equalent of resuming all its contained components.
		 */
		void resumeComponents();
		
		/**
		 * \brief Determines minimal time of configuration until all components will be idle
		 * Used to determine how long it will take till all components within the configuration 
		 * finish their currently running executions.
		 * \return sc_time specifying the time till idle
		 */
		sc_time* minTimeToIdle();

	};
 
}

#endif /*HSCD_VPC_CONFIGURATION_H_*/ 
