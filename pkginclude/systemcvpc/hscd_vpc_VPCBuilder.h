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

#include <map>
#include <string>
#include <vector>

#include "hscd_vpc_AbstractComponent.h"

#include "hscd_vpc_InvalidArgumentException.h"

#include "Timing.h"
#include "Attribute.h"

XERCES_CPP_NAMESPACE_USE
namespace SystemC_VPC{

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
    XMLCh* measurefileStr;
    XMLCh* resultfileStr;
    XMLCh* resourcesStr;
    XMLCh* mappingsStr;
    XMLCh* componentStr;
    XMLCh* mappingStr;
    XMLCh* attributeStr;
    XMLCh* timingStr;
    XMLCh* parameterStr;
    XMLCh* topologyStr;
    XMLCh *hopStr;
    XMLCh *routeStr;
    XMLCh *powerModeStr;
    
    XMLCh* nameAttrStr;
    XMLCh* countAttrStr;
    XMLCh* typeAttrStr;
    XMLCh* dividerAttrStr;
    XMLCh* schedulerAttrStr;
    XMLCh* valueAttrStr;
    XMLCh* targetAttrStr;
    XMLCh* sourceAttrStr;
    XMLCh *delayAttrStr;
    XMLCh *diiAttrStr;
    XMLCh *latencyAttrStr;
    XMLCh *fnameAttrStr;
    XMLCh *destinationAttrStr;
    
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
        cerr << "Director> Error initializing Xerces:\n"
             << XMLString::transcode(e.getMessage()) << std::endl;
      }
      /*
       * SECTION: initialization of init tag values for comparison while initializing
       */
      resultfileStr   = XMLString::transcode("resultfile");
      resourcesStr    = XMLString::transcode("resources");
      mappingsStr     = XMLString::transcode("mappings");
      componentStr    = XMLString::transcode("component");
      mappingStr      = XMLString::transcode("mapping");
      attributeStr    = XMLString::transcode("attribute");
      timingStr       = XMLString::transcode("timing");
      parameterStr    = XMLString::transcode("parameter");
      topologyStr     = XMLString::transcode("topology");
      hopStr          = XMLString::transcode("hop");
      routeStr        = XMLString::transcode("route");
      powerModeStr    = XMLString::transcode("powermode");
      //XMLCh* VPCBuilder::Str = XMLString::transcode("");
      
      nameAttrStr    = XMLString::transcode("name");
      countAttrStr  = XMLString::transcode("count");
      typeAttrStr    = XMLString::transcode("type");
      dividerAttrStr  = XMLString::transcode("divider");
      schedulerAttrStr= XMLString::transcode("scheduler");
      valueAttrStr  = XMLString::transcode("value");
      targetAttrStr  = XMLString::transcode("target");
      sourceAttrStr  = XMLString::transcode("source");
      delayAttrStr        = XMLString::transcode("delay");
      diiAttrStr          = XMLString::transcode("dii");
      latencyAttrStr      = XMLString::transcode("latency");
      fnameAttrStr        = XMLString::transcode("fname");
      destinationAttrStr  = XMLString::transcode("destination");
      //XMLCh* VPCBuilder::AttrStr   = XMLString::transcode("");
      
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
    //AbstractComponent* initComponent(DOMNode* node) throw(InvalidArgumentException);
    AbstractComponent* initComponent() throw(InvalidArgumentException);
    
    /**
     * \brief Performs initialization of attribute values for a component
     * \param comp specifies the component to set attributes for
     */
    //void initCompAttributes(AbstractComponent* comp, DOMNode* node);
    void initCompAttributes(AbstractComponent* comp);
    
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
    Timing parseTiming(DOMNode* node);
  };
    
}

#endif /*HSCD_VPC_VPCBUILDER_H_*/
