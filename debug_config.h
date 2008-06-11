#include <CoSupport/Streams/DebugOStream.hpp>

#ifndef INCLUDED__DEBUG_CONFIG__H__
#define INCLUDED__DEBUG_CONFIG__H__

/** enable/disable debugging for any module */
//#define DBG_ENABLE

#ifdef DBG_ENABLE
extern CoSupport::DebugOstream dbgout;
#endif //DBG_ENABLE

/** enable/disable debugging for StaticRoute */
#define DBG_STATIC_ROUTE

/** enable/disable debugging for Director */
#define DBG_DIRECTOR

/** enable/disable debugging for Component */
#define DBG_COMPONENT

#endif // INCLUDED__DEBUG_CONFIG__H__
