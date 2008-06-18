/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Tracing.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef HSCD_VPC_TRACING_H
#define HSCD_VPC_TRACING_H

#include <systemc.h>

namespace SystemC_VPC {
#define S_BLOCKED 'b'
#define S_READY   'w'
#define S_RUNNING 'R'

  typedef char trace_value;

  /** ASCII lower case bit is  2^5 */
  const unsigned int LOWER_CASE = 32;
  const unsigned int UPPER_CASE = ~LOWER_CASE;
  
  /**
   * tiny little helper: toggeling ASCII characters used for VCD tracing
   */
  class Tracing{

  public:
    Tracing() :
      traceSignal( new sc_signal<trace_value>() ),
      lastChange( SC_ZERO_TIME ),
      lastValue( 0 )
      { }

    /** signal for VCD tracing */ 
    sc_signal<trace_value>* traceSignal;

    /**
     * Set the value for tracing.
     * If the signal is identic to lastValue then the ascii bit for
     * lowercase id toggled
     */
    void value(trace_value value){
      if(lastChange != sc_time_stamp()){
        // remember value from last real changing (ignore delta cycle changing)
        lastValue    = *traceSignal;
        lastChange   = sc_time_stamp();
      }
      if(lastValue == value){
        // if value does not change toggle between upper and lower case
        if(value & LOWER_CASE) *traceSignal  = value & UPPER_CASE;
        else                   *traceSignal  = value | LOWER_CASE;
      }else{
        *traceSignal  = value;
      }
    }

    private:

    /** remeber last time of signal changing */
    sc_time                 lastChange;

    /** rember last signal value */
    trace_value             lastValue;

  }; // struct Tracing

} // namespace SystemC_VPC
#endif //  HSCD_VPC_TRACING_H
