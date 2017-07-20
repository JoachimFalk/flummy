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

#ifndef _INCLUDED_SYSTEMCVPC_TIMETRIGGERED_TT_SUPPORT_HPP
#define _INCLUDED_SYSTEMCVPC_TIMETRIGGERED_TT_SUPPORT_HPP

#include <queue>

namespace SystemC_VPC
{
namespace TT
{

/* struct used to store an event with a certain release-time */
struct TimeNodePair{
  TimeNodePair(sc_core::sc_time time,  ScheduledTask *node)
    : time(time), node(node) {}
  sc_core::sc_time time;
  ScheduledTask *node;
};

/* struct used for comparison
 * needed by the priority_queue */
struct nodeCompare{
  bool operator()(const TimeNodePair& tnp1,
                  const TimeNodePair& tnp2) const
  {
    sc_core::sc_time p1=tnp1.time;
    sc_core::sc_time p2=tnp2.time;
    if (p1 > p2)
      return true;
    else
      return false;
  }
};

typedef std::priority_queue <TimeNodePair,
                             std::vector<TimeNodePair>,
                             nodeCompare>  TimedQueue;

} // namespace TT
} // namespace SystemC_VPC
#endif /* _INCLUDED_SYSTEMCVPC_TIMETRIGGERED_TT_SUPPORT_HPP */
