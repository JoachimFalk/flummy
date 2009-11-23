#ifndef __INCLUDED_PLUGGABLEPOWERGOVERNOR_H_
#define __INCLUDED_PLUGGABLEPOWERGOVERNOR_H_

#include "PowerGovernor.h"
#include "PowerMode.h"
#include "dynload/dll.h"

namespace SystemC_VPC{

  /**
   * base class for custom parameters
   */
  class GenericParameter{
  public:
    virtual ~GenericParameter();
  };

  /**
   * simple parameter containing only a PowerMode
   */
  class PowerModeParameter : public GenericParameter {
  public:
    const PowerMode      *powerMode;
    PowerModeParameter(PowerMode *mode);
    virtual ~PowerModeParameter();
  };

  /**
   * plug-in interface for GlobalPowerGovernor
   */
  class PluggableGlobalPowerGovernor :
    public GlobalPowerGovernor<GenericParameter *> {
  public:
    PluggableGlobalPowerGovernor()
    {}

    virtual ~PluggableGlobalPowerGovernor()
    {}

    virtual void notify_top(ComponentInfo *ci,
                            GenericParameter *param) = 0;
  };

  /**
   * plug-in interface for LocalPowerGovernor
   */
  class PluggableLocalPowerGovernor :
    public LocalPowerGovernor<GenericParameter *> 
  {
  public:
    PluggableLocalPowerGovernor() :
      LocalPowerGovernor<GenericParameter *> ()
    {}

    virtual ~PluggableLocalPowerGovernor()
    {}

    virtual void notify(ComponentInfo *ci) = 0;

  };

  /**
   * factory for plug-in creation
   */
  template<class PlugIn>
  class PlugInFactory {
  public:
    PlugInFactory() {
      //std::cout << "PlugInFactory Created" << std::endl;
    }
	
    virtual ~PlugInFactory() {
      //std::cout << "PlugInFactory Destroy" << std::endl;		
    }

    virtual void processAttributes(Attribute att) = 0;
	
    virtual PlugIn * createPlugIn() = 0;
  };

  /**
   * factory for plug-in creation
   */
  template<class ConcretePlugIn, class PlugIn>
  class ConcretePlugInFactory : public PlugInFactory<PlugIn> {
  public:
    ConcretePlugInFactory() {
      //std::cout << "Concrete__PlugInFactory Created" << std::endl;
    }
	
    virtual ~ConcretePlugInFactory() {
      //std::cout << "Concrete__PlugInFactory Destroy" << std::endl;		
    }
	
    virtual PlugIn * createPlugIn(){
      return new ConcretePlugIn();
    }
    virtual void processAttributes(Attribute att){
      //std::cerr << "Concrete__PlugInFactory::processAttributes" << std::endl;
    }
  };

}

#endif // __INCLUDED_PLUGGABLEPOWERGOVERNOR_H_
