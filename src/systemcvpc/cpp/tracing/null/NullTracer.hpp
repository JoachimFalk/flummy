/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#ifndef _INCLUDED_SYSTEMCVPC_TRACING_NULL_NULLTRACER_HPP
#define _INCLUDED_SYSTEMCVPC_TRACING_NULL_NULLTRACER_HPP

#include <systemcvpc/Task.hpp>
#include <systemcvpc/config/Component.hpp>

namespace SystemC_VPC { namespace Trace {

// FIXME: Remove this after method getOrCreateTraceSignal has been removed!
class Tracing;

class NullTracer {
public:
  //
  NullTracer(Config::Component::Ptr component)
    {}

  void release(const Task * task) const
    {}

  void finishDii(const Task * task) const
    {}

  void finishLatency(const Task * task) const
    {}

  void assign(const Task * task) const
    {}

  void resign(const Task * task) const
    {}

  void block(const Task * task) const
    {}

  // TODO: Can we avoid this function somehow?
  Tracing *getOrCreateTraceSignal(const std::string name) const
    { return NULL; }

};

} } // namespace SystemC_VPC::Trace

#endif /* _INCLUDED_SYSTEMCVPC_TRACING_NULL_NULLTRACER_HPP */
