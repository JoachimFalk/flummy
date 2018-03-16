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

#ifndef _INCLUDED_SYSTEMCVPC_CONFIG_TIMING_HPP
#define _INCLUDED_SYSTEMCVPC_CONFIG_TIMING_HPP

#include "../FastLink.hpp"
#include <boost/smart_ptr/shared_ptr.hpp>
#include "../TimingModifier.hpp"

#include <boost/shared_ptr.hpp>

#include <string>
#include <map>

#include <systemc>

namespace SystemC_VPC
{

namespace Config
{




/*
 *
 */
class Timing
{
public:
  Timing(std::string function, sc_core::sc_time dii, sc_core::sc_time latency);

  Timing(std::string function, sc_core::sc_time dii);

  Timing(sc_core::sc_time dii, sc_core::sc_time latency);

  Timing(sc_core::sc_time dii);

  Timing();

  bool operator<(const Timing & other) const;
  sc_core::sc_time getDii() const;
  FunctionId getFunctionId() const;
  std::string getFunction() const;
  sc_core::sc_time getLatency() const;
  std::string getPowerMode() const;
  void setTimingModifier(boost::shared_ptr<TimingModifier> timingModifier_);
  boost::shared_ptr<TimingModifier> getTimingModifier() const;

  void setDii(sc_core::sc_time dii_);
  void setFunction(std::string function_);
  void setLatency(sc_core::sc_time latency_);
  void setPowerMode(std::string powerMode_);
private:
  std::string function_;
  sc_core::sc_time dii_;
  sc_core::sc_time latency_;

  FunctionId fid_;
  std::string powerMode_;
  boost::shared_ptr<TimingModifier> timingModifier_;
};

typedef std::map<std::string, Timing> functionTimingsPM;

/*
 *
 */
class TimingsProvider
{


public:
  typedef boost::shared_ptr<const TimingsProvider> Ptr;

  virtual bool hasActionTiming(const std::string &functionName,const std::string &powermode) const = 0;
  virtual bool hasActionTimings(const std::string &functionName) const = 0;
  virtual Timing getActionTiming(const std::string &functionName,const std::string &powermode) const = 0;
  virtual functionTimingsPM getActionTimings(const std::string &functionName) const = 0;
  //virtual bool hasGuardTiming(const std::string &functionName,const std::string &powermode) const = 0;
  virtual bool hasGuardTimings(const std::string &functionName) const = 0;
  virtual Timing getGuardTiming(const std::string &functionName,const std::string &powermode) const = 0;
  virtual functionTimingsPM getGuardTimings(const std::string &functionName) const = 0;

  //optional interface: default implementation returns false
  virtual bool hasDefaultActorTiming(const std::string& actorName) const;

  //optional interface: default implementation throws error
  virtual Timing getDefaultActorTiming(const std::string& actorName,const std::string &powermode) const;

  virtual ~TimingsProvider() {}
};

/*
 *
 */
class DefaultTimingsProvider : public TimingsProvider
{
public:
  typedef boost::shared_ptr<DefaultTimingsProvider> Ptr;

  virtual bool hasActionTiming(const std::string &functionName,const std::string &powermode) const;
  virtual bool hasActionTimings(const std::string &functionName) const;
  virtual Timing getActionTiming(const std::string &functionName,const std::string &powermode) const;
  virtual functionTimingsPM getActionTimings(const std::string &functionName) const;
  //virtual bool hasGuardTiming(const std::string &functionName,const std::string &powermode) const;
  virtual bool hasGuardTimings(const std::string &functionName) const;
  virtual Timing getGuardTiming(const std::string &functionName,const std::string &powermode) const;
  virtual functionTimingsPM getGuardTimings(const std::string &functionName) const;

  virtual bool hasDefaultActorTiming(const std::string& actorName) const;
  virtual Timing getDefaultActorTiming(const std::string& actorName,const std::string &powermode) const;

  virtual void add(Timing timing);
  virtual void addDefaultActorTiming(std::string actorName, Timing timing);
private:
  std::map<std::string, functionTimingsPM> functionTimings_;

};



} // namespace Config
} // namespace SystemC_VPC
#endif /* _INCLUDED_SYSTEMCVPC_CONFIG_TIMING_HPP */
