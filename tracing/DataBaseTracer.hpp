/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#ifndef HSCD_VPC_DATABASETRACER_H
#define HSCD_VPC_DATABASETRACER_H

#include <systemcvpc/Task.hpp>
#include <systemcvpc/config/Component.hpp>

#include <systemc.h>

#include <CoSupport/compatibility-glue/integertypes.h>

//#define VPC_ENABLE_PLAIN_TRACING
#include <CoSupport/Streams/AlternateStream.hpp>

namespace SystemC_VPC
{
namespace Trace
{
class DataBaseProxy
{

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

class DataBaseTrace
{
public:
  //
  DataBaseTrace(Config::Component::Ptr component) :
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
  Tracing * getOrCreateTraceSignal(const std::string name) const
  {
    return NULL;
  }
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

} // namespace Trace
} // namespace SystemC_VPC
#endif // HSCD_VPC_DATABASETRACER_H
