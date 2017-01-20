/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <systemcvpc/config/Timing.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/TimingModifier.hpp>

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

void Timing::setTimingModifier(boost::shared_ptr<TimingModifier> timingModifier_)
{
  this->timingModifier_ = timingModifier_;
}

boost::shared_ptr<TimingModifier> Timing::getTimingModifier() const
{
  return this->timingModifier_;
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
