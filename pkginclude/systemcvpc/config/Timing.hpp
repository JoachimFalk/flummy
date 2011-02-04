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

#include <string>

#include <systemc>

namespace SystemC_VPC
{

namespace Config
{

class Timing
{
public:
  Timing(std::string function, sc_core::sc_time dii, sc_core::sc_time latency);
  Timing(std::string function, sc_core::sc_time dii);
  bool operator<(const Timing & other) const;
private:
  std::string function_;
  sc_core::sc_time dii_;
  sc_core::sc_time latency_;
};
} // namespace Config
} // namespace SystemC_VPC
#endif /* TIMING_H_ */
