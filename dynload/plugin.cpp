
#include <typeinfo>
#include <iostream>
#include "plugin.h"

//
// Announce to the world that the PlugIn base
// class has been created or destroyed
//

PlugIn::PlugIn() 
{
	std::cout << "PlugIn Created" << std::endl;
}

PlugIn::~PlugIn()
{
	std::cout << "PlugIn Destroyed" << std::endl;	
}

