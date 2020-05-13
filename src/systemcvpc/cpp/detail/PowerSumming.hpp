// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_POWERSUMMING_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_POWERSUMMING_HPP

#include "LegacyComponentObserver.hpp"

#include <ostream>
#include <map>
#include <systemc>

namespace SystemC_VPC { namespace Detail {

  class PowerSumming : public LegacyComponentObserver
  {
  public:
    PowerSumming(std::ostream &os);
    ~PowerSumming();

    void notify(Component *ci);

  private:
    std::ostream    &m_output;
    sc_core::sc_time m_changedTime;
    sc_core::sc_time m_lastVirtualTime;
    double           m_powerSum;
    double           m_previousPowerSum;
    std::map<const Component *, double> m_powerConsumption;
    std::map<const Component *, double> m_lastChangedPowerConsumption;
    std::map<const Component *, PowerMode> m_powerMode;
    std::map<const Component *, PowerMode> m_lastChangedPowerMode;

    //sc_core::sc_time m_lastChangedTime;
    double           m_previousEnergySum;
    double           m_energySum;

    Component       *m_lastCi;

    /*
     * Flag to print the inital power change at 0s
     */
    bool init_print;


    sc_core::sc_time notifyTimeStamp;

    void printPowerChange(std::string mode);
    void calculateNewEnergySum();
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_POWERSUMMING_HPP */
