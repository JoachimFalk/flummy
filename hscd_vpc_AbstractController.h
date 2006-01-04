#ifndef HSCD_VPC_ABSTRACTCONTROLLER_H_
#define HSCD_VPC_ABSTRACTCONTROLLER_H_

#include <string>
#include <map>
#include <queue>
#include <deque>
#include <vector>

#include <hscd_vpc_AbstractDirector.h>
#include <hscd_vpc_Configuration.h>
#include <hscd_vpc_datatypes.h>

namespace SystemC_VPC{
	
	class ReconfigurableComponent;
	
	struct my_taskInfo{
		const char* name;
		const char* funcname;
		CoSupport::SystemC::Event* end;
	};
	
	class AbstractController : public AbstractDirector{
	
	protected:
		
		char controllerName [VPC_MAX_STRING_LENGTH];
		
	protected:
	
		// controlled component of instance
		ReconfigurableComponent* managedComponent;
		// time caused by reconfiguration
		sc_time loadTime;
		// time indicating next request wish
		sc_time* waitInterval;
		
		// maps tasks to their corresponding names of configuration
		std::map<std::string, std::string > mapping_map_configs;
		
		// maps tasks to their corresponding names of component
		std::map<std::string, std::string > mapping_map_component_ids;
		
		/*
		 * map of maps for reconfigration times
		 * first key specifies current configuration
		 * second key specifies next configuration
		 * value specifies the time needed to switch btw first and second
		 */
		std::map<std::string, sc_time> loadTime_map;
		std::map<std::string, sc_time> storeTime_map;
				
	public:

		virtual ~AbstractController(){}
		
		/**
		 * \brief Getter for controller name
		 */
		char* getName(){
			
			return this->controllerName;
			
		}
		
		/**
		 * \brief Adds specific load time for a configuration
		 * \param config specifies the name of the configuration
		 * \param time specifies the corresponding load time
		 */
		virtual void addLoadTime(const char* config, const char* time){
		
			assert(this->managedComponent != NULL);
			assert(config != NULL);
			
			double timeVal = atof(time);

#ifdef VPC_DEBUG
			std::cerr << "AbstractController> adding load time: " 
				<< YELLOW( config << " = " << time) << std::endl;
#endif //VPC_DEBUG
		
			this->loadTime_map[config] = sc_time(timeVal, SC_NS);
		}

		/**
		 * \brief Adds specific store time for a configuration
		 * \param config specifies the name of the configuration
		 * \param time specifies the corresponding store time
		 */
		virtual void addStoreTime(const char* config, const char* time){
		
			assert(this->managedComponent != NULL);
			assert(config != NULL);
			
			double timeVal = atof(time);

#ifdef VPC_DEBUG
			std::cerr << "AbstractController> adding store time: " 
				<< YELLOW( config << " = " << time) << std::endl;
#endif //VPC_DEBUG
		
			this->storeTime_map[config] = sc_time(timeVal, SC_NS);
		}		

		/**
		 * \brief Sets the currently controlled reconfigurable Component of instance
		 */
		void setManagedComponent(ReconfigurableComponent* managedComponent){
			
			assert(managedComponent != NULL);
			this->managedComponent = managedComponent;
		
		}
		
		/**
		 * \brief Gets the currently conrtolled reconfigurable Component of instance
		 */
		ReconfigurableComponent* getManagedComponent(){
			
			return this->managedComponent;
			
		}
		
		/**
		 * \brief Returns time needed for reconfiguration
		 * Returns the time needed for reconfiguration, if any happened.
		 */
		virtual sc_time getReconfigurationTime(){
			
			return this->loadTime;
			
		}
		
		/**
		 * \brief Returns time to wait until next notification of controller is needed
		 * Returns time interval indicating when controlled component should invoke controller
		 * next time.
		 * \return time interval to wait or NULL if no time interval required
		 */
		sc_time* getWaitInterval(){
			
			return this->waitInterval;
			
		}
		
		/**
		 * \brief Used to set controller specific values
		 * \param key specifies the identy of the property
		 * \param value specifies the actual value
		 */
		virtual void setProperty(char* key, char* value)=0;
		
		/**
	 	* \brief Realizes scheduling decision for tasks to be forwarded to configurations
	 	* This method is used to perform scheduling decision for tasks and within this context
	 	* their corresponding configurationgs depending on the strategie of the different
	 	* controller. It is used to initialize and set up all necessary data for a new "round" of
	 	* scheduling. 
	 	*/
	 	virtual void addTasksToSchedule(std::deque<std::pair<p_struct* , const char* > >& newTasks)=0;
	 	
	 	/**
	 	 * \brief Returns next configuration to be loaded
	 	 * Used to indicate if a new configuration should be loaded by the controller
	 	 * component.
	 	 * \return pointer to next configuration to be loaded or NULL if no configuration
	 	 * is selected up to now.
	 	 */
	 	virtual Configuration* getNextConfiguration()=0;
	 	 
	 	/**
	 	 * \brief Returns mapped component for a given task
	 	 * \param task specifies the task to get component for
	 	 * \return pointer to AbstractComponent refering to mapped component
	 	 */
	 	virtual AbstractComponent* getMappedComponent(p_struct* task)=0;
	 	 
	 	/**
	 	 * \brief Indicates if controller still can forward tasks
	 	 * \return TRUE if there are still task to be forwarded else FALSE
	 	 */
	 	virtual bool hasTaskToProcess()=0;

	 	/**
	 	 * \brief Returns next task to be forwarded
	 	 * This method should only be called after calling hasTaskToProcess
	 	 * to ensure that there are still existing task to process.
	 	 * \return pair containing p_struct of task and requested function
	 	 * to be simulated.
	 	 */
	 	virtual std::pair<p_struct*, const char* > getNextTask()=0;
	
		sc_time* getNotifyInterval(){
		
			return this->waitInterval;
		
		}
	};

}

#endif /*HSCD_VPC_ABSTRACTCONTROLLER_H_*/
