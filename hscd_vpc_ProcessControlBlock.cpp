#include "hscd_vpc_ProcessControlBlock.h"

namespace SystemC_VPC{

  int ProcessControlBlock::global_pid = 0;

  ProcessControlBlock::ComponentDelay::ComponentDelay(std::string name, double base_delay) : name(name), base_delay(base_delay) {}

  void ProcessControlBlock::ComponentDelay::addDelay(const char* funcname, double delay){
    if(funcname != NULL){
      std::string key(funcname, strlen(funcname));
      this->funcDelays[key] = delay;
    }
  }

  double ProcessControlBlock::ComponentDelay::getDelay(const char* funcname){
    if(funcname == NULL){
      return this->base_delay;
    }else{
      std::map<std::string, double>::iterator iter;
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

  ProcessControlBlock::DelayMapper::~DelayMapper(){

    std::map<std::string, ComponentDelay*>::iterator iter;
    for(iter = this->compDelays.begin(); iter != this->compDelays.end(); iter++){
      delete iter->second;
    }
    this->compDelays.clear();

  }

  void ProcessControlBlock::DelayMapper::registerDelay(std::string comp, double delay){

    std::map<std::string, ComponentDelay*>::iterator iter;
    iter = this->compDelays.find(comp);
    if(iter == this->compDelays.end()){
      this->compDelays[comp] = new ComponentDelay(comp, delay);
    }

  }  

  void ProcessControlBlock::DelayMapper::addDelay(std::string comp, const char* funcname, double delay){
    
    std::map<std::string, ComponentDelay*>::iterator iter;
    iter = this->compDelays.find(comp);
    if(iter == this->compDelays.end()){
      this->registerDelay(comp, delay);
      iter = this->compDelays.find(comp);
    }
    iter->second->addDelay(funcname, delay);

  }

  double ProcessControlBlock::DelayMapper::getDelay(std::string comp, const char* funcname){

    std::map<std::string, ComponentDelay* >::iterator iter;
    iter = this->compDelays.find(comp);
    if(iter != compDelays.end()){
      return iter->second->getDelay(funcname);
    }else{
      return 0;
    }

  }

  bool ProcessControlBlock::DelayMapper::hasDelay(std::string comp, const char* funcname){

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


  ProcessControlBlock::ActivationCounter::ActivationCounter() : activation_count(0){}

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
    }else{
      *(this->copyCount)--;
    }
  }
  
  ProcessControlBlock::ProcessControlBlock(const ProcessControlBlock& pcb){

    this->setName(pcb.getName());
    this->setDeadline(pcb.getDeadline());
    this->setPeriod(pcb.getPeriod());
    this->pid = ProcessControlBlock::global_pid++;
    this->setPriority(pcb.getPriority());
    
   
    this->blockEvent = EventPair();
    this->setDelay(0);
    this->setFuncName(NULL);
    this->setInterrupt(NULL);
    this->setRemainingDelay(0);
    this->setTraceSignal(NULL);

    this->activationCount = pcb.activationCount;
    this->dmapper = pcb.dmapper;

    // remember amount of copies for later clean up 
    this->copyCount = pcb.copyCount;
    *(this->copyCount)++; 
  }

  void ProcessControlBlock::init(){

    this->blockEvent = EventPair();
    this->deadline = DBL_MAX;
    this->delay = 0;
    this->funcname = NULL;
    this->interrupt = NULL;
    this->remainingDelay = 0;
    this->period = DBL_MAX;
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

  void ProcessControlBlock::setDelay(double delay){
    if(delay > 0){
      this->delay = delay;
    }else{
      this->delay = 0;
    }
  }

  double ProcessControlBlock::getDelay() const{
    return this->delay;
  }

  void ProcessControlBlock::setRemainingDelay(double delay){
    if(delay > 0){
      this->remainingDelay = delay;
    }else{
      this->remainingDelay = 0;
    }
  }

  double ProcessControlBlock::getRemainingDelay() const{
    return this->remainingDelay;
  }

  void ProcessControlBlock::setPeriod(double period){
    this->period = period;
  }

  double ProcessControlBlock::getPeriod() const{
    return this->period;
  }

  void ProcessControlBlock::setPriority(int priority){
    this->priority = priority;
  }

  int ProcessControlBlock::getPriority() const{
    return this->priority;
  }

  void ProcessControlBlock::setDeadline(double deadline){
    if(deadline > 0){
      this->deadline = deadline;
    }else{
      this->deadline = 0;
    }
  }

  double ProcessControlBlock::getDeadline() const{
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

  void ProcessControlBlock::setTraceSignal(sc_signal<trace_value>* signal){
    this->traceSignal = signal;
  }

  sc_signal<trace_value>* ProcessControlBlock::getTraceSignal(){
    return this->traceSignal;
  }

  void ProcessControlBlock::addFuncDelay(std::string comp, const char* funcname, double delay){

#ifdef VPC_DEBUG
    std::cerr << "ProcessControlBlock> Adding Function Delay for " << comp;
    if(funcname != NULL){
      std::cerr << " function " << funcname;
    }
    std::cerr << " delay " << delay << std::endl;
#endif //VPC_DEBUG
    
    this->dmapper->addDelay(comp, funcname, delay);
  }

  double ProcessControlBlock::getFuncDelay(std::string comp, const char* funcname) const{

#ifdef VPC_DEBUG
    std::cerr << "ProcessControlBlock> Delay for " << comp;
    if(funcname != NULL){
      std::cerr << "->" << funcname;
    }
    std::cerr << " is " << this->dmapper->getDelay(comp, funcname) << std::endl;
#endif //VPC_DEBUG 
    
    return this->dmapper->getDelay(comp, funcname);
  }

  bool ProcessControlBlock::hasDelay(std::string comp, const char* funcname) const{

#ifdef VPC_DEBUG 
    std::cerr << "ProcessControlBlock> Request for " << comp;
    if(funcname != NULL){
      std::cerr << " with function " << funcname;
    }
    std::cerr << " has special delay ? " << this->dmapper->hasDelay(comp, funcname) << std::endl;
#endif //VPC_DEBUG
    
    return this->dmapper->hasDelay(comp, funcname);
  }

}
