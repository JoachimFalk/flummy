/*
 * Timing.cpp
 *
 *  Created on: Feb 4, 2011
 *      Author: streubuehr
 */

#include <systemcvpc/config/Timing.hpp>
#include <systemcvpc/Director.hpp>

namespace SystemC_VPC
{

namespace Config
{

Timing::Timing(std::string function, sc_core::sc_time dii,
    sc_core::sc_time latency) :
  function_(function), dii_(dii), latency_(latency), fid_(
      Director::createFunctionId(function)), powerMode_("SLOW")
{
}

Timing::Timing(std::string function, sc_core::sc_time dii) :
  function_(function), dii_(dii), latency_(dii), fid_(
      Director::createFunctionId(function)), powerMode_("SLOW")
{
}

Timing::Timing() :
  function_(""), dii_(0, SC_NS), latency_(0, SC_NS), fid_(0),
      powerMode_("SLOW")
{
}

bool Timing::operator<(const Timing & other) const
{
  // use function name as unique IDs
  return this->function_ < other.function_;
}

sc_core::sc_time Timing::getDii() const
{
  return dii_;
}

FunctionId Timing::getFunctionId() const
{
  return fid_;
}

std::string Timing::getFunction() const
{
  return function_;
}

sc_core::sc_time Timing::getLatency() const
{
  return latency_;
}

std::string Timing::getPowerMode() const
{
  return powerMode_;
}

void Timing::setDii(sc_core::sc_time dii_)
{
  this->dii_ = dii_;
}

void Timing::setFunction(std::string function_)
{
  this->fid_ = Director::createFunctionId(function_);
  this->function_ = function_;
}

void Timing::setLatency(sc_core::sc_time latency_)
{
  this->latency_ = latency_;
}

void Timing::setPowerMode(std::string powerMode_)
{
  this->powerMode_ = powerMode_;
}

} // namespace Config
} // namespace SystemC_VPC
