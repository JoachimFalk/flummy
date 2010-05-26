
#include "world.h"
#include "plugin.h"


// Class PlugInHappy inherits from PlugIn
// and shows the world when one is created/destroyed
// and supplies a Show() method that announces its name 

class PlugInHappy : public PlugIn
{
 public:
	PlugInHappy()
	{
		std::cout << "PlugInHappy() Created" << std::endl;
	}
	
	virtual ~PlugInHappy()
	{
		std::cout << "PlugInHappy() destroyed" << std::endl;
	}
	
	virtual void Show()
	{
		std::cout << "I am a New Happy PlugIn" << std::endl;
	}
};


//
// The PlugInHappyFactory class inherits from PlugInFactory
// and creates a PlugInHappy object when requested.
//

class PlugInHappyFactory : public PlugInFactory
{
 public:
	PlugInHappyFactory()
	{
	}
	
	~PlugInHappyFactory()
	{
	}
	
	virtual PlugIn * CreatePlugIn()
	{
		return new PlugInHappy;
	}
	
};


//
// The "C" linkage factory0() function creates the PlugInHappyFactory
// class for this library
//

DLL_EXPORT void * factory0( void )
{
	return new PlugInHappyFactory;
}

