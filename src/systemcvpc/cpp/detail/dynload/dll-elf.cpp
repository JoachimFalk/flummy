// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
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

#ifndef WIN32

#include <iostream>

#include <dlfcn.h>

#include "dll.hpp"

namespace SystemC_VPC { namespace Detail {

DLLManager::DLLManager( const char *fname )
{
    // Try to open the library now and get any error message.
	
	h=dlopen( fname, RTLD_LAZY | RTLD_LOCAL);
	err=dlerror();
        if(err) std::cerr << err << std::endl;
}

DLLManager::~DLLManager()
{
	// close the library if it isn't null
	if( h!=0 )
    	dlclose(h);
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
          *v = dlsym( h, sym_name );
          err=dlerror();
          if(err) std::cerr << err << std::endl;
        
          if( err==0 )
            return true;
          else
            return false;
	}
	else
	{	
        return false;
	}
	
}


const char * DLLManager::GetDLLExtension()
{
	return ".so";
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

#endif /* ndef WIN32 */
