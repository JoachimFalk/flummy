/******************************************************************************
 *                        Copyright 2008
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * hscd_vpc_AbstractComponent.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#ifndef HSCD_VPC_DELAYER_H
#define HSCD_VPC_DELAYER_H

#include <vector>

#include "ComponentInfo.h"
#include "ComponentObserver.h"
#include "Task.h"

namespace SystemC_VPC{

  /**
   * \brief Interface for classes implementing delay simulation.
   */
  class Delayer{
  public:
    /**
       * \brief Simulate the delay caused by this Delayer.
       *
       * While this simulation is running SystemC simulation time is consumed.
       */
    virtual void compute(Task* task) = 0;

    virtual const char* getName() const = 0;

    ComponentId getComponentId();

    void addObserver(ComponentObserver *obs);

    void removeObserver(ComponentObserver *obs);
    
    void fireNotification(ComponentInfo *compInf);

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
