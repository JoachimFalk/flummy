#include "hscd_vpc_OnlineBinder.h"

#define VPC_DEBUG
namespace SystemC_VPC {
  /**
   * \brief Implementation of OnlineBinder constructor
   */
  OnlineBinder::OnlineBinder(char* algorithm) : DynamicBinder() {
    this->algorithm = algorithm;
    numberofcomp = 0;
    std::cerr << "OnlineAlgorithmBinder> Chosen algorithm: " << this->algorithm << std::endl;
  }
  
  /**
   * \brief Implementation of OnlineBinder destructor
   */
  OnlineBinder::~OnlineBinder() {}
  
  /**
   * \brief Implemetation of OnlineBinder::performBinding
   */
  std::pair<std::string, MappingInformation* > OnlineBinder::performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp)
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
      while(RecomponentBindingChildIter->hasNext()){
        Binding * Slot = RecomponentBindingChildIter->getNext();
        timesTable_entry entry = timesTable_entry (SC_ZERO_TIME, numberofcomp);
        this->timesTable.push_back(entry);
        slotTable_entry sentry = slotTable_entry (numberofcomp, Slot->getID());
        this->slotTable.push_back(sentry);
        numberofcomp++;
      }
    }
    //reset
    RecomponentBindingChildIter->reset();

#ifdef VPC_DEBUG  
    std::cerr << "***************************************"<< std::endl;
    std::cerr << "OnlineBinder> Simulation time: " << sc_time_stamp() << endl;
    //std::cerr << "OnlineBinder> Number of ReComponents "<< numberofcomp << std::endl;
    if (comp != NULL) std::cerr << "OnlineBinder> ReComponent: "<< comp->basename() <<"> Task: " << task.getName() << endl;
    if (comp == NULL) std::cerr << "OnlineBinder> ReComponent: NULL"<<"> Task: " << task.getName() << endl;
#endif
    
int chosen;

sc_time RCWaitInterval = SC_ZERO_TIME;
//********************************************************************************
//Following Online-Scheduling-Algorithms
//********************************************************************************
if(strcmp(algorithm,"List") == 0){
    
    chosen = 0;
    
    //find lowest Slot
    for(int i=0; i < numberofcomp; i++){
      if(timesTable[i].time < timesTable[chosen].time)
        chosen = i;
    }
    timesTable[chosen].time += getSetuptime(task);
    
    //set all Slottimes to new border
    for(int i=0; i < numberofcomp; i++){
      if(timesTable[i].time < timesTable[chosen].time)
        timesTable[i].time = timesTable[chosen].time;
    }
    timesTable[chosen].time += getRuntime(task);
    
//End Algorithm List

}else if(strcmp(algorithm,"Bartal") == 0){
    
    sort(timesTable.begin(), timesTable.end());
    sc_time job = getSetuptime(task) + getRuntime(task);
    
    //Calculate lowest in Li 44,5%
    int lowestLi = (int)floor(0.445 * numberofcomp);

    //Calculate A(Ri)
    sc_time ARi = SC_ZERO_TIME;
    for(int i=0; i < lowestLi;i++){
      ARi += timesTable[i].time;
    }
    ARi = ARi / lowestLi;
    
    //Make decision
    if( (timesTable[lowestLi].time + job) <= (1.986 * ARi) ){
      chosen = timesTable[lowestLi].recomponentnumber;
      timesTable[lowestLi].time += job;
      //RCWaitInterval = 2*getSetuptime(task);
    }else{
      chosen = timesTable[0].recomponentnumber;
      timesTable[0].time += job;
    }
//End Algorithm Bartal

}else if(strcmp(algorithm,"Karger") == 0){

    sort(timesTable.begin(), timesTable.end());
  
    double alpha = 1.945;
    sc_time job = getSetuptime(task) + getRuntime(task);
    
    //h(k)+j <= alpha * A(tk)
    chosen = -1;    
    sc_time Aall = SC_ZERO_TIME;
    for(int i = numberofcomp//RCWaitInterval        

        -1; i > 0; i--){
      Aall += timesTable[i].time;
    }
    sc_time Ai;
    for(int i = numberofcomp-1; i > 0; i--){
      Ai = Aall / i;
      if(timesTable[i].time + job <= alpha * Ai){
        chosen = timesTable[i].recomponentnumber;
        timesTable[i].time += job;
        break; 
      }
      Aall -= timesTable[i].time;
    }
    if(chosen == -1){
        chosen = timesTable[0].recomponentnumber;
        timesTable[0].time += job;
    }

//End Algorithm Karger

}else if(strcmp(algorithm,"Albers") == 0){

    sort(timesTable.begin(), timesTable.end());
    int m = numberofcomp;
    double c = 1.923;
    int i = (int)floor(0.5 * m);
    double j = 0.29 * m;
    double alpha = ( (c-1)*i-j/2 ) / ( (c-1)*(m-i) );

    sc_time job = getSetuptime(task) + getRuntime(task);

    sc_time Ll = SC_ZERO_TIME;
    for(int l=0; l < i;l++){
      Ll += timesTable[l].time;
    }
    Ll += job;    

    sc_time Lh = SC_ZERO_TIME;
    for(int l=i; l < m;l++){
      Lh += timesTable[l].time;
    }    
    Lh += job;

    //Make decision
    if( Ll <= alpha * Lh ){ //least loaded
      chosen = timesTable[0].recomponentnumber;
      timesTable[0].time += job;
    }else if ((timesTable[i+1].time + job > timesTable[m-1].time) 
          && (timesTable[i+1].time + job > (c *(Ll+Lh)/m))){ //least loaded
      chosen = timesTable[0].recomponentnumber;
      timesTable[0].time += job;
    }else{ //i+1 loaded
      chosen = timesTable[i+1].recomponentnumber;
      timesTable[i+1].time += job;
    }
//End Algorithm Albers
//*****************************************************************************************
// Following Algorithms were modified by slot time reservation
//*****************************************************************************************
}else if(strcmp(algorithm,"BartalMod") == 0){
    
  sort(timesTable.begin(), timesTable.end());
  sc_time job = getSetuptime(task) + getRuntime(task);
    
    //Calculate lowest in Li 44,5%
  int lowestLi = (int)floor(0.445 * numberofcomp);

    //Calculate A(Ri)
  sc_time ARi = SC_ZERO_TIME;
  for(int i=0; i < lowestLi;i++){
    ARi += timesTable[i].time;
  }
  ARi = ARi / lowestLi;
    
    //Make decision
  if( (timesTable[lowestLi].time + job) <= (1.986 * ARi) ){
    chosen = timesTable[lowestLi].recomponentnumber;
    RCWaitInterval = lowestLi * getSetuptime(task);
    timesTable[lowestLi].time += RCWaitInterval;
    timesTable[lowestLi].time += job;
  }else{
    chosen = timesTable[0].recomponentnumber;
    timesTable[0].time += job;
  }
//End Algorithm BartalMod

}else if(strcmp(algorithm,"KargerMod") == 0){

  sort(timesTable.begin(), timesTable.end());
  
  double alpha = 1.945;
  sc_time job = getSetuptime(task) + getRuntime(task);
    
    //h(k)+j <= alpha * A(tk)
  chosen = -1;    
  sc_time Aall = SC_ZERO_TIME;
  for(int i = numberofcomp-1; i > 0; i--){
    Aall += timesTable[i].time;
  }
  sc_time Ai;
  int zero_slots = 0;
  for(int i = numberofcomp-1; i > 0; i--){
    Ai = Aall / i;
    if(timesTable[i].time == SC_ZERO_TIME)
      ++zero_slots;
    if(timesTable[i].time + job <= alpha * Ai){
      chosen = timesTable[i].recomponentnumber;
    //RCWaitInterval        
      if(zero_slots != 0){
        RCWaitInterval = zero_slots * getSetuptime(task);
      }else{
        RCWaitInterval = i * getSetuptime(task);
      }
      std::cerr << "RCWaitInterval: " << RCWaitInterval << std::endl;
      timesTable[i].time += RCWaitInterval;
      timesTable[i].time += job;
      break; 
    }
    Aall -= timesTable[i].time;
  }
  if(chosen == -1){
    chosen = timesTable[0].recomponentnumber;
    if(zero_slots != 0){
      RCWaitInterval = zero_slots * getSetuptime(task);
      std::cerr << "RCWaitInterval: " << RCWaitInterval << std::endl;
      timesTable[0].time += RCWaitInterval;
    }
    timesTable[0].time += job;
  }

//End Algorithm KargerMod

}else if(strcmp(algorithm,"AlbersMod") == 0){

  sort(timesTable.begin(), timesTable.end());
  int m = numberofcomp;
  double c = 1.923;
  int i = (int)floor(0.5 * m);
  double j = 0.29 * m;
  double alpha = ( (c-1)*i-j/2 ) / ( (c-1)*(m-i) );

  sc_time job = getSetuptime(task) + getRuntime(task);

  sc_time Ll = SC_ZERO_TIME;
  for(int l=0; l < i;l++){
    Ll += timesTable[l].time;
  }
  Ll += job;    

  sc_time Lh = SC_ZERO_TIME;
  for(int l=i; l < m;l++){
    Lh += timesTable[l].time;
  }    
  Lh += job;

    //Make decision
  if( Ll <= alpha * Lh ){ //least loaded
    chosen = timesTable[0].recomponentnumber;
    timesTable[0].time += job;
  }else if ((timesTable[i+1].time + job > timesTable[m-1].time) 
             && (timesTable[i+1].time + job > (c *(Ll+Lh)/m))){ //least loaded
               chosen = timesTable[0].recomponentnumber;
               timesTable[0].time += job;
             }else{ //i+1 loaded
               chosen = timesTable[i+1].recomponentnumber;
               RCWaitInterval = i * getSetuptime(task);
               timesTable[i+1].time += RCWaitInterval;
               timesTable[i+1].time += job;
             }
//End Algorithm AlbersMod
}else{
  std::cerr << std::endl << "Algorithm ";
  std::cerr << algorithm;
  std::cerr << " not found!" << std::endl << std::endl;
  exit(1);
  }
//**************************************************************************************************
//End all Algorithms
//**************************************************************************************************
    //scroll to chosen Recomponent
    Binding * RecomponentBindingChild;
    if(RecomponentBindingChildIter->hasNext())
        RecomponentBindingChild = RecomponentBindingChildIter->getNext();
    while(slotTable[chosen].recomponentname != RecomponentBindingChild->getID()){
      if(RecomponentBindingChildIter->hasNext()){
        RecomponentBindingChild = RecomponentBindingChildIter->getNext();
      }else{    
        delete RecomponentBindingChildIter;
        std::string msg = "OnlineBinder> No target specified for "+ task.getName() +"->?";
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

    //set Setuptime Reservation
      mInfo->setRCWaitInterval(RCWaitInterval);
#ifdef VPC_DEBUG
      std::cerr << "OnlineBinder> Chosen Slot: " << chosen +1 << std::endl;
      std::cerr << "OnlineBinder> Chose Mapping: "<< RecomponentBindingChild->getID() << endl;
      std::cerr << "OnlineBinder> SetupTime: "<< setuptime << std::endl;
      std::cerr << "OnlineBinder> Runtime: " << mInfo->getDelay() << endl;
      for(int i=0; i < numberofcomp; i++){
        std::cerr << "OnlineBinder> time-border for Slot" << i+1 << " = " << timesTable[i].time << std::endl;
      }
#endif      
      
    //return MappingInformation
    return std::pair<std::string, MappingInformation*>(RecomponentBindingChild->getID(), mInfo);
      
    }else{
      // also free iterator
      delete MapInfoIter;
      return std::pair<std::string, MappingInformation*>("",NULL);
    }
    
  }//end of OnlineBinder::performBinding()

  /**
   * \brief Implementation of OnlineBinder::signalProcessEvent
   */
  void OnlineBinder::signalProcessEvent(ProcessControlBlock* pcb, std::string compID) {
    //std::cerr << "ReComp "<< compID << " ist frei" <<endl;
  }
  
  /**
   * \brief Implementation of OnlineBinder::getConfiguration
   * Used e.g. to the setuptime with getLoadtime()
   */
  Configuration* OnlineBinder::getConfiguration(ProcessControlBlock task){

    Director* myDir = &Director::getInstance();
    std::string aReComp =
      task.getBindingGraph().getRoot()->getChildIterator()->getNext()->getID();
    ReconfigurableComponent* myComp = myDir->getCompByName(aReComp);
    

    if(myComp == NULL){
      std::cerr << "OnlineBinder> MyComp ist NULL" << std::endl;
    }
    AbstractController* myCtrl = myComp->getController();
    if(myCtrl == NULL){
      std::cerr << "OnlineBinder> MyCtrl ist NULL" << std::endl;
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
   * \brief Implementation of OnlineBinder::getSetuptime
   */
  sc_time OnlineBinder::getSetuptime(ProcessControlBlock task){
    sc_time setuptime;
    Configuration* myConfig = this->getConfiguration(task);
    if(myConfig) setuptime = myConfig->getLoadTime();
    else setuptime = SC_ZERO_TIME;
    
    return setuptime;
  }
  
  /**
   * \brief Implementation of OnlineBinder::getRuntime
   */
  sc_time OnlineBinder::getRuntime(ProcessControlBlock task){
    
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
