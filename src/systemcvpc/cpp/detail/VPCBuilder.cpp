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

#include <systemcvpc/VpcApi.hpp>

#include "VPCBuilder.hpp"
#include "common.hpp"
#include "Director.hpp"
#include "DebugOStream.hpp"

#include <systemcvpc/ExecModelling/LookupPowerTimeModel.hpp>

#include <CoSupport/DataTypes/MaybeValue.hpp>
#include <CoSupport/Tracing/TracingFactory.hpp>

#include <boost/random/mersenne_twister.hpp> // for boost::mt19937

#include <boost/smart_ptr/shared_ptr.hpp>

#include <boost/filesystem.hpp>

#include <iostream>
#include <limits.h> 
#include <algorithm>
#include <cctype>
#include <string>
#include <stdlib.h>

#include <time.h>

#define MAX(x,y) ((x > y) ? x : y)

#include "vpc-dtd.c" // get DTD

namespace SystemC_VPC { namespace Detail {

  //using namespace CoSupport::XML::Xerces;
  namespace VC = SystemC_VPC;
  namespace fs = boost::filesystem;

  using CoSupport::DataTypes::MaybeValue;

  const char *VPCBuilder::STR_VPC_THREADEDCOMPONENTSTRING =  "threaded";
  const char *VPCBuilder::STR_VPC_DELAY =                    "delay";
  const char *VPCBuilder::STR_VPC_LATENCY =                  "latency";
  const char *VPCBuilder::STR_VPC_PRIORITY =                 "priority";
  const char *VPCBuilder::STR_VPC_PERIOD =                   "period";
  const char *VPCBuilder::STR_VPC_DEADLINE =                 "deadline";

  /**
   * Simple error handler deriviative to install on parser
   */
  class VpcDomErrorHandler : public xercesc::DOMErrorHandler {
  public:
    VpcDomErrorHandler() : failed(false){}
    ~VpcDomErrorHandler(){}

    /**
     * Implementation of the DOM ErrorHandler interface
     */
    bool handleError(const xercesc::DOMError& domError){
      std::cerr<< "DOMError";
      if (domError.getSeverity() == xercesc::DOMError::DOM_SEVERITY_WARNING){
          std::cerr << "\nWarning at file ";
      }else if (domError.getSeverity() == xercesc::DOMError::DOM_SEVERITY_ERROR){
          this->failed = true;
          std::cerr << "\nError at file ";
      }else{
          this->failed = true;
          std::cerr << "\nFatal Error at file ";
      }

      std::cerr << xercesc::XMLString::transcode( domError.getLocation()->getURI())
           << ", line " << domError.getLocation()->getLineNumber()
           << ", char " << domError.getLocation()->getColumnNumber()
           << "\n  Message: " << xercesc::XMLString::transcode( domError.getMessage())
           << std::endl;

      return !failed;

    }

    bool parseFailed(){
      return this->failed;
    }

  private :
    bool failed;
  };

  void testAndRemoveFile(std::string fileName){
    std::ofstream file(fileName.c_str());
    if (file.good()) {
      file.close();
      std::remove(fileName.c_str());
    }
  }

  VPCBuilder::VPCBuilder(Director *director)
    : FALLBACKMODE(false)
    , vpcConfigTreeWalker(NULL)
    , director(director)
  {
    handler.setTopElementName(XMLCH("vpcconfiguration"));
    handler.setDTDUrl(XMLCH("vpc.dtd"));
    handler.setDTD(vpcDTD, sizeof(vpcDTD));
  }

  VPCBuilder::~VPCBuilder() {};

  /**
   * \brief sets ups VPC Framework
   */
  void VPCBuilder::buildVPC(std::string const &vpcConfigFile) {
    if (!exists(fs::path(vpcConfigFile))) {
      std::cerr << "VPCBuilder> Warning: could not open '" << vpcConfigFile << "'" << std::endl;
      FALLBACKMODE=true;
    } else
      FALLBACKMODE=false;

    if (FALLBACKMODE) {
      this->director->FALLBACKMODE = true;
      DBG_OUT("running fallbackmode" << std::endl);
    } else { //!FALLBACKMODE
      DBG_OUT("VPCBuilder> VPCCONFIGURATION set to " << vpcConfigFile << std::endl);
      handler.load(vpcConfigFile.c_str());
      
      // set treewalker to documentroot
      vpcConfigTreeWalker =
        handler.getDocument()->createTreeWalker(
          handler.getDocument()->getDocumentElement(),
          CX::XN::DOMNodeFilter::SHOW_ELEMENT, 0,
          true);

      vpcConfigTreeWalker->setCurrentNode(
        handler.getDocument()->getDocumentElement());

      // set central seed for random number generation
      CX::XN::DOMNode *nodetest = handler.getDocument()->getDocumentElement();

      MaybeValue<double> seed = CX::getAttrValueAs<MaybeValue<double> >(nodetest, XMLCH("seed"));
      this->gen = boost::shared_ptr<boost::mt19937>(new boost::mt19937(
          seed.isDefined() ? seed.get() : time(NULL)));
      
      // moves the Treewalker to the first Child 
      CX::XN::DOMNode* node = vpcConfigTreeWalker->firstChild();
      // name of xmlTag
      while (node!=0) {
        const CX::XStr xmlName = node->getNodeName();
        
        // find distributions tag
        if( xmlName == XMLCH("distributions") ){

          DBG_OUT("VPCBuilder> processing distributions " << std::endl);
            
          node = vpcConfigTreeWalker->firstChild();
          if(node != NULL){ 
            //generate a distribution for each distribution tag
            for(; node!=0; node = this->vpcConfigTreeWalker->nextSibling()){
              this->initDistribution();
            }
        
            node = vpcConfigTreeWalker->parentNode();
          }

        } else if( xmlName == XMLCH("resources") ){
        
          // walk down hierachy to components
          node = vpcConfigTreeWalker->firstChild();
          if(node != NULL){
            // pointer to currently initiated component
            // init all components
            for(; node != NULL; node = vpcConfigTreeWalker->nextSibling()){
              const CX::XStr xmlName = node->getNodeName();
              try{
                if( xmlName == XMLCH("component") ) {
                  VC::Component::Ptr comp = initComponent();
                  DBG_OUT("VPCBuilder> register component: "
                          << comp->getName() << " to Director" << std::endl);


                }else{
                  if( xmlName == XMLCH("attribute") ){

                    const CX::XStr sType =
                      node->getAttributes()->getNamedItem(XMLCH("type"))->getNodeValue();
                    CX::XStr sValue = "";
                    CX::XN::DOMNode * value = node->getAttributes()->getNamedItem(XMLCH("value"));
                    if( value  != NULL){
                      sValue=
                        node->getAttributes()->getNamedItem(XMLCH("value"))->getNodeValue();
                    }

                    if( sType == "global_governor" ){
                      AttributePtr gov(new Attribute("global_governor",
                                                     sValue));
                      nextAttribute(gov, node->getFirstChild());
                      director->loadGlobalGovernorPlugin(sValue, gov);
                    }
                  }
                }
              }catch(InvalidArgumentException &e){
                std::cerr << "VPCBuilder> " << e.what() << std::endl;
                std::cerr << "VPCBuilder> ignoring specification of component,"
                  " going on with initialization" << std::endl;
                continue;
              }
            }

            node = vpcConfigTreeWalker->parentNode();
          }
        // find mappings tag (not mapping)
        }else if( xmlName == XMLCH("mappings") ){

          DBG_OUT("VPCBuilder> processing mappings " << std::endl);
            
          node = vpcConfigTreeWalker->firstChild();
          if(node != NULL){ 
            //foreach mapping of configuration perfom initialization  
            for(; node!=0; node = this->vpcConfigTreeWalker->nextSibling()){
              //this->initMappingAPStruct(node);
              this->initMappingAPStruct();
            }
        
            node = vpcConfigTreeWalker->parentNode();
          }

        }else if( xmlName == XMLCH("resultfile") ){
           
           CX::XN::DOMNamedNodeMap * atts=node->getAttributes();
           CX::XStr vpc_result_file =
             atts->getNamedItem(XMLCH("name"))->getNodeValue();
           this->director->setResultFile(vpc_result_file);
        
        }else if( xmlName == XMLCH("topology") ){
          node = vpcConfigTreeWalker->getCurrentNode();
          parseTopology( node );
        }else{
        
        }

        node = vpcConfigTreeWalker->nextSibling();
      }
    } //!FALLBACKMODE
    DBG_OUT("Initializing VPC finished!" << std::endl);
  }

  /**
   * \brief Initialize a distribution from the configuration file
   **/
  void VPCBuilder::initDistribution(){

    CX::XN::DOMNode* node = this->vpcConfigTreeWalker->getCurrentNode();
    
    CX::XStr xmlName=node->getNodeName();

    if( xmlName == XMLCH("distribution") ){
      CX::XN::DOMNamedNodeMap* atts=node->getAttributes();
      CX::NStr sName = atts->getNamedItem(XMLCH("name"))->getNodeValue();
      VC::createDistribution(sName,this->parseTimingModifier(node));
    }
  }

  /**
   * \brief Initialize a component from the configuration file
   * \return pointer to the initialized component
   * \throws InvalidArgumentException if requested component within
   * configuration file is unknown
   */
  VC::Component::Ptr VPCBuilder::initComponent() {
 
    CX::XN::DOMNode* node = this->vpcConfigTreeWalker->getCurrentNode();
    const CX::XStr xmlName = node->getNodeName(); 

    // check for component tag
    if( xmlName == XMLCH("component") ){
      
      CX::XN::DOMNamedNodeMap* atts = node->getAttributes();
  
      CX::XStr sScheduler =
        atts->getNamedItem(XMLCH("scheduler"))->getNodeValue();
      CX::XStr sName = atts->getNamedItem(XMLCH("name"))->getNodeValue();
  
      DBG_OUT("VPCBuilder> initComponent: " << sName << std::endl);

      VC::Component::Ptr comp =
          VC::createComponent(sName, VC::parseScheduler(sScheduler));

      this->initCompAttributes(comp);
        
      return comp;    
    }

    std::string msg("Unknown configuration tag: ");
    msg += CX::NStr(xmlName);
    throw InvalidArgumentException(msg);

  }
 
  /**
   * \brief Performs initialization of attribute values for a component
   * \param comp specifies the component to set attributes for
   * \param node specifies current position within dom tree
   */
  void VPCBuilder::initCompAttributes(VC::Component::Ptr comp){
    CX::XN::DOMNode* node = this->vpcConfigTreeWalker->firstChild();
    DBG_OUT("VPC> InitAttribute for Component name=" << comp->getName()
         << std::endl);
    if(node != NULL){
      // find all attributes
      for(; node != NULL; node = this->vpcConfigTreeWalker->nextSibling()){
        const CX::XStr xmlName = node->getNodeName();
        CX::XN::DOMNamedNodeMap * atts = node->getAttributes();

        // check if its an attribute to add
        if( xmlName == XMLCH("attribute") ){

          //CX::NStr sType;
          CX::NStr sValue = "";
          CX::NStr sType = atts->getNamedItem(XMLCH("type"))->getNodeValue();

          CX::XN::DOMNode * value = atts->getNamedItem(XMLCH("value"));
          if( value  != NULL){
            sValue = atts->getNamedItem(XMLCH("value"))->getNodeValue();
          }

          AttributePtr attributes(new Attribute( sType, sValue));

          nextAttribute(attributes, node->getFirstChild());

          // add distribution to the timing
          if (sType == "distribution"){
            comp->setTransferTimingModifier(getDistribution(sValue));
          }
          comp->addAttribute(attributes);
        }
      }
      vpcConfigTreeWalker->parentNode();
    }
    if (!comp->getExecModel())
      comp->setExecModel(createExecModel<VC::ExecModelling::LookupPowerTimeModel>());

  }

  /**
   * \brief Initializes the mappings and process structures.
   */
  //void VPCBuilder::initMappingAPStruct(DOMNode* node){
  void VPCBuilder::initMappingAPStruct(){

    CX::XN::DOMNode* node = this->vpcConfigTreeWalker->getCurrentNode();
    
    CX::XStr xmlName=node->getNodeName();

    DBG_OUT("VPCBuilder> entering initMappingAPStruct"<< std::endl);    
   
    // find mapping tag (not mappings)
    if( xmlName == XMLCH("mapping") ){
      CX::XN::DOMNamedNodeMap* atts=node->getAttributes();
      CX::NStr sTarget = atts->getNamedItem(XMLCH("target"))->getNodeValue();

      CX::NStr sSource = atts->getNamedItem(XMLCH("source"))->getNodeValue();


      DBG_OUT( "VPCBuilder> Found mapping attribute: source=" << sSource
               << " target=" << sTarget << std::endl); 

      {
        if(VC::hasComponent(sTarget)){
          //AbstractComponent* comp = iterComp->second;
          DBG_OUT( "VPCBuilder> Configure mapping: " << sSource << "<->"
                   << sTarget << std::endl); 

          VC::Component::Ptr comp = getComponent(sTarget);
          VC::VpcTask::Ptr   task = createTask(sSource);
          task->mapTo(comp);
          VC::ExecModel::Ptr execModel = comp->getExecModel();
          if (!execModel)
            throw ConfigException("\tComponent \"" + comp->getName()
                + "\" has NO execution model!");

          //walk down hierarchy to attributes
          CX::XN::DOMNode* attnode = node->getFirstChild();
          
          // find all attributes
          for(; attnode!=NULL; attnode = attnode->getNextSibling()){

            xmlName=attnode->getNodeName();
            CX::XN::DOMNamedNodeMap * atts=attnode->getAttributes();

            if( xmlName == XMLCH("timing") ){
              try {
                VC::Timing t = this->parseTiming( attnode );
                if (t.getFunction().empty()) {
                  execModel->addDefaultActorTiming(sSource, t);
                } else {
                  execModel->add(t);
                }

              } catch(InvalidArgumentException &e) {
                std::string msg("Error with mapping: ");
                msg += sSource + " -> " + sTarget + "\n";
                msg += e.what();
                throw InvalidArgumentException(msg);
              }
              
            }else if( xmlName == XMLCH("attribute") ){
              CX::XStr sType  = atts->getNamedItem(XMLCH("type"))->getNodeValue();
              CX::XStr sValue = atts->getNamedItem(XMLCH("value"))->getNodeValue();

              DBG_OUT("attribute values are: " <<sType
                      << " and " << sValue << std::endl);
          
              if( sType == STR_VPC_PRIORITY){
                int priority = 0;
                sscanf(CX::NStr(sValue), "%d", &priority);
                task->setPriority(priority);
              }else if( sType == STR_VPC_DEADLINE){
//                task->setDeadline(Director::createSC_Time(sValue));
              }else if( sType == STR_VPC_PERIOD){
//                task->setPeriod(Director::createSC_Time(sValue));
              }else if( sType == STR_VPC_DELAY){
//              sc_core::sc_time delay = Director::createSC_Time(sValue);
//                task->setBaseDelay( delay );
              }else if( sType == STR_VPC_LATENCY){
//              sc_core::sc_time latency = Director::createSC_Time(sValue);
//                task->setBaseLatency( latency );
              }else{
                DBG_OUT("VPCBuilder> Unknown mapping attribute: type="
                          << sType << " value=" << sValue << std::endl); 
                DBG_OUT("VPCBuilder> Try to interpret as function"
                  " specific delay!!" << std::endl);
              }
            }
          }
        }else{
          std::cerr << "VPCBuilder> No valid component found for mapping:"
            " source=" << sSource << " target=" << sTarget<< std::endl;
        }
      }
    }
  }

  void VPCBuilder::nextAttribute(SystemC_VPC::AttributePtr attribute,
                                 CX::XN::DOMNode* node){
    //walk down hierarchy to attributes
    for(; node != NULL; node = node->getNextSibling()){
      const CX::XStr xmlName = node->getNodeName();
      CX::XN::DOMNamedNodeMap * atts = node->getAttributes();

      // check if its an attribute to add
      if( xmlName == XMLCH("attribute") ){
        CX::XStr sValue="";
        CX::XStr sType = atts->getNamedItem(XMLCH("type"))->getNodeValue();
        if(atts->getNamedItem(XMLCH("value"))!=NULL){
          sValue = atts->getNamedItem(XMLCH("value"))->getNodeValue();
        }

        AttributePtr fr_Attribute2(new Attribute(sType, sValue));

        //fr_Attribute.addNewAttribute(fr_Attribute2, sValue);
        nextAttribute(fr_Attribute2,node->getFirstChild());
        attribute->addAttribute(sType, fr_Attribute2);
      }
      // check if its an Parameter to add
      if( xmlName == XMLCH("parameter") ){
        CX::XStr sType  = atts->getNamedItem(XMLCH("type"))->getNodeValue();
        CX::XStr sValue = atts->getNamedItem(XMLCH("value"))->getNodeValue();
        attribute->addParameter( sType, sValue);
      }
    }
  }

  void VPCBuilder::parseTopology(CX::XN::DOMNode *top ){
    try {
      // scan <topology>
      MaybeValue<bool> topologyTracing =
          CX::getMaybeAttrValueAs<bool>(top, XMLCH("tracing"));
      if (!topologyTracing.isDefined())
        topologyTracing = false;
      MaybeValue<std::string> defaultRoute =
          CX::getMaybeAttrValueAs<std::string>(top, XMLCH("default"));
      director->defaultRoute = defaultRoute.isDefined() &&
          defaultRoute == "ignore";

      // check if tracing is enabled for any route -> open trace file
      bool tracingEnabled = false;

      // Iterate over child tags of <topology>
      for(CX::XN::DOMNode * routeNode = top->getFirstChild();
          routeNode != NULL;
          routeNode = routeNode->getNextSibling()){
        
        CX::XStr const xmlName = routeNode->getNodeName();

        if (xmlName == XMLCH("route")) {
          // scan <route>
          MaybeValue<bool> tracing =
              CX::getMaybeAttrValueAs<bool>(routeNode, XMLCH("tracing"));
          if (!tracing.isDefined())
            tracing = topologyTracing;

          Route::Ptr  route;
          std::string type = CX::getAttrValueAs<std::string>(routeNode, XMLCH("type"));

          if (type ==  Routing::Static::Type) {
            route = parseStaticRoute(routeNode);
          } else {
            // FIXME: Support other routing types
            assert(type ==  Routing::Static::Type);
          }

          tracingEnabled |= tracing;
          route->setTracing(tracing);
        } else {
          assert(!"WTF?! Unknown tag in <topology>!");
        }
      }

      testAndRemoveFile("tracing.log");
      testAndRemoveFile("absolute_tracing.log");
      if (tracingEnabled) {
        CoSupport::Tracing::TracingFactory::getInstance().setTraceFile(
            "tracing.log");
        CoSupport::Tracing::TracingFactory::getInstance().setAbsoluteTraceFile(
                    "absolute_tracing.log");
      }

    }catch(InvalidArgumentException &e){
      std::cerr << "VPCBuilder> " << e.what() << std::endl;
      exit(-1);
    }
  }

  Routing::Static::Ptr VPCBuilder::parseStaticRoute(CX::XN::DOMNode *routeNode) {
    std::string name = CX::getAttrValueAs<std::string>(routeNode, XMLCH("name"));
    Routing::Static::Ptr route = createRoute<Routing::Static>(name);

    // Add <hop>s
    for (CX::XN::DOMNode *hopNode = routeNode->getFirstChild();
         hopNode != NULL;
         hopNode = hopNode->getNextSibling()) {

      CX::XStr const xmlName = hopNode->getNodeName();

      if (xmlName == XMLCH("hop")) {
        parseStaticHop(route.get(), nullptr, hopNode);
      } else {
        assert(!"WTF?! Unknown tag in <route>!");
      }
    }
    return route;
  }

  void VPCBuilder::parseStaticHop(
      Routing::Static      *route,
      Routing::Static::Hop *parentHop,
      CX::XN::DOMNode      *hopNode)
  {
    std::string hopComponent = CX::getAttrValueAs<std::string>(hopNode, XMLCH("component"));
    try {
      Component::Ptr comp = getComponent(hopComponent);

      Routing::Static::Hop *hop = route->addHop(comp, parentHop);

      // parse <timing>s
      for(CX::XN::DOMNode *node = hopNode->getFirstChild();
          node != NULL;
          node = node->getNextSibling()){
        CX::XStr const nodeName = node->getNodeName();
        if (nodeName == XMLCH("hop")) {
          parseStaticHop(route, hop, node);
        } else if (nodeName == XMLCH("desthop")) {
          MaybeValue<std::string> channel = CX::getMaybeAttrValueAs<std::string>(node, XMLCH("channel"));
          if (channel.isDefined())
            route->addDest(channel, hop);
          else
            route->addDest("DEFAULT", hop);
        } else if (nodeName == XMLCH("timing")) {
          VC::Timing t = this->parseTiming(node);
          hop->setTransferTiming(t);
        } else if (nodeName == XMLCH("attribute")) {
          std::string type = CX::getAttrValueAs<std::string>(node, XMLCH("type"));
          if (type == STR_VPC_PRIORITY) {
            hop->setPriority(CX::getAttrValueAs<int>(node, XMLCH("value")));
          } else {
            assert(!"WTF?! Unknown attribute in <hop>!");
          }
        } else {
          assert(!"WTF?! Unknown tag in <hop>!");
        }
      }
    } catch (std::exception const &e) {
      std::stringstream msg;
      msg << "Error with route " << route->getName() << " at " << hopComponent << ": " << e.what();
      throw ConfigException(msg.str().c_str());
    }
  }

  //
  VC::Timing VPCBuilder::parseTiming(CX::XN::DOMNode* node) {
    VC::Timing t;

    CX::XN::DOMNamedNodeMap* atts = node->getAttributes();
    if( NULL != atts->getNamedItem(XMLCH("powermode")) ) {
      t.setPowerMode(CX::NStr(atts->getNamedItem(XMLCH("powermode"))->getNodeValue()));
    }
    if( NULL != atts->getNamedItem(XMLCH("fname")) ) {
      CX::XStr attribute = atts->getNamedItem(XMLCH("fname"))->getNodeValue();
      t.setFunction(attribute);
    }

    CX::XN::DOMNode* dii     = atts->getNamedItem(XMLCH("dii"));
    CX::XN::DOMNode* delay   = atts->getNamedItem(XMLCH("delay"));
    CX::XN::DOMNode* latency = atts->getNamedItem(XMLCH("latency"));
    bool hasDii     = (dii != NULL);
    bool hasDelay   = (delay != NULL);
    bool hasLatency = (latency != NULL);
    if (hasDelay && !hasDii && !hasLatency) {
      sc_core::sc_time d = createSC_Time(CX::NStr(delay->getNodeValue()).c_str());
      t.setDii(d);
      t.setLatency(d);
    } else if (!hasDelay && hasDii && hasLatency) {
      t.setDii(createSC_Time(CX::NStr(dii->getNodeValue()).c_str()));
      t.setLatency(createSC_Time(CX::NStr(latency->getNodeValue()).c_str()));
    } else {
      std::string msg("Invalid timing annotation.\n");
      for(unsigned int i=0; i<atts->getLength(); i++){
        CX::XN::DOMNode* a=atts->item(i);
        CX::XStr val  = a->getNodeValue();
        CX::XStr name = a->getNodeName();
        msg += "timing: " + CX::NStr(name) + " = " + CX::NStr(val) + "\n";
      }
      msg += "Please specify values for dii and latency only. Alternatively, specify only the delay value. (E.g. use a delay when having identical values for dii and latency.)";

      throw InvalidArgumentException(msg);
    }

    //add the distribution to the timing if existant
    CX::XN::DOMNode* distribution = atts->getNamedItem(XMLCH("distribution"));
    bool hasDistribution = (distribution != NULL);
    if (hasDistribution){
      std::string distr = CX::NStr(distribution->getNodeValue());
      t.setTimingModifier(getDistribution(distr));
    }

    return t;
  }


  /**
   * \brief build a timingModifier from the configuration file
   * \param node the parent node the timingModififer is build from
   * \return pointer to the build timing modifier
   * \throws InvalidArgumentException if parameter are missing or invalid
   */
  boost::shared_ptr<DistributionTimingModifier> VPCBuilder::parseTimingModifier(CX::XN::DOMNode* node) {
    //parse attributes
    CX::XN::DOMNamedNodeMap* atts = node->getAttributes();
    CX::XN::DOMNode* min = atts->getNamedItem(XMLCH("min"));
    CX::XN::DOMNode* max = atts->getNamedItem(XMLCH("max"));
    CX::XN::DOMNode* parameter1 = atts->getNamedItem(XMLCH("parameter1"));
    CX::XN::DOMNode* parameter2 = atts->getNamedItem(XMLCH("parameter2"));
    CX::XN::DOMNode* parameter3 = atts->getNamedItem(XMLCH("parameter3"));
    CX::XN::DOMNode* seed = atts->getNamedItem(XMLCH("seed"));
    CX::XN::DOMNode* data = atts->getNamedItem(XMLCH("data"));
    CX::XN::DOMNode* scale = atts->getNamedItem(XMLCH("scale"));
    CX::XN::DOMNode* fixed = atts->getNamedItem(XMLCH("fixed"));
    CX::XN::DOMNode* base = atts->getNamedItem(XMLCH("base"));
    CX::XN::DOMNode* distribution = atts->getNamedItem(XMLCH("type"));
    bool hasMin = (min != NULL);
    bool hasMax = (max != NULL);
    bool hasParameter1 = (parameter1 != NULL);
    bool hasParameter2 = (parameter2 != NULL);
    bool hasParameter3 = (parameter3 != NULL);
    bool hasSeed = (seed != NULL);
    bool hasData = (data != NULL);
    bool hasScale = (scale != NULL);
    bool hasfixed = (fixed != NULL);
    bool hasBase = (base != NULL);
    std::string distr = CX::NStr(distribution->getNodeValue());

    //set min/max values 
    double minValue = -1;
    double maxValue = -1;
    if (hasMin){
      std::istringstream stm;
      stm.str(CX::NStr(min->getNodeValue()));
      stm >> minValue;
    }
    if (minValue<-1){
      minValue=0;
    }
    if (hasMax){
      std::istringstream stm;
      stm.str(CX::NStr(max->getNodeValue()));
      stm >> maxValue;
    }
    if (minValue >= maxValue){
      maxValue=-1;
    } 

    //create an extra randomgerator if an individual seed if given
    boost::shared_ptr<boost::mt19937> generator = this->gen;
    if (hasSeed){
      std::cout << "fixed seed found" << std::endl;
      std::istringstream stm;
      stm.str(CX::NStr(seed->getNodeValue()));
      double value;
      stm >> value;
      generator = boost::shared_ptr<boost::mt19937>(new boost::mt19937(value));
    }

    //set up an container for the result timingModifier and a marker whether a timingModifier was found
    boost::shared_ptr<DistributionTimingModifier> result;
    bool foundDistribution = false;

    //set up the timingModifier for the bernulli distribution
    if (distr.compare("bernoulli")==0){
      if (hasParameter1){
        std::istringstream stm;
        double param1;
        stm.str(CX::NStr(parameter1->getNodeValue()));
        stm >> param1;
         
        if (param1>=0 && param1<=1){

          //create the timingModifier
          result = boost::shared_ptr<DistributionTimingModifier>(new BernoulliTimingModifier(generator,param1,minValue,maxValue,hasfixed));
          foundDistribution = true;
        } else {
          throw InvalidArgumentException("invalid parameter for distribution");
        }
      } else {
        throw InvalidArgumentException("missing parameter for distribution");
      }
    }

    //set up the timingModifier for the binomial distribution
    else if (distr.compare("binomial")==0){
      if (hasParameter1 && hasParameter2){
        std::istringstream stm;
        int param1;
        double param2;
        stm.str(CX::NStr(parameter1->getNodeValue()));
        stm >> param1;
        std::istringstream stm2;
        stm2.str(CX::NStr(parameter2->getNodeValue()));
        stm2 >> param2;
         
        if (param2>=0 && param2<=1 && param1>=0){

          //create the timingModifier
          result = boost::shared_ptr<DistributionTimingModifier>(new BinomialTimingModifier(generator,param1,param2,minValue,maxValue,hasfixed));
          foundDistribution = true;
        } else {
          throw InvalidArgumentException("invalid parameter for distribution");
        }
      } else {
        throw InvalidArgumentException("missing parameter for distribution");
      }
    } 

    //set up the timingModifier for the cauchy distribution
    else if (distr.compare("cauchy")==0){
      if (hasParameter1 && hasParameter2){
        std::istringstream stm;
        double param1;
        double param2;
        stm.str(CX::NStr(parameter1->getNodeValue()));
        stm >> param1;
        std::istringstream stm2;
        stm2.str(CX::NStr(parameter2->getNodeValue()));
        stm2 >> param2;
         
        //create the timingModifier
        result = boost::shared_ptr<DistributionTimingModifier>(new CauchyTimingModifier(generator,param1,param2,minValue,maxValue,hasfixed));
        foundDistribution = true;
      } else {
        throw InvalidArgumentException("missing parameter for distribution");
      }

    //set up the timingModifier for the exponential distribution
    } else if (distr.compare("exponential")==0){
      if (hasParameter1){
        std::istringstream stm;
        double param1;
        stm.str(CX::NStr(parameter1->getNodeValue()));
        stm >> param1;
         
        if (param1>0){

          //create the timingModifier
          result = boost::shared_ptr<DistributionTimingModifier>(new ExponentialTimingModifier(generator,param1,minValue,maxValue,hasfixed));
          foundDistribution = true;
        } else {
          throw InvalidArgumentException("invalid parameter for distribution");
        }
      } else {
        throw InvalidArgumentException("missing parameter for distribution");
      }

    //set up the timingModifier for the exponential distribution
    } else if (distr.compare("gamma")==0){
      if (hasParameter1 && hasParameter2){
        std::istringstream stm;
        double param1;
        stm.str(CX::NStr(parameter1->getNodeValue()));
        stm >> param1;
        std::istringstream stm2;
        double param2;
        stm2.str(CX::NStr(parameter2->getNodeValue()));
        stm2 >> param2;
         
        if (param1>0 && param2>0){

          //create the timingModifier
          result = boost::shared_ptr<DistributionTimingModifier>(new GammaTimingModifier(generator,param1,param2,minValue,maxValue,hasfixed));
          foundDistribution = true;
        } else {
          throw InvalidArgumentException("invalid parameter for distribution");
        }
      } else {
        throw InvalidArgumentException("missing parameter for distribution");
      }
    }

    //set up the timingModifier for the geometric distribution
    else if (distr.compare("geometric")==0){
      if (hasParameter1){
        std::istringstream stm;
        double param1;
        stm.str(CX::NStr(parameter1->getNodeValue()));
        stm >> param1;
         
        if (param1>0 && param1<1){

          //create the timingModifier
          result = boost::shared_ptr<DistributionTimingModifier>(new GeometricTimingModifier(generator,param1,minValue,maxValue,hasfixed));
          foundDistribution = true;
        } else {
          throw InvalidArgumentException("invalid parameter for distribution");
        }
      } else {
        throw InvalidArgumentException("missing parameter for distribution");
      }
    }

    //set up the timingModifier for the lognormal distribution
    else if (distr.compare("lognormal")==0){
      if (hasParameter1 && hasParameter2){
        double param1;
        double param2;
        std::istringstream stm;
        stm.str(CX::NStr(parameter1->getNodeValue()));
        stm >> param1;
        std::istringstream stm2;
        stm2.str(CX::NStr(parameter2->getNodeValue()));
        stm2 >> param2;
         
        if (param1>0){

          //create the timingModifier
          result = boost::shared_ptr<DistributionTimingModifier>(new LognormalTimingModifier(generator,param1,param2,minValue,maxValue,hasfixed));
          foundDistribution = true;
        } else {
          throw InvalidArgumentException("invalid parameter for distribution");
        }
      } else {
        throw InvalidArgumentException("missing parameter for distribution");
      }
    } 

    //set up the timingModifier for the normal distribution
    else if (distr.compare("normal")==0){
      if (hasParameter1 && hasParameter2){
        double param1;
        double param2;
        std::istringstream stm;
        stm.str(CX::NStr(parameter1->getNodeValue()));
        stm >> param1;
        std::istringstream stm2;
        stm2.str(CX::NStr(parameter2->getNodeValue()));
        stm2 >> param2;
         
        if (param2>=0){

          //create the timingModifier
          result = boost::shared_ptr<DistributionTimingModifier>(new NormalTimingModifier(generator,param1,param2,minValue,maxValue,hasfixed));
          foundDistribution = true;
        } else {
          throw InvalidArgumentException("invalid parameter for distribution");
        }
      } else {
        throw InvalidArgumentException("missing parameter for distribution");
      }
    }

    //set up the timingModifier for the normal distribution
    else if (distr.compare("poisson")==0){
      if (hasParameter1){
        std::istringstream stm;
        double param1;
        stm.str(CX::NStr(parameter1->getNodeValue()));
        stm >> param1;
         
        if (param1>0){

          //create the timingModifier
          result = boost::shared_ptr<DistributionTimingModifier>(new PoissonTimingModifier(generator,param1,minValue,maxValue,hasfixed));
          foundDistribution = true;
        } else {
          throw InvalidArgumentException("invalid parameter for distribution");
        }
      } else {
        throw InvalidArgumentException("missing parameter for distribution");
      }
    }

    //set up the timingModifier for the triangle distribution
    else if (distr.compare("triangle")==0){
      if (hasParameter1 && hasParameter2 && hasParameter3){
        double param1;
        double param2;
        double param3;
        std::istringstream stm;
        stm.str(CX::NStr(parameter1->getNodeValue()));
        stm >> param1;
        std::istringstream stm2;
        stm2.str(CX::NStr(parameter2->getNodeValue()));
        stm2 >> param2;
        std::istringstream stm3;
        stm3.str(CX::NStr(parameter3->getNodeValue()));
        stm3 >> param3;
         
        if (param1<=param2 && param2<=param3){

          //create the timingModifier
          result = boost::shared_ptr<DistributionTimingModifier>(new TriangleTimingModifier(generator,param1,param2,param3,minValue,maxValue,hasfixed));
          foundDistribution = true;
        } else {
          throw InvalidArgumentException("invalid parameter for distribution");
        }
      } else {
        throw InvalidArgumentException("missing parameter for distribution");
      }
    }

    //set up the timingModifier for a distribution based on empiric data
    else if (distr.compare("empiric")==0){
      if (hasData && hasScale){

        //create the timingModifier
        result = boost::shared_ptr<DistributionTimingModifier>(new EmpiricTimingModifier(generator, createSC_Time(CX::NStr(scale->getNodeValue()).c_str()),CX::NStr(data->getNodeValue()),minValue,maxValue,hasfixed));
        foundDistribution = true;
      } else {
        throw InvalidArgumentException("invalid parameter for distribution");
      }
    }

    //set up the timingModifier for the uniform distribution
    else if (distr.compare("uniformReal")==0){
      if (hasParameter1 && hasParameter2){
        double param1;
        double param2;
        std::istringstream stm;
        stm.str(CX::NStr(parameter1->getNodeValue()));
        stm >> param1;
        std::istringstream stm2;
        stm2.str(CX::NStr(parameter2->getNodeValue()));
        stm2 >> param2;
        std::cout << param1 << "," << param2 << std::endl;
         
        if (param1<param2){

          //create the timingModifier
          result = boost::shared_ptr<DistributionTimingModifier>(new UniformRealTimingModifier(generator,param1,param2,minValue,maxValue,hasfixed));
          foundDistribution = true;
        } else {
          throw InvalidArgumentException("invalid parameter for distribution");
        }
      } else {
        throw InvalidArgumentException("missing parameter for distribution");
      }
    }

    //set the base and return the result if there is one
    if (foundDistribution == true) {
      if (hasBase) {
        std::cout << "base:";
        result->setBase(getDistribution(CX::NStr(base->getNodeValue())));
      }
      return result;
    }
    throw InvalidArgumentException("unknown distribution");
  }

} } // namespace SystemC_VPC::Detail
