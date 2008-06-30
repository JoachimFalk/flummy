#ifndef HSCD_VPC_COMPONENTINFO_H_
#define HSCD_VPC_COMPONENTINFO_H_

#include "PowerMode.h"

#include <cstddef>

namespace SystemC_VPC{

  class ComponentInfo
  {
  public:
    ComponentInfo() : powerConsumption(0.0) {}

    virtual ~ComponentInfo(){};

    double getPowerConsumption() const
    {
      return powerConsumption;
    }

    void setPowerConsumption(const double pc)
    {
      powerConsumption = pc;
    }

    PowerMode getPowerMode() const
    {
      return powerMode;
    }

    virtual void setPowerMode(const PowerMode& mode)
    {
      powerMode = mode;
    }
  protected:
    double powerConsumption;

    PowerMode powerMode;
  };
} //namespace SystemC_VPC
#endif // HSCD_VPC_COMPONENTINFO_H_
