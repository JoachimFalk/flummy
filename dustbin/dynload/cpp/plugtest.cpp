/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <systemcvpc/dynload/world.hpp>
#include <systemcvpc/dynload/dll.hpp>
#include <systemcvpc/dynload/plugin.hpp>

using namespace std;

int main(int argc, char *argv[])
{
	
	// Ask the user for the name of the DLL
	
 	string fname;
	std::cout << "Type in a full pathname of a dll:";
	std::cin >> fname;

	
	// show it
	
	std::cout << std::endl << "Opening '" << fname << "'" << std::endl;
	
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
		
		std::cout << "type name = " << typeid(*c).name() << std::endl;
		
		// kill the plug in.
		
		delete c;
	}
	else
	{
		std::cout << "Error opening dll!" << std::endl;
		
		// show the error message if any
		
        if( dll.LastError() )
		{
		    std::cout << "DLL Error: " << dll.LastError() << std::endl;
		}
		
	}

}
