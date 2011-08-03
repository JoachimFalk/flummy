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

#include <systemcvpc/config/common.hpp>

#include <string>
#include <iostream>

namespace SystemC_VPC
{
namespace Config
{

//
Traceable::Type Traceable::parseTracing(std::string name)
{
  if (name == "NONE") {
    return NONE;
  } else if (name == "VCD"){
    return VCD;
  } else {
    std::cerr << "[VPC warning] unknown tracing option: " << name << std::endl;
  }
  return VCD;
}

//
void Traceable::setTracing(Traceable::Type tracing)
{
  tracing_ = tracing;
}

//
Traceable::Type Traceable::getTracing() const
{
  return tracing_;
}

//
Traceable::Traceable() :
  tracing_(Traceable::VCD)
{
}

} // namespace Config
} // namespace SystemC_VPC
