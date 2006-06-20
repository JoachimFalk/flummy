#ifndef HSCD_VPC_ABSTRACTCONFIGURATIONMAPPER_H_
#define HSCD_VPC_ABSTRACTCONFIGURATIONMAPPER_H_

#define MAPPERPREFIX "mapper_"

#include <map>
#include <string>

#include "hscd_vpc_Configuration.h"
#include "hscd_vpc_InvalidArgumentException.h"

namespace SystemC_VPC {

  /**
   * \brief Abstract class specifying interface for configuration mapper
   */
  class AbstractConfigurationMapper {

    public:

      AbstractConfigurationMapper() {}

      virtual ~AbstractConfigurationMapper() {}

      virtual void registerConfiguration(Configuration* config)=0;

      virtual unsigned int getConfigForComp(std::string compName) throw(InvalidArgumentException)=0;

      /**
       * \brief Used to set Mapper specific values
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if property value has been used else false
       */
      virtual bool setProperty(char* key, char* value)=0;

  };

}

#endif /*HSCD_VPC_ABSTRACTCONFIGURATIONMAPPER_H_*/
