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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_MOSTSCHEDULER_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_MOSTSCHEDULER_HPP

#include <systemcvpc/datatypes.hpp>

#include "Scheduler.hpp"
#include "MostSecondaryScheduler.hpp"

#include <systemc>

#include <map>
#include <deque>
#include <string.h>

namespace SystemC_VPC { namespace Detail {

  class PreemptiveComponent;

  typedef size_t ProcessId;

  struct MostSlot{
    sc_core::sc_time length;
    ProcessId process;
    int Id;
    std::string name;
  };
  
  class MostScheduler : public Scheduler{
  public:
    
    MostScheduler()
      : secondaryScheduler() {

      slicecount = 0;
      streamcount = 0;
      lastassign = sc_core::SC_ZERO_TIME;
      this->remainingSlice = sc_core::SC_ZERO_TIME;
      curr_slicecount = -1;
      sysFreq = 48000;
      cycleSize = 372;
      std::map<sc_core::sc_time, unsigned int> IDmap;

      currSlotStartTime = sc_core::sc_time(0, sc_core::SC_NS);
    }
    
    MostScheduler(const char *schedulername);
    
    
    bool getSchedulerTimeSlice(sc_core::sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);

    sc_core::sc_time setboundary(int sysFreq,int framesize);

    sc_core::sc_time cycle(int sysFreq);

    bool area(int sysFreq,int framesize);
 
    void addedNewTask(TaskInstance *task);
    
    void removedTask(TaskInstance *task);
    
    sc_core::sc_event& getNotifyEvent();
    
    scheduling_decision schedulingDecision(int& task_to_resign,
                                           int& task_to_assign,
                                           const  TaskMap &ready_tasks,
                                           const  TaskMap &running_tasks);
    
    sc_core::sc_time* schedulingOverhead(){
      return NULL;
    }
    
    void initialize(){}

    bool addStream(ProcessId pid);
    bool closeStream(ProcessId pid);

  private:

    
    std::map<sc_core::sc_time, unsigned int> slotOffsets;

    std::map<ProcessId, bool> areaMap;

    MostSecondaryScheduler secondaryScheduler;

    sc_core::sc_time lastassign;
    sc_core::sc_time remainingSlice;
    int slicecount;
    int curr_slicecount;
    int curr_slicecount_help;
    bool already_avail;
    std::deque<MostSlot> Most_slots;
    int sysFreq;
    int cycleSize;
    int streamcount;
    bool flag;
    sc_core::sc_time currSlotStartTime;
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PREEMPTIVESCHEDULER_MOSTSCHEDULER_HPP */
