#ifndef HSCD_VPC_STRINGPARSER_H_
#define HSCD_VPC_STRINGPARSER_H_

#include <string>
#include <iostream>
#include <sstream>
#include <systemc.h>

namespace SystemC_VPC{
  
  class StringParser{
    
    public:  
      StringParser::StringParser();
      StringParser::~StringParser();
      void StringParser::cleanstring(std::string*);
      sc_time StringParser::generate_sctime(std::string);
  };
} 
#endif //HSCD_VPC_STRINGPARSER_H_
