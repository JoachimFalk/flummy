#ifndef HSCD_VPC_ABSTRACTBINDER_H_
#define HSCD_VPC_ABSTRACTBINDER_H_

#include <exception>
#include <map>
#include <string>

#include "hscd_vpc_AbstractComponent.h"
#include "hscd_vpc_Bindings.h"

namespace SystemC_VPC {

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

  class AbstractBinder {

    protected:

      /**
       * \brief Used internally for accessing managed Bindings
       */
      virtual AbstractBinding& getBinding(std::string& task, AbstractComponent* comp)=0;
    
    public:

      virtual ~AbstractBinder(){}

      virtual std::string resolveBinding(std::string task, AbstractComponent* comp) throw(UnknownBindingException) =0;

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

    protected:

      std::map<std::string, AbstractBinding*> bindings;  

      /**
       * \brief Implementation of getBinding intended to enable access to managed Bindings
       * As this implementation only covers resolving  binding on single hierarchy the information
       * of requesting component is neglected and set to NULL by default.
       */
      AbstractBinding& getBinding(std::string& task, AbstractComponent* comp=NULL) throw(UnknownBindingException){

        std::map<std::string, AbstractBinding* >::iterator iter;
        iter = bindings.find(task);
        if(iter != bindings.end() && iter->second != NULL){
          return *(iter->second);
        }

        std::string msg = task + "->?";
        throw UnknownBindingException(msg);

      }
    
    public:

      virtual ~LocalBinder() {
      
        std::map<std::string, AbstractBinding*>::iterator iter;

        for(iter = bindings.begin(); iter != bindings.end(); iter++){
          delete iter->second;
        }

      }
  
  };
  /**
   * \brief Base class for all Binder using only one binding possibility to perfom task binding
   */
  class StaticBinder : public LocalBinder{
    
    public:

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
