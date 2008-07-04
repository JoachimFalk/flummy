#include <cassert>

#include "PowerSumming.h"

namespace SystemC_VPC{

PowerSumming::PowerSumming(std::ostream &os) :
  m_output(os),
  m_changedTime(sc_core::SC_ZERO_TIME),
  m_powerSum(0.0),
  m_lastChangedTime(sc_core::SC_ZERO_TIME),
  m_lastPowerSum(0.0),
  m_energySum(0.0)
{
  assert(m_output.good());
}

PowerSumming::~PowerSumming()
{
  printPowerChange();
}

void PowerSumming::notify(ComponentInfo *ci)
{
  double old_powerConsumption = m_powerConsumption[ci];
  double new_powerConsumption = ci->getPowerConsumption();

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
  unsigned long long timeStamp = m_changedTime.to_seconds() * 1000000000.0;

  if( (m_lastPowerSum == m_powerSum) && (timeStamp != 0) )
    return;

  double duration = (m_changedTime - m_lastChangedTime).to_seconds();

  m_energySum      += m_lastPowerSum * duration;
  m_lastPowerSum    = m_powerSum;
  m_lastChangedTime = m_changedTime;

  m_output << timeStamp << '\t' << m_powerSum << '\t' << m_energySum << std::endl;
}

} //namespace SystemC_VPC
