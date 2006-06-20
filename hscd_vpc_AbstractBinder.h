#ifndef HSCD_VPC_ABSTRACTBINDER_H_
#define HSCD_VPC_ABSTRACTBINDER_H_

#define BINDERPREFIX "binder_"

#include <map>
#include <string>

//#include "hscd_vpc_ReconfigurableComponent.h"
#include "hscd_vpc_BindingGraph.h"
#include "hscd_vpc_Binding.h"
#include "hscd_vpc_ProcessControlBlock.h"
#include "hscd_vpc_MappingInformation.h"
#include "hscd_vpc_TaskEventListener.h"
#include "hscd_vpc_InvalidArgumentException.h"

namespace SystemC_VPC {

  class ReconfigurableComponent;
  class AbstractController;

  /**
   * \brief Basic binder class specifying interface for all realized binder sub-classes
   */
  class AbstractBinder : public virtual TaskEventListener {
		
    protected:

      AbstractBinder() {}
    
			/**
			 * \brief Used to get controlling instance for associated component ID
			 * In localized case this method just returns singel aussociated controller
			 * for an associated component ID, in global case this method may return different
			 * controllers depending on key entry.
			 * \throws InvalidArgumentException if specified component ID is unknown.
			 */
			virtual AbstractController& getController(std::string c) throw(InvalidArgumentException) =0;
			
    public:

      virtual ~AbstractBinder(){}
      
      /**
       * \brief Resolves bindings possibility for a given task on an given component
       * Uses specialized binding strategy to resolve binding for one task out of a set of possiblities.
       */
      virtual std::string resolveBinding(ProcessControlBlock& task, ReconfigurableComponent* comp) throw(UnknownBindingException) =0;

			/**
			 * \brief Register additional controlling instance with associated component ID
			 */
			virtual void registerCompToCtrl(std::string c, AbstractController* ctrl)=0;
			
      /**
       * \brief Used to set Binder specific values
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if property value has been used else false
       */
      virtual bool setProperty(char* key, char* value)=0;

  };

  /**
   * \brief Binder class used as superclass for binder using strategies based on local view
   * This class provides basic implementation for all binder using localized strategies for
   * determing proper binding.
   */
  class LocalBinder : public AbstractBinder {

    private:

      /**
       * \brief Updates data fields of pcb to binding decision
       * This method ensures that at each hierarchy the data fields
       * of a PCB are set to the currently selected binding decision.
       * It has to be called after binding decision has taken place!
       * \param pcb specifies the PCB to update
       * \param mapping refers to the selected MappingInformation to apply
       */
      void updatePCBData(ProcessControlBlock& pcb, MappingInformation* mapping);

			// map containing associated controlling instances
			std::map<std::string, AbstractController* > ctrls;

    protected:

      // map of binding decisions pid -> target
      std::map<int, std::string> bindDecisions;
      
			/**
			 * \brief Used to get controlling instance for associated component ID
			 * In localized case this method just returns singel aussociated controller
			 * for an associated component ID, in global case this method may return different
			 * controllers depending on key entry.
			 * \throws InvalidArgumentException if specified component ID is unknown.
			 */
			AbstractController& getController(std::string c) throw(InvalidArgumentException);

      LocalBinder() {}
      
			/**
       * \brief Internal required method for performing binding strategy
       */
      virtual std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock& pcb, ReconfigurableComponent* comp) 
        throw(UnknownBindingException) =0;

    
    public:

      virtual ~LocalBinder() {}

			virtual void registerCompToCtrl(std::string c, AbstractController* ctrl) throw(InvalidArgumentException);
			
      /**
       * \brief Implementation of AbstractBinder::resolveBinding
       * Generic implementation of required method:
       *  - perform binding strategy depending on subclasses
       *  - update PCB to made decision
       * \sa AbstractBinder::resolveBinding
       */
      std::string resolveBinding(ProcessControlBlock& task, ReconfigurableComponent* comp) throw(UnknownBindingException);

  };

  /**
   * \brief Base class for all Binder using only one binding possibility to perfom task binding
   */
  class StaticBinder : public LocalBinder{

    public:

      StaticBinder(); // {}

      virtual ~StaticBinder(); // {}

      /**
       * \brief Used to set Binder specific values
       * Dummy implementation does nothing.
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if property value has been used else false
       * \sa AbstractBinder
       */
      virtual bool setProperty(char* key, char* value);

  };

  /**
   * \brief Base class for all Binder using multiple binding possibilities to perform task binding
   */
  class DynamicBinder : public LocalBinder{

    public:

      DynamicBinder(); // {}

      virtual ~DynamicBinder(); // {}

      /**
       * \brief Used to set Binder specific values
       * Dummy implementation does nothing.
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if property value has been used else false
       * \sa AbstractBinder
       */
      virtual bool setProperty(char* key, char* value);

  };

}

#endif //HSCD_VPC_ABSTRACTBINDER_H_
