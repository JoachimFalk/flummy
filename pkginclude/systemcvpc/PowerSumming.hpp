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
    double           m_previousPowerSum;
    double           m_powerSum;
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
