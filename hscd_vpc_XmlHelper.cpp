/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_XmlHelper.cpp
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#include <systemcvpc/hscd_vpc_XmlHelper.h>
namespace SystemC_VPC{

  ///init xml chars
  //const XMLCh *XmlHelper::vpcconstraintsetStr = XMLString::transcode("vpcconstraintset"); //unused
  /*  
      XMLCh *XmlHelper::constraintStr  = XMLString::transcode("constraint");
  XMLCh *XmlHelper::excludeStr     = XMLString::transcode("exclude");
  XMLCh *XmlHelper::anytermStr     = XMLString::transcode("anyterm");
  //XMLCh *XmlHelper::Str = XMLString::transcode("");
  
  XMLCh *XmlHelper::nameAttrStr    = XMLString::transcode("name");
  XMLCh *XmlHelper::countAttrStr   = XMLString::transcode("count");
  XMLCh *XmlHelper::processAttrStr = XMLString::transcode("process");
  XMLCh *XmlHelper::dividerAttrStr = XMLString::transcode("divider");
  XMLCh *XmlHelper::stateAttrStr   = XMLString::transcode("state");
  */

  /**
   *
   */
  void XmlHelper::xmlFillConstraint(Constraint *cons, DOMNode *node){
    XMLCh *processAttrStr = XMLString::transcode("process");
    XMLCh *anytermStr     = XMLString::transcode("anyterm");
    XMLCh *stateAttrStr   = XMLString::transcode("state");
    XMLCh *excludeStr     = XMLString::transcode("exclude");

    while(node){
      if(node->getNodeType()==DOMNode::ELEMENT_NODE){
        if(0==XMLString::compareNString(node->getNodeName(),excludeStr,sizeof(excludeStr))){

          DOMNamedNodeMap *atts=node->getAttributes();
          char *excludename=XMLString::transcode(atts->getNamedItem(processAttrStr)->getNodeValue());
          cons->addExclude(excludename);
          XMLString::release(&excludename);//debug

        }else if(0==XMLString::compareNString(node->getNodeName(),anytermStr,sizeof(anytermStr))){

          DOMNamedNodeMap *atts=node->getAttributes();
          char *statename=XMLString::transcode(atts->getNamedItem(stateAttrStr)->getNodeValue());
          cons->addAnyTerm(statename);
          XMLString::release(&statename);//debug

        }  
      }

      node=node->getNextSibling();
    } 

  }

   /**
   *
   */
  void XmlHelper::xmlFillTerm(Term *term, DOMNode *node){}
}
