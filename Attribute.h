#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <deque>
#include "hscd_vpc_InvalidArgumentException.h"


namespace SystemC_VPC{

  class Attribute{
  private:
    char* value;
    char* type;
    std::deque<std::pair<std::string, std::string> > parameters;
    std::deque<std::pair<std::string, Attribute > > attributes;
	
  public:	
    Attribute();
    Attribute( char* newType, char* newValue);
    std::pair<std::string, std::string> getNextParameter(size_t pos)
      throw (InvalidArgumentException);
    void addNewParameter(char* newType, char* newValue);
		
    std::pair<std::string, Attribute > getNextAttribute(size_t pos)
      throw (InvalidArgumentException);
    void addNewAttribute( char* newType, char* newValue);
    void addNewAttribute( Attribute& toadd, char* newValue);
    int getParameterSize();
    int getAttributeSize();
		
    char* getValue();
    char* getType();
    void setValue(char*);
    void setType(char*);
		
  };

}

#endif

