// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include "dequant.hpp"

template <typename T>
class m_adder: public smoc_actor {
  public:
    smoc_port_in<T>  in1;
    smoc_port_in<T>  in2;
    smoc_port_out<T> out;
  private:
    void process() {
      out[0] = in1[0] + in1[1] + in2[0]; 
      std::cout << name() << " adding " << in1[0] << " + " << in1[1] << " + " << in2[0] << " = " << out[0] << std::endl;
    }
    
    smoc_firing_state start;
  public:
    m_adder( sc_module_name name )
      :smoc_actor( name, start ) {
      start = (in1(2) && in2(1) && out(1)) >> call(&m_adder::process) >> start;
    }
};

class m_source: public smoc_actor {
  public:
    smoc_port_out<int> out;
  private:
    int i;
    
    void process() {
      std::cout << name() << " generating " << i << std::endl;
      out[0] = i++;
    }
    
    smoc_firing_state start;
  public:
    m_source( sc_module_name name )
      :smoc_actor( name, start ), i(0) {
      start =  out(1) >> call(&m_source::process) >> start;
    }
};

class m_sink: public smoc_actor {
  public:
    smoc_port_in<int> in;
  private:
    void process() {
      std::cout << name() << " receiving " << in[0] << std::endl;
    }
    
    smoc_firing_state start;
  public:
    m_sink( sc_module_name name )
      :smoc_actor( name, start ) {
      start = in(1) >> call(&m_sink::process) >> start;
    }
};

class m_top
: public smoc_ndf_constraintset {
  public:
    m_top( sc_module_name name )
      : smoc_ndf_constraintset(name) {
      m_top2        &top2 = registerNode(new smoc_ndf_moc<m_top2>("top2"));
      m_source      &src1 = registerNode(new m_source("src1"));
      m_source      &src2 = registerNode(new m_source("src2"));
      m_sink        &sink = registerNode(new m_sink("sink"));
      
      connectNodePorts( src1.out, top2.in1, smoc_fifo<int>(2) );
      connectNodePorts( src2.out, top2.in2, smoc_fifo<int>(2) );
      connectNodePorts( top2.out, sink.in,  smoc_fifo<int>(2) );
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<smoc_ndf_moc<m_top> > top("top");
  
  sc_start(-1);
  return 0;
}
