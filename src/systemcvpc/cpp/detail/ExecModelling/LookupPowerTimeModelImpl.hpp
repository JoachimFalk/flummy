// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2020 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELLING_DEFAULTTIMINGPROVIDER_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELLING_DEFAULTTIMINGPROVIDER_HPP

#include <systemcvpc/ExecModelling/LookupPowerTimeModel.hpp>

#include "../AbstractExecModel.hpp"



#include <systemcvpc/Timing.hpp>


//#include <systemcvpc/datatypes.hpp>

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
    typedef boost::intrusive_ptr<this_type> const ConstPtr;

    LookupPowerTimeModelImpl();

    ~LookupPowerTimeModelImpl();

    ///
    /// Handle interfaces for SystemC_VPC::ExecModel
    ///

    void add(Timing timing);
    void addDefaultActorTiming(std::string actorName, Timing timing);

    bool addAttribute(AttributePtr attr);

    ///
    /// Handle interfaces for SystemC_VPC::ExecModelling::LookupPowerTimeModel
    ///
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
    CompState  *attachToComponent(AbstractComponent *comp);

    ActionInfo *registerAction(AbstractExecModel::CompState *&execModelComponentState
      , TaskInterface const *actor
      , PossibleAction const *action);

    /// Change the power mode of a component. This should update the
    /// opaque object pointed to by execModelComponentState.
    void  setPowerMode(AbstractExecModel::CompState *&execModelComponentState
      , std::string const &mode) const;

    /// Initialize ti with action or guard timing and power values.
    void  initTaskInstance(AbstractExecModel::CompState *&execModelComponentState
      , ActionInfo *ai
      , TaskInstance *ti
      , bool forGuard = false) const;

    ///
    /// Other stuff
    ///

    struct PowerModeTiming;

    typedef std::map<std::string, Timing>  Timings;
    typedef std::map<std::string, Timings> PowerModeDependentTimings;
    typedef std::map<std::string, size_t>  PowerModeToIndex;
    typedef std::vector<PowerModeTiming *> PowerModeTimingArrays;

    bool                      registeredActions;
    PowerModeDependentTimings powerModeDependentTimings;
    PowerModeToIndex          powerModeToIndex;
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

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELLING_DEFAULTTIMINGPROVIDER_HPP */
