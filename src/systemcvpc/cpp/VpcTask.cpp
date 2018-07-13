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

#include "config.h"

#include <systemcvpc/VpcTask.hpp>
#include <systemcvpc/Mappings.hpp>
#include <systemcvpc/Component.hpp>
#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/ScheduledTask.hpp>

namespace SystemC_VPC {

  VpcTask::VpcTask(ScheduledTask const *actor)
    : task(actor)
    , taskName(actor->name())
    , priority(0)
    , psm(false) {}

  VpcTask::VpcTask(std::string   const &actorName)
    : task(nullptr) // Later, must be set by setActor
    , taskName(actorName)
    , priority(0)
    , psm(false) {}

  VpcTask::~VpcTask() {}

  void VpcTask::mapTo(Component::Ptr component) {
    Mappings::getConfiguredMappings()[Ptr(this)] = component;
  }

  void VpcTask::setPriority(size_t priority) {
    this->priority = priority;
  }

  size_t VpcTask::getPriority() const {
    return this->priority;
  }

  const ScheduledTask *VpcTask::getActor() const {
    return this->task;
  }

  void VpcTask::setActor(ScheduledTask const *actor) {
    assert(actor && actor->name() == taskName && "Only a valid actor can be set!");
    assert((!task || task == actor) && "Setting the actor can be done only once!");
    task = actor;
  }

  void VpcTask::setActorAsPSM(bool psm) {
    this->psm = psm;
  }

  bool VpcTask::isPSM() {
    return this->psm;
  }

} // namespace SystemC_VPC
