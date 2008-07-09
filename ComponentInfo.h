#ifndef HSCD_VPC_COMPONENTINFO_H_
#define HSCD_VPC_COMPONENTINFO_H_

#include "PowerMode.h"

#include <cstddef>
#include <string>
#include <iostream>
#include <map>
#include <string>

namespace SystemC_VPC{

  typedef std::map<std::string, PowerMode> PowerModes;

  class ComponentState
  {
    public:
      ComponentState()
      {
        *this = IDLE;
      }

      ComponentState(const size_t &_state) : state(_state) {}

      bool operator==(const ComponentState &rhs) const
      {
        return state == rhs.state;
      }

      bool operator!=(const ComponentState &rhs) const
      {
        return state != rhs.state;
      }

      bool operator<(const ComponentState &rhs) const
      {
        return state < rhs.state;
      }

      static const ComponentState IDLE;
      static const ComponentState RUNNING;
      static const ComponentState STALLED;

    private:
      size_t state;
  };

  class ComponentInfo
  {
    public:
      ComponentInfo() : powerConsumption(0.0) {}

      virtual ~ComponentInfo(){};

      ComponentState getComponentState() const
      {
        return componentState;
      }

      void setComponentState(const ComponentState cs)
      {
        componentState = cs;
      }

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

      PowerMode translatePowerMode(std::string mode)
      {
        PowerModes::const_iterator i = powerModes.find(mode);
        if(i == powerModes.end()) {
          size_t id = powerModes.size();
          powerModes[mode] = PowerMode(id);
        }
        return powerModes[mode];
      }

    protected:
      ComponentState componentState;
      double         powerConsumption;
      PowerModes     powerModes;
      PowerMode      powerMode;
  };
} //namespace SystemC_VPC
#endif // HSCD_VPC_COMPONENTINFO_H_
