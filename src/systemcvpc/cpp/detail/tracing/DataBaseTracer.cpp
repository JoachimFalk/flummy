// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of
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

#include "DataBaseTracer.hpp"

#include <sys/socket.h>
#include <netdb.h>

namespace SystemC_VPC { namespace Detail { namespace Tracing {

class DataBaseTracer::DataBaseProxy {
public:
  static DataBaseProxy &getDataBaseProxy() {
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

DataBaseTracer::DataBaseProxy::DataBaseProxy(const char* database_name, uint16_t port)
  : databaseName_(database_name)
  , portNumber_(port)
  { open(); }

DataBaseTracer::DataBaseProxy::~DataBaseProxy()
  { close(); }

void DataBaseTracer::DataBaseProxy::addEvent(
    const char *resourceName,
    const char *taskName,
    const char *status,
    const unsigned long long timeStamp,
    const unsigned int taskId)
{
  fprintf(socket_, "%s %s %s %llu %u\r\n", resourceName, taskName, status,
      timeStamp, taskId);
}

void DataBaseTracer::DataBaseProxy::open() {

  struct sockaddr_in6 socketAddr;
  int socketFD;

  socketAddr.sin6_family = AF_INET6;
  socketAddr.sin6_addr = in6addr_any;
  socketAddr.sin6_port = htons(portNumber_);

  if ((socketFD = socket(PF_INET6, SOCK_STREAM, 0)) == -1) {
    perror("server socket");
    exit(EXIT_FAILURE);
  }

  if (connect(socketFD, (const struct sockaddr *) &socketAddr,
      sizeof(socketAddr)) == -1) {
    perror("connection error");
    exit(EXIT_FAILURE);
  }

  if ((socket_ = fdopen(socketFD, "a+")) == nullptr) {
    perror("fdopen");
    exit(EXIT_FAILURE);
  }

  fprintf(socket_, "%s\r\n", databaseName_);
}

void DataBaseTracer::DataBaseProxy::close() {
  fprintf(socket_, "CLOSE\r\n");
}

class DataBaseTracer::DBTask: public TTask {
public:
  DBTask(std::string const &name)
    : name(name) {}

  std::string name;

  ~DBTask() {}
};

class DataBaseTracer::DBTaskInstance: public TTaskInstance {
public:
  DBTaskInstance(DBTask *dbTask)
    : dbTask(dbTask), instanceId(instanceIdCounter++) {}

  static size_t instanceIdCounter;

  DBTask *dbTask;
  size_t  instanceId;

  ~DBTaskInstance() {}
};

size_t DataBaseTracer::DBTaskInstance::instanceIdCounter = 0;

DataBaseTracer::DataBaseTracer(SystemC_VPC::Component::Ptr component)
  : dbProxy_(DataBaseProxy::getDataBaseProxy())
  , resourceName_(component->getName()) {}

TTask         *DataBaseTracer::registerTask(std::string const &name)
  { return new DBTask(name); }

TTaskInstance *DataBaseTracer::release(TTask *ttask) {
  TTaskInstance *ttaskInstance =
      new DBTaskInstance(static_cast<DBTask *>(ttask));
  this->addEvent(ttaskInstance, "s");
  return ttaskInstance;
}

void DataBaseTracer::finishDii(TTaskInstance *ttaskInstance)
  { this->addEvent(ttaskInstance, "d"); }

void DataBaseTracer::finishLatency(TTaskInstance *ttaskInstance)
  { this->addEvent(ttaskInstance, "l"); }

void DataBaseTracer::assign(TTaskInstance *ttaskInstance)
  { this->addEvent(ttaskInstance, "a"); }

void DataBaseTracer::resign(TTaskInstance *ttaskInstance)
  { this->addEvent(ttaskInstance, "r"); }

void DataBaseTracer::block(TTaskInstance *ttaskInstance)
  { this->addEvent(ttaskInstance, "b"); }

void DataBaseTracer::addEvent(TTaskInstance const *ttaskInstance_, char const *state) {
  DBTaskInstance const *ttaskInstance = static_cast<DBTaskInstance const *>(ttaskInstance_);

  dbProxy_.addEvent(resourceName_.c_str(),
      ttaskInstance->dbTask->name.c_str(),
      state,
      sc_core::sc_time_stamp().value(),
      ttaskInstance->instanceId);
}

} } } // namespace SystemC_VPC::Detail::Tracing
