// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_ROOT_NODE_HPP
#define _INCLUDED_HSCD_ROOT_NODE_HPP

#include <hscd_root_port_list.hpp>
#include <hscd_port.hpp>
#include <hscd_op.hpp>
#ifndef __SCFE__
# include <hscd_pggen.hpp>
#endif

//#include <oneof.hpp>

#include <systemc.h>

#include <list>

/*
#define linkme(instance,port,chan) do {     \
  (instance).registerPort((instance).port); \
  (instance).port(chan);                    \
} while ( 0 )
*/

#define linkme(instance,port,chan) do {     \
  (instance).port(chan);                    \
} while ( 0 )

class hscd_opbase_node
  : public hscd_root_node_op_if {
public:
  typedef hscd_opbase_node this_type;
protected:
//    typedef std::list<hscd_root_port *> ports_ty;
//    ports_ty                            ports;

//    void startTransact( hscd_op_transact op ) { startOp(op); }
//    void startChoice( hscd_op_choice op ) { startOp(op); }
    
//    void transact( hscd_op_transact op ) {
//      startTransact(op); waitFinished();
//    }
    
//    void choice( hscd_op_choice op ) {
//      startChoice(op); waitFinished();
//    }

  template <typename T>
  hscd_interface_action call(
      void (T::*f)(),
      const hscd_firing_state_list::value_type &v ) {
    std::cerr << "call" << std::endl;
    return hscd_interface_action(v);
  }
  template <typename T>
  hscd_interface_action branch(
      hscd_firing_state (T::*f)(),
      const hscd_firing_state_list &sl ) {
    std::cerr << "branch" << std::endl;
    return hscd_interface_action(sl);
  }
  hscd_firing_state Transact( const hscd_interface_transition &t ) {
    std::cerr << "Transact" << std::endl;
    return hscd_firing_state(t);
  }
  hscd_firing_state Choice( const hscd_transition_list &tl ) {
    std::cerr << "Choice" << std::endl;
    return hscd_firing_state(tl);
  }
  
  hscd_opbase_node(const hscd_firing_state &s)
    : hscd_root_node_op_if(s) {}
  
//    sc_event		  _opFinished;
  
//    void waitFinished() {
//      if ( !finished() )
//	  wait( _opFinished );
//      assert( finished() );
//    }
  
//    void opFinished() { _opFinished.notify_delayed(); }
public:
//    void registerPort( hscd_root_port &port ) {
//      ports.push_back(&port);
//    }
};

class hscd_root_node
  : public hscd_opbase_node {
protected:
//    void transact( hscd_op_transact op ) {
//      startTransact(op); waitFinished();
//      startTransact(fire_port(1)); waitFinished();
//    }
  
//    void choice( hscd_op_choice op ) {
//      startChoice(op); waitFinished();
//      startTransact(fire_port(1)); waitFinished();
//    }
  
  hscd_root_node(const hscd_firing_state &s)
    : hscd_opbase_node(s) {}
public:
  //sc_event		_fire;
  //hscd_port_in<void>  fire_port;

#ifndef __SCFE__
  virtual
  void assemble( hscd_modes::PGWriter &pgw ) const = 0;
  
  void leafAssemble( const sc_module *m, hscd_modes::PGWriter &pgw ) const;
#endif
};

#endif // _INCLUDED_HSCD_ROOT_NODE_HPP
