// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
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

#ifndef _INCLUDED_MCOUNTERSOURCE_HPP
#define _INCLUDED_MCOUNTERSOURCE_HPP

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>

#include "smoc_synth_std_includes.hpp"

# define IMAGE_WIDTH  8
# define IMAGE_HEIGHT 8

class CounterSource: public smoc_actor {
public:
  smoc_port_out<int> out;
public:
  unsigned long long SRC_ITER;
private:
  unsigned long long SRC_ITERS;
  
  void process() {
    out[0] = (SRC_ITER & 63);
    ++SRC_ITER;
  }
 
  smoc_firing_state start, end;
public:
  CounterSource(sc_core::sc_module_name name, size_t periods)
    : smoc_actor(name, start), SRC_ITER(0)
  {
    SMOC_REGISTER_CPARAM(periods);
    {
      char *init = std::getenv("SRC_ITERS");
      if (init)
        SRC_ITERS = std::atoll(init);
      else
        SRC_ITERS = periods * 64;
    }
    start
      = (out(1) && VAR(SRC_ITER) < VAR(SRC_ITERS)) >>
        CALL(CounterSource::process)               >> start
      | (VAR(SRC_ITER) >= VAR(SRC_ITERS))          >> end
      ;
  }
};

#endif // _INCLUDED_MCOUNTERSOURCE_HPP
