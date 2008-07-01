#include "hscd_vpc_ProcessControlBlock.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_PCBPool.h"

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

  int ProcessControlBlock::globalInstanceId = 0;

  ComponentDelay::ComponentDelay( )
    : funcDelays(1, SC_ZERO_TIME),
      funcLatencies(1, SC_ZERO_TIME)
  {
    setBaseDelay(SC_ZERO_TIME);
    setBaseLatency(SC_ZERO_TIME);
  }

  ComponentDelay::ComponentDelay( const ComponentDelay &delay )
    : funcDelays(    delay.funcDelays    ),
      funcLatencies( delay.funcLatencies )
  {
    setBaseDelay(   delay.getBaseDelay()   );
    setBaseLatency( delay.getBaseLatency() );
  }

  void ComponentDelay::addDelay( FunctionId fid,
                                              sc_time delay ){
    DBG_OUT( "::addDelay(" << fid << ") " << delay
             << std::endl);
    if( fid >= funcDelays.size()){
      funcDelays.resize( fid + 100, SC_ZERO_TIME );
    }
    this->funcDelays[fid] = delay;
  }

  void ComponentDelay::setBaseDelay( sc_time delay ){
    DBG_OUT( "::setBaseDelay() " << delay
             << std::endl);
    this->funcDelays[defaultFunctionId] = delay;
  }

  sc_time ComponentDelay::getBaseDelay( ) const {
    return this->funcDelays[defaultFunctionId];
  }

  sc_time ComponentDelay::getDelay(
    FunctionId fid) const
  {
    DBG_OUT( "::getDelay(" << fid << ") " << funcDelays.size()
             << std::endl);
    assert(fid < funcDelays.size());
    sc_time ret = funcDelays[fid];
    return ret;
  }

  void ComponentDelay::addLatency( FunctionId fid,
                                                        sc_time latency ){
    if( fid >= funcLatencies.size())
      funcLatencies.resize( fid + 100, SC_ZERO_TIME );

    this->funcLatencies[fid] = latency;
  }

  void ComponentDelay::setBaseLatency( sc_time latency ){
    this->funcLatencies[defaultFunctionId] = latency;
  }

  sc_time ComponentDelay::getBaseLatency( ) const {
    return this->funcLatencies[defaultFunctionId];
  }

  sc_time ComponentDelay::getLatency(
    FunctionId fid) const
  {
    assert(fid < funcLatencies.size());
    sc_time ret = funcLatencies[fid];
    return ret;
  }

  ProcessControlBlock::ActivationCounter::ActivationCounter() :
    activation_count(0){}

  void ProcessControlBlock::ActivationCounter::increment(){
    this->activation_count++;
  }

  unsigned int ProcessControlBlock::ActivationCounter::getActivationCount(){
    return this->activation_count;
  }


  /**
   * SECTION ProcessControlBlock
   */
  
  
  ProcessControlBlock::ProcessControlBlock(PCBPool* parent)
    : name("NN"), parentPool(parent) {
    this->init();
  }
  
  ProcessControlBlock::ProcessControlBlock(std::string name): name(name){
    this->init();
  }

  ProcessControlBlock::~ProcessControlBlock(){
    if(*(this->copyCount) == 0){
      delete this->activationCount;
      delete this->copyCount; 
    }else{
      (*(this->copyCount))--;
    }
  }
  
  ProcessControlBlock::ProcessControlBlock(const ProcessControlBlock& pcb)
    : ComponentDelay(pcb)
  {
    this->setName(pcb.getName());
    this->setDeadline(pcb.getDeadline());
    this->setPeriod(pcb.getPeriod());
    this->instanceId = ProcessControlBlock::globalInstanceId++;
    this->setPriority(pcb.getPriority());
    
   
    this->blockEvent = EventPair();
    this->setTraceSignal(NULL);

    this->activationCount = pcb.activationCount;

    this->setPid(pcb.getPid());
    this->setFunctionId(pcb.getFunctionId());

    // remember amount of copies for later clean up 
    this->copyCount = pcb.copyCount;
    (*(this->copyCount))++;
    this->parentPool = pcb.parentPool;
  }

  void ProcessControlBlock::init(){

    this->blockEvent = EventPair();
    this->deadline = sc_time(DBL_MAX, SC_SEC);
    this->period = sc_time(DBL_MAX, SC_SEC);
    this->instanceId = ProcessControlBlock::globalInstanceId++;
    this->priority = 0;
    this->traceSignal = NULL;

    this->activationCount = new ActivationCounter();

    this->copyCount = new int(0);
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
      
  int ProcessControlBlock::getInstanceId() const{
    return this->instanceId;
  }

  const char* ProcessControlBlock::getFuncName() const{
    assert(0);
    return "";
  }

  void ProcessControlBlock::setBlockEvent(EventPair blockEvent){
    this->blockEvent = blockEvent;
  }

  EventPair ProcessControlBlock::getBlockEvent() const{
    return this->blockEvent;
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

  void ProcessControlBlock::incrementActivationCount(){
    this->activationCount->increment();
  }

  unsigned int ProcessControlBlock::getActivationCount() const{
    return this->activationCount->getActivationCount();
  }

  void ProcessControlBlock::setTraceSignal(Tracing* signal){
    this->traceSignal = signal;
  }

  Tracing* ProcessControlBlock::getTraceSignal(){
    return this->traceSignal;
  }

  void ProcessControlBlock::release(){
    this->parentPool->free(this);
  }
}
