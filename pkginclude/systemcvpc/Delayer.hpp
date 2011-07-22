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

#ifndef HSCD_VPC_DELAYER_H
#define HSCD_VPC_DELAYER_H

#include <vector>

#include "ComponentInfo.hpp"
#include "ComponentObserver.hpp"
#include "Task.hpp"

namespace SystemC_VPC{

  class Director;
  class ScheduledTask;

  /**
   * \brief Interface for classes implementing delay simulation.
   */
  class Delayer{
  public:
    /**
       * \brief Simulate the delay caused by this Delayer.
       *
       * While the simulation is running SystemC simulation time is consumed.
       */
    virtual void compute(Task* task) = 0;

    const std::string& getName() const
    {
      return name_;
    }

    const ComponentId getComponentId() const;

    void addObserver(ComponentObserver *obs);

    void removeObserver(ComponentObserver *obs);
    
    void fireNotification(ComponentInfo *compInf);

    virtual void initialize(const Director* d) {};

    virtual void notifyActivation(ScheduledTask * scheduledTask,
        bool active) {}

    virtual ~Delayer() {}

  protected:

    Delayer(ComponentId id, std::string name) : componentId_(id), name_(name) {}

    typedef std::vector<ComponentObserver *> Observers;
    
    Observers observers;
    
  private:

    //
    ComponentId componentId_;

    //
    std::string name_;
  };
}

#endif // HSCD_VPC_DELAYER_H
