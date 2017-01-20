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

#ifndef SECONDARYSCHEDULER_H
#define SECONDARYSCHEDULER_H
#include <systemc.h>
#include "Scheduler.hpp"
#include <systemcvpc/datatypes.hpp>
#include <map>
#include <deque>



namespace SystemC_VPC{
  class Component;

  struct AsynchSlot{
      sc_time length;
      int process;
      int Id;
      int priority;
      std::string name;
    };

class MostSecondaryScheduler :public Scheduler{
public:
  MostSecondaryScheduler(){
    sysFreq = 48000;
  }


  sc_time cycle(int sysFreq);

  void addedNewTask(Task *task);

  void removedTask(Task *task);

  bool getSchedulerTimeSlice(sc_time& time,
                              const TaskMap &ready_tasks,
                              const TaskMap &running_tasks);

  scheduling_decision schedulingDecision(
       int& task_to_resign,
       int& task_to_assign,
       const  TaskMap &ready_tasks,
       const  TaskMap &running_tasks);

  sc_time* schedulingOverhead(){return 0;}


private:

std::deque<AsynchSlot> Asynch_slots;
sc_time lastassignasynch;
int sysFreq;


};

}

#endif
