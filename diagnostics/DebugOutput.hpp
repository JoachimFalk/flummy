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

#ifndef DEBUGOUTPUT_HPP_
#define DEBUGOUTPUT_HPP_

#include <CoSupport/Streams/AlternateStream.hpp>

namespace SystemC_VPC
{
namespace Diagnostics
{

class PrintDebug: public CoSupport::Streams::AOStream
{
public:
  PrintDebug(std::string fileName) :
    CoSupport::Streams::AOStream(std::cout, fileName, "-")
  {
  }
};

class DiscardOutput: public std::ostream
{
public:

  DiscardOutput(std::string fileName) : std::ostream() {}

  /// discard output for any type
  template<class T>
  inline const DiscardOutput &operator<<(const T &t) const
  {
    return *this;
  }

  /// discard output for stream manipulators
  inline const DiscardOutput &operator<<(std::ostream &(*manip)(std::ostream &)) const
  {
    return *this;
  }

  /// discard output for stream manipulators
  inline const DiscardOutput &operator<<(std::ios &(*manip)(std::ios &)) const
  {
    return *this;
  }

  /// discard output for stream manipulators
  inline const DiscardOutput &operator<<(
      std::ios_base &(*manip)(std::ios_base &)) const
  {
    return *this;
  }
};

} // SystemC_VPC
} // Diagnostics
#endif /* DEBUGOUTPUT_HPP_ */
