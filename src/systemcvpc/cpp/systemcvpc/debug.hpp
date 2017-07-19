/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
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

#ifdef KASCPAR_PARSING
# undef DBG_ENABLE
#endif

/*
 * no guard. can be included multiple times to define or undefine debug
 * statements for some code section - debug_on.h and debug_off.h will do this
 * for you so include them instead of this file.
 *
 */

// processed multiple times. undef macros to avoid warnings.
#undef DBG
#undef DBG_OUT
#undef DBG_SC_OUT
#undef DBG_DOBJ

// make sure this stream exists
#ifndef DBG_STREAM
  #define DBG_STREAM getDbgOut()
#endif

// make DBG.*() statements disapear in non-debug builds
#if defined(ENABLE_DEBUG) && defined(DBG_ENABLE)
  #define DBG(e) e
  #define DBG_OUT(s) DBG_STREAM <<  s
  #define DBG_SC_OUT(s) DBG_STREAM << "[" << sc_core::sc_time_stamp() << "]: " << s
  #define DBG_DOBJ(o) DBG_STREAM << " Object " #o ": " << o << std::endl
#else
  #define DBG(e) do {} while(0)
  #define DBG_OUT(s) do {} while(0)
  #define DBG_SC_OUT(s) do {} while(0)
  #define DBG_DOBJ(s) do {} while(0)
#endif

#undef ENABLE_DEBUG
