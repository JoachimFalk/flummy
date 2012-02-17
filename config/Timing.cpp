/*
 * Timing.cpp
 *
 *  Created on: Feb 4, 2011
 *      Author: streubuehr
 */

#include <systemcvpc/config/Timing.hpp>
#include <systemcvpc/Director.hpp>
//grocki: random
#include <systemcvpc/TimingModifier.hpp>
//grocki: end

namespace SystemC_VPC
{

namespace Config
{

Timing::Timing(std::string function, sc_core::sc_time dii,
    sc_core::sc_time latency) :
  function_(function), dii_(dii), latency_(latency), fid_(
      Director::createFunctionId(function)), powerMode_("SLOW"),
      timingModifier_(new TimingModifier)
{
}

Timing::Timing(std::string function, sc_core::sc_time dii) :
  function_(function), dii_(dii), latency_(dii), fid_(
      Director::createFunctionId(function)), powerMode_("SLOW"),
      timingModifier_(new TimingModifier)
{
}

Timing::Timing(sc_core::sc_time dii, sc_core::sc_time latency) :
  function_(""), dii_(dii), latency_(latency), fid_(Director::createFunctionId(
      "")), powerMode_("SLOW"),timingModifier_(new TimingModifier)
{
}

Timing::Timing(sc_core::sc_time dii) :
  function_(""), dii_(dii), latency_(dii),
      fid_(Director::createFunctionId("")), powerMode_("SLOW"),
      timingModifier_(new TimingModifier)
{
}

Timing::Timing() :
  function_(""), dii_(0, SC_NS), latency_(0, SC_NS), fid_(0),
      powerMode_("SLOW"),timingModifier_(new TimingModifier)
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

//grocki: random test
void Timing::setTimingModifier(boost::shared_ptr<TimingModifier> timingModifier_)
{
  this->timingModifier_ = timingModifier_;
}

boost::shared_ptr<TimingModifier> Timing::getTimingModifier() const
{
  return this->timingModifier_;
}

//grocki: end 

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

//
bool TimingsProvider::hasDefaultActorTiming(const std::string& actorName) const
{
    return false;
}

//
Timing TimingsProvider::getDefaultActorTiming(const std::string& actorName,const std::string &powermode) const
{
  throw ConfigException("TimingsProvider has default actor timing "
      + actorName);
}


bool DefaultTimingsProvider::hasActionTiming(const std::string &functionName,const std::string &powermode) const
{
  if (functionTimings_.find(functionName) != functionTimings_.end())
	  return functionTimings_.find(functionName)->second.find(powermode) != functionTimings_.find(functionName)->second.end();
  else
	  return false;
}

bool DefaultTimingsProvider::hasActionTimings(const std::string &functionName) const
{
  return functionTimings_.find(functionName) != functionTimings_.end();
}

Timing DefaultTimingsProvider::getActionTiming(const std::string &functionName,const std::string &powermode) const
{
  if(this->hasActionTiming(functionName,powermode)) {
    return functionTimings_.find(functionName)->second.find(powermode)->second;
  }

  throw ConfigException("DefaultTimingsProvider has NO timing for function "
      + functionName);
}

functionTimingsPM DefaultTimingsProvider::getActionTimings(const std::string &functionName) const
{
  if(this->hasActionTimings(functionName)) {
    return functionTimings_.find(functionName)->second;
  }

  throw ConfigException("DefaultTimingsProvider has NO timing for function "
      + functionName);
}

bool DefaultTimingsProvider::hasGuardTimings(const std::string &functionName) const
{
  return hasActionTimings(functionName);
}

Timing DefaultTimingsProvider::getGuardTiming(const std::string &functionName,const std::string &powermode) const
{
  return getActionTiming(functionName,powermode);
}

functionTimingsPM DefaultTimingsProvider::getGuardTimings(const std::string &functionName) const
{
	return functionTimings_.find(functionName)->second;
  //return functionTimings_.find(functionName);
}

//
bool DefaultTimingsProvider::hasDefaultActorTiming(const std::string& actorName) const
{
    return hasActionTimings(actorName);
}

//
Timing DefaultTimingsProvider::getDefaultActorTiming(const std::string& actorName,const std::string &powermode) const
{
  return getActionTiming(actorName,powermode);
}


void DefaultTimingsProvider::add(Timing timing)
{
  functionTimings_[timing.getFunction()][timing.getPowerMode()] = timing;
}

void DefaultTimingsProvider::addDefaultActorTiming(std::string actorName,Timing timing)
{
  functionTimings_[actorName][timing.getPowerMode()] = timing;
}

} // namespace Config
} // namespace SystemC_VPC
