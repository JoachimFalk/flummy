#include "hscd_vpc_OfflineBinder.h"

//#define OFFLINEFILENAME "/home/killer/systemoc-top--k--0.6/Examples/benchmarks/schedule.cfg"

#define VPC_DEBUG
namespace SystemC_VPC {

  OfflineBinder::OfflineBinder(char * OfflineFileName) : StaticBinder() {
    this->OfflineFileName = OfflineFileName;
  }

  OfflineBinder::~OfflineBinder() {}

  std::pair<std::string, MappingInformation* > OfflineBinder::performBinding(ProcessControlBlock& task, ReconfigurableComponent* comp) throw(UnknownBindingException){
    Binding* b = NULL;
    if(comp == NULL){
      b = task.getBindingGraph().getRoot();
    }else{
      b = task.getBindingGraph().getBinding(comp->basename());
    }
#ifdef VPC_DEBUG
  if (comp != NULL) std::cerr << "OfflineBinder> Component: "<< comp->basename() <<"> Task: " << task.getName() << endl;
  if (comp == NULL) std::cerr << "OfflineBinder> Component: NULL"<<"> Task: " << task.getName() << endl;
#endif

//Statische Bindung, aus Datei einlesen
    std::cerr << "FILE:" << this->OfflineFileName << std::endl;
    OfflineFile *myFile = new OfflineFile(this->OfflineFileName);
    if(!myFile->open()){
      std::cerr << "OfflineBinder> Offlinefile open error" << std::endl;
    }
      // Lese Datei in Puffer und schliessen
      std::string buffer = myFile->getbuffer();
      myFile->close();
    
//Parse Puffer nach Werten für ComponentTable
    
    // find section SCHEDULE
    std::string::size_type position = buffer.find("SCHEDULE");
    if(position==string::npos) 
      std::cerr << "OfflineBinder> Offlinefile: No Schedule found in file" << std::endl;
    std::string schedule = buffer.substr(position, buffer.end()-buffer.begin());
    
    
    // find task name in section SCHEDULE
    std::string recomp;
    position = schedule.find(task.getName());
    if(position==string::npos){
      std::cerr << "OfflineBinder> Offlinefile: task " << task.getName() << " not found in schedule" << std::endl;
    }else{
#ifdef VPC_DEBUG
      std::cerr << "OfflineBinder> Offlinefile accepted line:" << std::endl;
      for (std::string::iterator scheduleiter = schedule.begin()+position; scheduleiter != schedule.end(); ++scheduleiter){
        if (*scheduleiter == '\n') {
          std::cerr << endl;
          break;
        }
        std::cerr << *scheduleiter;
      }
#endif
      // find component name for previous found task
      unsigned int compstart, compend;
      compstart = schedule.find(';', position+1);
      if(compstart==string::npos){
        std::cerr << "OfflineBinder> Offlinefile: parse error after position" << position << std::endl;
      }else{
        compend = schedule.find(';', compstart+1);
        if(compend==string::npos){
          std::cerr << "OfflineBinder> Offlinefile: parse error after position" << compstart << std::endl;
        }else{
          recomp = schedule.substr(compstart+1, compend-compstart-1);
          myFile->cleanstring(&recomp);
        }
      }
     }// end of task found in schedule
#ifdef VPC_DEBUG    
    std::cerr << "OfflineBinder> Offlinefile: Comp for Task "<< task.getName() << " should be '" << recomp << "'"<<std::endl;
#endif    
    int chosenComponent = 1; //Default
    if(recomp=="ESM-Slot1") chosenComponent = 1;
    if(recomp=="ESM-Slot2") chosenComponent = 2;
    if(recomp=="ESM-Slot3") chosenComponent = 3;
    
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
#ifdef VPC_DEBUG
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
