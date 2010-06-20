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
#include <string>

namespace SystemC_VPC{

  /**
   * 
   */
  class PowerMode
  {
  public:
    PowerMode(const size_t &_mode, std::string name)
      : mode(_mode),
        name(name) {}

    PowerMode(const PowerMode &powerMode)
      : mode(powerMode.mode),
        name(powerMode.name) {}

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
      return mode < rhs.mode;
    }

    bool operator>(const PowerMode &rhs) const
    {
      return mode > rhs.mode;
    }

    std::string getName() const {
      return name;
    }

  private:
    size_t mode;
    std::string name;
  };

}
#endif // __INCLUDED__POWERMODE__H__
