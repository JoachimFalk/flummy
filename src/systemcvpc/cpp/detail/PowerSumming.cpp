// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <cassert>

#include "PowerSumming.hpp"
#include "Director.hpp"

namespace SystemC_VPC { namespace Detail {

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
  std::map<const ComponentInfo *, PowerMode>::iterator iter =
      m_powerMode.find(m_lastCi);

  if (iter != m_powerMode.end()) {
    calculateNewEnergySum();
    printPowerChange(iter->second);

    m_changedTime = m_lastVirtualTime;
    m_lastVirtualTime=Director::getEnd();

    //Print last change
    calculateNewEnergySum();
    printPowerChange(iter->second);
  }
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

  const PowerMode currentCi_powerMode = ci->getPowerMode();
  std::pair<std::map<const ComponentInfo *, PowerMode>::iterator, bool> status =
      m_powerMode.insert(std::make_pair(ci, currentCi_powerMode));
  const PowerMode oldCi_powerMode = status.second
      ? PowerMode("?")        // Insertion success, no old value
      : status.first->second; // Insertion fail, get old value
  if (!status.second)
    // Insertion fail, overwrite old value
    status.first->second = currentCi_powerMode;

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
  if(m_lastVirtualTime != notifyTimeStamp) {
    std::pair<std::map<const ComponentInfo *, PowerMode>::iterator, bool> status =
        m_lastChangedPowerMode.insert(std::make_pair(ci, oldCi_powerMode));
    PowerMode const powerMode = status.second
        ? PowerMode("-")        // Insertion success, no old value
        : status.first->second; // Insertion fail, get old value
    if (!status.second)
      // Insertion fail, overwrite old value
      status.first->second = oldCi_powerMode;
    if (m_previousPowerSum != m_powerSum || oldCi_powerMode != powerMode) {
      calculateNewEnergySum();

      printPowerChange( powerMode);

      m_changedTime = m_lastVirtualTime;

      //Only replace power consumption of component on the total components aggregate
      //printing will take place at next power change instant
      m_lastChangedPowerConsumption[ci] = currentCi_powerConsumption;
      m_lastCi = ci;
      m_previousPowerSum = m_powerSum;
    }
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

} } // namespace SystemC_VPC::Detail
