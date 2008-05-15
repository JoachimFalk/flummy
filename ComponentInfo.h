#ifndef HSCD_VPC_COMPONENTINFO_H_
#define HSCD_VPC_COMPONENTINFO_H_

#include <cstddef>

class ComponentInfo
{
  public:
    ComponentInfo() : powerConsumption(0) {}

    std::size_t getPowerConsumption() const
    {
      return powerConsumption;
    }

    void setPowerConsumption(const std::size_t pc)
    {
      powerConsumption = pc;
    }

  protected:
    std::size_t powerConsumption;
};

#endif // HSCD_VPC_COMPONENTINFO_H_
