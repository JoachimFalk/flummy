// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_DIAGNOSTICS_DEBUGOUTPUT_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_DIAGNOSTICS_DEBUGOUTPUT_HPP

#include <CoSupport/Streams/AlternateStream.hpp>

//FIXME: There is also DebugOStream.hpp
namespace SystemC_VPC { namespace Detail { namespace Diagnostics {

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

} } } // namespace SystemC_VPC::Detail::Diagnostics

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_DIAGNOSTICS_DEBUGOUTPUT_HPP */
