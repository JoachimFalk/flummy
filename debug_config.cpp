#include <systemcvpc/debug_config.hpp>
#include <iostream>

#ifdef DBG_ENABLE
CoSupport::Streams::DebugOStream dbgout(std::cerr);
#endif //DBG_ENABLE
