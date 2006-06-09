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
#include "hscd_vpc_ReconfigurableComponent.h"

#include "hscd_vpc_AbstractController.h"
#include "hscd_vpc_AbstractBinder.h"
#include "hscd_vpc_AbstractConfigurationMapper.h"
#include "hscd_vpc_AbstractConfigurationScheduler.h"

#include "hscd_vpc_Configuration.h"

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
    XMLCh* recomponentStr;
    XMLCh* mappingStr;
    XMLCh* attributeStr;
    XMLCh* configurationStr;
    XMLCh* defaultConfStr;
    XMLCh* templateSectionStr;
    XMLCh* templateStr;
    XMLCh* refTemplateStr;
    XMLCh* controllerStr;
    XMLCh* binderStr;
    XMLCh* mapperStr;
    XMLCh* schedulerStr;
    XMLCh* timingStr;
    //XMLCh *Str;
   
    // tags for attributes 
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
    XMLCh *delayAttrStr;
    XMLCh *diiAttrStr;
    XMLCh *latencyAttrStr;
    XMLCh *fnameAttrStr;
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
    // map from all subComponents to their configs
    std::multimap<std::string, std::string > subComp_to_Config;
    // map from all configs to their parents components
    std::map<std::string, std::string > config_to_ParentComp;
    // map containing specified templates
    std::map<std::string, std::vector<std::pair<char*, char* > > > templates;

    //helper struct
    struct Timing{
      sc_time delay;
      sc_time latency;
      char*   fname;
    };
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
      recomponentStr  = XMLString::transcode("recomponent");
      mappingStr      = XMLString::transcode("mapping");
      attributeStr    = XMLString::transcode("attribute");
      configurationStr= XMLString::transcode("configuration");
      defaultConfStr  = XMLString::transcode("defaultconfiguration");
      templateSectionStr    = XMLString::transcode("templates");
      templateStr     = XMLString::transcode("template");
      refTemplateStr  = XMLString::transcode("reftemplate");
      timingStr       = XMLString::transcode("timing");
      controllerStr   = XMLString::transcode("controller");
      binderStr       = XMLString::transcode("binder");
      mapperStr       = XMLString::transcode("mapper");
      schedulerStr    = XMLString::transcode("scheduler");
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
      delayAttrStr        = XMLString::transcode("delay");
      diiAttrStr          = XMLString::transcode("dii");
      latencyAttrStr      = XMLString::transcode("latency");
      fnameAttrStr        = XMLString::transcode("fname");
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
     * \brief Initializes the Configurations of an ReconfigurableComponent
     * As long as there are defined Configurations, they will be register and
     * added to the given component.
     * \param comp represents the component for which to initialize the configurations
     * \param node specifies current position within dom tree
     */
    //void initConfigurations(ReconfigurableComponent* comp, DOMNode* node);
    void initConfigurations(ReconfigurableComponent* comp);
    
    /**
     * \brief Initializes one Configuration of an ReconfigurableComponent
     * As long as there are defined inner Components, they will be register and
     * added to the given Configuration.
     * \param comp represents the component for which to initialize the configurations
     * \param node specifies current postion within dom tree
     */
    //void initConfiguration(ReconfigurableComponent* comp, Configuration* conf, DOMNode* node);
    void initConfiguration(ReconfigurableComponent* comp, Configuration* conf);
    
    /**
     * \brief Initializes the Configuration switch times for a component
     * As long as there are defined switch times, they will be
     * added to the associated Controller of the ReconfigurableComponent.
     * \param comp represents the component for which to initialize the switch times
     */
    void initSwitchTimesOfComponent(ReconfigurableComponent* comp);
    
    /**
     * \brief Passes attributes of a specified template to a given component instance
     * \param comp represents the component to apply the attributes on
     * \param key references the key of the template to apply
     */
    void applyTemplateOnComponent(AbstractComponent* comp, std::string key);
    
    /**
     * \brief Interprets template for setting up parameter for a given ProcessControlBlock
     * \param mInfo represents the MappingInformation to be updated
     * \param target specifies the target of mapping
     * \param key references the key of the template to apply
     */
    void applyTemplateOnMappingInformation(MappingInformation& mInfo, const char* target, std::string key);
    
    /**
     * \brief Initializes mapping between tasks and components
     */
    //void initMappingAPStruct(DOMNode* node);
    void initMappingAPStruct();

    /**
     * \brief Used to build up bind hierarchy within vpc framework
     * This method is used to add corresponding binding information at each
     * level within the control hierarchy of vpc
     */
    void buildUpBindHierarchy(const char* source, const char* target, MappingInformation* mInfo);
    
    /**
     * \brief Generates controller instance for Component
     * \param id is the id to be set for the controller     
     */
    //AbstractController* generateController(const char* type, const char* id) throw(InvalidArgumentException);
    AbstractController* generateController(const char* id) throw(InvalidArgumentException);
   
    /**
     * \brief Generates controller instance for Component
     * \param id is the id to be set for the controller     
     */
    AbstractBinder* generateBinder(const char* type, DOMNode* node, Controller* controller)throw(InvalidArgumentException);
    
    /**
     * \brief Generates controller instance for Component
     * \param id is the id to be set for the controller     
     */
    AbstractConfigurationMapper* generateMapper(const char* type, DOMNode* node, Controller* controller)throw(InvalidArgumentException);
    
    /**
     * \brief Generates controller instance for Component
     * \param id is the id to be set for the controller     
     */
    AbstractConfigurationScheduler* generateConfigScheduler(const char* type, DOMNode* node, Controller* controller)throw(InvalidArgumentException);

    /**
     * \brief Takes a string representation of a time (e.g. a delay) and constructs a sc_time object.
     */
    sc_time createSC_Time(char* timeString) throw(InvalidArgumentException);

  };
    
}

#endif /*HSCD_VPC_VPCBUILDER_H_*/
