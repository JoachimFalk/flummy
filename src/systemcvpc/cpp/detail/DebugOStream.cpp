// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
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

#include "DebugOStream.hpp"

#include <iostream>

namespace SystemC_VPC { namespace Detail {

// Global CTOR initialization order is undefined between translation units.
// Hence, using a global variable CoSupport::Streams::DebugOStream dbgout does
// not insure that this variable will already have been initialized during the
// CTOR call used for other global variables. Hence, we use the below given
// helper function to guarantee this property.
DebugOStream &getDbgOut() {
  static DebugOStream *dbgout = NULL;

  if (!dbgout)
    dbgout = new DebugOStream(std::cerr);
  return *dbgout;
}

} } // namespace SystemC_VPC::Detail
