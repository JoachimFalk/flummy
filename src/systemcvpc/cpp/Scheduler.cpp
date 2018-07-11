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

#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/Scheduler.hpp>

#include <string>

namespace SystemC_VPC {

Scheduler parseScheduler(std::string name) {
  if (name == "TDMA") {
    return Scheduler::TDMA;
  } else if (name == "FlexRay") {
    return Scheduler::FlexRay;
  } else if (name == "TTCC") {
    return Scheduler::TTCC;
  } else if (name == "RR" || name == "RoundRobin") {
    return Scheduler::RoundRobin;
  } else if (name == "RRNOPRE" || name == "RoundRobinNoPreempt") {
    return Scheduler::RoundRobinNoPreempt;
  } else if (name == "SP" || name == "StaticPriority") {
    return Scheduler::StaticPriority;
  } else if (name == "SPNOPRE" || name == "StaticPriorityNoPreempt") {
    return Scheduler::StaticPriorityNoPreempt;
  } else if (name == "RM" || name == "RateMonotonic") {
    return Scheduler::RateMonotonic;
  } else if (name == "FCFS" || name == "FirstComeFirstServed") {
    return Scheduler::FCFS;
  } else if (name == "AVB") {
    return Scheduler::AVB;
  } else if (name == "MOST"){
    return Scheduler::MOST;
  } else if (name == "StreamShaper"){
    return Scheduler::StreamShaper;
  } else {
    throw SystemC_VPC::ConfigException("Unknown scheduler \"" + name + "\"!");
  }
}

}  // namespace SystemC_VPC
