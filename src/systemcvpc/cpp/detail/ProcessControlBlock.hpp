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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PROCESSCONTROLBLOCK_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PROCESSCONTROLBLOCK_HPP

#include <systemcvpc/EventPair.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Timing.hpp>
#include <systemcvpc/ScheduledTask.hpp>

#include "tracing/ComponentTracerIf.hpp"

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TaskTracer.hpp>

#include <systemc>

#include <vector>
#include <string>

namespace SystemC_VPC { namespace Detail {

  class AbstractComponent;

 /**
  * This class represents all necessary data of a simulated process within VPC
  * and provides necessary access methods for its data.
  */
  class ProcessControlBlock
    : public Tracing::TTaskHolder {
  public:
    /**
     * \brief Default constructor of an PCB instance
     * Initialize a newly created instance of ProcessControlBlock
     */
    ProcessControlBlock(AbstractComponent *component, std::string const &taskName);

    ProcessId getPid() const
      { return pid; }

    void      setPriority(int priority)
      { this->priority = priority; }
    int       getPriority() const
      { return priority; }

    sc_core::sc_time getPeriod() const;

    void setTaskIsPSM(bool flag)
      { psm = flag; }
    bool getTaskIsPSM() const
      { return psm; }

    std::string const &getName() const
      { return name; }

    void setScheduledTask(TaskInterface * st)
      { this->scheduledTask = st; }
    TaskInterface * getScheduledTask()
      { return this->scheduledTask; }
    bool hasScheduledTask() const
      { return this->scheduledTask != NULL; }

    ~ProcessControlBlock();
  private:
    smoc::SimulatorAPI::TaskInterface   *scheduledTask;
    AbstractComponent                   *component;
    std::string                          name;
    ProcessId                            pid;
    int                                  priority;
    bool                                 psm;
  };

  static inline
  ProcessControlBlock *getPCBOfTaskInterface(TaskInterface const *task)
    { return reinterpret_cast<ProcessControlBlock *>(task->getSchedulerInfo()); }

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PROCESSCONTROLBLOCK_HPP */
