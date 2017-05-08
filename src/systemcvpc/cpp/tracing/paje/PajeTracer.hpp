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

#ifndef _INCLUDED_SYSTEMCVPC_TRACING_PAJE_PAJETRACER_HPP
#define _INCLUDED_SYSTEMCVPC_TRACING_PAJE_PAJETRACER_HPP

#include <systemcvpc/ProcessControlBlock.hpp>
#include <systemcvpc/Task.hpp>
#include <systemcvpc/config/Component.hpp>

#include "../vcd/Tracing.hpp"

#include <CoSupport/Tracing/PajeTracer.hpp>
#include <CoSupport/String/color.hpp>

#include <vector>
//#include <map>
using namespace std;

namespace SystemC_VPC { namespace Trace {

class PajeTracer {
public:
  //
  PajeTracer(Config::Component::Ptr component);

  ~PajeTracer();

  std::string getName() const;

  void release(Task * task);

  void finishDii(Task * task) const;

  void finishLatency(Task * task) const;

  void assign(Task * task);

  void resign(Task * task) const;

  void block(Task * task) const;

  Tracing *getOrCreateTraceSignal(std::string name);


private:
  sc_trace_file *traceFile_;
  string name_;
  map<std::string, Trace::Tracing*> trace_map_by_name_;
  CoSupport::Tracing::PajeTracer::Resource const *res_;
  map<string, CoSupport::Tracing::PajeTracer::Activity*> my_map_;

};

} } // namespace SystemC_VPC::Trace

#endif /* _INCLUDED_SYSTEMCVPC_TRACING_PAJE_PAJETRACER_HPP */
