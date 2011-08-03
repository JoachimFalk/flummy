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

#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <string>

namespace SystemC_VPC
{
namespace Config
{

class Traceable
{
public:
  enum Type
  {
    VCD, // trace to vcd file
    NONE // disable tracing
  };

  static Type parseTracing(std::string name);
  Traceable();
  void setTracing(Type tracing);
  Type getTracing() const;
private:
  Type tracing_;
};

} // namespace Config
} // namespace SystemC_VPC
#endif /* COMMON_HPP_ */
