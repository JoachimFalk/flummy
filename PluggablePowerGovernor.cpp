#include "PluggablePowerGovernor.hpp"

namespace SystemC_VPC{
  GenericParameter::~GenericParameter() {}

  PowerModeParameter::PowerModeParameter(PowerMode *mode) : powerMode(mode){}
  PowerModeParameter::~PowerModeParameter() {}
}
