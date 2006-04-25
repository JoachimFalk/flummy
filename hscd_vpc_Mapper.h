#ifndef HSCD_VPC_MAPPER_H_
#define HSCD_VPC_MAPPER_H_

#include <map>

#include "hscd_vpc_UnkownMappingException.h"

namespace SystemC_VPC{
  
  template<class SRC, class TARGET>
  class Mapper{
    
    private:
      std::map<SRC, std::vector<TARGET> > mapping;
      
    public:
      
      Mapper(){}
      ~Mapper(){}
      
      void addMapping(SRC src, TARGET target){
        
        this->mapping[src].push_back(target);
        
      }
      
      TARGET & getMapping(SRC src) throw(UnkownMappingException){
        return this[src];
      }
      
      TARGET & operator[](SRC src) throw(UnkownMappingException){
        
        if(this->mapping.find(src) == this->mapping.end()){
          std::string msg = "Unkown mapping ";
          msg += src;
          msg += " to ??";
          throw UnkownMappingException(msg);
        }
        
        std::vector<TARGET>& v = this->mapping[src];
        return v.front();
      }
      
  };
  
}    
#endif /*HSCD_VPC_MAPPER_H_*/
