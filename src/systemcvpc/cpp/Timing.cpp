// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#include <systemcvpc/Timing.hpp>
#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/TimingModifier.hpp>

//#include "detail/Director.hpp"

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
//this->fid_ = Detail::Director::createFunctionId(function_);
  this->function_ = function_;
}

void Timing::setPowerMode(std::string powerMode_)
{
  this->powerMode_ = powerMode_;
}

} // namespace SystemC_VPC
