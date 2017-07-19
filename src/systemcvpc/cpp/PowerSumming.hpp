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

#ifndef HSCD_VPC_POWERSUMMING_H_
#define HSCD_VPC_POWERSUMMING_H_

#include <ostream>
#include <map>
#include <systemc>

#include "ComponentObserver.hpp"


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
	sc_core::sc_time m_lastVirtualTime;
    double           m_powerSum;
    double           m_previousPowerSum;
    std::map<const ComponentInfo *, double> m_powerConsumption;
    std::map<const ComponentInfo *, double> m_lastChangedPowerConsumption;
    std::map<const ComponentInfo *, const PowerMode *> m_powerMode;
    std::map<const ComponentInfo *, const PowerMode *> m_lastChangedPowerMode;

    //sc_core::sc_time m_lastChangedTime;
    double           m_previousEnergySum;
    double           m_energySum;

    ComponentInfo *m_lastCi;

    /*
     * Flag to print the inital power change at 0s
     */
    bool init_print;


    sc_core::sc_time notifyTimeStamp;

    void printPowerChange(std::string mode);
    void calculateNewEnergySum();
  };
} //namespace SystemC_VPC
#endif // HSCD_VPC_POWERSUMMING_H_
