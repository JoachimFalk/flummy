#include "hscd_vpc_ProcessControlBlock.h"

#include "hscd_vpc_MappingInformation.h"
#include "hscd_vpc_datatypes.h"

namespace SystemC_VPC{

  int ProcessControlBlock::global_pid = 0;

  /**
   * SECTION ActivationCounter
   */
    
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

      std::set<MappingInformation* >::iterator iter;
      for(iter = this->mInfos->begin(); iter != this->mInfos->end(); iter++){
        delete *iter;
      }
      
      delete this->mInfos;
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
    
   
    this->blockEvent = NULL;
    this->setDelay(0);
    this->setFuncName(NULL);
    this->setInterrupt(NULL);
    this->setRemainingDelay(0);
    this->setTraceSignal(NULL);

    this->activationCount = pcb.activationCount;
    this->mInfos = pcb.mInfos;

    // remember amount of copies for later clean up 
    this->copyCount = pcb.copyCount;
    *(this->copyCount)++; 
  }

  void ProcessControlBlock::init(){

    this->blockEvent = NULL;
    this->deadline = DBL_MAX;
    this->delay = 0;
    this->funcname = NULL;
    this->interrupt = NULL;
    this->remainingDelay = 0;
    this->period = DBL_MAX;
    this->pid = ProcessControlBlock::global_pid++;
    this->priority = INT_MAX;
    this->traceSignal = NULL;


    this->activationCount = new ActivationCounter();
    this->mInfos = new std::set<MappingInformation* >();
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

  void ProcessControlBlock::setBlockEvent(CoSupport::SystemC::Event* blockEvent){
    this->blockEvent = blockEvent;
  }

  CoSupport::SystemC::Event* ProcessControlBlock::getBlockEvent() const{
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
 
  void ProcessControlBlock::addMappingInformation(MappingInformation* mInfo){
    this->mInfos->insert(mInfo);
  }
}
