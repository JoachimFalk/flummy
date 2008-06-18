#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <deque>
#include <utility>
#include <iostream>
#include "hscd_vpc_InvalidArgumentException.h"


namespace SystemC_VPC{

  class Attribute;

  typedef std::deque<std::pair<std::string, Attribute > >  Attributes;
  typedef std::deque<std::pair<std::string, std::string>  >  Parameters;

  class Attribute{
  public:
    Attribute();

    /**
     *
     */
    Attribute( std::string type, std::string value);

    std::pair<std::string, std::string> getNextParameter(size_t pos)
      throw (InvalidArgumentException);

    /**
     *
     */
    std::string getParameter(const std::string type)
      throw (InvalidArgumentException);

    /**
     *
     */
    bool hasParameter(const std::string type);

    void addParameter(std::string type, std::string value);

    std::pair<std::string, Attribute > getNextAttribute(size_t pos)
      throw (InvalidArgumentException);

    /**
     *
     */
    Attribute getAttribute(const std::string name)
      throw (InvalidArgumentException);

    /**
     *
     */
    bool hasAttribute(const std::string name);

    void addAttribute( std::string type, std::string value);

    void addAttribute( std::string type, Attribute& att );

    int getParameterSize();
    int getAttributeSize();

    std::string getValue();
    std::string getType();

    /**
     *
     */
    bool isType(std::string check){
      return this->type==check;
    }

    void setValue(std::string);
    void setType(std::string);

    ~Attribute(){
      std::cerr << "~Attribute(" << getType() << ", ";
      std::cerr << getValue();
      std::cerr << ")" << std::endl;
    }
  private:
    std::string type;
    std::string value;

    Parameters  parameters;
    Attributes  attributes;
  };
}

#endif

