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

#ifndef __INCLUDED_POWERGOVERNOR_H_
#define __INCLUDED_POWERGOVERNOR_H_

#include "ComponentObserver.hpp"
#include "Attribute.hpp"

namespace SystemC_VPC{

template <typename T>
class GlobalPowerGovernor
{
  public:
    GlobalPowerGovernor()
    {}

    virtual ~GlobalPowerGovernor()
    {}

    virtual void notify_top(ComponentInfo *ci, T val) = 0;
};

template <class T>
class LocalPowerGovernor : public ComponentObserver
{
  public:
    LocalPowerGovernor() :
      m_tpg(NULL)
    {}

    virtual ~LocalPowerGovernor()
    {}

    virtual void notify(ComponentInfo *ci) = 0;

    void setGlobalGovernor(GlobalPowerGovernor<T> *tpg)
    {
      //std::cerr << "LocalPowerGovernor::setGlobalGovernor" << std::endl;
      this->m_tpg = tpg;
    }

  protected:
    GlobalPowerGovernor<T> *m_tpg;
};


}

#endif // __INCLUDED_POWERGOVERNOR_H_
