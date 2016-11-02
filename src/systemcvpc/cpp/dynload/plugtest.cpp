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

#include <systemcvpc/dynload/world.hpp>
#include <systemcvpc/dynload/dll.hpp>
#include <systemcvpc/dynload/plugin.hpp>

using namespace std;

main()
{
	
	// Ask the user for the name of the DLL
	
 	string fname;
	cout << "Type in a full pathname of a dll:";
	cin >> fname;

	
	// show it
	
	cout << endl << "Opening '" << fname << "'" << endl;
	
	// Now create a DLLFactory for our PlugInFactory attached
	// to the requested file
	
	DLLFactory<PlugInFactory> dll( fname.c_str() );
	
	//
	// If it worked we should have dll.factory pointing
	// to a subclass of PlugInFactory
	//
	
	if( dll.factory )
	{
		//
		// yes, we have a factory. Ask the factory to create a
		// PlugIn object for us.
		//
		
		PlugIn *c=dll.factory->CreatePlugIn();
		
		// tell it to show itself
		
		c->Show();
		
		// show the C++ RTTI typeid name to the user
		
		cout << "type name = " << typeid(*c).name() << endl;
		
		// kill the plug in.
		
		delete c;
	}
	else
	{
		cout << "Error opening dll!" << endl;
		
		// show the error message if any
		
        if( dll.LastError() )
		{
		    cout << "DLL Error: " << dll.LastError() << endl;
		}
		
	}

}
