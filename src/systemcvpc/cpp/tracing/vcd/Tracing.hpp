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

#ifndef HSCD_VPC_TRACING_H
#define HSCD_VPC_TRACING_H

#include <systemc>

//#define VPC_ENABLE_PLAIN_TRACING
#include <CoSupport/Streams/AlternateStream.hpp>


namespace SystemC_VPC {
namespace Trace {
  typedef char trace_value;

  /** ASCII lower case bit is  2^5 */
  const unsigned int LOWER_CASE = 32;
  const unsigned int UPPER_CASE = ~LOWER_CASE;
  
  /**
   * tiny little helper: toggle ASCII characters used for VCD tracing
   */
  class Tracing{
  public:

    static const trace_value S_SLEEP;
    static const trace_value S_BLOCKED;
    static const trace_value S_READY;
    static const trace_value S_RUNNING;

    Tracing( std::string resource, std::string task ) :
      traceSignal( new sc_core::sc_signal<trace_value>() ),
      resource(resource),
      task(task),
      lastChange( sc_core::SC_ZERO_TIME ),
      lastValue( 0 )
      { }

    /** signal for VCD tracing */ 
    sc_core::sc_signal<trace_value>* traceSignal;

    void traceRunning(){
      this->tracePlain("RUN");
      this->setValueWithCaseCorrection(S_RUNNING);
    }

    void traceBlocking(){
      this->tracePlain("BLOCK");
      this->setValueWithCaseCorrection(S_BLOCKED);
    }

    void traceSleeping(){
      this->tracePlain("SLEEP");
      this->writeValue(S_SLEEP);
    }

    void traceReady(){
      this->tracePlain("WAIT");
      this->setValueWithCaseCorrection(S_READY);
    }

    void tracePlain(std::string traceValue){
#ifdef VPC_ENABLE_PLAIN_TRACING
      if(plainTrace != NULL){
        *plainTrace << sc_core::sc_time_stamp().value()
                    << "\t" << resource
                    << "\t" << task
                    << "\t" << traceValue
                    <<  std::endl;
      }
#endif // VPC_ENABLE_PLAIN_TRACING

    }
  private:

    /**
     * remember last value and time stamp of change
     */
    void rememberLastValue(){
      if(lastChange != sc_core::sc_time_stamp()){
        // remember value from last real changing (ignore delta cycle changing)
        lastValue    = *traceSignal;
        lastChange   = sc_core::sc_time_stamp();
      }
    }

    /**
     * write value to signal
     */
    void writeValue(trace_value value){
      rememberLastValue();
      *traceSignal  = value;
    }

    /**
     * Set trace value.
     * If the signal is identical to lastValue then the ASCII bit for
     * lower case is toggled
     */
    void setValueWithCaseCorrection(trace_value value){
      rememberLastValue();

      if(lastValue == value){
        // if value does not change toggle between upper and lower case
        if(value & LOWER_CASE){
          writeValue(value & UPPER_CASE);
        } else {
          writeValue(value | LOWER_CASE);
        }
      }else{
        writeValue(value);
      }
    }

#ifdef VPC_ENABLE_PLAIN_TRACING
    static std::ostream * plainTrace;
#endif // VPC_ENABLE_PLAIN_TRACING

    /// name of traced resource
    std::string resource;

    /// name of traced task
    std::string task;

    /** remeber last time of signal changing */
    sc_core::sc_time                 lastChange;

    /** rember last signal value */
    trace_value             lastValue;

  }; // struct Tracing

} // namespace Trace
} // namespace SystemC_VPC
#endif //  HSCD_VPC_TRACING_H
