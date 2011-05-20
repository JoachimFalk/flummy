/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#include <cassert>

#include <systemcvpc/PowerSumming.hpp>
#include <systemcvpc/Director.hpp>

namespace SystemC_VPC{

PowerSumming::PowerSumming(std::ostream &os) :
  m_output(os),
  m_changedTime(sc_core::SC_ZERO_TIME),
  m_powerSum(0.0),
  m_lastChangedTime(sc_core::SC_ZERO_TIME),
  m_lastPowerSum(0.0),
  m_energySum(0.0),
  m_currentPowerMode(NULL),
  m_lastPowerMode(NULL)
{
  assert(m_output.good());
}

PowerSumming::~PowerSumming()
{
  m_changedTime = Director::getInstance().getEnd();

  if(m_changedTime <= m_lastChangedTime)
    return;

  unsigned long long timeStamp = m_changedTime.to_seconds() * 1000000000.0;
  double duration = (m_changedTime - m_lastChangedTime).to_seconds();
  m_energySum += m_lastPowerSum * duration;


  if(m_currentPowerMode != NULL)
  {
  m_output << timeStamp << '\t' << m_powerSum << '\t' << m_energySum << '\t'
           << m_currentPowerMode->getName() << std::endl;
  }
}

void PowerSumming::notify(ComponentInfo *ci)
{
  double old_powerConsumption = m_powerConsumption[ci];
  double current_powerConsumption = ci->getPowerConsumption();

  //if(old_powerConsumption == new_powerConsumption) {
  //    m_lastPowerMode = ci->getPowerMode();
  //    return;
  //}

  m_powerSum -= old_powerConsumption;
  m_powerSum += current_powerConsumption;
  m_powerConsumption[ci] = current_powerConsumption;
  m_currentPowerMode = ci->getPowerMode();

  sc_core::sc_time notifyTimeStamp = sc_core::sc_time_stamp();

  //If current time is different than last time the power consumption changed
  if(m_changedTime != notifyTimeStamp) {

    //If change
    if(m_lastPowerMode != m_currentPowerMode)
    {
    	//special case to print power at t=0s
    	if (!init_print)
    	{

    		//If first notification did not occur at t=0s, skip init_print
    		if(m_lastPowerMode != NULL)
    		{
    		  m_output << sc_time(0,SC_MS) << '\t' << m_lastPowerSum << '\t' << 0 << '\t'
    	  	           << m_lastPowerMode->getName() << std::endl;
    		}
    	  	init_print=true;

    	}
    	m_changedTime = notifyTimeStamp;
    	printPowerChange();

    }

  }

  m_lastPowerMode = m_currentPowerMode ;



}

void PowerSumming::printPowerChange()
{
  unsigned long long timeStamp = m_changedTime.to_seconds() * 1000000000.0;

  if( (m_lastPowerSum == m_powerSum) && (timeStamp != 0) && (m_currentPowerMode == m_lastPowerMode ))
    return;

  double duration = (m_changedTime - m_lastChangedTime).to_seconds();

  m_energySum      += m_lastPowerSum * duration;
  m_lastPowerSum    = m_powerSum;
  m_lastChangedTime = m_changedTime;

  m_output << timeStamp << '\t' << m_powerSum << '\t' << m_energySum << '\t'
           << m_currentPowerMode->getName() << std::endl;
}

} //namespace SystemC_VPC
