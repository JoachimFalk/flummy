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

#ifndef HSCD_VPC_VPCBUILDER_H_
#define HSCD_VPC_VPCBUILDER_H_

#include <xercesc/dom/DOMTreeWalker.hpp>
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

#include <CoSupport/XML/xerces_support.hpp>

#include <map>
#include <string>
#include <vector>

#include "AbstractComponent.hpp"

#include "InvalidArgumentException.hpp"

#include "config/Component.hpp"
#include "config/Timing.hpp"
#include "Attribute.hpp"

XERCES_CPP_NAMESPACE_USE
namespace SystemC_VPC{
  namespace CX = CoSupport::XML::Xerces;
  class Director;

  /**
   * VPCBuilder sets up VPC framework through a given specification file before
   * simulation start.
   */
  class VPCBuilder{

    static const char* B_TRANSPORT;
    static const char* STATIC_ROUTE;
    static const char* STR_VPC_THREADEDCOMPONENTSTRING;
    static const char* STR_VPC_DELAY;
    static const char* STR_VPC_LATENCY;
    static const char* STR_VPC_PRIORITY;
    static const char* STR_VPC_PERIOD;
    static const char* STR_VPC_DEADLINE;

    /*
     * SECTION: init tag values for comparison while initializing
     */
    CX::XStr measurefileStr;
    CX::XStr resultfileStr;
    CX::XStr resourcesStr;
    CX::XStr mappingsStr;
    CX::XStr componentStr;
    CX::XStr mappingStr;
    CX::XStr attributeStr;
    CX::XStr timingStr;
    CX::XStr parameterStr;
    CX::XStr topologyStr;
    CX::XStr hopStr;
    CX::XStr routeStr;
    CX::XStr powerModeStr;
    
    CX::XStr nameAttrStr;
    CX::XStr countAttrStr;
    CX::XStr typeAttrStr;
    CX::XStr dividerAttrStr;
    CX::XStr schedulerAttrStr;
    CX::XStr valueAttrStr;
    CX::XStr targetAttrStr;
    CX::XStr sourceAttrStr;
    CX::XStr delayAttrStr;
    CX::XStr diiAttrStr;
    CX::XStr latencyAttrStr;
    CX::XStr fnameAttrStr;
    CX::XStr destinationAttrStr;
    CX::XStr tracingAttrStr;
    CX::XStr defaultRouteAttrStr;
    
    // walker over parsed configure file
    // used as instance variable to enable code modularization
    DOMTreeWalker* vpcConfigTreeWalker;
    
    /*
     * HELPER STRUCTURES FOR INITIALIZATION
     */
    // map of all created components
    std::map<std::string, AbstractComponent* > knownComps;

    // pointer to Director to be initialized
    Director* director;
    
  public:
    
    VPCBuilder(Director* director){
      
      this->director = director;
      //init xml
      try {
        XMLPlatformUtils::Initialize();
      }
      catch(const XMLException& e){
        std::cerr << "Director> Error initializing Xerces:\n"
             << e.getMessage() << std::endl;
      }
      /*
       * SECTION: initialization of init tag values for comparison while initializing
       */
      resultfileStr       = "resultfile";
      resourcesStr        = "resources";
      mappingsStr         = "mappings";
      componentStr        = "component";
      mappingStr          = "mapping";
      attributeStr        = "attribute";
      timingStr           = "timing";
      parameterStr        = "parameter";
      topologyStr         = "topology";
      hopStr              = "hop";
      routeStr            = "route";
      powerModeStr        = "powermode";
      nameAttrStr         = "name";
      countAttrStr        = "count";
      typeAttrStr         = "type";
      dividerAttrStr      = "divider";
      schedulerAttrStr    = "scheduler";
      valueAttrStr        = "value";
      targetAttrStr       = "target";
      sourceAttrStr       = "source";
      delayAttrStr        = "delay";
      diiAttrStr          = "dii";
      latencyAttrStr      = "latency";
      fnameAttrStr        = "fname";
      destinationAttrStr  = "destination";
      tracingAttrStr      = "tracing";
      defaultRouteAttrStr = "default";
      
      /*
       * END OF SECTION: init tag values for comparison while initializing
       */
    }
      
    ~VPCBuilder(){
      XMLPlatformUtils::Terminate();
    }
      
      
    void setDirector(Director* director){
      this->director = director;
    }
      
    bool FALLBACKMODE;
      
    /**
     * \brief Initializes VPC Framework using a configuration file
     */
    void buildVPC();
    
  private:
    
    /**
     * \brief Initialize a component from the configuration file
     * \return pointer to the initialized component
     */
    Config::Component::Ptr initComponent() throw(InvalidArgumentException);
    
    /**
     * \brief Performs initialization of attribute values for a component
     * \param comp specifies the component to set attributes for
     */
    void initCompAttributes(Config::Component::Ptr comp);
    
    /**
     * \brief Initializes mapping between tasks and components
     */
    //void initMappingAPStruct(DOMNode* node);
    void initMappingAPStruct();

    /**
    * \brief Used to create the Attribute-Object recursively
    */
    void nextAttribute(AttributePtr attributePtr, DOMNode* node);
     
    /**
    * \brief Topology parsing related code
    */
    void parseTopology(DOMNode* node);

    /**
    * \brief Parsing helper for <timing>
    */
    Config::Timing parseTiming(DOMNode* node) throw(InvalidArgumentException);
  };
    
}

#endif /*HSCD_VPC_VPCBUILDER_H_*/
