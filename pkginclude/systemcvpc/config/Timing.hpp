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

/*
 *
 */
class TimingsProvider
{
public:
  typedef boost::shared_ptr<TimingsProvider> Ptr;

  virtual bool has(const std::string &functionName) const = 0;
  virtual Timing get(const std::string &functionName) const = 0;
  virtual ~TimingsProvider() {}
};

/*
 *
 */
class DefaultTimingsProvider : public TimingsProvider
{
public:
  typedef boost::shared_ptr<DefaultTimingsProvider> Ptr;

  virtual bool has(const std::string &functionName) const;
  virtual Timing get(const std::string &functionName) const;

  virtual void add(Timing timing);
private:
  std::map<std::string, Timing> timings_;
};


} // namespace Config
} // namespace SystemC_VPC
#endif /* TIMING_H_ */
