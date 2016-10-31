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
  m_lastVirtualTime(sc_core::SC_ZERO_TIME),
  m_powerSum(0.0),
  m_previousPowerSum(0.0),
  m_energySum(0.0)
{
  assert(m_output.good());
}

PowerSumming::~PowerSumming()
{
  /* calculateNewEnergySum() and printPowerChange is required to be performed twice
   * to get correct last PowerSumming - entries
   */
  calculateNewEnergySum();
  printPowerChange( m_powerMode[m_lastCi]->getName());

  m_changedTime = m_lastVirtualTime;
  m_lastVirtualTime=Director::getEnd();

  //Print last change
  calculateNewEnergySum();
  printPowerChange( m_powerMode[m_lastCi]->getName());

}


/*
 * This method process events to calculate the power consumption.
 *
 * It determines if there has been a time period since the last notification, in order to print only one message per timestamp
 * It also determines if there has been a change on the power consumption as several calls during a single simulation delta should be ignored
 * The power change is refreshed in all cases, and only during a valid condition the power info will be printed. This is done at the next time stamp
 *
 */
void PowerSumming::notify(ComponentInfo *ci)
{
  notifyTimeStamp = sc_core::sc_time_stamp();

  double oldCi_powerConsumption = m_powerConsumption[ci];
  double currentCi_powerConsumption = ci->getPowerConsumption();
  m_powerConsumption[ci] = currentCi_powerConsumption;

  const PowerMode* oldCi_powerMode = m_powerMode[ci];
  const PowerMode* currentCi_powerMode = ci->getPowerMode();
  m_powerMode[ci] = currentCi_powerMode;

  //special case for previousPowerSum
  // idea is to set it equal to the power sum for t=0
  if(!init_print)//speed up
  {
	if(notifyTimeStamp != sc_core::SC_ZERO_TIME)
	{
		m_previousPowerSum=m_powerSum;
		init_print=true;
	}
  }
  
  
  //if current time is different than last time the power consumption changed
  // and power consumption actually changed (fake exec. state transtitions should be ignored)
    if(m_lastVirtualTime != notifyTimeStamp && ( m_previousPowerSum != m_powerSum || oldCi_powerMode != m_lastChangedPowerMode[ci] ) )// && (currentCi_powerConsumption != m_lastChangedPowerConsumption[ci] || oldCi_powerMode != currentCi_powerMode))
    {

    	calculateNewEnergySum();

    	std::string powerMode;
    	    	if(m_lastChangedPowerMode[ci] != NULL)
    	    		powerMode = m_lastChangedPowerMode[ci]->getName();
    	    	else
    	    		powerMode = "-";
    	    	printPowerChange( powerMode);

    	m_changedTime = m_lastVirtualTime;

    	//Only replace power consumption of component on the total components aggregate
    	    	//printing will take place at next power change instant
    	    	m_lastChangedPowerConsumption[ci] = currentCi_powerConsumption;
    	    	m_lastChangedPowerMode[ci] = oldCi_powerMode;
    	    	m_lastCi = ci;
    	    	m_previousPowerSum = m_powerSum;
    }


	m_lastVirtualTime = notifyTimeStamp;



    m_powerSum -= oldCi_powerConsumption;
    m_powerSum += currentCi_powerConsumption;




}


/*
 * This method calculates the energy by integrating the power throughout the time period between the last power mode change and the current timestamp
 * It also stores the current energysum as the previous energy sum, which will be printed along the previous power mode change
 */
void PowerSumming::calculateNewEnergySum()
{
  m_previousEnergySum = m_energySum;

  double duration = (m_lastVirtualTime - m_changedTime).to_seconds();

  m_energySum      += m_previousPowerSum * duration;

}

/*
 * This method prints the last power mode change including the energy up to that point
 */
void PowerSumming::printPowerChange(std::string mode)
{
	unsigned long long timeStamp = m_changedTime.to_seconds() * 1000000000.0;
	m_output << timeStamp << '\t' << m_previousPowerSum << '\t' << m_previousEnergySum << '\t'<< mode << std::endl;
}

} //namespace SystemC_VPC
