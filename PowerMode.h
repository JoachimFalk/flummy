/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * PowerMode.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
#ifndef __INCLUDED__POWERMODE__H__
#define __INCLUDED__POWERMODE__H__

#include <cstddef>

namespace SystemC_VPC{

  /**
   * 
   */
  class PowerMode
  {
  public:
    PowerMode(const size_t &_mode) : mode(_mode) {}

    PowerMode(const PowerMode &powerMode) : mode(powerMode.mode) {}

    //FIXME: needed for std::map only
    PowerMode() : mode(0) {}

    bool operator==(const PowerMode &rhs) const
    {
      return mode == rhs.mode;
    }

    bool operator!=(const PowerMode &rhs) const
    {
      return mode != rhs.mode;
    }

    bool operator<(const PowerMode &rhs) const
    {
      // FIXME !!
      return mode > rhs.mode;
    }

    bool operator>(const PowerMode &rhs) const
    {
      // FIXME !!
      return mode < rhs.mode;
    }

  private:
    size_t mode;
  };

}
#endif // __INCLUDED__POWERMODE__H__
