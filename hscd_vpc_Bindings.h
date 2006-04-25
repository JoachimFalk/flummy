#ifndef HSCD_VPC_BINDINGS_H_
#define HSCD_VPC_BINDINGS_H_

#include <string>
#include <set>

namespace SystemC_VPC {

  /**
   * \brief Abstract class defining basic properties and methods of an binding
   */
  class AbstractBinding {
    
    private:

      std::string source;
      
    public:
      
      AbstractBinding(std::string source);
      
      virtual ~AbstractBinding();

      /**
       * \brief Returns the associated source of the Binding instance
       */
      std::string& getSource();

      /**
       * \brief Sets iterator back to beginning of binding possibilites
       */
      virtual void reset()=0;
      
      /**
       * \brief Indicates if further binding targets are available
       * \return true if further binding possibilites exists else false
       */
      virtual bool hasNext()=0;

      /**
       * \brief Gets next possible binding target
       */
      virtual std::string getNext()=0;

      /**
       * \brief registers additional binding possibility to a Binding
       * \param target refers to the additional target
       */
      virtual void addBinding(std::string& target)=0;
  };
  
  /**
   * \brief Implementation of AbstractBinding representing a 1-to-1 binding
   */
  class SimpleBinding : public AbstractBinding {
    
    private:

      bool hasTarget;
      std::string target;

    public:

      SimpleBinding(std::string);

      virtual ~SimpleBinding();

      void reset();

      bool hasNext();

      std::string getNext();

      /**
       * \brief registers binding possibility to a Binding
       *  registers binding possibility to a Binding, as only one
       *  binding possibility is supported by SingleBinding earlier
       *  bindings are discarded.
       *  \sa AbstractBinding::addBinding
       */
      void addBinding(std::string& target);
  
  };
  
  /**
   * \brief Represents a binding between a task and a set of possible target components.
   */
  class Binding : public AbstractBinding {

    private:

      std::set<std::string> targets;
      std::set<std::string>::iterator iter;
      
    public:

      /**
       * \brief Default constructor of an binding
       * \param source specifies the task the binding is associated to
       */
      Binding(std::string source);

      virtual ~Binding();

      void reset();
      
      bool hasNext();

      std::string getNext();

      void addBinding(std::string& target);
  };

}
#endif //HSCD_VPC_BINDINGS_H_
