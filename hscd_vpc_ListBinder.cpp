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
        RecomponentBindingChildIter->getNext();
        rctime.push_back(sc_time(SC_ZERO_TIME));
        numberofcomp++;
      }
      //reset
      RecomponentBindingChildIter = RecomponentBinding->getChildIterator();
    }
#ifdef VPC_DEBUG  
    std::cerr << "**************************************"<< std::endl;
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
    for(int i=0; i <= chosen; i++){
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
          if(i == chosen)
            rctime[chosen] += mInfo->getDelay();
#ifdef VPC_DEBUG
        std::cerr << "ListBinder> time-border for Slot" << i+1 << " = " << rctime[i] << std::endl;
#endif
      }
    
      //wait till last configuration finished
      if (sc_time_stamp() < config_blocked_until){
        wait(config_blocked_until - sc_time_stamp());
      }
      config_blocked_until = sc_time_stamp() + setuptime;
#ifdef VPC_DEBUG
      std::cerr << "ListBinder> config_blocked_until: " << config_blocked_until << endl;
#endif            
    
    //return MappingInformation
    return std::pair<std::string, MappingInformation*>(RecomponentBindingChild->getID(), mInfo);
      
    }else{
      // also free iterator
      delete MapInfoIter;
    }
    
  }//end of ListBinder::performBinding()

  /**
   * \brief Helper for ListBinder::generate_sctime
   */
  void ListBinder::cleanstring(std::string *output){
    std::string::iterator iter = output->begin();
        while(*iter == ' ' || *iter == '\t' ) {
          iter = output->erase(iter);
        }
        iter = output->end()-1;
        while(*iter == ' ' || *iter == '\t') {
          output->erase(iter);
          iter = output->end()-1;
        }
    return;
  }
  
  /**
   * \brief Implementation of ListBinder::generate_sctime
   */
  sc_time ListBinder::generate_sctime(std::string starttime){
    //trenne Zahl und einheit
    std::string numbers = "0123456789.";
    std::string::iterator iter = starttime.begin();
    while( numbers.find(*iter) != std::string::npos) iter++;
    std::string time = starttime.substr(0, iter - starttime.begin());
    std::string unit = starttime.substr(iter - starttime.begin(), starttime.end() - iter);
    
    //std::cerr << "time:"<<time<<"Unit:"<<unit<<std::endl;
      
    std::istringstream timex;
    double timeindouble;
    timex.str( time );
    timex >> timeindouble;
    
    cleanstring(&unit);
    //generiere sc_time(zahl,einheit)
    sc_time_unit scUnit = SC_NS;
    if(      0==unit.compare(0, 2, "fs") ) scUnit = SC_FS;
    else if( 0==unit.compare(0, 2, "ps") ) scUnit = SC_PS;
    else if( 0==unit.compare(0, 2, "ns") ) scUnit = SC_NS;
    else if( 0==unit.compare(0, 2, "us") ) scUnit = SC_US;
    else if( 0==unit.compare(0, 2, "ms") ) scUnit = SC_MS;
    else if( 0==unit.compare(0, 1, "s" ) ) scUnit = SC_SEC;
    
    return sc_time(timeindouble,scUnit);
  }
  
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
    Director* myDir = dynamic_cast<Director*>(getDirector());
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
    Configuration* myConfig = this->getConfiguration(task);
    sc_time setuptime = myConfig->getLoadTime();
    
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
