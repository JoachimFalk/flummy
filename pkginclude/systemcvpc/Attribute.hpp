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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <boost/shared_ptr.hpp>

#include <deque>
#include <utility>
#include <iostream>
#include <systemcvpc/InvalidArgumentException.hpp>

namespace SystemC_VPC{

  class Attribute;

  typedef boost::shared_ptr<Attribute> AttributePtr;
  typedef std::deque<std::pair<std::string, AttributePtr> >  Attributes;
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

    std::pair<std::string, AttributePtr > getNextAttribute(size_t pos)
      throw (InvalidArgumentException);

    /**
     *
     */
    AttributePtr getAttribute(const std::string name)
      throw (InvalidArgumentException);

    /**
     *
     */
    bool hasAttribute(const std::string name);

    void addAttribute( std::string type, std::string value);

    void addAttribute( std::string type, AttributePtr att );

    size_t getParameterSize();
    size_t getAttributeSize();

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

    ~Attribute(){}
  private:
    std::string type;
    std::string value;

    Parameters  parameters;
    Attributes  attributes;
  };
}

#endif

