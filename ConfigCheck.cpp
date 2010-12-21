/*
 * ConfigCheck.cpp
 *
 *  Created on: 21 Dec 2010
 *      Author: streubue
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
    std::cerr << "[VPC warning] These functions are configured in the XML file, but not used in the SysteMoC model:\t";
    dumpSet(overSpecified);
    std::cerr << std::endl;
  }

  if (!underSpecified.empty()) {
    std::cerr << "[VPC warning] These functions are modelled in SysteMoC, but not configured in the XML file:\t";
    dumpSet(underSpecified);
    std::cerr << std::endl;
  }

}
} // namespace SystemC_VPC
