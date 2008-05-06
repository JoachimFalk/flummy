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
#include "hscd_vpc_EventPair.h"

typedef size_t ProcessId;
typedef size_t FunctionId;

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


    FastLink(ProcessId pid, FunctionId fid)
      : process(pid),
      func(fid) { }

    FastLink()
      : process(),
      func() { }

    ProcessId            process;
    FunctionId           func;
  };
}

#endif // __INCLUDED__FASTLINK__H__
