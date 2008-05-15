#ifndef HSCD_VPC_POWERSUMMING_H_
#define HSCD_VPC_POWERSUMMING_H_

#include <ostream>
#include <map>
#include <systemc>

#include "ComponentObserver.h"

class PowerSumming : public ComponentObserver
{
  public:
    PowerSumming(std::ostream &os);
    ~PowerSumming();

    void notify(const ComponentInfo *ci);

  private:
    std::ostream    &m_output;
    sc_core::sc_time m_changedTime;
    std::size_t      m_powerSum;
    std::map<const ComponentInfo *, std::size_t> m_powerConsumption;

    void printPowerChange();
};

#endif // HSCD_VPC_POWERSUMMING_H_
