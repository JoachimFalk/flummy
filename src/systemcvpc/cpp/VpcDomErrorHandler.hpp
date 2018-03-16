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

#ifndef _INCLUDED_SYSTEMCVPC_VPCDOMERRORHANDLER_HPP
#define _INCLUDED_SYSTEMCVPC_VPCDOMERRORHANDLER_HPP

#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <iostream>

//XERCES_CPP_NAMESPACE_USE
using namespace xercesc;

/**
 * Simple error handler deriviative to install on parser
 */
class VpcDomErrorHandler : public DOMErrorHandler {
public:
  VpcDomErrorHandler() : failed(false){}
  ~VpcDomErrorHandler(){}

  /**
   * Implementation of the DOM ErrorHandler interface
   */
  bool handleError(const DOMError& domError){
    std::cerr<< "DOMError";
    if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING){
        std::cerr << "\nWarning at file ";
    }else if (domError.getSeverity() == DOMError::DOM_SEVERITY_ERROR){
        this->failed = true;
        std::cerr << "\nError at file ";
    }else{
        this->failed = true;
        std::cerr << "\nFatal Error at file ";
    }
    
    std::cerr << XMLString::transcode( domError.getLocation()->getURI())
         << ", line " << domError.getLocation()->getLineNumber()
         << ", char " << domError.getLocation()->getColumnNumber()
         << "\n  Message: " << XMLString::transcode( domError.getMessage())
         << std::endl;
    
    return !failed;
    
  }
  
  bool parseFailed(){
    return this->failed;
  }

private :
  bool failed;
};

#endif /* _INCLUDED_SYSTEMCVPC_VPCDOMERRORHANDLER_HPP */
