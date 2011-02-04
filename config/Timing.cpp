/*
 * Timing.cpp
 *
 *  Created on: Feb 4, 2011
 *      Author: streubuehr
 */

#include <systemcvpc/config/Timing.hpp>

namespace SystemC_VPC
{

namespace Config
{

Timing::Timing(std::string function, sc_core::sc_time dii, sc_core::sc_time latency)
{
}

Timing::Timing(std::string function, sc_core::sc_time dii)
{
}

bool Timing::operator<(const Timing & other) const
{
  // use function name as unique IDs
  return this->function_ < other.function_;
}

} // namespace Config
} // namespace SystemC_VPC
