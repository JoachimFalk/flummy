#include "hscd_vpc_ListBinder.h"

#define VPC_DEBUG
namespace SystemC_VPC {
  /**
   * \brief Implementation of ListBinder constructor
   */
  ListBinder::ListBinder() : DynamicBinder() {

    numberofcomp = 0;
    config_blocked_until = sc_time_stamp();
    
  }
  
  /**
   * \brief Implementation of ListBinder destructor
   */
  ListBinder::~ListBinder() {}
  
  /**
   * \brief Implemetation of ListBinder::performBinding
   */
  std::pair<std::string, MappingInformation* > ListBinder::performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp)
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
        Binding* Slot = RecomponentBindingChildIter->getNext();
        rctime.push_back(sc_time(SC_ZERO_TIME));
        slotTable_entry sentry = slotTable_entry (numberofcomp, Slot->getID());
        this->slotTable.push_back(sentry);
        numberofcomp++;
      }
      //reset
      RecomponentBindingChildIter = RecomponentBinding->getChildIterator();
    }
#ifdef VPC_DEBUG  
    std::cerr << "***************************************"<< std::endl;
    std::cerr << "ListBinder> Simulation time: " << sc_time_stamp() << endl;
    //std::cerr << "ListBinder> Number of ReComponents "<< numberofcomp << std::endl;
    if (comp != NULL) std::cerr << "ListBinder> ReComponent: "<< comp->basename() <<"> Task: " << task.getName() << endl;
    if (comp == NULL) std::cerr << "ListBinder> ReComponent: NULL"<<"> Task: " << task.getName() << endl;
#endif

    //check queue for rc with minimum runtime left
    int chosen = 0;
    for(int i=0; i < numberofcomp; i++){
      if (rctime[i] < rctime[chosen])
        chosen = i;
    }
    
    //scroll to chosen Recomponent
    Binding * RecomponentBindingChild;
    if(RecomponentBindingChildIter->hasNext())
        RecomponentBindingChild = RecomponentBindingChildIter->getNext();
    while(slotTable[chosen].recomponentname != RecomponentBindingChild->getID()){
      if(RecomponentBindingChildIter->hasNext()){
        RecomponentBindingChild = RecomponentBindingChildIter->getNext();
      }else{    
        delete RecomponentBindingChildIter;
        std::string msg = "ListBinder> No target specified for "+ task.getName() +"->?";
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
      std::cerr << "ListBinder> Chose Mapping: "<< RecomponentBindingChild->getID() << endl;
      std::cerr << "ListBinder> SetupTime: "<< setuptime << std::endl;
      std::cerr << "ListBinder> Runtime: " << mInfo->getDelay() << endl;   
#endif      
      
      //move all slots time-border for next possible configuration by this->setuptime
      sc_time chosentime = rctime[chosen];
      for(int i=0; i<numberofcomp; i++){
        if(rctime[i] < (chosentime+setuptime))
          rctime[i] = chosentime+setuptime;
        if(i == chosen){
          rctime[chosen] += mInfo->getDelay();
          std::cerr << "chosen:" << chosen << std::endl;
        }
#ifdef VPC_DEBUG
        std::cerr << "ListBinder> time-border for Slot" << i+1 << " = " << rctime[i] << std::endl;
#endif
      }
    
      //wait till last configuration finished
       if (sc_time_stamp() < config_blocked_until){
         wait(config_blocked_until - sc_time_stamp());
       }
      config_blocked_until = chosentime + setuptime;
#ifdef VPC_DEBUG
      std::cerr << "ListBinder> config_blocked_until: " << config_blocked_until << endl;
#endif
    
    //return MappingInformation
    return std::pair<std::string, MappingInformation*>(RecomponentBindingChild->getID(), mInfo);
      
    }else{
      // also free iterator
      delete MapInfoIter;
      return std::pair<std::string, MappingInformation*>("",NULL);
    }
    
  }//end of ListBinder::performBinding()

  /**
   * \brief Implementation of ListBinder::signalProcessEvent
   */
  void ListBinder::signalProcessEvent(ProcessControlBlock* pcb, std::string compID) {
    //std::cerr << "ReComp "<< compID << " ist frei" <<endl;
  }
  
  /**
   * \brief Implementation of ListBinder::getConfiguration
   * Used e.g. to the setuptime with getLoadtime()
   */
  Configuration* ListBinder::getConfiguration(ProcessControlBlock task){
    //Director* myDir = dynamic_cast<Director*>(getDirector());
    Director* myDir = &Director::getInstance();
    //ReconfigurableComponent* myComp = myDir->getReComp();
    std::string aReComp =
      task.getBindingGraph().getRoot()->getChildIterator()->getNext()->getID();
    ReconfigurableComponent* myComp = myDir->getCompByName(aReComp);
    

    if(myComp == NULL){
      std::cerr << "ListBinder> MyComp ist NULL" << std::endl;
    }
    AbstractController* myCtrl = myComp->getController();
    if(myCtrl == NULL){
      std::cerr << "ListBinder> MyCtrl ist NULL" << std::endl;
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
   * \brief Implementation of ListBinder::getSetuptime
   */
  sc_time ListBinder::getSetuptime(ProcessControlBlock task){
    sc_time setuptime;
    Configuration* myConfig = this->getConfiguration(task);
    if(myConfig) setuptime = myConfig->getLoadTime();
    else setuptime = SC_ZERO_TIME;
    
    return setuptime;
  }
  
  /**
   * \brief Implementation of ListBinder::getRuntime
   */
  sc_time ListBinder::getRuntime(ProcessControlBlock task){
    
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
