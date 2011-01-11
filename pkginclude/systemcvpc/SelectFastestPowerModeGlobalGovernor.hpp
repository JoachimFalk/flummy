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

#ifndef __INCLUDED_SELECTFASTESTPOWERMODEGOVERNOR_IMPL_H_
#define __INCLUDED_SELECTFASTESTPOWERMODEGOVERNOR_IMPL_H_

#include <map>
#include <deque>
#include <systemc.h>

#include <systemcvpc/PowerMode.hpp>
#include <systemcvpc/PluggablePowerGovernor.hpp>

namespace SystemC_VPC{

  class InternalSelectFastestPowerModeGovernor :
    public PluggableGlobalPowerGovernor
  {
  public:
    InternalSelectFastestPowerModeGovernor();

    void notify_top(ComponentInfo *ci, GenericParameter *param);

  private:
    const PowerMode                           *m_lastMode;
    std::map<ComponentInfo*, const PowerMode*> m_components;
  };
}

#endif // __INCLUDED_SELECTFASTESTPOWERMODEGOVERNOR_IMPL_H_
