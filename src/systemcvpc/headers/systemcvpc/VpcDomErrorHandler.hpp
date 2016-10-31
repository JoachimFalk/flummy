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

#ifndef VPC_ERROR_HANDLER_H
#define VPC_ERROR_HANDLER_H

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

#endif
