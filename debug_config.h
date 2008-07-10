#include <CoSupport/Streams/DebugOStream.hpp>

#ifndef INCLUDED__DEBUG_CONFIG__H__
#define INCLUDED__DEBUG_CONFIG__H__

/** enable/disable debugging for any module */
#define DBG_ENABLE

#ifdef DBG_ENABLE
extern CoSupport::Streams::DebugOStream dbgout;
#endif //DBG_ENABLE

/** enable/disable debugging for StaticRoute */
#define DBG_STATIC_ROUTE

/** enable/disable debugging for Director */
#define DBG_DIRECTOR

/** enable/disable debugging for Component */
#define DBG_COMPONENT

/** enable/disable debugging for ProcessControlBlock */
#define DBG_PCB

/** enable/disable debugging for VPCBuilder */
#define DBG_VPCBUILDER
#endif // INCLUDED__DEBUG_CONFIG__H__
