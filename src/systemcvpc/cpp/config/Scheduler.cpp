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

#include <systemcvpc/config/ConfigException.hpp>
#include <systemcvpc/config/Scheduler.hpp>

#include <string>

namespace SystemC_VPC
{

namespace Config
{

namespace Scheduler
{

Type parseScheduler(std::string name)
{
  static const std::string STR_TDMA = "TDMA";
  static const std::string STR_FLEXRAY = "FlexRay";
  static const std::string STR_TTCC = "TTCC";
  static const std::string STR_ROUNDROBIN = "RoundRobin";
  static const std::string STR_RR = "RR";
  static const std::string STR_PRIORITYSCHEDULER = "PriorityScheduler";
  static const std::string STR_PS = "PS";
  static const std::string STR_PRIORITYSCHEDULERNOPREEMPT = "PrioritySchedulerNoPreempt";
  static const std::string STR_PSNOPRE = "PSNOPRE";
  static const std::string STR_PSNOPRE_NO_TT = "PSNOPRE-noTT";
  static const std::string STR_RATEMONOTONIC = "RateMonotonic";
  static const std::string STR_RM = "RM";
  static const std::string STR_FIRSTCOMEFIRSTSERVED = "FirstComeFirstServed";
  static const std::string STR_FCFS = "FCFS";
  static const std::string STR_FCFS_NO_TT = "FCFS-noTT";
  static const std::string STR_AVB = "AVB";
  static const std::string STR_MOST = "MOST";
  static const std::string STR_STREAMSHAPER = "StreamShaper";

  if (name == STR_TDMA) {
    return TDMA;
  } else if (name == STR_FLEXRAY) {
    return FlexRay;
  } else if (name == STR_TTCC) {
    return TTCC;
  } else if (name == STR_RR || name == STR_ROUNDROBIN) {
    return RoundRobin;
  } else if (name == STR_PS || name == STR_PRIORITYSCHEDULER) {
    return StaticPriority_P;
  } else if (name == STR_PSNOPRE || name == STR_PRIORITYSCHEDULERNOPREEMPT) {
    return StaticPriority_NP;
  } else if (name == STR_PSNOPRE_NO_TT) {
    return StaticPriority_NP_noTT;
  } else if (name == STR_RM || name == STR_RATEMONOTONIC) {
    return RateMonotonic;
  } else if (name == STR_FCFS || name == STR_FIRSTCOMEFIRSTSERVED) {
    return FCFS;
  } else if (name == STR_FCFS_NO_TT) {
    return FCFS_noTT;
  } else if (name == STR_AVB) {
    return AVB;
  } else if (name == STR_MOST){
    return MOST;
  } else if (name == STR_STREAMSHAPER){
    return StreamShaper;
  } else {
    throw Config::ConfigException("Unknown scheduler \"" + name
        + "\" for component: " + name);
    return FCFS;
  }

}

} // namespace Scheduler
} // namespace Config
} // namespace SystemC_VPC

