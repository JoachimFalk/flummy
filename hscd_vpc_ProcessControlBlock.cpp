#include "hscd_vpc_ProcessControlBlock.h"

namespace SystemC_VPC{

  int ProcessControlBlock::global_pid = 0;

  ProcessControlBlock::ComponentDelay::ComponentDelay(std::string name)
    : name(name), base_delay(SC_ZERO_TIME), base_latency(SC_ZERO_TIME) {}

  void ProcessControlBlock::ComponentDelay::addDelay(const char* funcname,
                                                     sc_time delay){
    if(funcname != NULL){
      std::string key(funcname, strlen(funcname));
      this->funcDelays[key] = delay;
    }else{
      this->base_delay = delay;
    }

  }

  sc_time ProcessControlBlock::ComponentDelay::getDelay(const char* funcname){
    if(funcname == NULL){
      return this->base_delay;
    }else{
      std::map<std::string, sc_time>::iterator iter;
      std::string key(funcname, strlen(funcname));
      iter = this->funcDelays.find(key);
      if(iter != this->funcDelays.end()){
        return iter->second;
      }
    }

    return this->base_delay;
  }

  bool ProcessControlBlock::ComponentDelay::hasDelay(const char* funcname){
    if(funcname == NULL){
      return false;
    }else{
      std::string key(funcname, strlen(funcname));
      return (this->funcDelays.count(key) > 0);
    }
  }

  void ProcessControlBlock::ComponentDelay::addLatency(const char* funcname,
                                                       sc_time latency){
    if(funcname != NULL){
      std::string key(funcname, strlen(funcname));
      this->funcLatencies[key] = latency;
    }else{
      this->base_latency = latency;
    }
  }

  sc_time ProcessControlBlock::ComponentDelay::getLatency(
    const char* funcname)
  {
    if(funcname == NULL){
      return this->base_latency;
    }else{
      std::map<std::string, sc_time>::iterator iter;
      std::string key(funcname, strlen(funcname));
      iter = this->funcLatencies.find(key);
      if(iter != this->funcLatencies.end()){
        return iter->second;
      }
    }

    return this->base_latency;
  }

  bool ProcessControlBlock::ComponentDelay::hasLatency(const char* funcname){
    if(funcname == NULL){
      return false;
    }else{
      std::string key(funcname, strlen(funcname));
      return (this->funcLatencies.count(key) > 0);
    }
  }

  ProcessControlBlock::DelayMapper::~DelayMapper(){

    std::map<std::string, ComponentDelay*>::iterator iter;
    for(iter = this->compDelays.begin();
        iter != this->compDelays.end();
        ++iter){
      delete iter->second;
    }
    this->compDelays.clear();
  
  }

  void ProcessControlBlock::DelayMapper::addDelay(std::string comp,
                                                  const char* funcname,
                                                  sc_time delay){
    
    std::map<std::string, ComponentDelay*>::iterator iter;
    iter = this->compDelays.find(comp);
    if(iter == this->compDelays.end()){
      this->compDelays[comp] = new ComponentDelay(comp);
      iter = this->compDelays.find(comp);
    }
    iter->second->addDelay(funcname, delay);

  }

  sc_time ProcessControlBlock::DelayMapper::getDelay(std::string comp,
                                                     const char* funcname){

    std::map<std::string, ComponentDelay* >::iterator iter;
    iter = this->compDelays.find(comp);
    if(iter != compDelays.end()){
      return iter->second->getDelay(funcname);
    }else{
      return SC_ZERO_TIME;
    }

  }

  bool ProcessControlBlock::DelayMapper::hasDelay(std::string comp,
                                                  const char* funcname){

    if(this->compDelays.count(comp) > 0){
      
      if(funcname != NULL){
        std::map<std::string, ComponentDelay* >::iterator iter;
        iter = this->compDelays.find(comp);
        if(iter != this->compDelays.end()){
          return iter->second->hasDelay(funcname);
        }            
      }else{
        return true;
      }
    }
    return false;
  }

  void ProcessControlBlock::DelayMapper::addLatency(std::string comp,
                                                    const char* funcname,
                                                    sc_time latency){
    
    std::map<std::string, ComponentDelay*>::iterator iter;
    iter = this->compDelays.find(comp);
    if(iter == this->compDelays.end()){
      this->compDelays[comp] = new ComponentDelay(comp);
      iter = this->compDelays.find(comp);
    }
    iter->second->addLatency(funcname, latency);

  }

  sc_time ProcessControlBlock::DelayMapper::getLatency(std::string comp,
                                                       const char* funcname){

    std::map<std::string, ComponentDelay* >::iterator iter;
    iter = this->compDelays.find(comp);
    if(iter != compDelays.end()){
      return iter->second->getLatency(funcname);
    }else{
      return SC_ZERO_TIME;
    }

  }

  bool ProcessControlBlock::DelayMapper::hasLatency(std::string comp,
                                                    const char* funcname){

    if(this->compDelays.count(comp) > 0){
      
      if(funcname != NULL){
        std::map<std::string, ComponentDelay* >::iterator iter;
        iter = this->compDelays.find(comp);
        if(iter != this->compDelays.end()){
          return iter->second->hasLatency(funcname);
        }            
      }else{
        return true;
      }
    }
    return false;
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
      delete this->dmapper;
      delete this->copyCount; 
    }else{
      (*(this->copyCount))--;
    }
  }
  
  ProcessControlBlock::ProcessControlBlock(const ProcessControlBlock& pcb){

    this->setName(pcb.getName());
    this->setDeadline(pcb.getDeadline());
    this->setPeriod(pcb.getPeriod());
    this->pid = ProcessControlBlock::global_pid++;
    this->setPriority(pcb.getPriority());
    
   
    this->blockEvent = EventPair();
    this->setDelay(SC_ZERO_TIME);
    this->setLatency(SC_ZERO_TIME);
    this->setFuncName(NULL);
    this->setInterrupt(NULL);
    this->setRemainingDelay(SC_ZERO_TIME);
    this->setTraceSignal(NULL);

    this->activationCount = pcb.activationCount;
    this->dmapper = pcb.dmapper;

    // remember amount of copies for later clean up 
    this->copyCount = pcb.copyCount;
    (*(this->copyCount))++;
  }

  void ProcessControlBlock::init(){

    this->blockEvent = EventPair();
    this->deadline = sc_time(DBL_MAX, SC_SEC);
    this->delay = SC_ZERO_TIME;
    this->latency = SC_ZERO_TIME;
    this->funcname = NULL;
    this->interrupt = NULL;
    this->remainingDelay = SC_ZERO_TIME;
    this->period = sc_time(DBL_MAX, SC_SEC);
    this->pid = ProcessControlBlock::global_pid++;
    this->priority = 0;
    this->traceSignal = NULL;


    this->activationCount = new ActivationCounter();
    this->dmapper = new DelayMapper();

    this->copyCount = new int(0);
  }

  void ProcessControlBlock::setName(std::string name){
    this->name = name;
  }

  std::string const& ProcessControlBlock::getName() const{
    return this->name;
  }

  void ProcessControlBlock::setPID(int pid){
    this->pid = pid;
  }

  int ProcessControlBlock::getPID() const{
    return pid;
  }

  void ProcessControlBlock::setFuncName(const char* funcname){
    this->funcname = funcname;
  }

  const char* ProcessControlBlock::getFuncName() const{
    return this->funcname;
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

  void ProcessControlBlock::addFuncDelay(std::string comp,
                                         const char* funcname,
                                         sc_time delay){

#ifdef VPC_DEBUG
    std::cerr << "ProcessControlBlock> Adding Function Delay for " << comp;
    if(funcname != NULL){
      std::cerr << " function " << funcname;
    }
    std::cerr << " delay " << delay << std::endl;
#endif //VPC_DEBUG
    
    this->dmapper->addDelay(comp, funcname, delay);
  }

  sc_time ProcessControlBlock::getFuncDelay(std::string comp,
                                            const char* funcname) const{

#ifdef VPC_DEBUG
    std::cerr << "ProcessControlBlock> Delay for " << comp;
    if(funcname != NULL){
      std::cerr << "->" << funcname;
    }
    std::cerr << " is " << this->dmapper->getDelay(comp, funcname)
              << std::endl;
#endif //VPC_DEBUG 
    
    return this->dmapper->getDelay(comp, funcname);
  }

  bool ProcessControlBlock::hasDelay(std::string comp,
                                     const char* funcname) const{

#ifdef VPC_DEBUG 
    std::cerr << "ProcessControlBlock> Request for " << comp;
    if(funcname != NULL){
      std::cerr << " with function " << funcname;
    }
    std::cerr << " has special delay ? "
              << this->dmapper->hasDelay(comp, funcname) << std::endl;
#endif //VPC_DEBUG
    
    return this->dmapper->hasDelay(comp, funcname);
  }

  void ProcessControlBlock::addFuncLatency(std::string comp,
                                           const char* funcname,
                                           sc_time delay){

#ifdef VPC_DEBUG
    std::cerr << "ProcessControlBlock> Adding Function Latency for " << comp;
    if(funcname != NULL){
      std::cerr << " function " << funcname;
    }
    std::cerr << " delay " << delay << std::endl;
#endif //VPC_DEBUG
    
    this->dmapper->addLatency(comp, funcname, delay);
  }

  sc_time ProcessControlBlock::getFuncLatency(std::string comp,
                                              const char* funcname) const{

#ifdef VPC_DEBUG
    std::cerr << "ProcessControlBlock> Latency for " << comp;
    if(funcname != NULL){
      std::cerr << "->" << funcname;
    }
    std::cerr << " is " << this->dmapper->getLatency(comp, funcname)
              << std::endl;
#endif //VPC_DEBUG 
    
    return this->dmapper->getLatency(comp, funcname);
  }

  bool ProcessControlBlock::hasLatency(std::string comp,
                                       const char* funcname) const{

#ifdef VPC_DEBUG 
    std::cerr << "ProcessControlBlock> Request for " << comp;
    if(funcname != NULL){
      std::cerr << " with function " << funcname;
    }
    std::cerr << " has special latency ? "
              << this->dmapper->hasLatency(comp, funcname) << std::endl;
#endif //VPC_DEBUG
    
    return this->dmapper->hasLatency(comp, funcname);
  }

}
