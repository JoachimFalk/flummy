/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#include <typeinfo>
#include <iostream>
#include <systemcvpc/dynload/plugin.hpp>

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

