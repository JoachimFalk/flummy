#ifndef HSCD_VPC_COMPONENTINFO_H_
#define HSCD_VPC_COMPONENTINFO_H_

#include <cstddef>

class ComponentInfo
{
  public:
    ComponentInfo() : powerConsumption(0.0) {}

    double getPowerConsumption() const
    {
      return powerConsumption;
    }

    void setPowerConsumption(const double pc)
    {
      powerConsumption = pc;
    }

  protected:
    double powerConsumption;
};

#endif // HSCD_VPC_COMPONENTINFO_H_
