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

#include <systemcvpc/ScheduledTask.hpp>
#include "Delayer.hpp"

namespace SystemC_VPC
{

ScheduledTask::ScheduledTask(sc_core::sc_module_name name)
  : sc_core::sc_module(name)
  , component(NULL), pid(-1), active(true)
{
  SC_METHOD(scheduleRequestMethod);
  sensitive << scheduleRequest;
  dont_initialize();
}

ScheduledTask::~ScheduledTask()
{
}
void ScheduledTask::setDelayer(Delayer *component)
{
  this->component = component;
}

Delayer* ScheduledTask::getDelayer()
{
  return this->component;
}

void ScheduledTask::setActivation(bool active)
{
  if (component != NULL) {
    component->notifyActivation(this, active);
  } else {
    if (active)
      scheduleRequest.notify(getNextReleaseTime()-sc_core::sc_time_stamp());
    else
      scheduleRequest.cancel();
  }
}

void ScheduledTask::scheduleRequestMethod() {
  while (canFire())
    schedule();
}

void ScheduledTask::setPid(ProcessId pid)
{
  this->pid = pid;
}

ProcessId ScheduledTask::getPid() const
{
  return this->pid;
}

void ScheduledTask::setActive(bool a){
  if (a && !active) {
    component->notifyActivation(this, true);
  }
  active=a;
}

} // namespace SystemC_VPC
