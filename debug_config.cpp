#include <debug_config.h>
#include <iostream>

#ifdef DBG_ENABLE
CoSupport::DebugOstream dbgout(std::cerr);
#endif //DBG_ENABLE
