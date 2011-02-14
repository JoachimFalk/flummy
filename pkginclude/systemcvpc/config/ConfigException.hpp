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

#ifndef CONFIGEXCEPTION_H_
#define CONFIGEXCEPTION_H_

#include <stdexcept>
#include <string>

namespace SystemC_VPC
{

class ScheduledTask;

namespace Config
{
class ConfigException: public std::runtime_error
{
public:
  ConfigException(std::string msg) :
    std::runtime_error(msg)
  {

  }
};

}

}

#endif /* CONFIGEXCEPTION_H_ */
