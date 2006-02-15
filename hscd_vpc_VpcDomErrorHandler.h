/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Director.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef VPC_ERROR_HANDLER_H
#define VPC_ERROR_HANDLER_H

#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <iostream>
using namespace std;

XERCES_CPP_NAMESPACE_USE
using namespace xercesc;

/**
 * Simple error handler deriviative to install on parser
 */
class VpcDomErrorHandler : public DOMErrorHandler {
public:
  VpcDomErrorHandler() : failed(false){}
  ~VpcDomErrorHandler(){}

  /**
   * Getter methods
   */
  //bool getSawErrors() {return fSawErrors;}

  /**
   * Implementation of the DOM ErrorHandler interface
   */
  bool handleError(const DOMError& domError){
    cerr<< VPC_ERROR << "DOMError";
    if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING){
        cerr << "\nWarning at file ";
    }else if (domError.getSeverity() == DOMError::DOM_SEVERITY_ERROR){
        this->failed = true;
        cerr << "\nError at file ";
    }else{
        this->failed = true;
        cerr << "\nFatal Error at file ";
    }
    
    cerr << XMLString::transcode( domError.getLocation()->getURI())
         << ", line " << domError.getLocation()->getLineNumber()
         << ", char " << domError.getLocation()->getColumnNumber()
         << "\n  Message: " << XMLString::transcode( domError.getMessage()) <<NORMAL<<endl;
    
    return !failed;
    
  }
  
  bool parseFailed(){
    return this->failed;
  }
  
  //void resetErrors();

private :
  bool failed;
  
  /**
   * Unimplemented constructor
   */
  //VpcDomErrorHandler(const VpcDomErrorHandler&);

  /**
   * Unimplemented operator
   */
  //void operator=(const VpcDomErrorHandler&);

  /**
   * This is set if we get any errors, and is queryable via a getter
   * method. Its used by the main code to suppress output if there are
   * errors.
   */
  //bool    fSawErrors;
};

#endif
