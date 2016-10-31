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

#include <systemcvpc/debug_config.hpp>
#include <iostream>

#ifdef DBG_ENABLE
static CoSupport::Streams::DebugOStream *dbgout = NULL;

// Global CTOR initialization order is undefined between translation units.
// Hence, using a global variable CoSupport::Streams::DebugOStream dbgout does
// not insure that this variable will already have been initialized during the
// CTOR call used for other global variables. Hence, we use the below given
// helper function to guarantee this property.
std::ostream &getDbgOut() {
  if (!dbgout)
    dbgout = new CoSupport::Streams::DebugOStream(std::cerr);
  return *dbgout;
}
#endif //DBG_ENABLE
