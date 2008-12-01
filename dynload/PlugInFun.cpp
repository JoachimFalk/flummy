
#include <typeinfo>
#include <iostream>
#include "plugin.h"

// Class PlugInFun inherits from PlugIn
// and shows the world when one is created/destroyed
// and supplies a Show() method that announces its name 


class PlugInFun : public PlugIn
{
 public:
	PlugInFun()
	{
		std::cout << "PlugInFun created" << std::endl;
	}
	
	virtual ~PlugInFun()
	{
		std::cout << "PlugInFun destroyed" << std::endl;
	}
	
	virtual void Show()
	{
		std::cout << "I am a Fun PlugIn" << std::endl;
	}
};

//
// The PlugInFunFactory class inherits from PlugInFactory
// and creates a PlugInFun object when requested.
//


class PlugInFunFactory : public PlugInFactory
{
 public:
	PlugInFunFactory()
	{
	}
	
	~PlugInFunFactory()
	{
	}
	
	virtual PlugIn * CreatePlugIn()
	{
		return new PlugInFun;
	}
	
};


//
// The "C" linkage factory0() function creates the PlugInFunFactory
// class for this library
//

DLL_EXPORT void * factory0( void )
{
	return new PlugInFunFactory;
}


