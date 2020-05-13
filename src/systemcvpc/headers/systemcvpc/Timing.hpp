// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_TIMING_HPP
#define _INCLUDED_SYSTEMCVPC_TIMING_HPP

#include "TimingModifier.hpp"
#include "datatypes.hpp"

#include <systemc>

#include <boost/shared_ptr.hpp>

#include <string>
#include <map>

namespace SystemC_VPC {

/*
 *
 */
class Timing
{
public:
  Timing(sc_core::sc_time const &delay = sc_core::SC_ZERO_TIME);

  Timing(
      sc_core::sc_time const &dii
    , sc_core::sc_time const &latency);

  Timing(std::string const &function
    , sc_core::sc_time const &delay = sc_core::SC_ZERO_TIME);

  Timing(std::string const &function
    , sc_core::sc_time const &dii
    , sc_core::sc_time const &latency);

  sc_core::sc_time const &getDii() const
    { return dii_; }
  void                    setDii(sc_core::sc_time const &dii_)
    { this->dii_ = dii_; }
  sc_core::sc_time const &getLatency() const
    { return latency_; }
  void                    setLatency(sc_core::sc_time const &latency_)
    { this->latency_ = latency_; }

//FunctionId getFunctionId() const;
  std::string getFunction() const;
  void setFunction(std::string function_);

  std::string getPowerMode() const;
  void setPowerMode(std::string powerMode_);

  boost::shared_ptr<TimingModifier> getTimingModifier() const;
  void setTimingModifier(boost::shared_ptr<TimingModifier> timingModifier_);

  bool operator<(const Timing & other) const;
private:
  std::string function_;
  sc_core::sc_time dii_;
  sc_core::sc_time latency_;

//FunctionId fid_;
  std::string powerMode_;
  boost::shared_ptr<TimingModifier> timingModifier_;
};

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_TIMING_HPP */
