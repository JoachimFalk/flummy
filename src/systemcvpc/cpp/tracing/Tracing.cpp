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

#include "Tracing.hpp"

namespace SystemC_VPC{
namespace Trace{

const trace_value Tracing::S_SLEEP   = ' ';
const trace_value Tracing::S_BLOCKED = 'b';
const trace_value Tracing::S_READY   = 'w';
const trace_value Tracing::S_RUNNING = 'R';

#ifdef VPC_ENABLE_PLAIN_TRACING
std::ostream * Tracing::plainTrace = new CoSupport::Streams::AOStream(std::cout, "vpc.trace", "-");
#endif // VPC_ENABLE_PLAIN_TRACING
} // namespace Trace
} // namespace SystemC_VPC
