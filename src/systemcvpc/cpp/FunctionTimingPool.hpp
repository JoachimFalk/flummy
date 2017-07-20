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

#ifndef _INCLUDED_SYSTEMCVPC_FUNCTIONTIMINGPOOL_HPP
#define _INCLUDED_SYSTEMCVPC_FUNCTIONTIMINGPOOL_HPP

#include <map>
#include <boost/shared_ptr.hpp>

#include <systemcvpc/FastLink.hpp>
#include <systemcvpc/config/Timing.hpp>
#include <systemcvpc/TimingModifier.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>


namespace SystemC_VPC {

  typedef std::vector<sc_core::sc_time> FunctionTimes;
  typedef std::vector<boost::shared_ptr<TimingModifier> > FunctionTimingModifiers;

  /**
   * Internal helper class to manage  function specific delays.
   */
  class FunctionTiming{

  public:

    /**
     * \brief Default constuctor of an ComponentDealy instance
     * \param name specifies the name of the associated component
     * simulation
     */
    FunctionTiming( );

    /**
     * copy constructor
     */
    FunctionTiming( const FunctionTiming &delay );

    /**
     * \brief Adds a new function delay to the instance
     * \param funcname specifies the associated function
     * \param delay is the corresponding delay for the function execution
     */
    void addDelay(FunctionId fid, sc_core::sc_time delay);

    /**
     * \brief Set the base delay to the instance
     * \param funcname specifies the associated function
     * \param delay is the corresponding delay for the function execution
     */
    void setBaseDelay(sc_core::sc_time delay);
    sc_core::sc_time getBaseDelay( ) const;

    /**
     * \brief Used to access delay
     * \return Returns the sum of function delays. The default delay is
     * returned if no function names are given.
     */
    sc_core::sc_time getDelay(FunctionIds functions) const;

    /**
     * \brief Adds a new function latency to the instance
     * \param funcname specifies the associated function
     * \param latency is the corresponding latency for the function
     * execution
     */
    void addLatency(FunctionId fid, sc_core::sc_time latency);

    /**
     * \brief Set the base latency to the instance
     * \param latency is the corresponding latency for the function
     * execution
     */
    void setBaseLatency(sc_core::sc_time latency);
    sc_core::sc_time getBaseLatency( ) const;

    /**
     * \brief Used to access latency
     * \return Returns the sum of function latencies. The default latency is
     * returned if no function names are given.
     */
    sc_core::sc_time getLatency(FunctionIds functions);


    /**
     * \brief set the timing of the instace
		 * \param timing is the timing to set
     */
    void setTiming(const Config::Timing& timing);

    /**
		 * \brief adds TimingModifiers to the instance
		 * \param fids is the id of the function the timingModifiers should be added to
		 * \param timingModifier is the timingModifier to set
		 */
    void addTimingModifier(FunctionId fid, boost::shared_ptr<TimingModifier> timingModifier);

		/**
		 * \brief set the base timingModifier
		 * \param timingModifier is the timingModifier to set
		 */
    void setBaseTimingModifier( boost::shared_ptr<TimingModifier> timingModifier);
		/**
		 * \brief drop the history of the modifications
		 * \param functions is a list of functionids affected
		 */
    void reset(FunctionIds functions);

  private:

    // map of possible special delay depending on functions
    FunctionTimes funcDelays;

    // map of possible function specific latencies
    FunctionTimes funcLatencies;

    // map of possible Timing Modifiers
    FunctionTimingModifiers funcTimingModifiers;

  };

  typedef boost::shared_ptr<FunctionTiming> FunctionTimingPtr;
  typedef std::map<ProcessId, FunctionTimingPtr>  FunctionTimingPool;
  typedef boost::shared_ptr<FunctionTimingPool> FunctionTimingPoolPtr;

}
#endif /* _INCLUDED_SYSTEMCVPC_FUNCTIONTIMINGPOOL_HPP */
