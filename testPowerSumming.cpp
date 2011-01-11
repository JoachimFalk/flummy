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

#include <systemcvpc/ComponentInfo.hpp>
#include <systemcvpc/PowerSumming.hpp>

#include <fstream>
#include <systemc.h>

class testmodule : public sc_module, public ComponentInfo
{
  public:
  
  testmodule(sc_module_name nm, ComponentObserver &obs) : sc_module(nm), m_obs(obs)
  {
    SC_HAS_PROCESS(testmodule);
    SC_THREAD(process);
  }

  void process()
  {
    powerConsumption = 10;
    m_obs.notify(this);

    wait(10, SC_NS);

    powerConsumption = 20;
    m_obs.notify(this);

    wait(20, SC_NS);

    powerConsumption = 0;
    m_obs.notify(this);
  }

  ComponentObserver &m_obs;
};

int sc_main(int argc, char *argv[])
{
  ofstream outfile("test.log");
  PowerSumming powObs(outfile);

  testmodule top1("top1", powObs);
  testmodule top2("top2", powObs);

  sc_start();

  return 0;
}
