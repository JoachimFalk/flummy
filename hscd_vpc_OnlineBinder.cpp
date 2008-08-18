#include "hscd_vpc_OnlineBinder.h"

//#define VPC_DEBUG
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
    
int chosen = 0;

sc_time RCWaitInterval = SC_ZERO_TIME;
//********************************************************************************
//Following Online-Scheduling-Algorithms
//********************************************************************************
if(strcmp(algorithm,"List") == 0){
#define SETUPTIMES
    chosen = 0;
    
    //find lowest Slot
    for(int i=0; i < numberofcomp; i++){
      if(timesTable[i].time < timesTable[chosen].time)
        chosen = i;
    }
#ifdef SETUPTIMES    
    if(sc_time_stamp() < timesTable[chosen].time){
      RCWaitInterval = timesTable[chosen].time;
  #ifdef VPC_DEBUG      
      std::cerr << "OnlineBinder> RCWaitInterval: " << RCWaitInterval << std::endl;
  #endif
    }
#endif
    timesTable[chosen].time += getSetuptime(task);
    
#ifdef SETUPTIMES        
    //set all Slottimes to new border
    for(int i=0; i < numberofcomp; i++){
      if(timesTable[i].time < timesTable[chosen].time)
        timesTable[i].time = timesTable[chosen].time;
    }
#endif
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
    }else{
      chosen = timesTable[0].recomponentnumber;
      timesTable[0].time += job;
    }
//End Algorithm Bartal

}else if(strcmp(algorithm,"Karger") == 0){

    sort(timesTable.begin(), timesTable.end());
  
    double alpha = 1.945;
    sc_time job = getSetuptime(task) + getRuntime(task);
    
    chosen = -1;    
    sc_time Aall = SC_ZERO_TIME;
    for(int i = 0; i < numberofcomp-1; i++){
      Aall += timesTable[i].time;
    }
    sc_time Ai;
    for(int i = numberofcomp-1; i > 0; i--){
      Ai = Aall / i;
      //h(k)+j <= alpha * A(tk)
      if(timesTable[i].time + job <= alpha * Ai){
        chosen = timesTable[i].recomponentnumber;
        timesTable[i].time += job;
        break;
      }
      Aall -= timesTable[i-1].time;
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

//add job to Lowest and resort
    vector<timesTable_entry> timesTableTemp = timesTable;
    timesTableTemp[0].time += job;
    sort(timesTableTemp.begin(), timesTableTemp.end());
    
    sc_time Ll = SC_ZERO_TIME;
    for(int l=0; l < i;l++){
      Ll += timesTableTemp[l].time;
    }

    sc_time Lh = SC_ZERO_TIME;
    for(int l=i; l < m;l++){
      Lh += timesTableTemp[l].time;
    }
//Make decision
    if( Ll <= alpha * Lh ){ //least loaded
      chosen = timesTable[0].recomponentnumber;
      timesTable[0].time += job;
      //std::cerr << "OnlineBinder> Albers Rule 1 " << std::endl;
    }else
    if( (timesTable[i].time + job > timesTable[m-1].time) && (timesTable[i].time + job > (c *(Ll+Lh)/m)) ){
      chosen = timesTable[0].recomponentnumber;
      timesTable[0].time += job;
      //std::cerr << "OnlineBinder> Albers Rule 2 " << std::endl;
    }else{
  //else //i+1 loaded
      chosen = timesTable[i].recomponentnumber;
      timesTable[i].time += job;
      //std::cerr << "OnlineBinder> Albers Rule 3 " << std::endl;
    }
    
//End Algorithm Albers
}else if(strcmp(algorithm,"Karger12") == 0){

    sort(timesTable.begin(), timesTable.end());
  
    double alpha = 1.2;
    sc_time job = getSetuptime(task) + getRuntime(task);
    
    chosen = -1;    
    sc_time Aall = SC_ZERO_TIME;
    for(int i = numberofcomp-1; i > 0; i--){
      Aall += timesTable[i].time;
    }
    sc_time Ai;
    for(int i = numberofcomp-1; i > 0; i--){
      Ai = Aall / i;
      //h(k)+j <= alpha * A(tk)
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
//End Algorithm Karger12
}else if(strcmp(algorithm,"Karger075") == 0){

    sort(timesTable.begin(), timesTable.end());
  
    double alpha = 0.75;
    sc_time job = getSetuptime(task) + getRuntime(task);
    
    chosen = -1;    
    sc_time Aall = SC_ZERO_TIME;
    for(int i = numberofcomp-1; i >= 0; i--){
      Aall += timesTable[i].time;
    }
    sc_time Ai;
    for(int i = numberofcomp-1; i >= 0; i--){
      Ai = Aall / (i+1);
      //h(k)+j <= alpha * A(tk)
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
//End Algorithm Karger075
}else if(strcmp(algorithm,"Bartal15") == 0){

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
    if( (timesTable[lowestLi].time + job) <= (1.5 * ARi) ){
      chosen = timesTable[lowestLi].recomponentnumber;
      timesTable[lowestLi].time += job;
    }else{
      chosen = timesTable[0].recomponentnumber;
      timesTable[0].time += job;
    }
//End Algorithm Bartal15

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
    RCWaitInterval = timesTable[lowestLi].time + lowestLi * getSetuptime(task);
#ifdef VPC_DEBUG    
    std::cerr << "OnlineBinder> RCWaitInterval: " << RCWaitInterval << std::endl;
#endif    
    timesTable[lowestLi].time = RCWaitInterval;
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

  chosen = -1;    
  sc_time Aall = SC_ZERO_TIME;
  for(int i = numberofcomp-2; i >= 0; i--){
    Aall += timesTable[i].time;
  }
  sc_time Ai;
  int zero_slots = 0;
  for(int i = numberofcomp-1; i > 0; i--){
    Ai = Aall / i;
    if(timesTable[i].time == SC_ZERO_TIME)
      ++zero_slots;
    //h(k)+j <= alpha * A(tk)
    if(timesTable[i].time + job <= alpha * Ai){
      chosen = timesTable[i].recomponentnumber;
    //RCWaitInterval        
      if(zero_slots != 0){
        RCWaitInterval = zero_slots * getSetuptime(task);
      }else{
        RCWaitInterval = timesTable[i].time + getSetuptime(task);
      }
#ifdef VPC_DEBUG      
      std::cerr << "OnlineBinder> RCWaitInterval: " << RCWaitInterval << std::endl;
#endif
      timesTable[i].time = RCWaitInterval;
      timesTable[i].time += job;
      break; 
    }
    Aall -= timesTable[i-1].time;
  }
  if(chosen == -1){
    chosen = timesTable[0].recomponentnumber;
    if(zero_slots != 0){
      RCWaitInterval = zero_slots * getSetuptime(task);
#ifdef VPC_DEBUG      
      std::cerr << "OnlineBinder> RCWaitInterval: " << RCWaitInterval << std::endl;
#endif
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

//add job to Lowest and resort
    vector<timesTable_entry> timesTableTemp = timesTable;
    timesTableTemp[0].time += job;
    sort(timesTableTemp.begin(), timesTableTemp.end());
    
    sc_time Ll = SC_ZERO_TIME;
    for(int l=0; l < i;l++){
      Ll += timesTableTemp[l].time;
    }

    sc_time Lh = SC_ZERO_TIME;
    for(int l=i; l < m;l++){
      Lh += timesTableTemp[l].time;
    }
//Make decision
    if( Ll <= alpha * Lh ){ //least loaded
      chosen = timesTable[0].recomponentnumber;
      /*RCWaitInterval = timesTable[0].time + getSetuptime(task);
      timesTable[0].time = RCWaitInterval;*/
      timesTable[0].time += job;
      
    }else
    if( (timesTable[i].time + job > timesTable[m-1].time) && (timesTable[i].time + job > (c *(Ll+Lh)/m)) ){
      chosen = timesTable[0].recomponentnumber;
      /*RCWaitInterval = timesTable[0].time + getSetuptime(task);
      timesTable[0].time = RCWaitInterval;*/
      timesTable[0].time += job;
    }else{
  //else //i+1 loaded
      chosen = timesTable[i].recomponentnumber;
      RCWaitInterval = timesTable[i].time + i * getSetuptime(task);
      timesTable[i].time = RCWaitInterval;
      timesTable[i].time += job;
    }
//End Algorithm AlbersMod

}else if(strcmp(algorithm,"KargerXMod") == 0){

  sort(timesTable.begin(), timesTable.end());
  
  double alpha = 0.75;
  sc_time job = getSetuptime(task) + getRuntime(task);

  chosen = -1;    
  sc_time Aall = SC_ZERO_TIME;
  for(int i = numberofcomp-1; i >= 0; i--){
    Aall += timesTable[i].time;
  }
  sc_time Ai;
  int zero_slots = 0;
  for(int i = numberofcomp-1; i >= 0; i--){
    Ai = Aall / (i+1);
    if(timesTable[i].time == SC_ZERO_TIME)
      ++zero_slots;
    //h(k)+j <= alpha * A(tk)
    if(timesTable[i].time + job <= alpha * Ai){
      chosen = timesTable[i].recomponentnumber;
    //RCWaitInterval        
      if(zero_slots != 0){
        RCWaitInterval = zero_slots * getSetuptime(task);
      }else{
        RCWaitInterval = timesTable[i].time + getSetuptime(task);
      }
#ifdef VPC_DEBUG      
      std::cerr << "OnlineBinder> RCWaitInterval: " << RCWaitInterval << std::endl;
#endif
      timesTable[i].time = RCWaitInterval;
      timesTable[i].time += job;
      break; 
    }
    Aall -= timesTable[i].time;
  }
  if(chosen == -1){
    chosen = timesTable[0].recomponentnumber;
    if(zero_slots != 0){
      RCWaitInterval = zero_slots * getSetuptime(task);
#ifdef VPC_DEBUG      
      std::cerr << "OnlineBinder> RCWaitInterval: " << RCWaitInterval << std::endl;
#endif
      timesTable[0].time += RCWaitInterval;
    }
    timesTable[0].time += job;
  }
//End Algorithm KargerXMod

}else if(strcmp(algorithm,"BartalXMod") == 0){
    
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
  if( (timesTable[lowestLi].time + job) <= (1.5 * ARi) ){
    chosen = timesTable[lowestLi].recomponentnumber;
    RCWaitInterval = timesTable[lowestLi].time + lowestLi * getSetuptime(task);
#ifdef VPC_DEBUG    
    std::cerr << "OnlineBinder> RCWaitInterval: " << RCWaitInterval << std::endl;
#endif    
    timesTable[lowestLi].time = RCWaitInterval;
    timesTable[lowestLi].time += job;
  }else{
    chosen = timesTable[0].recomponentnumber;
    timesTable[0].time += job;
  }
//End Algorithm BartalXMod
}else{
  std::cerr << std::endl << "OnlineBinder> Algorithm ";
  std::cerr << algorithm;
  std::cerr << " not found!" << std::endl << std::endl;
  exit(1);
  }
//**************************************************************************************************
//End all Algorithms
//**************************************************************************************************
    //scroll to chosen Recomponent
    Binding * RecomponentBindingChild = NULL;
    if(RecomponentBindingChildIter->hasNext())
        RecomponentBindingChild = RecomponentBindingChildIter->getNext();
    if(!RecomponentBindingChild){
        std::string msg = "OnlineBinder> No target specified for "+ task.getName() +"->?";
        throw UnknownBindingException(msg);
    }
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
    if(!myBinding){
        std::string msg = "OnlineBinder> No target specified for "+ task.getName() +"->?";
        throw UnknownBindingException(msg);
    }
    ChildIterator* myBindingChildIter = myBinding->getChildIterator();
    if(!myBindingChildIter){
        std::string msg = "OnlineBinder> No target specified for "+ task.getName() +"->?";
        throw UnknownBindingException(msg);
    }
    Binding* myBindingChild = NULL;
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
    if(!RecomponentBinding){
        std::string msg = "OnlineBinder> No target specified for "+ task.getName() +"->?";
        throw UnknownBindingException(msg);
    }
    ChildIterator* RecomponentBindingChildIter = RecomponentBinding->getChildIterator();
    if(!RecomponentBindingChildIter){
        std::string msg = "OnlineBinder> No target specified for "+ task.getName() +"->?";
        throw UnknownBindingException(msg);
    }
    Binding* RecomponentBindingChild = NULL;
    if(RecomponentBindingChildIter->hasNext())
      RecomponentBindingChild = RecomponentBindingChildIter->getNext();
    delete RecomponentBindingChildIter;
    if(!RecomponentBindingChild){
        std::string msg = "OnlineBinder> No target specified for "+ task.getName() +"->?";
        throw UnknownBindingException(msg);
    }
    //getMappingInformation
    MappingInformationIterator* MapInfoIter = RecomponentBindingChild->getMappingInformationIterator();
    MappingInformation* mInfo = NULL;
    if(MapInfoIter->hasNext())
      mInfo = MapInfoIter->getNext();
    delete MapInfoIter;
    if(!mInfo) return SC_ZERO_TIME;    
    else return mInfo->getDelay();
  }
  
}
