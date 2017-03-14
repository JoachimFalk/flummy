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

#ifndef _INCLUDED_SYSTEMCVPC_TRACING_DB_DATABASETRACER_HPP
#define _INCLUDED_SYSTEMCVPC_TRACING_DB_DATABASETRACER_HPP

#include <systemcvpc/Task.hpp>
#include <systemcvpc/config/Component.hpp>

#include <systemc.h>

#include <CoSupport/compatibility-glue/integertypes.h>

//#define VPC_ENABLE_PLAIN_TRACING
#include <CoSupport/Streams/AlternateStream.hpp>

namespace SystemC_VPC { namespace Trace {

// FIXME: Remove this after method getOrCreateTraceSignal has been removed!
class Tracing;

class DataBaseProxy {
public:
  static DataBaseProxy & getDataBaseProxy()
  {
    static DataBaseProxy dbProxy("VPC");
    return dbProxy;
  }

  ~DataBaseProxy();

  // addEvent
  void addEvent(const char* resourceName, const char* taskName,
      const char* status, unsigned long long timeStamp, unsigned int taskId);

private:
  const char* databaseName_;
  uint16_t portNumber_;
  FILE* socket_;

  // create database with given name
  DataBaseProxy(const char* database_name, uint16_t port = 5555);

  void open();

  void close();
};

class DataBaseTracer
{
public:
  //
  DataBaseTracer(Config::Component::Ptr component) :
    dbProxy_(DataBaseProxy::getDataBaseProxy()),
        resourceName_(component->getName())
  {
  }

  void release(const Task * task)
  {
    this->addEvent(task, "s");
  }

  void finishDii(const Task * task)
  {
    this->addEvent(task, "d");
  }

  void finishLatency(const Task * task)
  {
    this->addEvent(task, "l");
  }

  void assign(const Task * task)
  {
    this->addEvent(task, "a");
  }

  void resign(const Task * task)
  {
    this->addEvent(task, "r");
  }

  void block(const Task * task)
  {
    this->addEvent(task, "b");
  }

  // TODO: Can we avoid this function somehow?
  Tracing *getOrCreateTraceSignal(const std::string name) const
    { return NULL; }
private:

  void addEvent(const Task * task, const char * state)
  {
    dbProxy_.addEvent(resourceName_.c_str(),
        task->getName().c_str(),
        state,
        sc_time_stamp().value(),
        task->getInstanceId());
  }
  DataBaseProxy & dbProxy_;
  std::string resourceName_;
};

} } // namespace SystemC_VPC::Trace

#endif // _INCLUDED_SYSTEMCVPC_TRACING_DB_DATABASETRACER_HPP
