#include "hscd_vpc_LPTBinder.h"

#define VPC_DEBUG
namespace SystemC_VPC {
  /**
   * \brief Implementation of LPTBinder constructor
   */
  LPTBinder::LPTBinder() : DynamicBinder() {

    numberofcomp = 0;
    config_blocked_until = sc_time_stamp();
    
  }
  
  /**
   * \brief Implementation of LPTBinder destructor
   */
  LPTBinder::~LPTBinder() {}
  
  /**
   * \brief Implemetation of LPTBinder::performBinding
   */
  std::pair<std::string, MappingInformation* > LPTBinder::performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp)
  throw(UnknownBindingException){
    
    //Get access to Components to count them
    Binding* RecomponentBinding = NULL;
    if(comp == NULL){
      RecomponentBinding = task.getBindingGraph().getRoot();
    }else{
      RecomponentBinding = task.getBindingGraph().getBinding(comp->basename());
    }
    //get Iterator on Components
    ChildIterator* RecomponentBindingChildIter = RecomponentBinding->getChildIterator();
    
    if(numberofcomp == 0){
      //ChildIterator* counter = bIter;
      while(RecomponentBindingChildIter->hasNext()){
        RecomponentBindingChildIter->getNext();
        rctime.push_back(sc_time(SC_ZERO_TIME));
        numberofcomp++;
      }
      //reset
      RecomponentBindingChildIter = RecomponentBinding->getChildIterator();
    }
#ifdef VPC_DEBUG  
    std::cerr << "**************************************"<< std::endl;
    std::cerr << "LPTBinder> Simulation time: " << sc_time_stamp() << endl;
    //std::cerr << "LPTBinder> Number of ReComponents "<< numberofcomp << std::endl;
    if (comp != NULL) std::cerr << "LPTBinder> ReComponent: "<< comp->basename() <<"> Task: " << task.getName() << endl;
    if (comp == NULL) std::cerr << "LPTBinder> ReComponent: NULL"<<"> Task: " << task.getName() << endl;
#endif

    //check queue for rc with minimum runtime left
    int chosen = 0;
    for(int i=0; i < numberofcomp; i++){
      if (rctime[i] < rctime[chosen])
        chosen = i;
    }
    
    //scroll to chosen Recomponent
    Binding * RecomponentBindingChild;
    for(int i=0; i <= chosen; i++){
      if(RecomponentBindingChildIter->hasNext()){
        RecomponentBindingChild = RecomponentBindingChildIter->getNext();
      }else{    
        delete RecomponentBindingChildIter;
        std::string msg = "LPTBinder> No target specified for "+ task.getName() +"->?";
        throw UnknownBindingException(msg);
      }
    }
    delete RecomponentBindingChildIter;
    
    //getMappingInformation
    MappingInformationIterator* MapInfoIter = RecomponentBindingChild->getMappingInformationIterator();
    if(MapInfoIter->hasNext()){
      MappingInformation* mInfo = MapInfoIter->getNext();
      delete MapInfoIter;

    //getSetuptime
      sc_time setuptime = this->getSetuptime(task);

#ifdef VPC_DEBUG
      std::cerr << "LPTBinder> Chose Mapping: "<< RecomponentBindingChild->getID() << endl;
      std::cerr << "LPTBinder> SetupTime: "<< setuptime << std::endl;
      std::cerr << "LPTBinder> Runtime: " << mInfo->getDelay() << endl;   
#endif      
      
      //move all slots time-border for next possible configuration by this->setuptime
      sc_time chosentime = rctime[chosen];
      for(int i=0; i<numberofcomp; i++){
        if(rctime[i] < (chosentime+setuptime))
          rctime[i] = chosentime+setuptime;
          if(i == chosen)
            rctime[chosen] += mInfo->getDelay();
#ifdef VPC_DEBUG
        std::cerr << "LPTBinder> time-border for Slot" << i+1 << " = " << rctime[i] << std::endl;
#endif
      }
    
      //wait till last configuration finished
      if (sc_time_stamp() < config_blocked_until){
        wait(config_blocked_until - sc_time_stamp());
      }
      config_blocked_until = sc_time_stamp() + setuptime;
#ifdef VPC_DEBUG
      std::cerr << "LPTBinder> config_blocked_until: " << config_blocked_until << endl;
#endif            
    
    //return MappingInformation
    return std::pair<std::string, MappingInformation*>(RecomponentBindingChild->getID(), mInfo);
      
    }else{
      // also free iterator
      delete MapInfoIter;
    }
    
  }//end of LPTBinder::performBinding()

  
  /**
   * \brief Implementation of LPTBinder::signalProcessEvent
   */
  void LPTBinder::signalProcessEvent(ProcessControlBlock* pcb, std::string compID) {
    //std::cerr << "ReComp "<< compID << " ist frei" <<endl;
  }
  
  /**
   * \brief Implementation of LPTBinder::getConfiguration
   * Used e.g. to the setuptime with getLoadtime()
   */
  Configuration* LPTBinder::getConfiguration(ProcessControlBlock task){
    Director* myDir = dynamic_cast<Director*>(getDirector());
    //ReconfigurableComponent* myComp = myDir->getReComp();
    std::string aReComp =
      task.getBindingGraph().getRoot()->getChildIterator()->getNext()->getID();
    ReconfigurableComponent* myComp = myDir->getCompByName(aReComp);
    

    if(myComp == NULL){
      std::cerr << "LPTBinder> MyComp ist NULL" << std::endl;
    }
    AbstractController* myCtrl = myComp->getController();
    if(myCtrl == NULL){
      std::cerr << "LPTBinder> MyCtrl ist NULL" << std::endl;
    }
    
    Binding* myBinding = task.getBindingGraph().getBinding(myComp->basename());
    ChildIterator* myBindingChildIter = myBinding->getChildIterator();
    Binding* myBindingChild;
    if (myBindingChildIter->hasNext()) 
      myBindingChild = myBindingChildIter->getNext();
    
    unsigned int ConfID = myCtrl->getConfigurationMapper()->getConfigForComp(myBindingChild->getID());
    
    Configuration* myConfig = myComp->getConfiguration(ConfID);
    
    return myConfig;
  }
  
  /**
   * \brief Implementation of LPTBinder::getSetuptime
   */
  sc_time LPTBinder::getSetuptime(ProcessControlBlock task){
    Configuration* myConfig = this->getConfiguration(task);
    sc_time setuptime = myConfig->getLoadTime();
    
    return setuptime;
  }
  
  /**
   * \brief Implementation of LPTBinder::getRuntime
   */
  sc_time LPTBinder::getRuntime(ProcessControlBlock task){
    
    //getReconfigurableComponent
    Binding* RecomponentBinding = task.getBindingGraph().getRoot();
    Binding* RecomponentBindingChild;
    ChildIterator* RecomponentBindingChildIter = RecomponentBinding->getChildIterator();
    if(RecomponentBindingChildIter->hasNext())
      RecomponentBindingChild = RecomponentBindingChildIter->getNext();
    delete RecomponentBindingChildIter;
    
    //getMappingInformation
    MappingInformationIterator* MapInfoIter = RecomponentBindingChild->getMappingInformationIterator();
    MappingInformation* mInfo;
    if(MapInfoIter->hasNext())
      mInfo = MapInfoIter->getNext();
    delete MapInfoIter;
    
    return mInfo->getDelay();
  }
  
}
