#ifndef HSCD_VPC_RECONFIGURABLECOMPONENT_H_
#define HSCD_VPC_RECONFIGURABLECOMPONENT_H_

#include <systemc.h>

#include <vector>
#include <map>
#include <deque>

//#include <cosupport/systemc_support.hpp>
#include <systemc_support.hpp>

#include <hscd_vpc_datatypes.h>
#include <hscd_vpc_AbstractController.h>
#include <hscd_vpc_NonPreemptivController.h>
#include <hscd_vpc_PreemptivController.h>
#include <hscd_vpc_AbstractComponent.h>
#include <hscd_vpc_Configuration.h>

namespace SystemC_VPC{
	
	class ReconfigurableComponent : public AbstractComponent, public sc_module {

		//virtual void compute(p_struct *actualTask);
		
	private:
		
		SC_HAS_PROCESS(ReconfigurableComponent);
		
		AbstractController* controller;
		// mapping of configurations by their name
		std::map<std::string, Configuration* > config_map_by_name;
		// pointer to currently activ Configuration
		Configuration* activConfiguration;
		
		// queue containing new task to be added
		std::deque<std::pair<p_struct*, const char* > > newTasks;
		// maps containing task ready to run or already running
		map<int,p_struct*> readyTasks,runningTasks;
		
		sc_event notify_schedule_thread;
		sc_event notify_preempt;
		sc_event notify_resume;
		
	public:

		/**
		 * \brief An implementation of AbstractComponent used together with passive actors and global SMoC v2 Schedulers.
		 */
		ReconfigurableComponent(sc_module_name name, const char* type);
		    
		virtual ~ReconfigurableComponent();
	
		/**
		 * \brief Executes main task of scheduling and runing task on configurations
		 * Within this method all scheduling and reconfiguration of
		 * the reconfigurable instance is handled. 
		 * Runs as SC_THREAD of ReconfigurableComponent
		 */
		void ReconfigurableComponent::schedule_thread();

		/**
		 * \brief An implementation of AbstractComponent::compute(const char *, const char *, CoSupport::SystemC::Event).
		 */
		virtual void compute( const char *name, const char *funcname, CoSupport::SystemC::Event *end=NULL);
		
		/**
		 * \brief An implementation of AbstractComponent::compute(const char *, CoSupport::SystemC::Event).
		 */
		virtual void compute( const char *name, CoSupport::SystemC::Event *end=NULL);
		
		/**
		 * \brief An implementation of AbstractComponent::compute(p_struct*, const char *).
		 */
		virtual void compute(p_struct* pcb, const char *funcname);
		
		/**
		 * \brief Used to create the Tracefiles.
		 *
		 * To create a vcd-trace-file in SystemC all the signals to 
		 * trace have to be in a "global" scope. The signals have to 
		 * be created in elaboration phase (before first sc_start).
		 */
		virtual void informAboutMapping(std::string module);
		
		/**
		 * \brief Set parameter for Component and Scheduler.
		 */
		virtual void processAndForwardParameter(char *sType,char *sValue);
		
		/**
		 * \brief Preempts execution of component
		 * Used to preempt the current execution of a component.
		 * Actual executed tasks are "stored" for late execution
		 */
		virtual void preempt();
		
		/**
		 * \brief Resumes preempted execution
		 * Used to resume execution of preempted component.
		 */
		virtual void resume();
		
		/**
		 * \brief Determines minimum time till next idle state of component
		 * Used to determine how long it will take till component finishes
		 * currently running tasks.
		 * \return sc_time specifying the time till idle
		 */
		virtual sc_time* minTimeToIdle();
		
		/**
		 * \brief Adds new Configuration to the reconfigurable component.
		 * \param config the Configuration to be added
		 */
		void addConfiguration(const char* name, Configuration* config);
			
		/**
		 * \brief Enables access to known Configurations of component
		 * Used to map vector of known Configurations of current instance.
		 */
		std::map<std::string, Configuration* >& getConfigurations();
		
		/**
	 	 * \brief Enables access to a single Component by its identifying name
	 	 * \param name specifies the component to retrieve
	 	 * \return requested component or NULL if identifier of component
	 	 * is not known.
		 */
		Configuration* getConfiguration(const char* name);

		/**
		 * \brief Gets currently loaded Configuration
		 */
		Configuration* getActivConfiguration();
			
		/**
		 * \brief Sets currently loaded Configuration
		 * \param name specifies the Configuration to be set as activ on
		 */
		/**
		 * \brief Sets the currently loaded Configuration
		 * \param identifying name of Configuration to be set loaded
		 */
		void setActivConfiguration(const char* name);
		
		/**
		 * \brief Gets associated controller of component
		 * \return controller of component
		 */	
		AbstractController* getController();
			
		/**
		 * \brief Sets controller for current instance
		 * \param controllertype specifies the type of requested controller to be
		 * associated to the component.
		 */
		void setController(const char* controllertype);
		
		/**
		 * \brief Loads a given configuration
		 * Loads a given configuration by simulating loadtime given as additional
		 * parameter.
		 * \param newConfig refers to the configuration to be loaded
		 * \param timeToLoad specifies the time it needs to load new configuration
		 * plus additional time needed for probably storing another configuration.
		 */ 
		void loadNewConfiguration(Configuration* newConfig, sc_time timeToLoad);
		
	};

}

#endif /*HSCD_VPC_RECONFIGURABLECOMPONENT_H_*/
