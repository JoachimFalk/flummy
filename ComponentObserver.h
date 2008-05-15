#ifndef HSCD_VPC_COMPONENTOBSERVER_H_
#define HSCD_VPC_COMPONENTOBSERVER_H_

#include "ComponentInfo.h"

class ComponentObserver
{
  public:
    virtual ~ComponentObserver() {}

    // this callback function shall be called on component state changes
    virtual void notify(const ComponentInfo *ci) = 0;
};

#endif // HSCD_VPC_COMPONENTOBSERVER_H_
