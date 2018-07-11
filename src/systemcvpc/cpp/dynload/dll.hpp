// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#ifndef _INCLUDED_SYSTEMCVPC_DYNLOAD_DLL_HPP
#define _INCLUDED_SYSTEMCVPC_DYNLOAD_DLL_HPP

#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#endif

#ifdef WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT extern "C"
#endif

namespace SystemC_VPC { namespace Detail {

//
// class DLLManager is the simple ELF C++ Library manager.
//
// It tries to dynamically load the specified shared library
// when it is construted.
//
// You should call LastError() before doing anything.  If it 
// returns NULL there is no error.
//

class DLLManager
{
 public:
	DLLManager( const char *fname );
	virtual ~DLLManager();


	bool GetSymbol( void **, const char *sym_name );
	
	static const char *GetDLLExtension();

	const char *LastError() 
	{
		 return err;
	}
	
 protected:
#ifdef WIN32
	HMODULE h;
#else
	void *h;
#endif
	char *err;
};


//
// class DLLFactoryBase is the base class used for the DLLFactory
// template class.  
// 
// It inherits from the DLLManager class and must be constructed with
// the file name of the shared library and the function name within that
// library which will create the desired C++ factory class.
// If you do not provide func_name to the constructor, it defaults to
// the undecorated "C" symbol "factory0"
//
// factory_func will be set to a pointer to the requested factory creator 
// function.  If there was an error linking to the shared library,
// factory_func will be 0.
//
// You can call 'LastError()' to find the error message that occurred.
//
//

class DLLFactoryBase : public DLLManager
{
 public:
	DLLFactoryBase(
		       const char *fname,
		       const char *func_name=0
		       );
		
	virtual ~DLLFactoryBase();
	
	void * (*factory_func)(void);	
};


//
// The DLLFactory template class inherits from DLLFactoryBase.
// The constructor takes the file name of the shared library
// and the undecorated "C" symbol name of the factory creator
// function.  The factory creator function in your shared library
// MUST either return a pointer to an object that is a subclass
// of 'T' or it must return 0.
//
// If everything is cool, then 'factory' will point to the
// requested factory class.  If not, it will be 0.
//
// Since the DLLFactory template ultimately inherits DLLManager,
// you can call LastError() to get any error code information
//
// The created factory is OWNED by the DLLFactory class.  
// The created factory will get deleted when the DLLFactory class
// is deleted, because the DLL will get unloaded as well.
//

template <class T>
class DLLFactory : public DLLFactoryBase
{
 public:
	DLLFactory(
		   const char *fname,
		   const char *func_name=0
		   ) : DLLFactoryBase( fname, func_name )
	{
		if( factory_func )
		  factory = (T *)factory_func();
		else 
		  factory = 0;
	}
	
	~DLLFactory()
	{
		delete factory;
	}

	T *factory;
};



} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DYNLOAD_DLL_HPP */
