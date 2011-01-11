/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

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
