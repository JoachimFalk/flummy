#ifndef INVALIDARGUMENTEXCEPTION_
#define INVALIDARGUMENTEXCEPTION_

#include <exception>
#include <string>

namespace SystemC_VPC{

  class InvalidArgumentException: public std::exception{
  
  private:
  
    string msg;
    
  public:
  
    InvalidArgumentException(const string& message) : msg(message){
      
    }
    
    ~InvalidArgumentException() throw(){}
    
    const string& what(){
      
      return this->msg;
      
    }
    
  };
  
}

#endif /*INVALIDARGUMENTEXCEPTION_*/
