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

#ifndef AVBSCHEDULER_H
#define AVBSCHEDULER_H

#include <PreemptiveScheduler/PriorityScheduler.hpp>
#include <PreemptiveScheduler/Scheduler.hpp>
#include <systemcvpc/datatypes.hpp>

#include <systemc.h>

#include <map>
#include <queue>
#include <vector>

namespace SystemC_VPC{
  class PreemptiveComponent;
  


  class AVBListEntry{
  public:
    AVBListEntry(){
      bw_alloc = 0;
      priority_level = 0;
      task_queue = 0;
      queue_credit = sc_core::SC_ZERO_TIME;
      has_credit_flag = true;
      wasEmpty_flag=true;
    }

    AVBListEntry(std::queue<Task*>* t_queue, int p_level, float b_alloc){
      bw_alloc = b_alloc;
      priority_level = p_level;
      task_queue = t_queue;
      queue_credit = sc_core::SC_ZERO_TIME;
      has_credit_flag = true;
      wasEmpty_flag=true;
    }

    float bw_alloc;
    sc_time queue_credit;
    bool has_credit_flag;
    bool wasEmpty_flag;
    int priority_level;
    std::queue<Task*>* task_queue;

    int get_priority_level(void){
      return priority_level;
    }

    float get_bw_alloc(void){
      return bw_alloc;
    }

    void increment_credit(sc_time increment){
    if(has_credit_flag){
	queue_credit += increment;
      }else{
	if(increment < queue_credit){
	  queue_credit -= increment;
	}else{
	  queue_credit = increment - queue_credit;
	  has_credit_flag = true;
	}
      }
    }

    void decrement_credit(sc_time decrement){
      if(has_credit_flag){
        if(decrement <= queue_credit){
          queue_credit -= decrement;
        }else{
          queue_credit = decrement - queue_credit;
          has_credit_flag = false;
        }
      }else{
        queue_credit += decrement;
      }
    }

    bool has_credit(void){
      //return (queue_credit >= sc_core::SC_ZERO_TIME);
      return has_credit_flag;
      
    }

    void setWasEmpty(bool flag){
      wasEmpty_flag = flag;
    }

    bool wasEmpty(void){
      return wasEmpty_flag;
    }

    void reset_credit(void){
      //return (queue_credit >= sc_core::SC_ZERO_TIME);
      has_credit_flag = true;
      queue_credit = sc_core::SC_ZERO_TIME;
      
    }

    sc_time get_credit(void){
      if(has_credit_flag){
	return queue_credit;
      }else{
	return SC_ZERO_TIME - queue_credit;
      }
    }

  };

  class AVBScheduler : public Scheduler{
  public:

    AVBScheduler();
    virtual ~AVBScheduler(){}
    bool getSchedulerTimeSlice(sc_time &time,
                               const TaskMap &ready_tasks,
                               const TaskMap &running_tasks);
    void addedNewTask(Task *task);
    void removedTask(Task *task);
    sc_event& getNotifyEvent();
    scheduling_decision schedulingDecision(int& task_to_resign,
                                           int& task_to_assign,
                                           const  TaskMap &ready_tasks,
                                           const  TaskMap &running_tasks);
    void setProperty(const char* key, const char* value);
    void setAttribute(AttributePtr attributePtr);

    sc_time* schedulingOverhead(){return 0;}//;
  protected:
    std::list<AVBListEntry*> p_list;
    std::deque<std::pair<std::string, std::string> > _properties;
    int last_active;
    sc_time time_last_assign;
  private:
    bool firstrun;

  };
}
#endif
