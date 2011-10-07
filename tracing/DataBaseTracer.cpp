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

#include "DataBaseTracer.hpp"

#include <sys/socket.h>
#include <netdb.h>

namespace SystemC_VPC
{
namespace Trace
{

DataBaseProxy::DataBaseProxy(const char* database_name, uint16_t port) :
  databaseName_(database_name), portNumber_(port)
{
  open();
}

DataBaseProxy::~DataBaseProxy()
{
  close();
}

void DataBaseProxy::addEvent(const char* resourceName, const char* taskName,
    const char* status, double timeStamp, unsigned int taskId)
{
  fprintf(socket_, "%s %s %s %f %u\r\n", resourceName, taskName, status,
      timeStamp, taskId);
  //  printf("%s %s %s %f %u\r\n", resourceName, taskName, status, timeStamp,
  //      taskId);
}

void DataBaseProxy::open()
{

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

  if ((socket_ = fdopen(socketFD, "a+")) == NULL) {
    perror("fdopen");
    exit(EXIT_FAILURE);
  }

  fprintf(socket_, "%s\r\n", databaseName_);
}

void DataBaseProxy::close()
{
  fprintf(socket_, "CLOSE\r\n");
}

} // namespace Trace
} // namespace SystemC_VPC
