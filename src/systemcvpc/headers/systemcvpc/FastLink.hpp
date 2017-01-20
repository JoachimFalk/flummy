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

#ifndef __INCLUDED__FASTLINK__H__
#define __INCLUDED__FASTLINK__H__
#include <systemc.h>
#include "EventPair.hpp"
#include "datatypes.hpp"

#include <vector>

namespace SystemC_VPC{

typedef std::vector<FunctionId> FunctionIds;

  /**
   *
   */
  class FastLink{
  public:

    /**
     *
     */
    void compute( EventPair p ) const;

    /**
     *
     */
    void write( size_t quantum, EventPair p ) const;

    /**
     *
     */
    void read( size_t quantum, EventPair p ) const;

    /**
     *
     */
    ComponentId getComponentId() const;

    /**
     *
     */
    void addDelay(size_t delay_ns){
      addDelay(sc_time(delay_ns, SC_NS));
    }

    /**
     *
     */
    void addDelay(sc_time delay);

    FastLink(ProcessId pid, FunctionIds fid)
      : process(pid),
      functions(fid),
      extraDelay(SC_ZERO_TIME)
    { }

    FastLink()
      : process(),
      functions(),
      extraDelay(SC_ZERO_TIME)
    { }

    ProcessId            process;
    FunctionIds          functions;

  private:
    mutable sc_time      extraDelay;
  };

  static const FunctionId defaultFunctionId = 0;
}

#endif // __INCLUDED__FASTLINK__H__
