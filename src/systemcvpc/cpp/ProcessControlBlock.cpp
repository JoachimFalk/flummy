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

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
typedef boost::minstd_rand base_generator_type;
#include <systemcvpc/TimingModifier.hpp>
#include <systemcvpc/vpc_config.h>

#include <systemcvpc/AbstractComponent.hpp>
#include <systemcvpc/ProcessControlBlock.hpp>
#include <systemcvpc/Director.hpp>
#include <CoSupport/Tracing/TracingFactory.hpp>
#include <ctime> 

#include <systemcvpc/debug_config.hpp>
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_PCB
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include <systemcvpc/debug_on.hpp>
#else
  #include <systemcvpc/debug_off.hpp>
#endif

namespace SystemC_VPC{

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

  void FunctionTiming::setTiming(const Config::Timing& timing){
    this->addDelay(timing.getFunctionId(),   timing.getDii());
    this->addLatency(timing.getFunctionId(), timing.getLatency());
    this->addTimingModifier(timing.getFunctionId(), timing.getTimingModifier());
  }

  /**
   * SECTION ProcessControlBlock
   */
  
  
  ProcessControlBlock::ProcessControlBlock( AbstractComponent * component )
    : name("NN"), component(component), psm(false) {
    this->init();
  }

  void ProcessControlBlock::init(){

    this->deadline = sc_core::sc_time(DBL_MAX, sc_core::SC_SEC);
    this->period = sc_core::sc_time(DBL_MAX, sc_core::SC_SEC);
    this->priority = 0;
    this->traceSignal = NULL;
    this->psm=false;
  }

  void ProcessControlBlock::configure(std::string name, bool tracing){
    this->name = name;
    taskTracer =
        CoSupport::Tracing::TracingFactory::getInstance().createTaskTracer(name,
            component->getName());
  }

  std::string const& ProcessControlBlock::getName() const{
    return this->name;
  }

  void ProcessControlBlock::setPid( ProcessId pid){
    this->pid=pid;
  }

  ProcessId ProcessControlBlock::getPid( ) const{
    return this->pid;
  }
      
  void ProcessControlBlock::setFunctionId( FunctionId fid){
    this->fid=fid;
  }

  FunctionId ProcessControlBlock::getFunctionId( ) const{
    return this->fid;
  }

  const char* ProcessControlBlock::getFuncName() const{
    assert(0);
    return "";
  }

  void ProcessControlBlock::setPeriod(sc_core::sc_time period){
    this->period = period;
  }

  sc_core::sc_time ProcessControlBlock::getPeriod() const{
    return this->period;
  }

  void ProcessControlBlock::setPriority(int priority){
    this->priority = priority;
  }

  int ProcessControlBlock::getPriority() const{
    return this->priority;
  }

  void ProcessControlBlock::setDeadline(sc_core::sc_time deadline){
    if(deadline > sc_core::SC_ZERO_TIME){
      this->deadline = deadline;
    }else{
      this->deadline = sc_core::SC_ZERO_TIME;
    }
  }

  sc_core::sc_time ProcessControlBlock::getDeadline() const{
    return this->deadline;
  }

  void ProcessControlBlock::setTraceSignal(Trace::Tracing* signal){
    this->traceSignal = signal;
  }

  Trace::Tracing* ProcessControlBlock::getTraceSignal(){
    return this->traceSignal;
  }

  void ProcessControlBlock::setTiming(const Config::Timing& timing){
    const PowerMode *mode = this->component->translatePowerMode(timing.getPowerMode());
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->setTiming(timing);
  }

  void ProcessControlBlock::setBaseDelay(sc_core::sc_time delay){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->setBaseDelay(delay);
  }

  void ProcessControlBlock::setBaseLatency(sc_core::sc_time latency){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->setBaseLatency(latency);
  }

  void ProcessControlBlock::addDelay(FunctionId fid, sc_core::sc_time delay){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->addDelay(fid, delay);
  }

  void ProcessControlBlock::addLatency(FunctionId fid, sc_core::sc_time latency){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->addLatency(fid, latency);
  }

  void ProcessControlBlock::setActorAsPSM(bool psm)
  {
	  this->psm = psm;
  }

  bool ProcessControlBlock::isPSM()
  {
	  return this->psm;
  }
}
