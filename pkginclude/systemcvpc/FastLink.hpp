/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * FastLink.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#ifndef __INCLUDED__FASTLINK__H__
#define __INCLUDED__FASTLINK__H__
#include <systemc.h>
#include "EventPair.hpp"

typedef size_t ProcessId;
typedef size_t FunctionId;
typedef size_t ComponentId;

namespace SystemC_VPC{
  /**
   *
   */
  class FastLink{
  public:

    /**
     *
     */
    void compute( EventPair p ) const;

    /**
     *
     */
    void write( size_t quantum, EventPair p ) const;

    /**
     *
     */
    void read( size_t quantum, EventPair p ) const;

    /**
     *
     */
    ComponentId getComponentId() const;

    /**
     *
     */
    void addDelay(size_t delay_ns){
      addDelay(sc_time(delay_ns, SC_NS));
    }

    /**
     *
     */
    void addDelay(sc_time delay);

    FastLink(ProcessId pid, FunctionId fid)
      : process(pid),
      func(fid),
      extraDelay(SC_ZERO_TIME)
    { }

    FastLink()
      : process(),
      func(),
      extraDelay(SC_ZERO_TIME)
    { }

    ProcessId            process;
    FunctionId           func;

  private:
    mutable sc_time      extraDelay;
  };

  static const FunctionId defaultFunctionId = 0;
}

#endif // __INCLUDED__FASTLINK__H__
