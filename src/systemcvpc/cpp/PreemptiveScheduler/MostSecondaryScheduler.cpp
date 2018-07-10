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

#include <systemcvpc/datatypes.hpp>

#include "MostScheduler.hpp"
#include "MostSecondaryScheduler.hpp"
#include "PreemptiveComponent.hpp"

#include <CoSupport/SystemC/algorithm.hpp>

#include <utility>
#include <queue>

namespace SystemC_VPC
{

  sc_core::sc_time
  MostSecondaryScheduler::cycle(int sysFreq)
  {//returns the time of one cycle depending on the system frequency
    sc_core::sc_time cycle = sc_core::sc_time(1.0 / sysFreq, sc_core::SC_SEC);
    return cycle;
  }

  void
  MostSecondaryScheduler::addedNewTask(TaskInstance *task)
  {/*creates a new slot and puts it into Asynch_slots
     the corresponding task is commited by the MostScheduler*/
    unsigned int slotIdasynch = Asynch_slots.size();
    AsynchSlot newSlot;
    newSlot.length = task->getDelay();
    newSlot.process = task->getInstanceId();
    newSlot.priority = task->getPriority();
    newSlot.Id = slotIdasynch;
    std::deque<AsynchSlot>::iterator iter;
    Asynch_slots.push_back(newSlot);
  }

  void
  MostSecondaryScheduler::removedTask(TaskInstance *task)
  { //removes a task by erasing the according element of Asynch_slots
    std::deque<AsynchSlot>::iterator iter;
    bool notFound = true;
    for (iter = Asynch_slots.begin(); ((iter != Asynch_slots.end()) &&
        notFound); iter++)
      {
        if (task->getInstanceId() == iter->process)
          {
            std::cout << "Stop and remove task with ID " << task->getProcessId()
                << " actual time = " << sc_core::sc_time_stamp()<<std::endl;
            Asynch_slots.erase(iter);
            notFound = false;
          }
      }
  }

  bool
  MostSecondaryScheduler::getSchedulerTimeSlice(sc_core::sc_time& time,
      const TaskMap &ready_tasks, const TaskMap &running_tasks)
  {//returns the time of the next scheduler call
    std::cout << "in MostSecondaryScheduler::getSchedulerTimeSlice actual time = "
        << sc_core::sc_time_stamp() << std::endl;

    //no waiting + no active thread
    if (ready_tasks.size() == 0 && running_tasks.size() == 0)
      {
        return false;
      }

    //find next valid point of time
    if (((CoSupport::SystemC::modulus(sc_core::sc_time_stamp(), sc_core::sc_time(
        1.0 / sysFreq, sc_core::SC_SEC)))+Asynch_slots[0].length > (cycle(sysFreq))))
      {
        time = cycle(sysFreq)-(CoSupport::SystemC::modulus(sc_core::sc_time_stamp(),
            sc_core::sc_time(1.0 /sysFreq, sc_core::SC_SEC)));
        return true;
    }

    if (running_tasks.size() == 0 && Asynch_slots.size() != 0)
      {// no running task at the moment and tasks in Asynch_slots
        time = sc_core::sc_time(0, sc_core::SC_NS);
        return true;
      }
    else
      {
        time = cycle(sysFreq) - CoSupport::SystemC::modulus(sc_core::sc_time_stamp(),
            cycle(sysFreq));
        return true;
      }

  }

  scheduling_decision
  MostSecondaryScheduler::schedulingDecision(int& task_to_resign,
      int& task_to_assign, const TaskMap &ready_tasks,
      const TaskMap &running_tasks)
  {//returns the scheduling decision according to the task and the actual time

    std::cout << "###SecondaryScheduler::schedulingDecision" << std::endl;
    //at the beginning no new scheduling decision
    scheduling_decision ret_decision = NOCHANGE;
//  sc_core::sc_time cycleTime = CoSupport::SystemC::modulus(sc_core::sc_time_stamp(), sc_core::sc_time(
//      1.0 / sysFreq, sc_core::SC_SEC)); //actual position in actual cycle
//    sc_core::sc_time remainingTime = sc_core::sc_time(1.0 / sysFreq, sc_core::SC_SEC) - cycleTime;
    if (running_tasks.begin() != running_tasks.end())
      { //running Task available
        //int runningTask = running_tasks.begin()->first;
        ret_decision = NOCHANGE;
        return ret_decision;
      }
    if (Asynch_slots.size() <= 0)
      { //no new task available
        std::cout << "no new task for asynchronous area!" << std::endl;
        return NOCHANGE;
      }
    if ((Asynch_slots.size() > 0) && ((CoSupport::SystemC::
        modulus(sc_core::sc_time_stamp(), sc_core::sc_time(1.0 / sysFreq, sc_core::SC_SEC)))
        +Asynch_slots[0].length <= (cycle(sysFreq))))
      { //no registered task
        if(Asynch_slots.size()>1)
        {
          int size = Asynch_slots.size();

          /*Spec:
          "Each node has fair access to this channel
          bandwidth can be controlled using the Boundary Descriptor
          priority is not depending on the network position."
          Implementation:
          The priorities of the two "oldest" Tasks are compared.
          Priorities are specified in the *.xml file*/

        for(int i = 1;i<size;i++)
          {
          if(Asynch_slots[i].priority>Asynch_slots[i-1].priority)
            {
            std::cout<<"priority of ["<<i<<"] is higher than priority of "
                "["<<i-1<<"]"<<std::endl;
            task_to_assign = Asynch_slots[i].process;
            }
          else
            {
            std::cout<<"new task_to_assign"<<std::endl;
            task_to_assign = Asynch_slots[i-1].process;
            }
          }
        }
      else
        {
        task_to_assign=Asynch_slots[0].process;
        }

       std::cout<< " task to assign = " << task_to_assign
            << " actual time = " << sc_core::sc_time_stamp()<< std::endl;
        if ((ready_tasks.find(task_to_assign) == ready_tasks.end()))
          {
            ret_decision = NOCHANGE;
            return ret_decision;
          }
         ret_decision = ONLY_ASSIGN;
        return ret_decision;

      }
    return ret_decision;
  }
}
