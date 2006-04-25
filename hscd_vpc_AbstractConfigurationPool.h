#ifndef HSCD_VPC_ABSTRACTCONFIGURATIONPOOL_H_
#define HSCD_VPC_ABSTRACTCONFIGURATIONPOOL_H_

#include <map>
#include <string>

#include "hscd_vpc_Configuration.h"
#include "hscd_vpc_InvalidArgumentException.h"

namespace SystemC_VPC {

  /**
   * \brief Abstract class specifying interface for a container managing configuration
   * instances
   */
  class AbstractConfigurationPool {
    
    public:

      AbstractConfigurationPool() {}

      virtual ~AbstractConfigurationPool() {}

      virtual void addConfiguration(Configuration* config)=0;

      virtual Configuration* getConfigByID(unsigned int id) throw(InvalidArgumentException)=0;

  };

}

#endif /*HSCD_VPC_ABSTRACTCONFIGURATIONPOOL_H_*/
