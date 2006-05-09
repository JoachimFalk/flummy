#include <hscd_vpc_PriorityController.h>

namespace SystemC_VPC{

  /**
   * \brief Initializes instance of PriorityController
   */
  PriorityController::PriorityController(AbstractController* controller, MIMapper* miMapper) 
    : ConfigurationScheduler(controller, miMapper),
      order_count(0) {}

  PriorityController::~PriorityController() {}

  /**
   * \brief Implementation of PriorityController::addTasksToSchedule
   */
  void PriorityController::addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config){

#ifdef VPC_DEBUG
    std::cerr << YELLOW("PriorityController "<< this->getController().getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

    this->tasksToProcess.push(newTask);
    // set priority of task to highest possible at this level of hierarchy
    newTask->setPriority(this->getHighestPriority(newTask->getPID()));
    
    std::list<PriorityListElement<unsigned int> >::iterator iter;

    iter = std::find(this->nextConfigurations.begin(), this->nextConfigurations.end(), config);
    // if configuration is not in scheduling list add it
    if(iter == this->nextConfigurations.end()){
      this->nextConfigurations.push_back(PriorityListElement<unsigned int>(config, newTask->getPriority(), order_count++));
    }else{
      iter->addPriority(newTask->getPriority());
    }
  }

  /**
   * \brief Implementation of PriorityController::performSchedule
   */
  void PriorityController::performSchedule(){

    this->nextConfigurations.sort();

  }

  /*
   * \brief Implementation of PriorityController::getNextConfiguration
   */  
  unsigned int PriorityController::getNextConfiguration(){

    if(this->nextConfigurations.size()){
      unsigned int next = this->nextConfigurations.front().getContained();

      if(this->getManagedComponent()->getActivConfiguration() == NULL
          || next != this->getManagedComponent()->getActivConfiguration()->getID()){

#ifdef VPC_DEBUG
        std::cerr << YELLOW("PriorityController " << this->getController().getName() << "> next config to load: "
            << next) << std::endl;
#endif //VPC_DEBUG

        return next;
      }
    }
    return 0;
  }

  /**
   * \brief Implementation of PriorityController::hasTaskToProcess()
   */
  bool PriorityController::hasTaskToProcess(){

    return (this->tasksToProcess.size() > 0);

  }

  /**
   * \brief Implementation of PriorityController::getNextTask()
   */
  ProcessControlBlock* PriorityController::getNextTask(){

    ProcessControlBlock* task;
    task = this->tasksToProcess.front();
    this->tasksToProcess.pop();
    return task;

  }

  /**
   * \brief Implementation of PriorityController::signalTaskEvent
   */
  void PriorityController::signalTaskEvent(ProcessControlBlock* pcb){

#ifdef VPC_DEBUG
    std::cerr << YELLOW("PriorityController " << this->getController().getName() << "> got notified by task: " << pcb->getName()) << std::endl;
#endif //VPC_DEBUG

    //get mapped configuration
    Decision d = this->getController().getDecision(pcb->getPID());
    std::list<PriorityListElement<unsigned int> >::iterator iter;
    iter = std::find(this->nextConfigurations.begin(), this->nextConfigurations.end(), d.conf);
    // if configuration is in scheduling list process it
    if(iter != this->nextConfigurations.end()){
      iter->removePriority(pcb->getPriority());

#ifdef VPC_DEBUG
      std::cerr << YELLOW("PriorityController " << this->getController().getName() << "> priority of mapped configuration after change is: "
          << iter->getPriority()) << std::endl;
#endif //VPC_DEBUG

      // if no more task on configuration
      if(iter->getPriority() == -1){
        this->nextConfigurations.erase(iter);
      }else{
        this->nextConfigurations.sort(); 
      } 
    }

    // if there are still ready configurations and current activ does not fit selected its time to wakeUp ReconfigurableComponent
    if(this->nextConfigurations.size() > 0
        && this->getManagedComponent()->getActivConfiguration()->getID() != this->getNextConfiguration()){

#ifdef VPC_DEBUG
      std::cerr << "Controller " << this->getController().getName() << "> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }

  unsigned int PriorityController::getHighestPriority(int pid){
    // first of all get Controller to retrieve Decision
    AbstractController& ctrl = this->getController();
    Decision d = ctrl.getDecision(pid);
    // next access binding possibilites of selected comp
    MappingInformationIterator* iter = this->getMIMapper().getMappingInformationIterator(d.comp);
    int priority = INT_MAX;
    while(iter->hasNext()){
      MappingInformation* mi = iter->getNext();
      if(mi->getPriority() < priority){
        priority = mi->getPriority();
      }
    }
    
    //free iterator
    delete iter;
    return priority;
  }
  
} //namespace SystemC_VPC
