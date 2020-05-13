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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELLING_LOOKUPPOWERTIMEMODELIMPL_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELLING_LOOKUPPOWERTIMEMODELIMPL_HPP

#include <systemcvpc/ExecModelling/LookupPowerTimeModel.hpp>
#include <systemcvpc/Timing.hpp>
#include <systemcvpc/Power.hpp>

#include "../AbstractExecModel.hpp"

#include <systemc>

#include <string>
#include <map>
#include <vector>

namespace SystemC_VPC { namespace Detail { namespace ExecModelling {

  /**
   *
   */
  class LookupPowerTimeModelImpl
    : public AbstractExecModel
    , public SystemC_VPC::ExecModelling::LookupPowerTimeModel
  {
    typedef LookupPowerTimeModelImpl this_type;
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type const> ConstPtr;

    LookupPowerTimeModelImpl();

    ~LookupPowerTimeModelImpl();

    ///
    /// Handle interfaces for SystemC_VPC::ExecModel
    ///

    void add(Timing timing);
    void addDefaultActorTiming(std::string actorName, Timing timing);

    bool addAttribute(Attribute const &attr);

    ///
    /// Handle interfaces for SystemC_VPC::ExecModelling::LookupPowerTimeModel
    ///

    PowerMode getStartPowerMode() const
      { return startPowerMode; }
    void      setStartPowerMode(PowerMode const &pm)
      { startPowerMode = pm; }

  private:
    ///
    /// Handle interfaces for AbstractExecModel
    ///

    struct CompState
      : public AbstractExecModel::CompState
    {
      CompState()
        : powerMode(0) {}

      size_t powerMode;
    };

    /// Allocate opaque CompState object when attaching to an abstract component.
    void attachToComponent(ComponentMixIn *comp);

    ActionInfo *registerAction(ComponentMixIn *comp
      , TaskInterface const *actor
      , PossibleAction const *action);

    /// Change the power mode of a component. This should update the
    /// opaque object pointed to by execModelComponentState.
    void  setPowerMode(ComponentMixIn *comp
      , std::string const &mode) const;

    /// Initialize ti with action or guard timing and power values.
    void  initTaskInstance(ComponentMixIn *comp
      , ActionInfo       *ai
      , TaskInstanceImpl *ti
      , bool              forGuard = false) const;

    ///
    /// Other stuff
    ///

    static CompState *getCompState(ComponentMixIn *c)
      { return static_cast<CompState *>(AbstractExecModel::getCompState(c)); }

    typedef std::map<std::string, Timing>  Timings;

    struct PowerModeInfo {
      size_t  index;      ///< Index of power mode.
      Timings timings;    ///< Power mode dependent action and guard timings
      Power   pwrIdle;    ///< Idle power in power mode
      Power   pwrRunning; ///< Default power consumption if a task is running.
      Power   pwrStalled; ///< Default power consumption for a stalled task.
      /// Delay factor for the guard complexity to determine the guard delay, i.e.,
      /// guard delay is guard complexity multiplied by guard complexity factor.
      sc_core::sc_time guardComplexityFactor;

      PowerModeInfo()
        : index(-1) {}
    };
    typedef std::map<std::string, PowerModeInfo> PowerModes;

    struct PowerModeTiming;
    typedef std::vector<PowerModeTiming *> PowerModeTimingArrays;

    bool                      registeredActions;
    std::string               startPowerMode;
    PowerModes                powerModes;
    PowerModeTimingArrays     powerModeTimingArrays;
  };

  inline
  void intrusive_ptr_add_ref(LookupPowerTimeModelImpl *p) {
    ::intrusive_ptr_add_ref(p);
  }

  inline
  void intrusive_ptr_release(LookupPowerTimeModelImpl *p) {
    ::intrusive_ptr_release(p);
  }

} } } // namespace SystemC_VPC::Detail::ExecModelling

namespace SystemC_VPC { namespace ExecModelling {

  static inline
  Detail::ExecModelling::LookupPowerTimeModelImpl       *getImpl(LookupPowerTimeModel       *m)
    { return static_cast<Detail::ExecModelling::LookupPowerTimeModelImpl       *>(m); }
  static inline
  Detail::ExecModelling::LookupPowerTimeModelImpl const *getImpl(LookupPowerTimeModel const *m)
    { return static_cast<Detail::ExecModelling::LookupPowerTimeModelImpl const *>(m); }

} } // namespace SystemC_VPC::Detail::Routing

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELLING_LOOKUPPOWERTIMEMODELIMPL_HPP */
