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
#include "hscd_vpc_Component.h"

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
    /*
     * SECTION: init tag values for comparison while initializing
     */
    XMLCh* constraintStr;
    XMLCh* measurefileStr;
    XMLCh* resultfileStr;
    XMLCh* resourcesStr;
    XMLCh* mappingsStr;
    XMLCh* componentStr;
    XMLCh* mappingStr;
    XMLCh* attributeStr;
    XMLCh* templateSectionStr;
    XMLCh* templateStr;
    XMLCh* refTemplateStr;
    XMLCh* timingStr;
    XMLCh* parameterStr;
    XMLCh* topologyStr;
    XMLCh *hopStr;
    XMLCh *routeStr;
    //XMLCh *Str;
    
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
    //XMLCh *AttrStr;
    
    /*
     * END OF SECTION: init tag values for comparison while initializing
     */
    
    // walker over parsed configure file
    // used as instance variable to enable code modularization
    DOMTreeWalker* vpcConfigTreeWalker;
    
    
    /*
     * HELPER STRUCTURES FOR INITIALIZATION
     */
    // map of all created components
    std::map<std::string, AbstractComponent* > knownComps;
    // map containing specified templates
    std::map<std::string, std::vector<std::pair<char*, char* > > > templates;

    // map containing template Timings
    std::map<std::string, std::vector<Timing> > timingTemplates;

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
        cerr << VPC_ERROR<<"Director> Error initializing Xerces:\n"<<XMLString::transcode(e.getMessage())<<NENDL; // << endl;
      }
      /*
       * SECTION: initialization of init tag values for comparison while initializing
       */
      constraintStr   = XMLString::transcode("constraint");
      measurefileStr  = XMLString::transcode("measurefile");
      resultfileStr   = XMLString::transcode("resultfile");
      resourcesStr    = XMLString::transcode("resources");
      mappingsStr     = XMLString::transcode("mappings");
      componentStr    = XMLString::transcode("component");
      mappingStr      = XMLString::transcode("mapping");
      attributeStr    = XMLString::transcode("attribute");
      templateSectionStr    = XMLString::transcode("templates");
      templateStr     = XMLString::transcode("template");
      refTemplateStr  = XMLString::transcode("reftemplate");
      timingStr       = XMLString::transcode("timing");
      parameterStr    = XMLString::transcode("parameter");
      topologyStr     = XMLString::transcode("topology");
      hopStr          = XMLString::transcode("hop");
      routeStr        = XMLString::transcode("route");
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
     * \brief initializes specified templates
     * \param tid specifies the id for an template
     * \param specifies the current position within dom tree
     */
    //void initTemplateSpecifications(char* tid, DOMNode* node);
    void initTemplateSpecifications(char* tid);
    
    /**
     * \brief Performs initialization of attribute values for a component
     * \param comp specifies the component to set attributes for
     */
    //void initCompAttributes(AbstractComponent* comp, DOMNode* node);
    void initCompAttributes(AbstractComponent* comp);
    
    /**
     * \brief Passes attributes of a specified template to a given component instance
     * \param comp represents the component to apply the attributes on
     * \param key references the key of the template to apply
     */
    void applyTemplateOnComponent(AbstractComponent* comp, std::string key);
    
    /**
     * \brief Interprets template for setting up parameter for a given ProcessControlBlock
     * \param p represents the ProcessControlBlock to be updated
     * \param target specifies the target of mapping
     * \param key references the key of the template to apply
     */
    void applyTemplateOnPStruct(ProcessControlBlock* p, const char* target, std::string key);
    
    /**
     * \brief Initializes mapping between tasks and components
     */
    //void initMappingAPStruct(DOMNode* node);
    void initMappingAPStruct();

    /**
    * \brief Used to create the Attribute-Object recursively
    */
    void nextAttribute(Attribute &fr_Attribute, DOMNode* node);
     
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
