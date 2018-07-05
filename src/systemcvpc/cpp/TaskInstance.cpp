// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#include "TaskInstance.hpp"
#include "ProcessControlBlock.hpp"

namespace SystemC_VPC {

  int TaskInstance::globalInstanceId = 0;

  TaskInstance::TaskInstance(TaskPool *pool)
    : instanceId(globalInstanceId++)
    , pid(-1)
    , fid()
    , gid()
    , blockEvent()
    , blockingCompute(nullptr)
    , blockAck(false)
    , exec(false)
    , write(false)
    , factorOverhead(0)
    , timing()
    , pcb()
    , pool(pool)
    , name("NN")
    , timingScale(1)
    , taskTracerTicket()
    , scheduledTask(nullptr)
    , firingRuleInterface(nullptr)
    {}

  TaskInstance::~TaskInstance() {
  }

  void TaskInstance::initDelays() {
    assert(pcb != NULL);
    FunctionIds fids = this->getFunctionIds();
    FunctionIds gids = this->getGuardIds();

    if(!gids.empty())
      this->setOverhead(this->timingScale * timing->getDelay(gids) + (double)this->factorOverhead * sc_core::sc_time(10, sc_core::SC_NS));
    else
      this->setOverhead((double)this->factorOverhead * sc_core::sc_time(0.1, sc_core::SC_NS));

    //ugly hack: to make the random timing work correctly getDelay has to be called before getLateny, see Processcontrollbock.cpp for more information
    // Initialize with DII
    this->setDelay(this->timingScale * timing->getDelay(fids));
    // Initialize with DII
    this->setRemainingDelay(this->getDelay());
    // Initialize with Latency
    this->setLatency(this->timingScale * timing->getLatency(fids));
  }

  // Adaptor setter / getter for ProcessControlBlock
  int TaskInstance::getPriority() const
    { assert(pcb != NULL); return pcb->getPriority();}
  sc_core::sc_time TaskInstance::getPeriod() const
    { assert(pcb != NULL); return pcb->getPeriod();}

  bool TaskInstance::isPSM() const
    { return pcb->getTaskIsPSM(); }

  void TaskInstance::traceReleaseTask(){
    taskTracerTicket = pcb->getTaskTracer()->releaseTask();
  }

  void TaskInstance::traceFinishTaskLatency(){
    pcb->getTaskTracer()->finishTaskLatency(taskTracerTicket);
  }

}
