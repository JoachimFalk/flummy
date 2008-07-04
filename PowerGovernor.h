#ifndef POWERGOVERNOR_H_
#define POWERGOVERNOR_H_

#include "ComponentObserver.h"

namespace SystemC_VPC{

template <class T>
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
    LocalPowerGovernor(GlobalPowerGovernor<T> *tpg) :
      m_tpg(tpg)
    {}

    virtual ~LocalPowerGovernor()
    {}

    virtual void notify(ComponentInfo *ci) = 0;

  protected:
    GlobalPowerGovernor<T> *m_tpg;
};

// Convenience class for non-hierarchical governors
class PowerGovernor : public GlobalPowerGovernor<int>,
                      public LocalPowerGovernor<int>
{
  public:
    PowerGovernor() : LocalPowerGovernor<int>(
      static_cast<GlobalPowerGovernor<int>*>(this))
    {}

    virtual ~PowerGovernor()
    {}

    virtual void notify(const ComponentInfo *ci) = 0;

    virtual void notify_top(const ComponentInfo *ci, int val)
    {}
};

}

#endif // POWERGOVERNOR_H_
