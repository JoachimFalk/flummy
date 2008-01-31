#include "hscd_vpc_OfflineBinder.h"

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
#ifdef VPC_DEBUG
    std::cerr << "OfflineBinder> Schedule filename: " << this->OfflineFileName << std::endl;
#endif
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
    ChildIterator* bIter = b->getChildIterator();
    while(bIter->hasNext()){
      b = bIter->getNext();
      if(b->getID() != recomp) continue;
        
      delete bIter;
      MappingInformationIterator* iter = b->getMappingInformationIterator();
      if(iter->hasNext()){
        MappingInformation* mInfo = iter->getNext();
        delete iter;
#ifdef VPC_DEBUG
  std::cerr << "OfflineBinder> Chose Mapping: "<< b->getID() << endl;
  std::cerr << mInfo->getDeadline() << endl;
  
#endif        
        return std::pair<std::string, MappingInformation*>(b->getID(), mInfo);
      }else{
        // also free iterator
        delete iter;
      }
    }
    delete bIter;
    std::string msg = "No target specified for "+ task.getName() +"->?";
    throw UnknownBindingException(msg);
  }

  void OfflineBinder::signalProcessEvent(ProcessControlBlock* pcb, std::string compID) {}
}
