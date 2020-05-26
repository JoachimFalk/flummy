// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <systemoc/smoc_moc.hpp>

#include "smoc_synth_std_includes.hpp"

class Src: public smoc_actor {
public:
  smoc_port_out<double> o1;
private:
  smoc_firing_state q0;

  int i;
  int iter;

  void src() {
#ifdef SQR_LOGGING
    std::cout << name() << ": " << i << std::endl;
#endif //defined(SQR_LOGGING)
    o1[0] = i++;
  }
public:
  Src(sc_core::sc_module_name name, int iter_)
    : smoc_actor(name, q0)
    , o1("o1"), q0("q0")
    , i(10), iter(iter_+10)
  {
    SMOC_REGISTER_CPARAM(iter);
    char *init = getenv("SRC_ITERS");
    if (init)
      iter = atoll(init);
    q0 =
       (SMOC_VAR(i) <= SMOC_VAR(iter)) >>
       o1(1)                          >>
       SMOC_CALL(Src::src)             >> q0
     ;
  }
};

// Definition of the SqrLoop actor class
class SqrLoop
  // All actor classes must be derived
  // from the smoc_actor base class
  : public smoc_actor {
public:
  // Declaration of input and output ports
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1, o2;
private:
  // Declaration of firing states for the FSM
  smoc_firing_state q0;
  smoc_firing_state q1;

  // Declaration of the actor functionality
  // via member variables and methods
  double tmp_i1;

  // action functions triggered by the
  // FSM declared in the constructor
  void copyStore()  { o1[0] = tmp_i1 = i1[0];  }
  void copyInput()  { o1[0] = tmp_i1;          }
  void copyApprox() { o2[0] = i2[0];           }

  // guard  functions used by the
  // FSM declared in the constructor
  bool check() const {
#ifdef SQR_LOGGING
    std::cout << "checking " << tmp_i1 << ", " << i2[0] << std::endl;
#endif //defined(SQR_LOGGING)
    return std::fabs(tmp_i1 - i2[0]*i2[0]) < 0.0001;
  }
public:
  // Constructor responsible for declaring the
  // communication FSM and initializing the actor
  SqrLoop(sc_core::sc_module_name name)
    : smoc_actor(name, q0)
    , i1("i1"), i2("i2"), o1("o1"), o2("o2")
    , q0("q0"), q1("q1")
    , tmp_i1(0)
  {
    q0 =
        i1(1)                                   >>
        o1(1)                                   >>
        SMOC_CALL(SqrLoop::copyStore)           >> q1
      ;
    q1 =
        (i2(1) &&  SMOC_GUARD(SqrLoop::check))  >>
        o2(1)                                   >>
        SMOC_CALL(SqrLoop::copyApprox)          >> q0
      | (i2(1) && !SMOC_GUARD(SqrLoop::check))  >>
        o1(1)                                   >>
        SMOC_CALL(SqrLoop::copyInput)           >> q1
      ;
  }
};

class Approx: public smoc_actor {
public:
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1;
private:
  smoc_firing_state q0;

  // Square root successive approximation step of Newton
  void approx(void) { o1[0] = (i1[0] / i2[0] + i2[0]) / 2; }
public:
  Approx(sc_core::sc_module_name name)
    : smoc_actor(name, q0)
    , i1("i1"), i2("i2"), o1("o1"), q0("q0")
  {
    q0 =
        (i1(1) && i2(1))          >>
        o1(1)                     >>
        SMOC_CALL(Approx::approx) >> q0
      ;
  }
};

class Dup: public smoc_actor {
public:
  smoc_port_in<double>  i1;
  smoc_port_out<double> o1, o2;

private:
  smoc_firing_state q0;

  void dup() {
    double in = i1[0];
    o1[0] = in;
    o2[0] = in;
  }
public:
  Dup(sc_core::sc_module_name name)
    : smoc_actor(name, q0)
    , i1("i1"), o1("o1"), o2("o2"), q0("q0")
  {
    q0 =
        i1(1)                    >>
        (o1(1) && o2(1))         >>
        SMOC_CALL(Dup::dup)           >> q0
      ;
  }
};

class Sink: public smoc_actor {
public:
  smoc_port_in<double> i1;
private:
  volatile double sinkDump;

  void sink(void) {
    sinkDump = i1[0];
#ifdef SQR_LOGGING
    std::cout << name() << ": " << sinkDump << std::endl;
#endif //defined(SQR_LOGGING)
  }

  smoc_firing_state q0;
public:
  Sink(sc_core::sc_module_name name)
    : smoc_actor(name, q0)
    , i1("i1"), q0("q0")
  {
    q0 =
        i1(1)                 >>
        SMOC_CALL(Sink::sink) >>
	q0
      ;
  }
};

class SqrRoot
: public smoc_graph {
public:
protected:
  Src      src;
  SqrLoop  sqrloop;
  Approx   approx;
  Dup      dup;
  Sink     sink;
public:
  SqrRoot(sc_core::sc_module_name name, const int from = 1)
    : smoc_graph(name)
    , src("a1", from)
    , sqrloop("a2")
    , approx("a3")
    , dup("a4")
    , sink("a5")
  {
    connectNodePorts(src.o1,     sqrloop.i1);
    connectNodePorts(sqrloop.o1, approx.i1);
    connectNodePorts(approx.o1,  dup.i1,
                     smoc_fifo<double>(1));
    connectNodePorts(dup.o1,     approx.i2,
                     smoc_fifo<double>() << 2 );
    connectNodePorts(dup.o2,     sqrloop.i2);
    connectNodePorts(sqrloop.o2, sink.i1);
  }
};

int sc_main (int argc, char **argv) {
  int iter = NUM_MAX_ITERATIONS;
  if (argc == 2) {
    iter = atoi(argv[1]);
    assert(iter <= NUM_MAX_ITERATIONS);
  }
  smoc_top_moc<SqrRoot> sqrroot("sqrroot", iter);
  sc_core::sc_start();
  return 0;
}
