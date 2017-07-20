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

#ifndef _INCLUDED_SYSTEMCVPC_TRACING_PAJETRACER_HPP
#define _INCLUDED_SYSTEMCVPC_TRACING_PAJETRACER_HPP

#include "TracerIf.hpp"

#include <CoSupport/Tracing/PajeTracer.hpp>

#include <vector>
#include <map>
#include <string>

namespace SystemC_VPC { namespace Trace {

class PajeTracer: public TracerIf {
public:
  //
  PajeTracer(Config::Component::Ptr component);

  ~PajeTracer();

  void release(Task const *task);

  void finishDii(Task const *task);

  void finishLatency(Task const *task);

  void assign(Task const *task);

  void resign(Task const *task);

  void block(Task const *task);

  Tracing *getOrCreateTraceSignal(std::string const &name);

protected:
  unsigned int keyCounter;
  int getNextKey();

private:
  std::string getName() const;

  std::string name_;
  std::map<std::string, Trace::Tracing*> trace_map_by_name_;
  CoSupport::Tracing::PajeTracer::Resource const *res_;
  sc_core::sc_time startTime;

  typedef std::map<std::string, CoSupport::Tracing::PajeTracer::Activity*> TaskToActivity;
  TaskToActivity taskToActivity;

  typedef std::map<std::string, CoSupport::Tracing::PajeTracer::Event*> TaskToEvent;
  TaskToEvent taskToEvent;

  typedef std::map<std::string, sc_core::sc_time> TaskToEndTime;
  TaskToEndTime taskToEndTime;

  typedef std::map<std::string, const CoSupport::Tracing::PajeTracer::Resource*> TaskToResource;
  TaskToResource taskToResource;

  typedef std::map<std::string, std::string> TaskToPreTask;
  TaskToPreTask taskToPreTask;

  typedef std::map<std::string, std::string> TaskToDestTask;
  TaskToDestTask taskToDestTask;

};

} } // namespace SystemC_VPC::Trace

#endif /* _INCLUDED_SYSTEMCVPC_TRACING_PAJETRACER_HPP */
