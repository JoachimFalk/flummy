#ifndef HSCD_VPC_BINDINGGRAPH_H_
#define HSCD_VPC_BINDINGGRAPH_H_

#include <exception>
#include <map>
#include <string>

#include "hscd_vpc_Binding.h"

namespace SystemC_VPC {
  
  /**
   * \brief Exception used to indicate if no binding for a specific value is present
   */
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
   
  
	/**
	 * \brief Graph structure managing binding possibilities for a process
   * Each PCB is associated with a BindingGraph representing the binding possibilites for the current process.
   * This structure enable navigation and access to the bindung possibilities of a process.
	 */
  class BindingGraph {
    
    private:
        
        std::map<std::string, Binding* > nodes;
        Binding* root;

    public:

        /**
         * \brief default constructor
         * \param top of type <code>string</code> specifies the id of top vertex
         */
        BindingGraph(std::string top);

        /**
         * \brief default destructor
         */
        ~BindingGraph();

        /**
         * \brief create a Binding instance
         * This instance represents a vertex within the BindingGraph.
         * If the id is already used the existing Binding instance is returned,
         * else a new instance is created and returned.
         */
        Binding* createBinding(std::string key);

        /**
         * \brief enables access to top element of BindingGraph
         */
        Binding* getRoot();
        
        /**
         * \brief returns a specific Binding instance by id
         * \param id specifies the id of the Binding instance
         * \return requested Binding instance for given id
         * \throws UnkownBindingException if id is not known
         */
        Binding* getBinding(std::string key) throw(UnknownBindingException);

  };
}

#endif //HSCD_VPC_BINDINGGRAPH_H_
