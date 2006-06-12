#include <hscd_vpc_PriorityConfScheduler.h>

#include "hscd_vpc_BindingGraph.h"

namespace SystemC_VPC{

  /**
   * \brief Initializes instance of PriorityConfScheduler
   */
  PriorityConfScheduler::PriorityConfScheduler(AbstractController* controller) 
    : ConfigurationScheduler(controller),
      order_count(0) {}

  PriorityConfScheduler::~PriorityConfScheduler() {}

  /**
   * \brief Implementation of PriorityConfScheduler::addTasksToSchedule
   */
  void PriorityConfScheduler::addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc){

#ifdef VPC_DEBUG
    std::cerr << VPC_YELLOW("PriorityConfScheduler "<< this->getController().getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

    this->tasksToProcess.push(newTask);
    // set priority of task to highest possible at this level of hierarchy
    newTask->setPriority(this->getHighestPriority(newTask, rc));
    
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
   * \brief Implementation of PriorityConfScheduler::performSchedule
   */
  void PriorityConfScheduler::performSchedule(ReconfigurableComponent* rc){

    this->nextConfigurations.sort();

  }

  /*
   * \brief Implementation of PriorityConfScheduler::getNextConfiguration
   */  
  unsigned int PriorityConfScheduler::getNextConfiguration(ReconfigurableComponent* rc){

    if(this->nextConfigurations.size()){
      unsigned int next = this->nextConfigurations.front().getContained();

      if(this->getManagedComponent()->getActivConfiguration() == NULL
          || next != this->getManagedComponent()->getActivConfiguration()->getID()){

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("PriorityConfScheduler " << this->getController().getName() << "> next config to load: "
            << next) << std::endl;
#endif //VPC_DEBUG

        return next;
      }
    }
    return 0;
  }

  /**
   * \brief Implementation of PriorityConfScheduler::hasTaskToProcess()
   */
  bool PriorityConfScheduler::hasTaskToProcess(ReconfigurableComponent* rc){

    return (this->tasksToProcess.size() > 0);

  }

  /**
   * \brief Implementation of PriorityConfScheduler::getNextTask()
   */
  ProcessControlBlock* PriorityConfScheduler::getNextTask(ReconfigurableComponent* rc){

    ProcessControlBlock* task;
    task = this->tasksToProcess.front();
    this->tasksToProcess.pop();
    return task;

  }

  /**
   * \brief Implementation of PriorityConfScheduler::signalTaskEvent
   */
  void PriorityConfScheduler::signalTaskEvent(ProcessControlBlock* pcb, std::string compID){

#ifdef VPC_DEBUG
    std::cerr << VPC_YELLOW("PriorityConfScheduler " << this->getController().getName() << "> got notified by task: " << pcb->getName()) << std::endl;
#endif //VPC_DEBUG

    //get mapped configuration
    Decision d = this->getController().getDecision(pcb->getPID(), this->getManagedComponent());
    std::list<PriorityListElement<unsigned int> >::iterator iter;
    iter = std::find(this->nextConfigurations.begin(), this->nextConfigurations.end(), d.conf);
    // if configuration is in scheduling list process it
    if(iter != this->nextConfigurations.end()){
      iter->removePriority(this->getHighestPriority(pcb, this->getManagedComponent()));

#ifdef VPC_DEBUG
      std::cerr << VPC_YELLOW("PriorityConfScheduler " << this->getController().getName() << "> priority of mapped configuration after change is: "
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
        && this->getManagedComponent()->getActivConfiguration()->getID() != this->getNextConfiguration(this->getManagedComponent())){

#ifdef VPC_DEBUG
      std::cerr << "Controller " << this->getController().getName() << "> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }

  unsigned int PriorityConfScheduler::getHighestPriority(ProcessControlBlock* pcb, ReconfigurableComponent* rc){
    // first of all get Controller to retrieve Decision
    AbstractController& ctrl = this->getController();
    Decision d = ctrl.getDecision(pcb->getPID(), rc);
    // next access binding possibilites of selected comp
    MappingInformationIterator* iter = pcb->getBindingGraph().getBinding(d.comp)->getMappingInformationIterator(); 
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
