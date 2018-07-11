// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2018 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SYSTEMCVPC_DEBUGOSTREAM_HPP
#define _INCLUDED_SYSTEMCVPC_DEBUGOSTREAM_HPP

#include <systemcvpc/vpc_config.h>

#include <CoSupport/Streams/NullStreambuf.hpp>
#include <CoSupport/Streams/DebugStreambuf.hpp>
#include <CoSupport/Streams/IndentStreambuf.hpp>
#include <CoSupport/Streams/HeaderFooterStreambuf.hpp>

namespace SystemC_VPC { namespace Detail {

using CoSupport::Streams::Debug;
using CoSupport::Streams::ScopedDebug;
using CoSupport::Streams::Indent;
using CoSupport::Streams::ScopedIndent;

typedef
#ifndef SYSTEMCVPC_ENABLE_DEBUG
  CoSupport::Streams::NullStreambuf::Stream<
#endif //SYSTEMCVPC_ENABLE_DEBUG
    CoSupport::Streams::DebugStreambuf::Stream<
      CoSupport::Streams::IndentStreambuf::Stream<
        CoSupport::Streams::HeaderFooterStreambuf::Stream<
    > > >
#ifndef SYSTEMCVPC_ENABLE_DEBUG
  >
#endif //SYSTEMCVPC_ENABLE_DEBUG
  DebugOStream;

// Global CTOR initialization order is undefined between translation units.
// Hence, using a global variable CoSupport::Streams::DebugOStream dbgout does
// not insure that this variable will already have been initialized during the
// CTOR call used for other global variables. Hence, we use the below given
// helper function to guarantee this property.
extern DebugOStream &getDbgOut();

} } // namespace SystemC_VPC::Detail

// set for debugging output
//#define VPC_DEBUG true;

#define DBG_STREAM getDbgOut()
#ifndef SYSTEMCVPC_ENABLE_DEBUG
# define DBG_OUT(s)    do {} while(0)
# define DBG_SC_OUT(s) do {} while(0)
# define DBG_DOBJ(s)   do {} while(0)
#else //defined(SYSTEMCVPC_ENABLE_DEBUG)
# define DBG_OUT(s)    do { if (DBG_STREAM.isVisible(Debug::Low)) { DBG_STREAM <<  s; } } while (0)
# define DBG_SC_OUT(s) do { if (DBG_STREAM.isVisible(Debug::Low)) { DBG_STREAM << "[" << sc_core::sc_time_stamp() << "]: " << s; } } while (0)
# define DBG_DOBJ(o)   do { if (DBG_STREAM.isVisible(Debug::Low)) { DBG_STREAM << " Object " #o ": " << o << std::endl; } } while (0)
#endif //defined(SYSTEMCVPC_ENABLE_DEBUG)

#endif // _INCLUDED_SYSTEMCVPC_DEBUGOSTREAM_HPP
