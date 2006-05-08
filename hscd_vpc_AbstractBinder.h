#ifndef HSCD_VPC_ABSTRACTBINDER_H_
#define HSCD_VPC_ABSTRACTBINDER_H_

#include <exception>
#include <map>
#include <string>

#include "hscd_vpc_AbstractComponent.h"
#include "hscd_vpc_Bindings.h"
#include "hscd_vpc_ProcessControlBlock.h"
#include "hscd_vpc_MappingInformation.h"
#include "hscd_vpc_TaskEventListener.h"

namespace SystemC_VPC {

  class Controller;
  class MIMapper;

  class UnknownBindingException : public std::exception {

    private:

      std::string msg;

    public:

      UnknownBindingException(const std::string& message){
        msg = "UnkownBindungException> ";
        msg.append(message);
      }

      ~UnknownBindingException() throw(){}

      const std::string& what(){

        return this->msg;

      }

  };

  class AbstractBinder : public virtual TaskEventListener {
    
    protected:

      // refers to associated
      Controller* controller;

      // refers to instance managing MappingInformation
      MIMapper* miMapper;

      AbstractBinder(Controller* controller, MIMapper* miMapper) : controller(controller), miMapper(miMapper) {}
    
      Controller& getController(){
        return *controller;
      }

      MIMapper& getMIMapper() {
        return *miMapper;
      }

      /**
       * \brief Used internally for accessing managed Bindings
       */
      virtual AbstractBinding& getBinding(std::string const& task, AbstractComponent* comp)=0;

    public:

      virtual ~AbstractBinder(){}

      virtual std::string resolveBinding(ProcessControlBlock& task, AbstractComponent* comp) throw(UnknownBindingException) =0;

      virtual void registerBinding(std::string src, std::string target)=0;

      /**
       * \brief Used to set Binder specific values
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if property value has been used else false
       */
      virtual bool setProperty(char* key, char* value)=0;

  };

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
      void updatePCBData(ProcessControlBlock& pcb, MappingInformation* mapping){

        pcb.setDeadline(mapping->getDeadline());
        pcb.setPeriod(mapping->getPeriod());
        pcb.setPriority(mapping->getPriority());

        pcb.setDelay(mapping->getDelay(pcb.getFuncName()));
        pcb.setRemainingDelay(pcb.getDelay());

      }

    protected:

      std::map<std::string, AbstractBinding*> bindings;  

      /**
       * \brief Implementation of getBinding intended to enable access to managed Bindings
       * As this implementation only covers resolving  binding on single hierarchy the information
       * of requesting component is neglected and set to NULL by default.
       */
      AbstractBinding& getBinding(std::string const& task, AbstractComponent* comp=NULL) throw(UnknownBindingException){

        std::map<std::string, AbstractBinding* >::iterator iter;
        iter = bindings.find(task);
        if(iter != bindings.end() && iter->second != NULL){
          return *(iter->second);
        }

        std::string msg = task + "->?";
        throw UnknownBindingException(msg);

      }

      /**
       * \brief Internal required method for performing binding strategy
       */
      virtual std::pair<std::string, MappingInformation* > performBinding(ProcessControlBlock& pcb, AbstractComponent* comp) 
        throw(UnknownBindingException) =0;

      LocalBinder(Controller* controller, MIMapper* miMapper) : AbstractBinder(controller, miMapper) {}
    
    public:

      virtual ~LocalBinder() {

        std::map<std::string, AbstractBinding* >::iterator iter;

        for(iter = bindings.begin(); iter != bindings.end(); iter++){
          delete iter->second;
        }

      }

      /**
       * \brief Implementation of AbstractBinder::resolveBinding
       * Generic implementation of required method:
       *  - perform binding strategy depending on subclasses
       *  - update PCB to made decision
       *  \sa AbstractBinder::resolveBinding
       */
      std::string resolveBinding(ProcessControlBlock& task, AbstractComponent* comp) throw(UnknownBindingException){

        std::pair<std::string, MappingInformation*> decision;
        decision = performBinding(task, comp);
        updatePCBData(task, decision.second);
        return decision.first;

      }

  };

  /**
   * \brief Base class for all Binder using only one binding possibility to perfom task binding
   */
  class StaticBinder : public LocalBinder{

    public:

      StaticBinder(Controller* controller, MIMapper* miMapper) : LocalBinder(controller, miMapper) {}

      virtual ~StaticBinder() {}

      virtual void registerBinding(std::string src, std::string target){

#ifdef VPC_DEBUG
        std::cerr << "StaticBinder> registerBinding " << src << " <-> " << target << std::endl;
#endif //VPC_DEBUG

        std::map<std::string, AbstractBinding* >::iterator iter;
        iter = bindings.find(src);
        if(iter != bindings.end() && iter->second != NULL) {
          iter->second->addBinding(target);
        }else{
          AbstractBinding* b = new SimpleBinding(src);
          b->addBinding(target);
          bindings[b->getSource()] = b;
        }

      }

      /**
       * \brief Used to set Binder specific values
       * Dummy implementation does nothing.
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if property value has been used else false
       * \sa AbstractBinder
       */
      virtual bool setProperty(char* key, char* value){ 
        return false; 
      }

  };

  /**
   * \brief Base class for all Binder using multiple binding possibilities to perform task binding
   */
  class DynamicBinder : public LocalBinder{

    public:

      DynamicBinder(Controller* controller, MIMapper* miMapper) : LocalBinder(controller, miMapper) {}

      virtual ~DynamicBinder() {}

      virtual void registerBinding(std::string src, std::string target){

#ifdef VPC_DEBUG
        std::cerr << "DynamicBinder> registerBinding " << src << " <-> " << target << std::endl;
#endif //VPC_DEBUG

        std::map<std::string, AbstractBinding* >::iterator iter;
        iter = bindings.find(src);
        if(iter != bindings.end() && iter->second != NULL) {
          iter->second->addBinding(target);
        }else{
          AbstractBinding* b = new Binding(src);
          b->addBinding(target);
          bindings[b->getSource()] = b;
        }

      }

      /**
       * \brief Used to set Binder specific values
       * Dummy implementation does nothing.
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if property value has been used else false
       * \sa AbstractBinder
       */
      virtual bool setProperty(char* key, char* value){
        return false;
      }

  };

}

#endif //HSCD_VPC_ABSTRACTBINDER_H_
