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

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>

template <typename T>
class m_h_src: public smoc_actor {
public:
  smoc_port_out<T> out;
private:
  T       i;
  size_t  iter;
  
  void src() {
    std::cout
      << name() << ": generate token with value " << i << std::endl;
    out[0] = i++; --iter;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_core::sc_module_name name, size_t _iter)
    : smoc_actor(name, start),
      i(1), iter(_iter) {
    start =
         (out(1) && SMOC_VAR(iter) > 0U)
      >> SMOC_CALL(m_h_src::src)       >> start;
  }
};


template <typename T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  void sink(void) {
    std::cout
      << name() << ": received token with value " << in[0] << std::endl;
  }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_core::sc_module_name name)
    : smoc_actor(name, start) {
    start = in(1) >> SMOC_CALL(m_h_sink::sink) >> start;
  }
};

template <typename T>
class m_h_foo: public smoc_graph {
public:
  smoc_port_out<T> out;
protected:
  m_h_src<T>     src;
  m_h_sink<T>    snk1, snk2;
public:
  m_h_foo(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      src("src", iter),
      snk1("snk1"), snk2("snk2") {
    smoc_fifo<T> cfSrcSnk1("cf:src->snk1", 1);
    cfSrcSnk1.connect(src.out).connect(snk1.in);
    smoc_fifo<T> cfSrcSnk2("cf:src->snk2", 1);
    cfSrcSnk2.connect(src.out).connect(snk2.in);
    src.out(out);
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_foo<int>   foo;
  m_h_sink<int>  snk3, snk4;
public:
  m_h_top(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      foo("foo", iter),
      snk3("snk3"), snk4("snk4") {
    smoc_fifo<int> cfFooSnk3("cf:foo->snk3", 1);
    cfFooSnk3.connect(foo.out).connect(snk3.in);
    smoc_fifo<int> cfFooSnk4("cf:foo->snk4", 1);
    cfFooSnk4.connect(foo.out).connect(snk4.in);
  }
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);
  
  if (argc >= 2)
    iter = atol(argv[1]);
  
  smoc_top_moc<m_h_top> top("top", iter);
  sc_core::sc_start();
  {
    std::ofstream result("throughput.txt");
    result << "Throughput: "
           << iter / sc_core::sc_time_stamp().to_seconds()
           << std::endl;
  }
  return 0;
}
