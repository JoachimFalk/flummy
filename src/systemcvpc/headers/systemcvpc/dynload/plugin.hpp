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

#ifndef __PLUGIN_H
#define __PLUGIN_H


#include "dll.hpp"


//
// PlugIn is an abstract class.
//
// This is an example plug in.  This plug in only has one method, Show(),
// which we will use to show its name.
//
//

class PlugIn
{
 public:
	PlugIn();
	
	virtual ~PlugIn();
	
	virtual void Show() = 0;	
};


//
// The is an example factory for plug ins. 
//
// This example factory only announces when it is created/destroyed and
// has the single abstract method CreatePlugIn() which returns a type 
// of plug in.
//
// In the real world, you may have multiple different classes in each
// shared library that are made to work together.  All these classes
// must be created by the Factory class.
//
// You may find it useful to have the objects that you create with
// the factory class be given a pointer to the factory class so
// they can create their own objects that they need, using the same
// factory class.  Compiler support of covariant return types is 
// real useful here.
//


class PlugInFactory
{
 public:
	PlugInFactory() 
	{
		std::cout << "PlugInFactory Created" << std::endl;
	}
	
	virtual ~PlugInFactory()
	{
		std::cout << "PlugInFactory Destroy" << std::endl;		
	}
	
	virtual PlugIn * CreatePlugIn() = 0;
	
};

#endif
