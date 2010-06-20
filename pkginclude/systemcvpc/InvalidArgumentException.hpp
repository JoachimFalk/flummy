#ifndef INVALIDARGUMENTEXCEPTION_
#define INVALIDARGUMENTEXCEPTION_

#include <exception>
#include <string>

namespace SystemC_VPC{

  class InvalidArgumentException: public std::exception{
  
  private:
  
    std::string msg;
    
  public:
  
    InvalidArgumentException(const std::string& message) : msg(message){
      
    }
    
    ~InvalidArgumentException() throw(){}
    
    const std::string& what(){
      
      return this->msg;
      
    }
    
  };
  
}

#endif /*INVALIDARGUMENTEXCEPTION_*/
