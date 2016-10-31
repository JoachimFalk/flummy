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

#include "ConfigCheck.hpp"

namespace SystemC_VPC
{
void dumpSet(FunctionSet s, std::ostream & stream = std::cerr)
{
  std::copy(s.begin(), s.end(),
      std::ostream_iterator<std::string>(stream, "  "));
}

void TaskConfig::check()
{
//  std::cerr << "   configured:\t";
//  dumpSet(configuredFunctionSet);
//  std::cerr << std::endl;
//
//  std::cerr << "   modelled:\t";
//  dumpSet(modelledFunctionSet);
//  std::cerr << std::endl;

  FunctionSet overSpecified;
  FunctionSet underSpecified;

  std::set_difference(configuredFunctionSet.begin(),
      configuredFunctionSet.end(), modelledFunctionSet.begin(),
      modelledFunctionSet.end(), std::inserter(overSpecified,
          overSpecified.begin()));
  std::set_difference(modelledFunctionSet.begin(), modelledFunctionSet.end(),
      configuredFunctionSet.begin(), configuredFunctionSet.end(),
      std::inserter(underSpecified, underSpecified.begin()));

  if (!overSpecified.empty()) {
    //FIXME:
//    std::cerr << "[VPC warning] These functions are configured in the XML file, but not used in the SysteMoC model:\t";
//    dumpSet(overSpecified);
//    std::cerr << std::endl;
  }

  if (!underSpecified.empty()) {
    //FIXME:
//    std::cerr << "[VPC warning] These functions are modelled in SysteMoC, but not configured in the XML file:\t";
//    dumpSet(underSpecified);
//    std::cerr << std::endl;
  }

}

//
std::map<ProcessId, std::string> &
ConfigCheck::processNames()
{
  static std::map<ProcessId, std::string> names;
  return names;
}

//
std::map<ProcessId, std::pair<std::string, std::string> > &
ConfigCheck::routeNames()
{
  static std::map<ProcessId, std::pair<std::string, std::string> > names;
  return names;
}

//
void
ConfigCheck::setProcessName(ProcessId pid, std::string name)
{
  processNames()[pid] = name;
}

//
void
ConfigCheck::setRouteName(ProcessId pid, std::string src, std::string dest)
{
  routeNames()[pid] = std::make_pair(src, dest);
}

//
bool ConfigCheck::hasProcessName(ProcessId pid)
{
  return ConfigCheck::processNames().find(pid) != ConfigCheck::processNames().end();
}

//
bool ConfigCheck::hasRouteName(ProcessId pid)
{
  return ConfigCheck::routeNames().find(pid) != ConfigCheck::routeNames().end();
}
//

std::string ConfigCheck::getProcessName(ProcessId pid)
{
  assert(hasProcessName(pid));
  return ConfigCheck::processNames()[pid];
}

//
std::pair<std::string, std::string> ConfigCheck::getRouteName(ProcessId pid)
{
  assert(hasRouteName(pid));
  return ConfigCheck::routeNames()[pid];
}


} // namespace SystemC_VPC
