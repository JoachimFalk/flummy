// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

typedef boost::minstd_rand base_generator_type;
#include <systemcvpc/TimingModifier.hpp>
#include <systemcvpc/vpc_config.h>

#include "FunctionTiming.hpp"
#include "DebugOStream.hpp"

namespace SystemC_VPC { namespace Detail {

  FunctionTiming::FunctionTiming( )
    : funcDelays(1, sc_core::SC_ZERO_TIME),
      funcLatencies(1, sc_core::SC_ZERO_TIME),
      funcTimingModifiers(1, boost::shared_ptr<TimingModifier>(new TimingModifier()))
  {
    setBaseDelay(sc_core::SC_ZERO_TIME);
    setBaseLatency(sc_core::SC_ZERO_TIME);
  }

  FunctionTiming::FunctionTiming( const FunctionTiming &delay )
    : funcDelays(    delay.funcDelays    ),
      funcLatencies( delay.funcLatencies ),
      funcTimingModifiers( delay.funcTimingModifiers)
  {
    setBaseDelay(   delay.getBaseDelay()   );
    setBaseLatency( delay.getBaseLatency() );
  }

  void FunctionTiming::addDelay( FunctionId fid,
                                              sc_core::sc_time delay ){
    DBG_OUT( "::addDelay(" << fid << ") " << delay
             << std::endl);
    if( fid >= funcDelays.size()){
      funcDelays.resize( fid + 100, sc_core::SC_ZERO_TIME );
    }
    this->funcDelays[fid] = delay;

  }

  void FunctionTiming::setBaseDelay( sc_core::sc_time delay ){
    DBG_OUT( "::setBaseDelay() " << delay
             << std::endl);
    this->funcDelays[defaultFunctionId] = delay;
  }

  sc_core::sc_time FunctionTiming::getBaseDelay( ) const {
    boost::shared_ptr<TimingModifier> modifier = this->funcTimingModifiers[defaultFunctionId];
    return modifier->modify(this->funcDelays[defaultFunctionId]);
  }

  void FunctionTiming::reset(
    FunctionIds functions)
  {
    if (functions.begin() == functions.end()){
      boost::shared_ptr<TimingModifier> modifier = this->funcTimingModifiers[defaultFunctionId];
      modifier->reset();
    }
    for(FunctionIds::const_iterator iter = functions.begin();
        iter != functions.end();
        ++iter) {
      FunctionId fid = *iter;
      boost::shared_ptr<TimingModifier> modifier = this->funcTimingModifiers[fid];
      modifier->reset();
    }
  }

  sc_core::sc_time summarizeFunctionTimes(const FunctionIds& functions,
      const FunctionTimes& functionTimes,
      const FunctionTimingModifiers& timingModifiers){
    sc_core::sc_time ret = sc_core::SC_ZERO_TIME;
    for(FunctionIds::const_iterator iter = functions.begin();
        iter != functions.end();
        ++iter) {
      FunctionId fid = *iter;
      assert(fid < functionTimes.size());
      boost::shared_ptr<TimingModifier> modifier = timingModifiers[fid];
      ret += modifier->modify(functionTimes[fid]);
    }
    return ret;
  }

  sc_core::sc_time rePlaySummarizeFunctionTimes(const FunctionIds& functions,
      const FunctionTimes& functionTimes,
      const FunctionTimingModifiers& timingModifiers){
    sc_core::sc_time ret = sc_core::SC_ZERO_TIME;
    for(FunctionIds::const_iterator iter = functions.begin();
        iter != functions.end();
        ++iter) {
      FunctionId fid = *iter;
      assert(fid < functionTimes.size());
      boost::shared_ptr<TimingModifier> modifier = timingModifiers[fid];
      ret += modifier->rePlay(functionTimes[fid]);
    }
    return ret;
  }

  sc_core::sc_time FunctionTiming::getDelay(
    FunctionIds functions) const
  {
    if (functions.begin() == functions.end()){
      return getBaseDelay();
    }
    return summarizeFunctionTimes(functions, funcDelays,funcTimingModifiers);
  }

  void FunctionTiming::addTimingModifier( FunctionId fid,
                                          boost::shared_ptr<TimingModifier> timingModifier ){
    if( fid >= funcTimingModifiers.size())
      funcTimingModifiers.resize( fid + 100, boost::shared_ptr<TimingModifier>(new TimingModifier()));

    this->funcTimingModifiers[fid] = timingModifier;
  }

  void FunctionTiming::setBaseTimingModifier( boost::shared_ptr<TimingModifier> timingModifier){
    this->funcTimingModifiers[defaultFunctionId] = timingModifier;
  }

  void FunctionTiming::addLatency( FunctionId fid,
                                                        sc_core::sc_time latency ){
    if( fid >= funcLatencies.size())
      funcLatencies.resize( fid + 100, sc_core::SC_ZERO_TIME );

    this->funcLatencies[fid] = latency;
  }

  void FunctionTiming::setBaseLatency( sc_core::sc_time latency ){
    this->funcLatencies[defaultFunctionId] = latency;
  }

  sc_core::sc_time FunctionTiming::getBaseLatency( ) const {
    boost::shared_ptr<TimingModifier> modifier = this->funcTimingModifiers[defaultFunctionId];
    //replay the result from getBaseDelay() to get identical modifications
    return modifier->rePlay(this->funcLatencies[defaultFunctionId]);
  }

  sc_core::sc_time FunctionTiming::getLatency(
    FunctionIds functions)
  {
    this->reset(functions);
    if (functions.begin() == functions.end()){
      return getBaseLatency();
    }
    //replay the result from getDelay() to get identical modifications
   return rePlaySummarizeFunctionTimes(functions, funcLatencies,funcTimingModifiers);
  }

  void FunctionTiming::setTiming(const SystemC_VPC::Timing& timing){
    this->addDelay(timing.getFunctionId(),   timing.getDii());
    this->addLatency(timing.getFunctionId(), timing.getLatency());
    this->addTimingModifier(timing.getFunctionId(), timing.getTimingModifier());
  }

} } // namespace SystemC_VPC::Detail
