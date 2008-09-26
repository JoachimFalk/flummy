#include "hscd_vpc_AbstractComponent.h"
#include "hscd_vpc_ProcessControlBlock.h"

#include "debug_config.h"
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_PCB
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif

namespace SystemC_VPC{

  FunctionTiming::FunctionTiming( )
    : funcDelays(1, SC_ZERO_TIME),
      funcLatencies(1, SC_ZERO_TIME)
  {
    setBaseDelay(SC_ZERO_TIME);
    setBaseLatency(SC_ZERO_TIME);
  }

  FunctionTiming::FunctionTiming( const FunctionTiming &delay )
    : funcDelays(    delay.funcDelays    ),
      funcLatencies( delay.funcLatencies )
  {
    setBaseDelay(   delay.getBaseDelay()   );
    setBaseLatency( delay.getBaseLatency() );
  }

  void FunctionTiming::addDelay( FunctionId fid,
                                              sc_time delay ){
    DBG_OUT( "::addDelay(" << fid << ") " << delay
             << std::endl);
    if( fid >= funcDelays.size()){
      funcDelays.resize( fid + 100, SC_ZERO_TIME );
    }
    this->funcDelays[fid] = delay;
  }

  void FunctionTiming::setBaseDelay( sc_time delay ){
    DBG_OUT( "::setBaseDelay() " << delay
             << std::endl);
    this->funcDelays[defaultFunctionId] = delay;
  }

  sc_time FunctionTiming::getBaseDelay( ) const {
    return this->funcDelays[defaultFunctionId];
  }

  sc_time FunctionTiming::getDelay(
    FunctionId fid) const
  {
    DBG_OUT( "::getDelay(" << fid << ") " << funcDelays.size()
             << std::endl);

    sc_time ret;
    if(fid < funcDelays.size()){
      ret = funcDelays[fid];
    }else{
      ret = getBaseDelay();
    }
    return ret;
  }

  void FunctionTiming::addLatency( FunctionId fid,
                                                        sc_time latency ){
    if( fid >= funcLatencies.size())
      funcLatencies.resize( fid + 100, SC_ZERO_TIME );

    this->funcLatencies[fid] = latency;
  }

  void FunctionTiming::setBaseLatency( sc_time latency ){
    this->funcLatencies[defaultFunctionId] = latency;
  }

  sc_time FunctionTiming::getBaseLatency( ) const {
    return this->funcLatencies[defaultFunctionId];
  }

  sc_time FunctionTiming::getLatency(
    FunctionId fid) const
  {
    sc_time ret;
    if(fid < funcLatencies.size()){
      ret = funcLatencies[fid];
    }else{
      ret = getBaseLatency();
    }
    return ret;
  }

  void FunctionTiming::setTiming(const Timing& timing){
    this->addDelay(timing.fid,   timing.dii);
    this->addLatency(timing.fid, timing.latency);
  }

  /**
   * SECTION ProcessControlBlock
   */
  
  
  ProcessControlBlock::ProcessControlBlock( AbstractComponent * component )
    : name("NN"), component(component) {
    this->init();
  }

  void ProcessControlBlock::init(){

    this->deadline = sc_time(DBL_MAX, SC_SEC);
    this->period = sc_time(DBL_MAX, SC_SEC);
    this->priority = 0;
    this->traceSignal = NULL;
  }

  void ProcessControlBlock::setName(std::string name){
    this->name = name;
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

  void ProcessControlBlock::setPeriod(sc_time period){
    this->period = period;
  }

  sc_time ProcessControlBlock::getPeriod() const{
    return this->period;
  }

  void ProcessControlBlock::setPriority(int priority){
    this->priority = priority;
  }

  int ProcessControlBlock::getPriority() const{
    return this->priority;
  }

  void ProcessControlBlock::setDeadline(sc_time deadline){
    if(deadline > SC_ZERO_TIME){
      this->deadline = deadline;
    }else{
      this->deadline = SC_ZERO_TIME;
    }
  }

  sc_time ProcessControlBlock::getDeadline() const{
    return this->deadline;
  }

  void ProcessControlBlock::setTraceSignal(Tracing* signal){
    this->traceSignal = signal;
  }

  Tracing* ProcessControlBlock::getTraceSignal(){
    return this->traceSignal;
  }

  void ProcessControlBlock::setTiming(const Timing& timing){
    const PowerMode *mode = this->component->translatePowerMode(timing.powerMode);
    FunctionTiming *ft =this->component->getTiming(mode, this->getPid());
    ft->setTiming(timing);
  }

  void ProcessControlBlock::setBaseDelay(sc_time delay){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTiming *ft =this->component->getTiming(mode, this->getPid());
    ft->setBaseDelay(delay);
  }

  void ProcessControlBlock::setBaseLatency(sc_time latency){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTiming *ft =this->component->getTiming(mode, this->getPid());
    ft->setBaseLatency(latency);
  }

  void ProcessControlBlock::addDelay(FunctionId fid, sc_time delay){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTiming *ft =this->component->getTiming(mode, this->getPid());
    ft->addDelay(fid, delay);
  }

  void ProcessControlBlock::addLatency(FunctionId fid, sc_time latency){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTiming *ft =this->component->getTiming(mode, this->getPid());
    ft->addLatency(fid, latency);
  }

}
