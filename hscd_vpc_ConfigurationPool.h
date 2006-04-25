#ifndef HSCD_VPC_CONFIGRUATIONPOOL_H_
#define HSCD_VPC_CONFIGURATIONPOOL_H_

#include <map>
#include <string>

#include "hscd_vpc_AbstractConfigurationPool.h"
#include "hscd_vpc_AbstractConfigurationMapper.h"

namespace SystemC_VPC {

  /**
   * \brief Simple combined implementation of a AbstractConfigurationPool and a AbstractConfigurationMapper
   */
  class ConfigurationPool : public AbstractConfigurationPool, public AbstractConfigurationMapper {

    private:

      std::map<unsigned int, Configuration*> configs;
      std::map<std::string, unsigned int>  comp_to_config;

    public:

      ConfigurationPool();

      ~ConfigurationPool();

      /**
       * INTERFACE inherited from AbstractConfigurationPool
       */

      void addConfiguration(Configuration* config);

      Configuration* getConfigByID(unsigned int configID) throw(InvalidArgumentException);

      /**
       * INTERFACE inherited from AbstractConfigurationMapper
       */

      void registerConfiguration(Configuration* config);

      unsigned int getConfigForComp(std::string compName) throw(InvalidArgumentException);

      /**
       * \brief Used to set Mapper specific values
       * Dummy implementation does nothing.
       * \param key specifies the identy of the property
       * \param value specifies the actual value
       * \return true if property value has been used else false
       * \sa AbstractConfigurationMapper
       */
      bool setProperty(char* key, char* value);


  };

}

#endif //HSCD_VPC_CONFIGURATIONPOOL_H_
