#include <utility>
#include <CoSupport/SystemC/algorithm.hpp>
#include "MostScheduler.hpp"
#include "MostSecondaryScheduler.hpp"
#include <ComponentImpl.hpp>


//////////////////
//// MOST150 ////
/////////////////
/*
 Zyklusgröße(Brutto)    384 Byte
 Zyklusgröße(Netto)     372 Byte
 Systemfreqeunz	        48000 Hz
 Zeit/Byte		54 ns
 Zeit/Zyklus            20088 ns
 */
namespace SystemC_VPC
{

  MostScheduler::MostScheduler(const char *schedulername) :
    secondaryScheduler()
  {
    slicecount = 0;
    streamcount = 0;
    lastassign = SC_ZERO_TIME;
    this->remainingSlice = SC_ZERO_TIME;
    curr_slicecount = -1;
    sysFreq = 48000;
    cycleSize = 372;
    std::map<sc_time, unsigned int> IDmap;

    currSlotStartTime = sc_time(0, SC_NS);

  }

  sc_time
  MostScheduler::cycle(int sysFreq)
  { //returns the time of one cycle depending on the system frequency
    sc_time cycle = sc_time(1.0 / sysFreq, SC_SEC);
    return cycle;
  }

  sc_time
  MostScheduler::setboundary(int sysFreq, int cycleSize)
  { /*the boundary between synchronous and asynchronous area will be
    calculated dynamically
    the boundaryTime represents the border between the two channels*/
    sc_time boundaryTime = SC_ZERO_TIME;
    if(Most_slots.size()>0)
      {
      for(size_t i = 0; i<Most_slots.size();i++)
        {
        boundaryTime += Most_slots[i].length;
        }
      std::cout<<"dynamic boundary set to time = "<<boundaryTime<<std::endl;
      return boundaryTime;
      }
    else
      {
      std::cout<<"dynamic boundary set to time = "<<SC_ZERO_TIME<<std::endl;
      return SC_ZERO_TIME;
      }
  }

  bool
  MostScheduler::area(int sysFreq, int cycleSize)
  {/*true if actual time is part of synchronous area
     false if actual time is part of asynchronous area*/
    sc_time actualTime = sc_time_stamp();
    sc_time temp1 = cycle(sysFreq);
    sc_time temp2 = CoSupport::SystemC::modulus(actualTime, temp1);
    if (temp2 < setboundary(sysFreq, cycleSize))
      {
        return true;
      }
    return false;
  }

  bool
  MostScheduler::getSchedulerTimeSlice(sc_time& time,
      const TaskMap &ready_tasks, const TaskMap &running_tasks)
  { //returns the time of the next scheduler call

    if (area(sysFreq, cycleSize))
      {
        //no waiting + no new task
        if (ready_tasks.size() == 0 && running_tasks.size() == 0)
          {
            return false;
          }
        else
          {//finding next valid point of time
            if (Most_slots.size() == 0)
              {
                time = setboundary(sysFreq, cycleSize);
                curr_slicecount = -1;
                return true;
              }
            if (curr_slicecount == -1)
              {
                time = sc_time(0, SC_NS);
                return false;
              }
            else
              {
                if (curr_slicecount == Most_slots.size() - 1)
                  {
                    /*nothing else to do for synch area, waiting for
                      SecondaryScheduler*/
                    time = setboundary(sysFreq, cycleSize)
                        -CoSupport::SystemC::modulus(sc_time_stamp(),
                            cycle(sysFreq) );
                    return true;
                  }
                else
                  {
                    /*new scheduling after the next quadlet
                    if task length < one quadlet ->
                    take time of one quadlet
                    if task length > one quadlet ->
                    take time of task + difference to the next full quadlet
                    if(Most_slots[curr_slicecount].length
                    <= sc_time(224 , SC_NS) )
                      {
                      time = sc_time(224 , SC_NS);
                      std::cout<<"xyz timeslice returned in if time= "
                      <<time<<std::endl;
                      return true;
                      }
                    else
                      {
                      time = Most_slots[curr_slicecount].length +
                      (sc_time(224,SC_NS)-
                      CoSupport::SystemC::modulus(
                      Most_slots[curr_slicecount].length,sc_time(224,SC_NS)));
                      std::cout<<"xyz timeslice returned in else time = "<<
                      time<<std::endl;
                      return true;
                      }*/
                   time = Most_slots[curr_slicecount].length;
                   return true;
                  }
              }
          }
        //asynch area
      }
    else
      {
        time = remainingSlice;
        return (secondaryScheduler.getSchedulerTimeSlice(time, ready_tasks,
            running_tasks));
      }

  }

  void
  MostScheduler::removedTask(Task *task)
  {

    if (areaMap[task->getProcessId()] == true)
      {
        std::deque<MostSlot>::iterator iter;
        bool notFound = true;
        for (iter = Most_slots.begin(); ((iter != Most_slots.end()) && notFound)
        ; iter++)
          {
            if (task->getProcessId() == iter->process)
              {
                cout << "Stop task with ID " << task->getProcessId()
                    << " actual time = " << sc_time_stamp() << endl;
                notFound = false;
              }
          }
      }
    else
      {//call SecondaryScheduler to remove task
        secondaryScheduler.removedTask(task);
      }
  }

  void MostScheduler::addStream(ProcessId pid){

    std::cout<<"MostScheduler::addStream (" << pid << ")" << std::endl;
    areaMap[pid] = true;
  }
  void
  MostScheduler::addedNewTask(Task *task)
  {
    std::cout<<"MostScheduler::addedNewTask " << task << " id: "
        << task->getProcessId() << " InstanceID=" << task->getInstanceId()
        << std::endl;

    if (areaMap[task->getProcessId()] == true)
      //task was marked as part of the synchronous area by addedNewTask
      {
        bool notFoundAdd = true;
        std::deque<MostSlot>::iterator iter1;
        already_avail = false;
        for (iter1 = Most_slots.begin(); ((iter1 != Most_slots.end())
            && notFoundAdd); iter1++)
          {
            if (task->getProcessId() == iter1->process)
              {
                already_avail = true;
                notFoundAdd = false;
              }
          }
        if (!already_avail)
          {//task will be scheduled in new slot with ID = slotID
            unsigned int slotId = Most_slots.size();
            std::deque<MostSlot>::iterator it;
            MostSlot newSlot;
            newSlot.length = task->getDelay();
            newSlot.Id = slotId;
            newSlot.process = task->getProcessId();
            Most_slots.push_back(newSlot);
            bool spaceAvail=true;
            //check if there is any available time in asynch area
            sc_time neededTime = SC_ZERO_TIME;
            for(size_t i =0; i< slotId; i++)
              {
              neededTime += Most_slots[i].length;
              if(neededTime+ task->getDelay() > setboundary(sysFreq,
                  cycleSize))
                {
                spaceAvail = false;
                }
              else
                {
                spaceAvail = true;
                }
              }
            //Not enough time available for static scheduling
            assert(spaceAvail);
            //int sizeOfMostSlots = Most_slots.size();
            //int temp = sizeOfMostSlots - 1;

            streamcount++;

          }
      }
    else
      {

       //call SecondaryScheduler to add new task
        secondaryScheduler.addedNewTask(task);
      }
  }
  void
  MostScheduler::closeStream(ProcessId pid)
  { //closes a stream connection by erasing the according element of Most_slots
    std::cout<<"MostScheduler::closeStream PID=" << pid << std::endl;
    std::deque<MostSlot>::iterator iter;
    for (iter = Most_slots.begin(); iter != Most_slots.end(); iter++)
      {
        if (pid == iter->process)
          {
            std::cout<<"erased task with id = "<<pid
            <<" from Most_slots iter is" << iter->length <<std::endl;
            Most_slots.erase(iter);
            --streamcount;
            return;
          }
      }
  }

  scheduling_decision
  MostScheduler::schedulingDecision(int& task_to_resign,
      int& task_to_assign,const TaskMap &ready_tasks,
      const TaskMap &running_tasks)
  {//returns the scheduling decision according to the task and the actual time
    if (area(sysFreq, cycleSize))
      { /*synchronous area -> TDMA Scheduling with variable slot size
          cycleTime is the actual position in the actual cycle*/
        sc_time cycleTime = CoSupport::SystemC::modulus(sc_time_stamp(),
            cycle(sysFreq)); //position in actual cycle
        if (curr_slicecount == -1)
          { /*the synchronous area has just started
              the last request was handled within the asynchronous area*/
            curr_slicecount = 0;
            lastassign = sc_time_stamp();
          }
        if ((currSlotStartTime + Most_slots[curr_slicecount].length)
            <= cycleTime)
          { //Most-Slot is over
           if (curr_slicecount == Most_slots.size() - 1)
              {
                curr_slicecount = -1;
                currSlotStartTime = sc_time(0, SC_NS);
                remainingSlice = setboundary(sysFreq, cycleSize)
                    - cycleTime; //time of dynamic boundary - cycleTime;
              }
            else
              {

                currSlotStartTime += Most_slots[curr_slicecount].length;
                curr_slicecount++;

                remainingSlice = Most_slots[Most_slots.size() - 1].length
                    - (sc_time_stamp() - currSlotStartTime
                        - Most_slots[Most_slots.size() - 1].length);
                if (curr_slicecount == Most_slots.size())
                  {

                    remainingSlice = (setboundary(sysFreq, cycleSize)
                        - currSlotStartTime);
                  }
              }
          }
        scheduling_decision ret_decision = NOCHANGE;
        //running task available ?

        if (running_tasks.begin() != running_tasks.end())
          {
            //int runningTask = running_tasks.begin()->first;
            ret_decision = NOCHANGE;
            return ret_decision;
          }


        if (curr_slicecount > Most_slots.size() - 1)
          {
            curr_slicecount = Most_slots.size() - 1;
            return NOCHANGE;
          }

         TaskMap::const_iterator it = ready_tasks.begin();
         while(it != ready_tasks.end()){
            if(it->second->getProcessId()==Most_slots[curr_slicecount].process){
            task_to_assign = it->second->getInstanceId();
            ret_decision = ONLY_ASSIGN;
            lastassign = sc_time_stamp();
            return ret_decision;
          }
          it++;
        }
        return NOCHANGE;

      }
    else
      {

        //asynchronous area -> call secondary scheduler for scheduling decision
        curr_slicecount = -1;
        currSlotStartTime = sc_time(0,SC_NS);
        return (secondaryScheduler.schedulingDecision(task_to_resign,
            task_to_assign, ready_tasks, running_tasks));
      }
  }
}
