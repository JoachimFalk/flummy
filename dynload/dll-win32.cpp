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

#ifdef WIN32

#include <systemcvpc/dynload/world.hpp>
#include <systemcvpc/dynload/dll.hpp>


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


#endif /* WIN32 */
