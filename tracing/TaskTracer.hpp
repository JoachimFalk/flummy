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

#ifndef TASKTRACER_HPP_
#define TASKTRACER_HPP_

#include <systemcvpc/ProcessControlBlock.hpp>
#include <systemcvpc/Task.hpp>

namespace SystemC_VPC
{
namespace Trace
{

class DiscardTace
{
public:
  void release(Task * task)
  {
  }

  void finishDii(Task * task)
  {
  }

  void finishLatency(Task * task)
  {
  }

  void assign(Task * task)
  {
  }
};

class VcdTace
{
public:
  void release(Task * task)
  {
    task->traceReleaseTask();
  }

  void finishDii(Task * task)
  {
    task->traceFinishTaskDii();
  }

  void finishLatency(Task * task)
  {
    task->traceFinishTaskLatency();
  }

  void assign(Task * task)
  {
    task->traceAssignTask();
  }
};

} // namespace Trace
} // namespace SystemC_VPC
#endif /* TASKTRACER_HPP_ */
