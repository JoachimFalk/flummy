#ifndef HSCD_VPC_BINDINGGRAPH_H_
#define HSCD_VPC_BINDINGGRAPH_H_

#include <exception>
#include <map>
#include <string>

#include "hscd_vpc_Binding.h"

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
   
  
	/**
	 * \brief Graph structure managing binding possibilities for a task
	 */
  class BindingGraph {
    
    private:
        
        std::map<std::string, Binding* > nodes;
        Binding* root;

    public:

        BindingGraph(std::string);

        ~BindingGraph();

        Binding* createBinding(std::string key);

        Binding* getRoot();
        
        Binding* getBinding(std::string key) throw(UnknownBindingException);

  };
}

#endif //HSCD_VPC_BINDINGGRAPH_H_
