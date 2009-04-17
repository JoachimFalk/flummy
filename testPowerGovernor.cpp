#include "systemcvpc/SelectFastestPowerModeGlobalGovernor.hpp"
#include "systemcvpc/HysteresisLocalGovernor.hpp"
#include "systemcvpc/hscd_vpc_Component.h"

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
