// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELIMPL_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELIMPL_HPP

#include <systemcvpc/PossibleAction.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/ExecModel.hpp>
#include <systemcvpc/Power.hpp>

#include "TaskInstanceImpl.hpp"

#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>

//#include <vector>
//#include <memory>

namespace SystemC_VPC { namespace Detail {

  /**
   * \brief Interface for classes implementing an execution model
   */
  class AbstractExecModel
    : private boost::noncopyable {
    typedef AbstractExecModel this_type;
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type const> ConstPtr;

    class ActionInfo {};

    AbstractExecModel(int facadeAdj);

    ExecModel       *getExecModel() {
      // Pointer magic. Shift our this pointer
      // so that it points to the ExecModel
      // base class of our real implementation.
      return reinterpret_cast<ExecModel *>(
          reinterpret_cast<char *>(this) + facadeAdj);
    }
    ExecModel const *getExecModel() const
      { return const_cast<this_type *>(this)->getExecModel(); }

    ///
    /// Interfaces to AbstractComponent
    ///

    class ComponentMixIn {
      friend AbstractExecModel;
    public:
      ComponentMixIn();

      AbstractExecModel *getExecModel()
        { return execModel.get(); }

      Power              getPowerIdle()
        { return pwrIdle; }

    private:
      class CompState {};

      Ptr        execModel;
      CompState *execModelComponentState;
      Power      pwrIdle;
    };

    ///
    /// Interfaces for AbstractComponent
    ///

    /// Allocate opaque CompState object when attaching to an abstract component.
    virtual void attachToComponent(ComponentMixIn *comp) = 0;

    virtual ActionInfo *registerAction(ComponentMixIn *comp
      , TaskInterface const *actor
      , PossibleAction const *action) = 0;

    /// Change the power mode of a component. This should update the
    /// opaque object pointed to by execModelComponentState.
    virtual void  setPowerMode(ComponentMixIn *comp
      , std::string const &mode) const = 0;

    /// Initialize ti with action or guard timing and power values.
    virtual void  initTaskInstance(ComponentMixIn *comp
      , ActionInfo *ai
      , TaskInstanceImpl *ti
      , bool forGuard = false) const = 0;

    virtual ~AbstractExecModel();
  protected:
    typedef ComponentMixIn::CompState CompState;

    // Accessor methods for execution models to inject information into task instances.
    static void setDelay(TaskInstanceImpl *ti, sc_core::sc_time delay)
      { ti->setDelay(delay); ti->setRemainingDelay(delay); }
    static void setLatency(TaskInstanceImpl *ti, sc_core::sc_time latency)
      { ti->setLatency(latency); }
    static void setPower(TaskInstanceImpl *ti, Power pwr)
      { ti->setPower(pwr); }

    // Accessor methods for execution models to inject information into the ComponentMixIn
    static void setExecModel(ComponentMixIn *c, this_type *m)
      { c->execModel.reset(m); }
    static void setPowerIdle(ComponentMixIn *c, Power p)
      { c->pwrIdle = p; }

    static CompState *getCompState(ComponentMixIn *c)
      { return c->execModelComponentState; }
    static void       setCompState(ComponentMixIn *c, CompState *cs)
      { c->execModelComponentState = cs; }
  private:
    int                         facadeAdj;
  };

  inline
  void intrusive_ptr_add_ref(AbstractExecModel *p) {
    p->getExecModel()->add_ref();
  }

  inline
  void intrusive_ptr_release(AbstractExecModel *p) {
    if (p->getExecModel()->del_ref())
      // AbstractExecModel has virtual destructor
      delete p;
  }

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELIMPL_HPP */
