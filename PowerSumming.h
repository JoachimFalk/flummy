#ifndef HSCD_VPC_POWERSUMMING_H_
#define HSCD_VPC_POWERSUMMING_H_

#include <ostream>
#include <map>
#include <systemc>

#include "ComponentObserver.h"

namespace SystemC_VPC{

  class PowerSumming : public ComponentObserver
  {
  public:
    PowerSumming(std::ostream &os);
    ~PowerSumming();

    void notify(ComponentInfo *ci);

  private:
    std::ostream    &m_output;
    sc_core::sc_time m_changedTime;
    double           m_powerSum;
    std::map<const ComponentInfo *, double> m_powerConsumption;

    sc_core::sc_time m_lastChangedTime;
    double           m_lastPowerSum;
    double           m_energySum;

    void printPowerChange();
  };
} //namespace SystemC_VPC
#endif // HSCD_VPC_POWERSUMMING_H_
