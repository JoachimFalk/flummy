#include <hscd_vpc_OfflineAllocator.h>
#define VPC_DEBUG
namespace SystemC_VPC{
  
  /**
   * \brief Initializes instance of OfflineAllocator
   */
  OfflineAllocator::OfflineAllocator(AbstractController* controller) : Allocator(controller){
    //no Config set on Startup
    this->nextConfiguration = 0;    
    this->controller = controller;
    this->currTask = NULL;
    this->reqConfig = 0;
  }

  /**
   * \brief Deletes instance of OfflineAllocator
   */
  OfflineAllocator::~OfflineAllocator(){
    //assert(this->tasks.size() == 0);
  }
  
  void OfflineAllocator::initController(){
   
  //Aus Datei Tabelle einlesen
    //Die Reihenfolge soll nach Zeiten sortiert sein
    //Loesung: priority_queue sortiert beim einfügen
 
  //read times table
    timesTable_entry entry;
    entry = timesTable_entry(sc_time(0, SC_MS), "periodic.T3", "ESM-Slot1");
    if(controller->getManagedComponent()->getName() == entry.recomponentname)
     this->timesTable.push(entry);
    entry = timesTable_entry(sc_time(7, SC_MS), "periodic.T6", "ESM-Slot2");
    if(controller->getManagedComponent()->getName() == entry.recomponentname)
     this->timesTable.push(entry);
    entry = timesTable_entry(sc_time(1, SC_MS), "periodic.T5", "ESM-Slot2");
    if(controller->getManagedComponent()->getName() == entry.recomponentname)
     this->timesTable.push(entry);
    entry = timesTable_entry(sc_time(2, SC_MS), "periodic.T1", "ESM-Slot3");
    if(controller->getManagedComponent()->getName() == entry.recomponentname)
     this->timesTable.push(entry);
    entry = timesTable_entry(sc_time(11, SC_MS), "periodic.T4", "ESM-Slot1");
    if(controller->getManagedComponent()->getName() == entry.recomponentname)
     this->timesTable.push(entry);
    entry = timesTable_entry(sc_time(13, SC_MS), "periodic.T2", "ESM-Slot3");
    if(controller->getManagedComponent()->getName() == entry.recomponentname)
     this->timesTable.push(entry);
     /*
     //periodic test
    entry = timesTable_entry(sc_time(30, SC_MS), "periodic.T1", "ESM-Slot3");
    if(controller->getManagedComponent()->getName() == entry.recomponentname)
     this->timesTable.push(entry);
     entry = timesTable_entry(sc_time(30, SC_MS), "periodic.T6", "ESM-Slot2");
    if(controller->getManagedComponent()->getName() == entry.recomponentname)
     this->timesTable.push(entry);*/
  //DEBUG
    /*while( !this->timesTable.empty() ){
      cerr << this->timesTable.top().time << ";\t"
           << this->timesTable.top().taskname << ";\t"
           << this->timesTable.top().recomponentname << ";" <<endl;
      this->timesTable.pop();
    }
    */
  //Assume the reconfigurable component has minimum one task
    //assert( !this->timesTable.empty() );
}
  /**
   * \brief Implementation of OfflineAllocator::addProcessToSchedule
   */
  void OfflineAllocator::addProcessToSchedule(ProcessControlBlock* newTask, unsigned int config, ReconfigurableComponent* rc){

    #ifdef VPC_DEBUG
        std::cerr << "OfflineAllocator "<< this->getController().getName() <<"> addProcessToSchedule called! " 
          << "For task " << newTask->getName() << " with required configuration id " << config << " at " << sc_time_stamp() << endl;
    #endif //VPC_DEBUG

    // add task to local storage structure
    std::pair<ProcessControlBlock*, unsigned int> entry(newTask, config);
    this->tasks.push_back(entry);
  }

  /**
   * \brief Implementation of OfflineAllocator::performSchedule
   */
  void OfflineAllocator::performSchedule(ReconfigurableComponent* rc){
     
    // if no times left do nothing, return
      if( (timesTable.size() == 0) ){
        this->waitInterval = NULL;
        return;
      }
    
    // Compare taskname in timesTable with tasks list
      std::vector<std::pair<ProcessControlBlock*,unsigned int> >::iterator taskiter;
      if(currTask == NULL){ // if no current Task already selected 
        for(taskiter = this->tasks.begin();taskiter != tasks.end();++taskiter){
          if ((*taskiter).first->getName() == timesTable.top().taskname){
            #ifdef VPC_DEBUG
              std::cerr << "OfflineAllocator> "<< VPC_RED("Task in timesTable found ")<< timesTable.top().taskname << endl;
            #endif
            // Set found Task and Config as current, delete task from list
            currTask = (*taskiter).first;
            reqConfig = (*taskiter).second;
            tasks.erase(taskiter);
            break;
          }
        }
      }
        
    // look if current time is newer than timesTable top entry
      if (sc_time_stamp() >= timesTable.top().time){
        // time to do something
        this->waitInterval = NULL;
        // load new configuration
        this->nextConfiguration = reqConfig;
        // delete outdated time entry from timesTable
        this->timesTable.pop();
        //if no task for this time, wait for next time intervall from times Table
        if( (currTask == NULL)&&(timesTable.size() != 0) ){
          this->waitInterval = new sc_time( this->timesTable.top().time - sc_time_stamp() );
        }
      }else{ // still time to wait
        // no config to load
        //this->nextConfiguration = 0;
        // wait until it's your time
        this->waitInterval = new sc_time( timesTable.top().time - sc_time_stamp() );
      }
        #ifdef VPC_DEBUG
          if (waitInterval != NULL) {std::cerr << "OfflineAllocator::performSchedule> "<< "waitIntervall=" << *this->waitInterval << std::endl;
          }else{std::cerr << "OfflineAllocator::performSchedule> "<< "waitIntervall=0" << std::endl;}
        #endif //VPC_DEBUG
    }//endof performSchedule
  
  /*
   * \brief Implementation of OfflineAllocator::getNextConfiguration
   */  
  unsigned int OfflineAllocator::getNextConfiguration(ReconfigurableComponent* rc){
    
    //Returns nextConfiguration set by performSchedule
    unsigned int next = this->nextConfiguration;
    this->nextConfiguration = 0;
    return next;
  }
   
  /**
   * \brief Implementation of OfflineAllocator::hasProcessToDispatch()
   */
  bool OfflineAllocator::hasProcessToDispatch(ReconfigurableComponent* rc){
  
     //return (this->hasProcess);
     /*
     std::vector<std::pair<ProcessControlBlock*,unsigned int> >::iterator taskiter;
     ProcessControlBlock* task;
     bool hasProcess = false;
     taskiter = this->tasks.begin();
     while (taskiter <= tasks.end()){
        task = (*taskiter).first;
        if (task->getName() == this->timesTable.top().taskname){
          hasProcess = true;
        }
     }
     */
     return (currTask != NULL);
  }
  
  /**
   * \brief Implementation of OfflineAllocator::getNextProcess()
   */
  ProcessControlBlock* OfflineAllocator::getNextProcess(ReconfigurableComponent* rc){
     
     /*std::vector<std::pair<ProcessControlBlock*,unsigned int> >::iterator taskiter;
     taskiter = this->tasks.begin();
     
     while (taskiter != tasks.end()){
        task = (*taskiter).first;
        if (task->getName() == this->timesTable.top().taskname){
          tasks.erase(taskiter);
          return task;
        }
        taskiter++;
     }
     */
     ProcessControlBlock* task = currTask;
     currTask = NULL;
     return task;        
   }
  
  /**
   * \brief Implementation of OfflineAllocator::signalProcessEvent
   */
  void OfflineAllocator::signalProcessEvent(ProcessControlBlock* pcb, std::string compID){
  
    #ifdef VPC_DEBUG
        std::cerr << "OfflineAllocator " << this->getController().getName() << "> got notified by task: " << pcb->getName() << "::" << pcb->getFuncName() << std::endl;
    #endif //VPC_DEBUG
    
    this->getManagedComponent()->wakeUp();
    
  }

  /**
   * \brief Implementation of StaticConfScheduler::signalDeallocation
   */
  void OfflineAllocator::signalDeallocation(bool kill, ReconfigurableComponent* rc){
    // only interested in Preemption with KILL
    if(kill){
        currTask->setState(activation_state(aborted));
        this->getManagedComponent()->notifyParentController(currTask);
      }
  }
  
} //namespace SystemC_VPC
