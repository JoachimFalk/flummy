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

//#define VPC_ENABLE_PLAIN_TRACING
#include <CoSupport/Streams/AlternateStream.hpp>


namespace SystemC_VPC {
  typedef char trace_value;

  /** ASCII lower case bit is  2^5 */
  const unsigned int LOWER_CASE = 32;
  const unsigned int UPPER_CASE = ~LOWER_CASE;
  
  /**
   * tiny little helper: toggeling ASCII characters used for VCD tracing
   */
  class Tracing{
  public:

    static const trace_value S_SLEEP;
    static const trace_value S_BLOCKED;
    static const trace_value S_READY;
    static const trace_value S_RUNNING;

    Tracing( std::string resource, std::string task ) :
      traceSignal( new sc_signal<trace_value>() ),
      resource(resource),
      task(task),
      lastChange( SC_ZERO_TIME ),
      lastValue( 0 )
      { }

    /** signal for VCD tracing */ 
    sc_signal<trace_value>* traceSignal;

    void traceRunning(){
      this->tracePlain("RUN");
      this->setValue(S_RUNNING);
    }

    void traceBlocking(){
      this->tracePlain("BLOCK");
      this->setValue(S_BLOCKED);
    }

    void traceSleeping(){
      this->tracePlain("SLEEP");
      this->setValue(S_SLEEP);
    }

    void traceReady(){
      this->tracePlain("WAIT");
      this->setValue(S_READY);
    }

    void tracePlain(std::string traceValue){
#ifdef VPC_ENABLE_PLAIN_TRACING
      if(plainTrace != NULL){
        *plainTrace << sc_time_stamp().value()
                    << "\t" << resource
                    << "\t" << task
                    << "\t" << traceValue
                    <<  std::endl;
      }
#endif // VPC_ENABLE_PLAIN_TRACING

    }
  private:

    /**
     * Set trace value.
     * If the signal is identic to lastValue then the ascii bit for
     * lowercase is toggled
     */
    void setValue(trace_value value){
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

#ifdef VPC_ENABLE_PLAIN_TRACING
    static std::ostream * plainTrace;
#endif // VPC_ENABLE_PLAIN_TRACING

    /// name of traced resource
    std::string resource;

    /// name of traced task
    std::string task;

    /** remeber last time of signal changing */
    sc_time                 lastChange;

    /** rember last signal value */
    trace_value             lastValue;

  }; // struct Tracing

} // namespace SystemC_VPC
#endif //  HSCD_VPC_TRACING_H
