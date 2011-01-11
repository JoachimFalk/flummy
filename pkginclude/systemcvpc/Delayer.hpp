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

    virtual const char* getName() const = 0;

    const ComponentId getComponentId() const;

    void addObserver(ComponentObserver *obs);

    void removeObserver(ComponentObserver *obs);
    
    void fireNotification(ComponentInfo *compInf);

    virtual void initialize(const Director* d) {};

    virtual void notifyActivation(ScheduledTask * scheduledTask,
        bool active) {}

    virtual ~Delayer() {}

  protected:

    Delayer(): componentId(globalComponentId++) {}

    typedef std::vector<ComponentObserver *> Observers;
    
    Observers observers;
    
  private:
    //
    static ComponentId globalComponentId;

    //
    ComponentId componentId;
  };
}

#endif // HSCD_VPC_DELAYER_H
