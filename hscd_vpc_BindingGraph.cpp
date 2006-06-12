#include "hscd_vpc_BindingGraph.h"

namespace SystemC_VPC {

  BindingGraph::BindingGraph(std::string top){
    this->root = new Binding(top);
  }
  
  BindingGraph::~BindingGraph(){

		std::map<std::string, Binding* >::iterator iter;
		for(iter = this->nodes.begin(); iter != this->nodes.end(); iter++){
			delete (iter->second);
		}

    delete root;
  }

  Binding* BindingGraph::createBinding(std::string key){
    
    std::map<std::string, Binding* >::iterator iter;
    iter = this->nodes.find(key);
    if(iter != this->nodes.end()){
      return (iter->second);
    }else{
      Binding* b = new Binding(key);
      this->nodes[key] = b;
      return b;
    }
  
  }

  Binding* BindingGraph::getRoot(){
    return (this->root);
  }

  Binding* BindingGraph::getBinding(std::string key) throw(UnknownBindingException){
    
    std::map<std::string, Binding* >::iterator iter;
    iter = this->nodes.find(key);
    if(iter != this->nodes.end()){
      return (iter->second);
    }

    throw UnknownBindingException(key);
  }

}
