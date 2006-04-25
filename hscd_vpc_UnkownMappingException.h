#ifndef HSCD_VPC_UNKOWNMAPPINGEXCEPTION_H_
#define HSCD_VPC_UNKOWNMAPPINGEXCEPTION_H_

#include <exception>
#include <string>

namespace SystemC_VPC{
  
  /**
   * UnkownMapping Exception is used to indicate request which
   * cant be matched to a concrete mapping and therefor cant be
   * processed.
   */
  class UnkownMappingException: public std::exception{
  
  private:
  
    string msg;
    
  public:
  
    UnkownMappingException(const string& message) : msg(message){
      
    }
    
    ~UnkownMappingException() throw(){}
    
    const string& what(){
      
      return this->msg;
      
    }
    
  };
  
}

#endif /*HSCD_VPC_UNKOWNMAPPINGEXCEPTION_H_*/
