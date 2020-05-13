// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2012 Tim Grocki <grocki@codesign.informatik.uni-erlangen.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#include <systemcvpc/Timing.hpp>
#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/TimingModifier.hpp>

namespace SystemC_VPC {

Timing::Timing(sc_core::sc_time const &delay)
  : function_("")
  , dii_(delay), latency_(delay)
  , powerMode_("DEFAULT")
  , timingModifier_(new TimingModifier)
  {}

Timing::Timing(
    sc_core::sc_time const &dii
  , sc_core::sc_time const &latency)
  : function_("")
  , dii_(dii), latency_(latency)
  , powerMode_("DEFAULT")
  , timingModifier_(new TimingModifier)
  {}

Timing::Timing(std::string const &function
  , sc_core::sc_time const &delay)
  : function_(function)
  , dii_(delay), latency_(delay)
  , powerMode_("DEFAULT")
  , timingModifier_(new TimingModifier)
  {}

Timing::Timing(std::string const &function
  , sc_core::sc_time const &dii
  , sc_core::sc_time const &latency)
  : function_(function)
  , dii_(dii), latency_(latency)
  , powerMode_("DEFAULT")
  , timingModifier_(new TimingModifier)
  {}

bool Timing::operator<(const Timing & other) const
{
  // use function name as unique IDs
  return this->function_ < other.function_;
}

//FunctionId Timing::getFunctionId() const
//{
//  return fid_;
//}

std::string Timing::getFunction() const
{
  return function_;
}

std::string Timing::getPowerMode() const
{
  return powerMode_;
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
  this->function_ = function_;
}

void Timing::setPowerMode(std::string powerMode_)
{
  this->powerMode_ = powerMode_;
}

} // namespace SystemC_VPC
