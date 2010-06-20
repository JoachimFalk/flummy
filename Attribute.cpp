#include <systemcvpc/Attribute.hpp>
#include <iostream>

namespace SystemC_VPC{

  //
  Attribute::Attribute() : type(), value() {}

  //
  Attribute::Attribute( std::string type, std::string value)
    : type(type), value(value){}

  std::pair<std::string, std::string> Attribute::getNextParameter(size_t pos)
    throw(InvalidArgumentException){
    if(pos<=parameters.size()) return parameters[pos];
    throw new InvalidArgumentException("getNextParameter");
  }

  //
  std::string Attribute::getParameter(const std::string type)
    throw (InvalidArgumentException){
    for(unsigned int i=0;
        i<this->getParameterSize();
        ++i){
      if(parameters[i].first == type){
        return parameters[i].second;
      }
    }
    throw new InvalidArgumentException("getParameter> unknown parameter:"
                                       + type);
  }

  //
  bool Attribute::hasParameter(const std::string type){
    for(unsigned int i=0;
        i<this->getParameterSize();
        ++i){
      if(parameters[i].first == type){
        return true;
      }
    }
    return false;
  }

  void Attribute::addParameter(std::string type,std::string value){
    this->parameters.push_back( std::make_pair(type, value) );
  }

  std::pair<std::string, AttributePtr > Attribute::getNextAttribute(size_t pos)
    throw(InvalidArgumentException){
    if(pos<=attributes.size()) return attributes[pos];
    throw new InvalidArgumentException("getNextAttribute");
        
  }

  //
  AttributePtr Attribute::getAttribute(const std::string name)
    throw(InvalidArgumentException){
    for(unsigned int i=0;
        i<this->getAttributeSize();
        ++i)
      {
        if(attributes[i].first == name)
          return attributes[i].second;
      }
    throw new InvalidArgumentException("getAttribute> Unknown Attribute:"
                                       + name);
  }

  //
  bool Attribute::hasAttribute(const std::string name) {
    for(unsigned int i=0;
        i<this->getAttributeSize();
        ++i)
      {
        if(attributes[i].first == name)
          return true;
      }
    return false;
  }

  void Attribute::addAttribute( std::string type, std::string value){
    AttributePtr toadd(new Attribute(type, value));
    attributes.push_back( std::make_pair(type, toadd) );
  }

  void Attribute::addAttribute( std::string type, AttributePtr att ){
    attributes.push_back( std::make_pair(type, att) );
  }

  size_t Attribute::getParameterSize(){
    return parameters.size();
  }

  size_t Attribute::getAttributeSize(){
    return attributes.size();
  }

  std::string Attribute::getValue(){
    return value;
  }

  std::string Attribute::getType(){
    return type;
  }

  void Attribute::setValue(std::string newValue){
    value=newValue;
  }

  void Attribute::setType(std::string newType){
    type=newType;
  }
}
