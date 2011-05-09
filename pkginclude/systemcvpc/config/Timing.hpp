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

#ifndef TIMING_H_
#define TIMING_H_

#include <systemcvpc/FastLink.hpp>

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
#endif /* TIMING_H_ */
