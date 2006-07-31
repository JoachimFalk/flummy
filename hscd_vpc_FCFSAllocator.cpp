#include <hscd_vpc_FCFSAllocator.h>

namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of FCFSAllocator
   */
  FCFSAllocator::FCFSAllocator(AbstractController* controller) : Allocator(controller){

    this->nextConfiguration = 0;
    
  }

  /**
   * \brief Deletes instance of FCFSAllocator
   */
  FCFSAllocator::~FCFSAllocator(){
    assert(this->readyTasks.size() == 0);
    assert(this->runningTasks.size() == 0);
    assert(this->tasksToProcess.size() == 0);
  }
  
  /**
   * \brief Implementation of FCFSAllocator::addTasksToSchedule
   */
  void FCFSAllocator::addTaskToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc){

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("FCFSAllocator "<< this->getController().getName() <<"> addTasksToSchedule called! ") 
          << "For task " << newTask->getName() << " with required configuration id " << config << " at " << sc_simulation_time() << endl;
//        std::cerr << VPC_YELLOW("FCFSAllocator "<< this->getController().getName() <<"> addTasksToSchedule called! ") << sc_simulation_time() << endl;
#endif //VPC_DEBUG

    // first of all add task to local storage structure
    std::pair<ProcessControlBlock*, unsigned int> entry(newTask, config);
    this->readyTasks.push_back(entry);
  }

  /**
   * \brief Implementation of FCFSAllocator::performSchedule
   */
  void FCFSAllocator::performSchedule(ReconfigurableComponent* rc){

    ProcessControlBlock* currTask;
    unsigned int reqConfig = 0;
    // now check which tasks to pass forward
    while(this->readyTasks.size()){
      currTask = this->readyTasks.front().first;
      reqConfig = this->readyTasks.front().second;

#ifdef VPC_DEBUG
      std::cerr << VPC_YELLOW("FCFSAllocator "<< this->getController().getName() <<"> processing task ") << currTask->getName()
        << " with required configuration id " << reqConfig << endl;
      if(this->getManagedComponent()->getActivConfiguration() == NULL){
        std::cerr << VPC_YELLOW("FCFSAllocator "<< this->getController().getName() <<"> no activ configuration for managed component ") << endl;
      }else{
        std::cerr << VPC_YELLOW("FCFSAllocator "<< this->getController().getName() <<"> activ configuration for managed component is ") 
          << this->getManagedComponent()->getActivConfiguration()->getName() << " with id " 
          << this->getManagedComponent()->getActivConfiguration()->getID() << endl;
        std::cerr << VPC_YELLOW("FCFSAllocator "<< this->getController().getName() <<"> currently running num of tasks on conf ") << this->runningTasks.size() << endl;
      }
      std::cerr << VPC_YELLOW("FCFSAllocator "<< this->getController().getName() <<"> next configuration is set to ") << this->nextConfiguration << endl;
#endif //VPC_DEBUG

      // check if
      if((this->getManagedComponent()->getActivConfiguration() == NULL && this->nextConfiguration == 0) // no activ and no next conf selected
          || (this->runningTasks.size() == 0 && this->nextConfiguration == 0) // or no running tasks and no next conf selected
          || (this->nextConfiguration == 0 && reqConfig == this->getManagedComponent()->getActivConfiguration()->getID()) // or no selected conf but actual one fits!
          || reqConfig == this->nextConfiguration){ // or required conf fits already selected one

#ifdef VPC_DEBUG
        std::cerr << VPC_YELLOW("FCFSAllocator "<< this->getController().getName() <<"> can process task ") << currTask->getName() 
          << " with required configuration " << reqConfig << endl;
#endif //VPC_DEBUG

        this->tasksToProcess.push(currTask);
        this->runningTasks[currTask->getPID()] = currTask;
        // remove task that can be passed from waiting queue
        this->readyTasks.pop_front();

        // load new configuration
        this->nextConfiguration = reqConfig;

      }else{
        // current task not processable, so leave it and stop processing any further tasks
        break;
      }
    }

  }
  
  /*
   * \brief Implementation of FCFSAllocator::getNextConfiguration
   */  
  unsigned int FCFSAllocator::getNextConfiguration(ReconfigurableComponent* rc){
    
    unsigned int next = this->nextConfiguration;
    this->nextConfiguration = 0;
    return next;
  
  }
   
  /**
   * \brief Implementation of FCFSAllocator::hasTaskToProcess()
   */
  bool FCFSAllocator::hasTaskToProcess(ReconfigurableComponent* rc){
  
     return (this->tasksToProcess.size() > 0);
  
  }
  
  /**
   * \brief Implementation of FCFSAllocator::getNextTask()
   */
  ProcessControlBlock* FCFSAllocator::getNextTask(ReconfigurableComponent* rc){
     
     ProcessControlBlock* task;
     task = this->tasksToProcess.front();
     this->tasksToProcess.pop();
     return task;
   
   }
  
  /**
   * \brief Implementation of FCFSAllocator::signalProcessEvent
   */
  void FCFSAllocator::signalProcessEvent(ProcessControlBlock* pcb, std::string compID){
  
#ifdef VPC_DEBUG
    std::cerr << "FCFSAllocator " << this->getController().getName() << "> got notified by task: " << pcb->getName() << "::" << pcb->getFuncName()
              << " with running tasks num= " << this->runningTasks.size() << std::endl;
#endif //VPC_DEBUG
    
    this->runningTasks.erase(pcb->getPID());
    
    // if there are no running task and still ready its time to wakeUp ReconfigurableComponent
    if(this->runningTasks.size() == 0 && this->readyTasks.size() != 0){
      // ensure that next configuration is reset
      this->nextConfiguration = 0;
#ifdef VPC_DEBUG
      std::cerr << "FCFSAllocator> waking up component thread!" << std::endl;
#endif //VPC_DEBUG

      this->getManagedComponent()->wakeUp();
    }
  }

  /**
   * \brief Implementation of FCFSConfScheudler::signalDeallocation
   */
  void FCFSAllocator::signalDeallocation(bool kill, ReconfigurableComponent* rc){
    // only interested in Preemption with KILL
    if(kill){
      
      std::pair<ProcessControlBlock*, unsigned int> p;
      ProcessControlBlock* currTask;

      //for all waiting task signal their abortion
      while(this->readyTasks.size()){
        p = this->readyTasks.front();
        currTask = p.first;
        this->readyTasks.pop_front();
        currTask->setState(activation_state(aborted));
        this->getManagedComponent()->notifyParentController(currTask);
      }

    }
  }
  
} //namespace SystemC_VPC