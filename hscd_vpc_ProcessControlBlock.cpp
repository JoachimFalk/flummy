#include "hscd_vpc_ProcessControlBlock.h"
#include "hscd_vpc_Director.h"

namespace SystemC_VPC{

  int ProcessControlBlock::globalInstanceId = 0;

  ProcessControlBlock::ComponentDelay::ComponentDelay( ComponentId cid )
    : cid(cid),
      funcDelays(1, SC_ZERO_TIME),
      funcLatencies(1, SC_ZERO_TIME)
  {
    setBaseDelay(SC_ZERO_TIME);
    setBaseLatency(SC_ZERO_TIME);
  }

  void ProcessControlBlock::ComponentDelay::addDelay( FunctionId fid,
                                                      sc_time delay ){
    if( fid >= funcDelays.size()){
      funcDelays.resize( fid + 100, SC_ZERO_TIME );
    }
    this->funcDelays[fid] = delay;
  }

  void ProcessControlBlock::ComponentDelay::setBaseDelay( sc_time delay ){
    this->funcDelays[defaultFunctionId] = delay;
  }

  sc_time ProcessControlBlock::ComponentDelay::getBaseDelay( ) const {
    return this->funcDelays[defaultFunctionId];
  }

  sc_time ProcessControlBlock::ComponentDelay::getDelay(
    FunctionId fid) const
  {
    assert(fid < funcDelays.size());
    sc_time ret = funcDelays[fid];
    return ret;
  }

  void ProcessControlBlock::ComponentDelay::addLatency( FunctionId fid,
                                                        sc_time latency ){
    if( fid >= funcLatencies.size())
      funcLatencies.resize( fid + 100, SC_ZERO_TIME );

    this->funcLatencies[fid] = latency;
  }

  void ProcessControlBlock::ComponentDelay::setBaseLatency( sc_time latency ){
    this->funcLatencies[defaultFunctionId] = latency;
  }

  sc_time ProcessControlBlock::ComponentDelay::getBaseLatency( ) const {
    return this->funcLatencies[defaultFunctionId];
  }

  sc_time ProcessControlBlock::ComponentDelay::getLatency(
    FunctionId fid) const
  {
    assert(fid < funcLatencies.size());
    sc_time ret = funcLatencies[fid];
    return ret;
  }

  DelayMapper::~DelayMapper(){}

  DelayMapper::DelayMapper(const DelayMapper & dm)
    : functionIdMap(dm.functionIdMap),
      globalFunctionId(dm.globalFunctionId),
      compDelays(dm.compDelays) {}

  DelayMapper::DelayMapper()
    : functionIdMap(),
      globalFunctionId( defaultFunctionId + 1 ),
      compDelays() {}

  void DelayMapper::addFuncDelay( Director* director,
                                  std::string comp,
                                  const char* funcname,
                                  sc_time delay ){
    
#ifdef VPC_DEBUG
    std::cerr << "DelayMapper> Adding Function Delay for " << comp;
    if(funcname != NULL){
      std::cerr << " function " << funcname;
    }
    std::cerr << " delay " << delay << std::endl;
#endif //VPC_DEBUG

    ComponentId cid = director->getComponentId(comp);

    if(cid >= compDelays.size()){
      compDelays.resize( cid + 100, NULL );
    }

    if( this->compDelays[cid] == NULL ){
      this->compDelays[cid] = new ComponentDelay(cid);
    }

    ComponentDelay *cd = this->compDelays[cid];

    if(funcname != NULL){
      FunctionId  fid = this->createFunctionId(funcname);
      cd->addDelay(fid, delay);
    } else {
      cd->setBaseDelay(delay);
    }
  }

  sc_time DelayMapper::getFuncDelay( ComponentId cid,
                                     FunctionId  fid ) const
  {
    ComponentDelay * cd = this->compDelays[cid];

    assert( cd != NULL );
    sc_time ret = cd->getDelay(fid);

#ifdef VPC_DEBUG
    std::cerr << "DelayMapper> Delay for " << cid;
    std::cerr << "->" << fid;
    std::cerr << " is " << ret << std::endl;
#endif //VPC_DEBUG 
    
    return ret;
  }

  void DelayMapper::addFuncLatency( Director* director,
                                    std::string comp,
                                    const char* funcname,
                                    sc_time latency ){
    
#ifdef VPC_DEBUG
    std::cerr << "DelayMapper> Adding Function Latency for -" << comp << "-";
    if(funcname != NULL){
      std::cerr << " function " << funcname;
    }
    std::cerr << " latency " << latency << std::endl;
#endif //VPC_DEBUG

    ComponentId cid = director->getComponentId(comp);

    if(cid >= compDelays.size()){
      compDelays.resize( cid + 100, NULL );
    }

    if( this->compDelays[cid] == NULL ){
      this->compDelays[cid] = new ComponentDelay(cid);
    }

    ComponentDelay *cd = this->compDelays[cid];

    if(funcname != NULL){
      FunctionId  fid = this->createFunctionId(funcname);
      cd->addLatency(fid, latency);
    } else {
      cd->setBaseLatency(latency);
    }

  }

  sc_time DelayMapper::getFuncLatency( ComponentId cid,
                                       FunctionId  fid ) const
  {
    ComponentDelay * cd = this->compDelays[cid];

    assert( cd != NULL );
    sc_time ret = cd->getDelay(fid);

#ifdef VPC_DEBUG
    std::cerr << "DelayMapper> Latency for " << cid;
    std::cerr << "->" << fid;
    std::cerr << " is " << ret << std::endl;
#endif //VPC_DEBUG 

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
  
  
  ProcessControlBlock::ProcessControlBlock(): name("NN"){
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
    : DelayMapper(pcb){

    this->setName(pcb.getName());
    this->setDeadline(pcb.getDeadline());
    this->setPeriod(pcb.getPeriod());
    this->instanceId = ProcessControlBlock::globalInstanceId++;
    this->setPriority(pcb.getPriority());
    
   
    this->blockEvent = EventPair();
    this->setDelay(SC_ZERO_TIME);
    this->setLatency(SC_ZERO_TIME);
    this->setInterrupt(NULL);
    this->setRemainingDelay(SC_ZERO_TIME);
    this->setTraceSignal(NULL);

    this->activationCount = pcb.activationCount;

    this->setPid(pcb.getPid());

    // remember amount of copies for later clean up 
    this->copyCount = pcb.copyCount;
    (*(this->copyCount))++;
  }

  void ProcessControlBlock::init(){

    this->blockEvent = EventPair();
    this->deadline = sc_time(DBL_MAX, SC_SEC);
    this->delay = SC_ZERO_TIME;
    this->latency = SC_ZERO_TIME;
    this->interrupt = NULL;
    this->remainingDelay = SC_ZERO_TIME;
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

  void ProcessControlBlock::setInterrupt(sc_event* interrupt){
    this->interrupt = interrupt;
  }

  sc_event* ProcessControlBlock::getInterrupt() const{
    return this->interrupt;
  }

  void ProcessControlBlock::setBlockEvent(EventPair blockEvent){
    this->blockEvent = blockEvent;
  }

  EventPair ProcessControlBlock::getBlockEvent() const{
    return this->blockEvent;
  }

  void ProcessControlBlock::setDelay(sc_time delay){
    this->delay = delay;
  }

  sc_time ProcessControlBlock::getDelay() const{
    return this->delay;
  }

  void ProcessControlBlock::setLatency(sc_time latency){
    this->latency = latency;
  }

  sc_time ProcessControlBlock::getLatency() const{
    return this->latency;
  }

  void ProcessControlBlock::setRemainingDelay(sc_time delay){
    this->remainingDelay = delay;
  }

  sc_time ProcessControlBlock::getRemainingDelay() const{
    return this->remainingDelay;
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

  void ProcessControlBlock::setState(activation_state state){
    this->state = state;
  }

  activation_state ProcessControlBlock::getState() const{
    return this->state;
  }

  void ProcessControlBlock::setTraceSignal(Tracing* signal){
    this->traceSignal = signal;
  }

  Tracing* ProcessControlBlock::getTraceSignal(){
    return this->traceSignal;
  }

  FunctionId DelayMapper::createFunctionId(std::string function) {
    FunctionIdMap::const_iterator iter = functionIdMap.find(function);
    if( iter == functionIdMap.end() ) {
      functionIdMap[function] = this->uniqueFunctionId();
    }
    iter = functionIdMap.find(function);
    return iter->second;
  }

  FunctionId DelayMapper::getFunctionId(std::string function) {
    FunctionIdMap::const_iterator iter = functionIdMap.find(function);

    // the function name was not set in configuration
    // -> we have to use default delay
    if( iter == functionIdMap.end() ) {
      return defaultFunctionId;
    }
    return iter->second;
    
  }

  FunctionId DelayMapper::uniqueFunctionId() {
    return globalFunctionId++;
  }

  const FunctionId DelayMapper::defaultFunctionId = 0;

}
