// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#ifndef _INCLUDED_ACTOR_A3
#define _INCLUDED_ACTOR_A3

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

class Actor_a3
  : public smoc_actor {

public:

  smoc_port_in<int> in1;
  smoc_port_in<int> in2;

  smoc_port_out<int> out1;

private:

  //waste some CPU memory
  void heat(){
    for(unsigned int i = 0; i < 5000; i++){
    }

    out1[0] = in1[0] + in2[0];
  }

  smoc_firing_state fsm_main;

public:
  Actor_a3(sc_module_name name)
    : smoc_actor(name, fsm_main)
  {

    fsm_main = 
      (in1(1) && in2(1) && out1(1)) >>
      CALL(Actor_a3::heat) >>
      fsm_main;

  }


};

#endif
