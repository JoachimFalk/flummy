#include <systemcvpc/Tracing.hpp>

const SystemC_VPC::trace_value SystemC_VPC::Tracing::S_SLEEP   = ' ';
const SystemC_VPC::trace_value SystemC_VPC::Tracing::S_BLOCKED = 'b';
const SystemC_VPC::trace_value SystemC_VPC::Tracing::S_READY   = 'w';
const SystemC_VPC::trace_value SystemC_VPC::Tracing::S_RUNNING = 'R';

#ifdef VPC_ENABLE_PLAIN_TRACING
std::ostream * SystemC_VPC::Tracing::plainTrace = new CoSupport::Streams::AOStream(std::cout, "vpc.trace", "-");
#endif // VPC_ENABLE_PLAIN_TRACING
