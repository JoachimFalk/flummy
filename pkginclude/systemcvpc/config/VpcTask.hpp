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

#ifndef VPCTASK_H_
#define VPCTASK_H_

#include "Component.hpp"

#include <boost/shared_ptr.hpp>

namespace SystemC_VPC
{

class ScheduledTask;

namespace Config
{

class VpcTask
{
public:
  typedef boost::shared_ptr<VpcTask> Ptr;

  VpcTask();

  VpcTask(const ScheduledTask & actor);

  void mapTo(Component::Ptr component);

  void setPriority(size_t priority);

  size_t getPriority() const;

  const ScheduledTask * getActor() const;

  //private:
  void inject(const ScheduledTask * actor);

private:
  // configured data
  const ScheduledTask * actor_;
  Component::Ptr component_;
  size_t priority_;
};
} // namespace Config
} // namespace SystemC_VPC
#endif /* VPCTASK_H_ */
