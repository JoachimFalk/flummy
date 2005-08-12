/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Director.cpp
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>                               
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>                              
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <iostream>
#include <xercesc/util/XMLString.hpp>

#include "hscd_vpc_Term.h"

XERCES_CPP_NAMESPACE_USE  
namespace SystemC_VPC{



  class XmlHelper{
  public:
    static void xmlFillConstraint(Constraint *cons, DOMNode *node);
    static void xmlFillTerm(Term *term, DOMNode *node);


    /*    //static const XMLCh *XmlHelper::vpcconstraintsetStr;
    static XMLCh *XmlHelper::constraintStr;
    static XMLCh *XmlHelper::excludeStr;
    static XMLCh *XmlHelper::anytermStr;
    //static XMLCh *XmlHelper::Str;
  
    static XMLCh *XmlHelper::nameAttrStr;
    static XMLCh *XmlHelper::countAttrStr;
    static XMLCh *XmlHelper::processAttrStr;
    static XMLCh *XmlHelper::dividerAttrStr;
    static XMLCh *XmlHelper::stateAttrStr;
    */
  };
}
