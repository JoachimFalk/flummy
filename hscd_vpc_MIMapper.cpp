#include "hscd_vpc_MIMapper.h"

#include "hscd_vpc_InvalidArgumentException.h"

namespace SystemC_VPC {

  MIMapper::MIMapper() {}

  MIMapper::~MIMapper() {}

  void MIMapper::addMappingInformation(std::string key, MappingInformation* mInfo){

    std::map<std::string, std::set<MappingInformation* > >::iterator iter;

    iter = this->mInfos.find(key);
    if(iter != this->mInfos.end()){
      iter->second.insert(mInfo);
    }else{
      std::set<MappingInformation* > info;
      info.insert(mInfo);
      this->mInfos[key] = info;
    }

  }

  MappingInformationIterator* MIMapper::getMappingInformationIterator(std::string& key){

    std::map<std::string, std::set<MappingInformation* > >::iterator iter;
    iter = this->mInfos.find(key);
    if(iter != this->mInfos.end()){
      return new MappingInformationIterator(&(iter->second));
    }
    
    throw InvalidArgumentException("No mapping information for "+ key);

  }

}

