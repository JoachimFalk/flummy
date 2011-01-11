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
CoSupport::Streams::DebugOStream dbgout(std::cerr);
#endif //DBG_ENABLE
