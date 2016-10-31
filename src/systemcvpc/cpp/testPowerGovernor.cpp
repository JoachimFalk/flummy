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

#include <systemcvpc/SelectFastestPowerModeGlobalGovernor.hpp>
#include <systemcvpc/HysteresisLocalGovernor.hpp>
#include <systemcvpc/Component.hpp>

using namespace SystemC_VPC;

int main()
{
  SelectFastestPowerModeGovernor *govTop = new SelectFastestPowerModeGovernor;

  LoadHysteresisGovernor *gov1 = new LoadHysteresisGovernor(govTop, sc_time(10, SC_MS), sc_time(7.5, SC_MS), sc_time(2.5, SC_MS));
  LoadHysteresisGovernor *gov2 = new LoadHysteresisGovernor(govTop, sc_time(10, SC_MS), sc_time(7.5, SC_MS), sc_time(2.5, SC_MS));
  
  Component *res1 = new Component("res1", "FCFS");
  Component *res2 = new Component("res2", "FCFS");
  
  res1->addObserver(gov1);
  res2->addObserver(gov2);

  return 0;
}
