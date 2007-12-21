#include <hscd_vpc_OfflineAllocator.h>
//#define VPC_DEBUG
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
  
  void OfflineAllocator::initController(char* OfflineFileName){
  
   //Statische Bindung, aus Datei einlesen
    OfflineFile *myFile = new OfflineFile(OfflineFileName);
    std::string buffer;
    if(!myFile->open()){
      std::cerr << "OfflineAllocator> Offlinefile open error" << std::endl;
    }else{
      // Lese Datei in Puffer und schliessen
      buffer = myFile->getbuffer();
      myFile->close();
    }
 
    //find section SCHEDULER
    std::string::size_type position = buffer.find("SCHEDULE");
    if(position==string::npos) std::cerr << "OfflineAllocator> Offlinefile: No Schedule found in file" << std::endl;
    std::string schedule = buffer.substr(position, buffer.end()-buffer.begin());
    do{        
      unsigned int startpos, endpos;
      std::string taskname, recomponentname,starttime;
      
      // find task in section SCHEDULE
      startpos = schedule.find("periodic.T");

      if(startpos==string::npos){
#ifdef VPC_DEBUG      
        std::cerr << "OfflineAllocator> Offlinefile: no more task found in schedule" << std::endl;
#endif //VPC_DEBUG     
        break;
      }
      endpos = schedule.find(';',startpos);
      if(endpos==string::npos){
        std::cerr << "OfflineAllocator> Offlinefile: task format error" << std::endl;break;}
      taskname = schedule.substr(startpos, endpos-startpos);
      myFile->cleanstring(&taskname);
#ifdef VPC_DEBUG
      std::cerr << "A taskname:" << taskname;
#endif //VPC_DEBUG     
      // get recomponent name
      startpos = endpos+1;
      endpos = schedule.find(';',startpos);
      if(endpos==string::npos){
        std::cerr << "OfflineAllocator> Offlinefile: recomponent format error" << std::endl;break;}
      recomponentname = schedule.substr(startpos, endpos-startpos);
      myFile->cleanstring(&recomponentname);
#ifdef VPC_DEBUG      
      std::cerr << "; a Recomponentname:" << recomponentname << std::endl;
#endif //VPC_DEBUG           
      //get starttime
      startpos = endpos+1;
      endpos = schedule.find(';',startpos);
      if(endpos==string::npos){
        std::cerr << "OfflineAllocator> Offlinefile: starttime format error" << std::endl;break;}
      starttime = schedule.substr(startpos, endpos-startpos);
      myFile->cleanstring(&starttime);
#ifdef VPC_DEBUG            
      std::cerr << "; a Starttime:" << starttime << std::endl;
#endif //VPC_DEBUG              
      //if we are on the right recomponent, add entry to timesTable
      if(controller->getManagedComponent()->getName() == recomponentname){
        timesTable_entry entry = timesTable_entry (myFile->generate_sctime(starttime), taskname, recomponentname);
        this->timesTable.push(entry);
      }
      //repeat
      startpos = schedule.find('\n',endpos);
      if(startpos==string::npos)
        break;
      if(!schedule.size() > (startpos+1))
        break;
      schedule = schedule.substr(startpos+1,schedule.size()-startpos-1);
    }while(true);
  }//end of initController
  
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
              std::cerr << "OfflineAllocator "<< this->getController().getName() << ">"<< VPC_RED("Task in timesTable found ")<< timesTable.top().taskname << endl;
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
          if (waitInterval != NULL) {std::cerr << "OfflineAllocator "<< this->getController().getName() << ">"<< "waitIntervall=" << *this->waitInterval << std::endl;
          }else{std::cerr << "OfflineAllocator "<< this->getController().getName() << ">"<< "waitIntervall=0" << std::endl;}
        #endif //VPC_DEBUG
  }//endof performSchedule
  
  
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
  
     return (currTask != NULL);
  }
  
  /**
   * \brief Implementation of OfflineAllocator::getNextProcess()
   */
  ProcessControlBlock* OfflineAllocator::getNextProcess(ReconfigurableComponent* rc){
     
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
