#ifndef HSCD_VPC_CONFIGURATION_H_
#define HSCD_VPC_CONFIGURATION_H_

#include <systemc.h>

#include <map>
#include <list>
#include <string>

#include "hscd_vpc_AbstractComponent.h"
#include "hscd_vpc_IPreemptable.h"
#include "hscd_vpc_InvalidArgumentException.h"

namespace SystemC_VPC{
  
  /**
   * \brief Interface definition of Configuration
   * A Configuration represents a set of components which can
   * be dynamically loaded by a reconfigurable Component represented
   * by an instance of ReconfigurableComponent.
   */
  class Configuration : public IPreemptable{

    public:
      
      /**
       * \brief Iterator to enable id extraction of contained components in a configuration
       */
      class ComponentIDIterator {

        private:

          Configuration* config;
          std::map<std::string, AbstractComponent* >::iterator iter;

        public:

          ComponentIDIterator(Configuration* conf);

          bool hasNext();

          const std::string getNext();
          
      };
      
    private:
      
      friend class ComponentIDIterator;

      // identifies instance of configuration
      char configName[VPC_MAX_STRING_LENGTH];

      static unsigned int global_id;
      
      unsigned int id;
      
      // map associating AbstractComponents with their identifying names
      std::map<std::string, AbstractComponent* > component_map_by_name;

      // signals if configuration is activ or not
      bool activ;

      // signals if configuration has been killed
      bool killed;

      // signals if configuration has been stored successful
      bool stored;

      // represents time needed to store configuration
      sc_time storeTime;
      // represents time needed to load configuration
      sc_time loadTime;

    public:

      /**
       * \brief Creates instance of Configuration
       */
      Configuration(const char* name);

      Configuration(const char* name, sc_time loadTime, sc_time storeTime);

      virtual ~Configuration();

      /**
       * \brief Getter to access name of Configuration
       */
      char* getName();

      /**
       * \brief Getter to access id of Configuration
       */
      unsigned int const& getID() const;
      
      /**
       * \brief Getter to access activ flag of configuration
       */
      bool isActiv() const;

      /**
       * \brief Getter to check if configuration has been stored
       */
      bool isStored() const;

      /**
       * \brief Setter to set if configuration has been stored
       */
      void setStored(bool stored);

      /**
       * \brief Adds a given component to current configuration
       * Adds a given component to the configuration and deactivates it,
       * if configuration is inactiv. As configurations are inactiv at
       * initiazation time, normally all added components are also deactivated!
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
      //AbstractComponent* getComponent(const char* name);
      AbstractComponent* getComponent(std::string name);

      /**
       * \brief Preempts execution of components within current configuration
       * Used to preempt the current execution of components.
       * Actual executed tasks are "stored" for later execution or discarded
       * depending on the parameter flag.
       * \param kill indicates if "hard" preemption should be initiated
       * and currently registered task are killed without restoring
       * \return pointer to sc_time specifying time need for preemption
       * \see IPreemptable::preempt
       */
      void preempt(bool killTasks);

      /**
       * \brief Resumes preempted execution
       * Used to resume execution of preempted configuration which
       * mean the equalent of resuming all its contained components.
       * \see IPreemptable::resume
       */
      void resume();

      /**
       * \brief Gets time needed to preempt all components of configuration
       * Used to determine time needed to store all execution information, if current state
       * of configuration should be recoverable after preemption.
       * \return time needed if configuration actually is preempted
       * \see IPreemptable::timeToPreempt
       */
      sc_time timeToPreempt();

      /**
       * \brief Gets time needed to restore all components of configuration after preemption
       * \return time needed for restoring state before preemption
       * \see IPreemptable::timeToResume
       */
      sc_time timeToResume();

      /**
       * \brief Determines minimal time of configuration until all components will be idle
       * Used to determine how long it will take till all components within the configuration 
       * finish their currently running executions.
       * \return sc_time specifying the time till idle
       */
      sc_time* minTimeToIdle();

      /**
       * \brief Sets store time for a configuration
       * \param time specifies the corresponding store time
       */
      void setStoreTime(sc_time time);

      /**
       * \brief Getter to access store time of configuration
       * Grants access to store time of configuration neglecting additional
       * time needed for storing encapsulated components.
       * \return sc_time specifying time to store configuration
       */
      const sc_time& getStoreTime();

      /**
       * \brief Sets store time for a configuration
       * \param time specifies the corresponding store time
       */
      void setLoadTime(sc_time time);

      /**
       * \brief Getter to acces load time of configuration
       * Grants access to load time of configuration neglecting additional
       * time needed for loading encapsulated components.
       * \return sc_time specifying time to load configuration
       */
      const sc_time& getLoadTime();

      /**
       * \brief Gets iterator over ids of contained components within configuration
       * \return const iterator to access ids of contained components
       */
      ComponentIDIterator getComponentIDIterator(); 

  };
 
}

#endif /*HSCD_VPC_CONFIGURATION_H_*/ 
