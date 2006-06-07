#include "hscd_vpc_EDFConfScheduler.h"

namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of EDFConfScheduler
   */
  EDFConfScheduler::EDFConfScheduler(AbstractController* controller, MIMapper* miMapper) 
    : ConfigurationScheduler(controller, miMapper), 
      order_count(0) {}

  /**
   * \brief Deletes instance of EDFConfScheduler
   */
  EDFConfScheduler::~EDFConfScheduler(){}
    
  /**
   * \brief Implementation of EDFConfScheduler::addTasksToSchedule
   */
  void EDFConfScheduler::addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc){

#ifdef VPC_DEBUG
    std::cerr << YELLOW("EDFConfScheduler "<< this->getController().getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

    this->tasksToProcess.push(newTask);
    // set deadline of task to earliest of possible deadlines
    newTask->setDeadline(this->getEarliestDeadline(newTask, rc));
       
    std::list<EDFListElement<unsigned int > >::iterator iter;
    iter = std::find(this->nextConfigurations.begin(), this->nextConfigurations.end(), config);
    // if configuration is not in scheduling list add it
    if(iter == this->nextConfigurations.end()){
      this->nextConfigurations.push_back(EDFListElement<unsigned int>(config, newTask->getDeadline(), order_count++));
    }else{
      iter->addDeadline(newTask->getDeadline());
    }
  }

  /**
   * \brief Implementation of EDFConfScheduler::performSchedule
   */
  void EDFConfScheduler::performSchedule(ReconfigurableComponent* rc){
    
    //finally sort EDF list
    this->nextConfigurations.sort();
    
  }
  
  /*
   * \brief Implementation of EDFConfScheduler::getNextConfiguration
   */  
  unsigned int EDFConfScheduler::getNextConfiguration(ReconfigurableComponent* rc){
    
    if(!this->nextConfigurations.empty()){
      unsigned int next = this->nextConfigurations.front().getContained();
    
      if(this->getManagedComponent()->getActivConfiguration() == NULL 
          || next != this->getManagedComponent()->getActivConfiguration()->getID()){

#ifdef VPC_DEBUG
        std::cerr << YELLOW("EDFConfScheduler " << this->getController().getName() << "> next config to load: "
              << next) << std::endl;
#endif //VPC_DEBUG

        return next;
      }
    }
    return 0;
  }
  
  /**
   * \brief Implementation of EDFConfScheduler::hasTaskToProcess()
   */
  bool EDFConfScheduler::hasTaskToProcess(ReconfigurableComponent* rc){
   
     return (this->tasksToProcess.size() > 0);
  
  }
  
  /**
   * \brief Implementation of EDFConfScheduler::getNextTask()
   */
  ProcessControlBlock* EDFConfScheduler::getNextTask(ReconfigurableComponent* rc){
     
     ProcessControlBlock* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of EDFConfScheduler::signalTaskEvent
   */
  void EDFConfScheduler::signalTaskEvent(ProcessControlBlock* pcb, std::string compID){

#ifdef VPC_DEBUG
    std::cerr << YELLOW("EDFConfScheduler " << this->getController().getName() << "> got notified by task: " << pcb->getName()) << std::endl;
#endif //VPC_DEBUG

    //get made decision for pcb
    Decision d = this->getController().getDecision(pcb->getPID(), this->getManagedComponent());

    // if there are still tasks running on configuration equal to (EDF != -1)
    std::list<EDFListElement<unsigned int> >::iterator iter;
    iter = std::find(this->nextConfigurations.begin(), this->nextConfigurations.end(), d.conf);

    if(iter != this->nextConfigurations.end()){

      iter->removeDeadline(pcb->getDeadline());

#ifdef VPC_DEBUG
      std::cerr << YELLOW("EDFConfScheduler " << this->getController().getName() << "> deadline of mapped configuration after change is: "
          << iter->getDeadline()) << std::endl;
#endif //VPC_DEBUG

      iter->removeDeadline(this->getEarliestDeadline(pcb, this->getManagedComponent()));

      if(iter->getDeadline() != -1){
        this->nextConfigurations.sort();
      }else{
        // remove configuration from EDF list
        this->nextConfigurations.erase(iter);
      }
    }

    // if there are still waiting configurations and actual scheduled isnt determined one its time to wakeUp ReconfigurableComponent
    if(this->nextConfigurations.size() > 0 &&
        this->getManagedComponent()->getActivConfiguration()->getID() != this->getNextConfiguration(this->getManagedComponent())){

#ifdef VPC_DEBUG
      std::cerr << "EDFConfScheduler> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }

  /**
   * \brief Implementation of EDFConfScheduler::getEarliestDeadline
   */
  double EDFConfScheduler::getEarliestDeadline(ProcessControlBlock* pcb, ReconfigurableComponent* rc){
    // first of all get controller to retrieve decision for task
    AbstractController& ctrl = this->getController();
    Decision d = ctrl.getDecision(pcb->getPID(), rc);
    // next access mapping information for made decision
    MIMapper& miMapper = this->getMIMapper();
    MappingInformationIterator* iter = miMapper.getMappingInformationIterator(pcb->getName(), d.comp);
    // now iteratate over possibilities and choose appropriate one
    double deadline = DBL_MAX;
    
    while(iter->hasNext()){
      MappingInformation* mi = iter->getNext();
      if(mi->getDeadline() < deadline){
        deadline = mi->getDeadline();
      }
    }
    
    //free iterator
    delete iter;
    return deadline;
  }
  
} //namespace SystemC_VPC
