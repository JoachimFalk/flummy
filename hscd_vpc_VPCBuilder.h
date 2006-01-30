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

#include <hscd_vpc_AbstractComponent.h>
#include <hscd_vpc_Component.h>
#include <hscd_vpc_ReconfigurableComponent.h>

#include <hscd_vpc_AbstractController.h>
#include <hscd_vpc_FCFSController.h>
#include <hscd_vpc_RoundRobinController.h>
#include <hscd_vpc_PriorityController.h>
#include <hscd_vpc_EDFController.h>

#include <hscd_vpc_Configuration.h>

XERCES_CPP_NAMESPACE_USE
namespace SystemC_VPC{

  class Director;

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
    XMLCh* configurationStr;
    XMLCh* switchtimesStr;
    XMLCh* switchtimeStr;
    XMLCh* defaultConfStr;
    //XMLCh *Str;
    
    XMLCh* nameAttrStr;
    XMLCh* countAttrStr;
    XMLCh* typeAttrStr;
    XMLCh* dividerAttrStr;
    XMLCh* schedulerAttrStr;
    XMLCh* valueAttrStr;
    XMLCh* targetAttrStr;
    XMLCh* sourceAttrStr;
    XMLCh* loadTimeAttrStr;
    XMLCh* storeTimeAttrStr;
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
    // map of all created configs
    std::map<std::string, Configuration* > knownConfigs;
    // map from all "virtualComponents" to their configs
    std::map<std::string, std::string > virtualComp_to_Config;
    // map from all configs to their components
      std::map<std::string, std::string > config_to_Comp;
    
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
      constraintStr  = XMLString::transcode("constraint");
      measurefileStr  = XMLString::transcode("measurefile");
      resultfileStr  = XMLString::transcode("resultfile");
      resourcesStr  = XMLString::transcode("resources");
      mappingsStr    = XMLString::transcode("mappings");
      componentStr  = XMLString::transcode("component");
      mappingStr    = XMLString::transcode("mapping");
      attributeStr  = XMLString::transcode("attribute");
      configurationStr= XMLString::transcode("configuration");
      switchtimesStr  = XMLString::transcode("switchtimes");
      switchtimeStr  = XMLString::transcode("switchtime");
      defaultConfStr  = XMLString::transcode("defaultconfiguration");
      //XMLCh* VPCBuilder::Str = XMLString::transcode("");
      
      nameAttrStr    = XMLString::transcode("name");
      countAttrStr  = XMLString::transcode("count");
      typeAttrStr    = XMLString::transcode("type");
      dividerAttrStr  = XMLString::transcode("divider");
      schedulerAttrStr= XMLString::transcode("scheduler");
      valueAttrStr  = XMLString::transcode("value");
      targetAttrStr  = XMLString::transcode("target");
      sourceAttrStr  = XMLString::transcode("source");
      loadTimeAttrStr  = XMLString::transcode("loadtime");
      storeTimeAttrStr= XMLString::transcode("storetime");
      
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
    AbstractComponent* initComponent();
    
    
    /**
     * \brief Performs initialization of attribute values for a component
     * \param comp specifies the component to set attributes for
     */
    void initCompAttributes(AbstractComponent* comp);
    
    /**
     * \brief Initializes the Configurations of an ReconfigurableComponent
     * As long as there are defined Configurations, they will be register and
     * added to the given component.
     * \param comp represents the component for which to initialize the configurations
     */
    void initConfigurations(ReconfigurableComponent* comp);
    
    /**
     * \brief Initializes one Configuration of an ReconfigurableComponent
     * As long as there are defined inner Components, they will be register and
     * added to the given Configuration.
     * \param comp represents the component for which to initialize the configurations
     */
    void initConfiguration(ReconfigurableComponent* comp, Configuration* conf);
    
    /**
     * \brief Initializes the Configuration switch times for a component
     * As long as there are defined switch times, they will be
     * added to the associated Controller of the ReconfigurableComponent.
     * \param comp represents the component for which to initialize the switch times
     */
    void initSwitchTimesOfComponent(ReconfigurableComponent* comp);
    
    void VPCBuilder::initMappingAPStruct();
    
    AbstractController* generateController(const char* type, const char* id);
    
  };
    
}

#endif /*HSCD_VPC_VPCBUILDER_H_*/
