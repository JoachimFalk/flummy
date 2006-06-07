#include "hscd_vpc_MIMapper.h"

#include "hscd_vpc_InvalidArgumentException.h"

namespace SystemC_VPC {

  MIMapper::MIMapper() {}

  MIMapper::~MIMapper() {}

  void MIMapper::addMappingInformation(std::string taskName, std::string compName, MappingInformation* mInfo){

    std::map<std::string, CompMapping* >::iterator iter;

    iter = this->mInfos.find(taskName);
    if(iter != this->mInfos.end()){
      //iter->second.insert(mInfo);
      (iter->second->cMap[compName]).insert(mInfo);
    }else{
      CompMapping* cM = new CompMapping();
      std::set<MappingInformation* > info;
      info.insert(mInfo);
      cM->cMap[compName] = info;
      this->mInfos[taskName] = cM;
    }

  }

  MappingInformationIterator* MIMapper::getMappingInformationIterator(const std::string& taskName, const std::string& compName){

    std::map<std::string, CompMapping* >::iterator iter;
    iter = this->mInfos.find(taskName);
    if(iter != this->mInfos.end()){
      return new MappingInformationIterator(&((iter->second)->cMap[compName]));
    }
    
    throw InvalidArgumentException("No mapping information for "+ taskName +" & "+ compName);

  }

}

