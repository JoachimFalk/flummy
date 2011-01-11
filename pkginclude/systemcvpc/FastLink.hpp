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

#ifndef __INCLUDED__FASTLINK__H__
#define __INCLUDED__FASTLINK__H__
#include <systemc.h>
#include "EventPair.hpp"

#include <vector>

typedef size_t ProcessId;
typedef size_t FunctionId;
typedef size_t ComponentId;
typedef std::vector<FunctionId> FunctionIds;

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

    FastLink(ProcessId pid, FunctionIds fid)
      : process(pid),
      functions(fid),
      extraDelay(SC_ZERO_TIME)
    { }

    FastLink()
      : process(),
      functions(),
      extraDelay(SC_ZERO_TIME)
    { }

    ProcessId            process;
    FunctionIds          functions;

  private:
    mutable sc_time      extraDelay;
  };

  static const FunctionId defaultFunctionId = 0;
}

#endif // __INCLUDED__FASTLINK__H__
