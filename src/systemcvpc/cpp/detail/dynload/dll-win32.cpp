// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2010 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#ifdef WIN32

#include <iostream>
#include "dll.hpp"

namespace SystemC_VPC { namespace Detail {

DLLManager::DLLManager( const char *fname )
{
    // Try to open the library now and get any error message.
	
	h = LoadLibrary(fname);
	
	if (h == NULL)
	{
		DWORD m;
		m = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					  FORMAT_MESSAGE_FROM_SYSTEM | 
					  FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL, 			/* Instance */
					  GetLastError(),   /* Message Number */
					  0,   				/* Language */
					  err,  			/* Buffer */
					  0,    			/* Min/Max Buffer size */
					  NULL);  			/* Arguments */
		std::cout << err << std::endl;
	}
	else
	{
		err = NULL;
	}
}

DLLManager::~DLLManager()
{
	// Free error string if allocated
	LocalFree(err);
	
	// close the library if it isn't null
	if (h != NULL)
    	FreeLibrary(h);
    	
}


bool DLLManager::GetSymbol( 
			   void **v,
			   const char *sym_name
			   )
{
	// try extract a symbol from the library
	// get any error message is there is any
	
	if( h!=0 )
	{
		*v = GetProcAddress(h, sym_name);
	    if (v != NULL)
       	  return true;
    	else
    	{
    		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					  FORMAT_MESSAGE_FROM_SYSTEM | 
					  FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL, 			/* Instance */
					  GetLastError(),   /* Message Number */
					  0,   				/* Language */
					  err,  			/* Buffer */
					  0,    			/* Min/Max Buffer size */
					  NULL);  			/* Arguments */
    		return false;
    	}
	}
	else
	{	
        return false;
	}
	
}

const char * DLLManager::GetDLLExtension()
{
	return ".dll";
}

DLLFactoryBase::DLLFactoryBase(
			       const char *fname,
			       const char *factory
			       ) : DLLManager(fname)
{
	// try get the factory function if there is no error yet
	
	factory_func=0;
	
	if( LastError()==0 )
	{		
    	GetSymbol( (void **)&factory_func, factory ? factory : "factory0" );
	}
	
}


DLLFactoryBase::~DLLFactoryBase()
{
}

} } // namespace SystemC_VPC::Detail

#endif /* WIN32 */
