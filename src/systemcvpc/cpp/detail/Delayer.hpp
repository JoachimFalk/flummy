// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_DELAYER_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_DELAYER_HPP

#include <systemcvpc/datatypes.hpp>

#include "ComponentInfo.hpp"
#include "ComponentObserver.hpp"
#include "TaskInstance.hpp"

#include <vector>

namespace SystemC_VPC { namespace Detail {

  class Director;

  /**
   * \brief Interface for classes implementing delay simulation.
   */
  class Delayer
    : private SequentiallyIdedObject<ComponentId> {
  public:
    /**
     * \brief Simulate the delay caused by the transition execution on this Delayer.
     *
     * While the simulation is running SystemC simulation time is consumed.
     */
    virtual void compute(TaskInstance* task) = 0;

    /**
     * \brief Simulate the delay caused by the transition guard check on this Delayer.
     *
     * While the simulation is running SystemC simulation time is consumed.
     */
    virtual void check(TaskInstance* task) {}

    const std::string &getName() const
      { return name_; }

    const ComponentId getComponentId() const;

    void addObserver(ComponentObserver *obs);

    void removeObserver(ComponentObserver *obs);
    
    void fireNotification(ComponentInfo *compInf);

    virtual void initialize(const Director* d) {};

    virtual ~Delayer() {}

  protected:
    Delayer(std::string const &name);

    typedef std::vector<ComponentObserver *> Observers;
    
    Observers observers;
    
  private:
    std::string name_;
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_DELAYER_HPP */
