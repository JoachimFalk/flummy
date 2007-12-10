#include "hscd_vpc_OfflineBinder.h"

#include "hscd_vpc_ReconfigurableComponent.h"
#define DEBUG
namespace SystemC_VPC {

  OfflineBinder::OfflineBinder() : StaticBinder() {}

  OfflineBinder::~OfflineBinder() {}

  std::pair<std::string, MappingInformation* > OfflineBinder::performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp) throw(UnknownBindingException){
    Binding* b = NULL;
    if(comp == NULL){
      b = task.getBindingGraph().getRoot();
    }else{
      b = task.getBindingGraph().getBinding(comp->basename());
    }
#ifdef DEBUG
  if (comp != NULL) std::cerr << "OfflineBinder> Component: "<< comp->basename() <<"> Task: " << task.getName() << endl;
  if (comp == NULL) std::cerr << "OfflineBinder> Component: NULL"<<"> Task: " << task.getName() << endl;
#endif
    
    //Statische Bindung, aus Datei einlesen
    int chosenComponent = 1;
    if(task.getName()=="periodic.T1")chosenComponent = 3;
    if(task.getName()=="periodic.T2")chosenComponent = 3;
    if(task.getName()=="periodic.T3")chosenComponent = 1;
    if(task.getName()=="periodic.T4")chosenComponent = 1;
    if(task.getName()=="periodic.T5")chosenComponent = 2;
    if(task.getName()=="periodic.T6")chosenComponent = 2;
    
    ChildIterator* bIter = b->getChildIterator();
    if(bIter->hasNext()){
    
      // Set Chosen Component for task
      for(int i=0;i < chosenComponent;i++)
        b = bIter->getNext();
        
      delete bIter;
      MappingInformationIterator* iter = b->getMappingInformationIterator();
      if(iter->hasNext()){
        MappingInformation* mInfo = iter->getNext();
        delete iter;
#ifdef DEBUG
  std::cerr << "OfflineBinder> Chose Mapping: "<< b->getID() << endl;
#endif        
        return std::pair<std::string, MappingInformation*>(b->getID(), mInfo);
      }else{
        // also free iterator
        delete iter;
      }
    }

    std::string msg = "No target specified for "+ task.getName() +"->?";
    throw UnknownBindingException(msg);
  }

  void OfflineBinder::signalProcessEvent(ProcessControlBlock* pcb, std::string compID) {}
}
