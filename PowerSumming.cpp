#include <cassert>

#include "PowerSumming.h"

PowerSumming::PowerSumming(std::ostream &os) :
  m_output(os),
  m_changedTime(sc_core::SC_ZERO_TIME),
  m_powerSum(0)
{
  assert(m_output.good());
}

PowerSumming::~PowerSumming()
{
  printPowerChange();
}

void PowerSumming::notify(const ComponentInfo *ci)
{
  std::size_t old_powerConsumption = m_powerConsumption[ci];
  std::size_t new_powerConsumption = ci->getPowerConsumption();

  if(old_powerConsumption == new_powerConsumption)
    return;

  sc_core::sc_time new_changedTime = sc_core::sc_time_stamp();
  if(m_changedTime != new_changedTime) {
    printPowerChange();
    m_changedTime = new_changedTime;
  }

  m_powerSum -= old_powerConsumption;
  m_powerSum += new_powerConsumption;
  m_powerConsumption[ci] = new_powerConsumption;
}

void PowerSumming::printPowerChange()
{
  static std::size_t lastPowerSum = 0;
  static double energySum = 0.0;

  int timeStamp = int(m_changedTime.to_seconds() * 1000000000.0);

  if( (lastPowerSum != m_powerSum) || (timeStamp == 0) ) {
    double duration = (sc_core::sc_time_stamp() - m_changedTime).to_seconds();

    m_output << timeStamp << '\t' << m_powerSum << '\t' << energySum << std::endl;

    energySum += double(lastPowerSum) * duration;
    lastPowerSum = m_powerSum;
  }
}
