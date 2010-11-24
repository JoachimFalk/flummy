/*
 * ScheduledTask.h
 *
 *  Created on: Nov 17, 2010
 *      Author: streubuehr
 */

#ifndef SCHEDULEDTASK_HPP_
#define SCHEDULEDTASK_HPP_

namespace SystemC_VPC
{

class Delayer;

class ScheduledTask
{
public:
  ScheduledTask();
  virtual ~ScheduledTask();
  void setDelayer(Delayer *component);
  void notifyActivation(bool active);
private:
  Delayer *component;
};

}

#endif /* SCHEDULEDTASK_HPP_ */
