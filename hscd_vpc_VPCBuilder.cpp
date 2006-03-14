#include <iostream>
#include <values.h> 

#include "hscd_vpc_VPCBuilder.h"
#include "hscd_vpc_XmlHelper.h"
#include "hscd_vpc_VpcDomErrorHandler.h"
#include "hscd_vpc_datatypes.h"

namespace SystemC_VPC{
   
  /**
   * \brief sets ups VPC Framework
   */
  void VPCBuilder::buildVPC(){

    // open file and check existence
    FILE* fconffile;
    char* cfile;
    char* vpc_evaluator_prefix = getenv("VPC_EVALUATOR");
    char vpc_conf_file[VPC_MAX_STRING_LENGTH];
    char* vpc_measure_file;
    
    FALLBACKMODE=false;
    
    // check if evaluator flag is set, in this case read given file
    // which ends with STR_VPC_CONGIF_FILE (see hscd_vpc_datatypes)
    if(vpc_evaluator_prefix){
      sprintf(vpc_conf_file,"%s%s",vpc_evaluator_prefix,STR_VPC_CONF_FILE);
#ifdef VPC_DEBUG
      cout <<"VPCBuilder> config found! File name is "<< vpc_conf_file << endl;
#endif //VPC_DEBUG
      cfile = vpc_conf_file;
    }else{
      cfile= getenv("VPCCONFIGURATION");
#ifdef VPC_DEBUG
      std::cerr << "VPCBuilder> VPCCONFIGURATION set to " << cfile << std::endl;
#endif //VPC_DEBUG
    }
    
    if(cfile){
      fconffile=fopen(cfile,"r");
      if( NULL == fconffile ){       // test if file exists
        assert( 0 == strncmp(cfile, "", sizeof("")) && "VPCCONFIGURATION is set, but points to nowhere" != NULL );  // for joachim
        FALLBACKMODE=true;
      }else{
        fclose(fconffile);
#ifdef VPC_DEBUG
        cout << "configuration: "<<cfile << endl;
#endif //VPC_DEBUG
      }
    }else{
      FALLBACKMODE=true;
    }
    
    // init vars for parsing
    if(FALLBACKMODE){
      FallbackComponent* fall=new FallbackComponent("Fallback-Component","FCFS");
      this->director->registerComponent(fall);
      this->director->FALLBACKMODE = true;
#ifdef VPC_DEBUG
      std::cout << "running fallbackmode" << std::endl;
#endif //VPC_DEBUG
    }else{
      // process xml
      DOMDocument* vpcConfigDoc;
      DOMBuilder* configParser;
      static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
      DOMImplementation* configImpl = DOMImplementationRegistry::getDOMImplementation(gLS);
      // create an error handler and install it
      VpcDomErrorHandler* configErrorh=new VpcDomErrorHandler();
      configParser = ((DOMImplementationLS*)configImpl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
      // turn on validation
      configParser->setFeature(XMLUni::fgDOMValidation, true);
      configParser->setErrorHandler(configErrorh);

      try {                                                             
        // reset document pool - clear all previous allocated data
        configParser->resetDocumentPool();                                    
        vpcConfigDoc = configParser->parseURI(cfile);
      }
      catch (const XMLException& toCatch) {
        std::cerr << "\nVPCBuilder> Error while parsing xml file: '" << cfile << "'\n"
          << "Exception message is:  \n"
          << XMLString::transcode( toCatch.getMessage()) << "\n" << endl;
          return;
      }
      catch (const DOMException& toCatch) {
        const unsigned int maxChars = 2047;
        XMLCh errText[maxChars + 1];
          
        std::cerr << "\nVPCBuilder> DOM Error while parsing xml file: '" << cfile << "'\n"
          << "DOMException code is:  " << XMLString::transcode( toCatch.msg) << endl;
          
        if (DOMImplementation::loadDOMExceptionMsg(toCatch.code, errText, maxChars))
        std::cerr << "Message is: " << XMLString::transcode( errText) << endl;
        return;
      }
      catch (...) {
        std::cerr << "\nVPCBuilder> Unexpected exception while parsing xml file: '" << cfile << "'\n";
        return;
      }
      
      //check if parsing failed
      if(configErrorh->parseFailed()){
        std::cerr << RED("VPCBuilder: Parsing of configuration failed, aborting initialization!") << std::endl;
        return;
      }
      
      // set treewalker to documentroot
      vpcConfigTreeWalker = vpcConfigDoc->createTreeWalker( (DOMNode*)vpcConfigDoc->getDocumentElement(), DOMNodeFilter::SHOW_ELEMENT, 0, true);
      vpcConfigTreeWalker->setCurrentNode( (DOMNode*)vpcConfigDoc->getDocumentElement());
      
      // moves the Treewalker to the first Child 
      DOMNode* node = vpcConfigTreeWalker->firstChild(); 
      // name of xmlTag
      const XMLCh* xmlName;
      
      while(node!=0){
        xmlName = node->getNodeName();
        
        // find resources tag
        if( 0==XMLString::compareNString( xmlName, resourcesStr, sizeof(resourcesStr) ) ){
        
          // walk down hierachy to components
          node = vpcConfigTreeWalker->firstChild();
          // pointer to currently initiated component
          AbstractComponent* comp;
          // init all components
          for(; node!=0; node = vpcConfigTreeWalker->nextSibling()){
            try{
              comp = initComponent();
            }catch(InvalidArgumentException &e){
              std::cerr << "VPCBuilder> " << e.what() << std::endl;
              std::cerr << "VPCBuilder> ignoring specification of component, going on with initialization" << std::endl;
              continue;
            }
            
#ifdef VPC_DEBUG
            std::cout << "registering component: "<< comp->basename() << " to Director" << endl;
#endif //VPC_DEBUG
            // register "upper-layer" components to Director
            this->director->registerComponent(comp);
            comp->setParentController(this->director);
           
          }
          
          node = vpcConfigTreeWalker->parentNode();
          
        // find mappings tag (not mapping)
        }else if( 0==XMLString::compareNString( xmlName, mappingsStr, sizeof(mappingsStr) ) ){

          node = vpcConfigTreeWalker->firstChild();
          
          //foreach mapping of configuration perfom initialization  
          while(node!=0){
            this->initMappingAPStruct();
            node = vpcConfigTreeWalker->nextSibling();
          }
        
          node = vpcConfigTreeWalker->parentNode();
        
        // find measure file declaration and store value for later use
        }else if( 0==XMLString::compareNString( xmlName, measurefileStr, sizeof(measurefileStr) ) ){
           
           DOMNamedNodeMap * atts=node->getAttributes();
            vpc_measure_file = XMLString::transcode(atts->getNamedItem(nameAttrStr)->getNodeValue());
        
        }else if( 0==XMLString::compareNString( xmlName, resultfileStr, sizeof(resultfileStr) ) ){
           
           DOMNamedNodeMap * atts=node->getAttributes();
            std::string vpc_result_file = XMLString::transcode(atts->getNamedItem(nameAttrStr)->getNodeValue());
            this->director->setResultFile(vpc_result_file);
            remove(vpc_result_file.c_str());
        
        }else{
        
        }

        node = vpcConfigTreeWalker->nextSibling();
      }

      // clean up pareser
      configParser->release();
      delete configErrorh;
      
#ifdef VPC_DEBUG 
      std::cerr << "VPCBuilder> finished initialization of components" << std::endl;
      std::cerr << "VPCBuilder> starting parsing of measurefile" << std::endl;
#endif // VPC_DEBUG

      std::cerr << "measure_file: "<< vpc_measure_file << endl;
      if(!vpc_measure_file){
          cerr << VPC_ERROR << "No vpc_measure_file"<< NENDL; //<< endl;
          return;
      }else{
        ifstream f;
        f.open(vpc_measure_file);
        if(!f) {
          cerr << "Warning: " << vpc_measure_file << " does not exists!" << endl; 
         return;
       }
       f.close();
      }
      
      DOMTreeWalker *vpc_measure_TreeWalker;
      DOMDocument *vpc_measure_doc;
      DOMBuilder *parser;
      //static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
      DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
      // create an error handler and install it
      VpcDomErrorHandler* errorh=new VpcDomErrorHandler();
      parser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
      // turn on validation
      parser->setFeature(XMLUni::fgDOMValidation, true);
      parser->setErrorHandler(errorh);
  
      try {                                                             
        // reset document pool - clear all previous allocated data
        parser->resetDocumentPool();                                    
        vpc_measure_doc = parser->parseURI(vpc_measure_file);
      }
      catch (const XMLException& toCatch) {
        std::cerr << "\nError while parsing xml file: '" << vpc_measure_file << "'\n"
          << "Exception message is:  \n"
          << XMLString::transcode( toCatch.getMessage()) << "\n" << endl;
          return;
      }
      catch (const DOMException& toCatch) {
        const unsigned int maxChars = 2047;
        XMLCh errText[maxChars + 1];
        
        std::cerr << "\nDOM Error while parsing xml file: '" << vpc_measure_file << "'\n"
          << "DOMException code is:  " << XMLString::transcode( toCatch.msg) << endl;
        
        if (DOMImplementation::loadDOMExceptionMsg(toCatch.code, errText, maxChars))
          std::cerr << "Message is: " << XMLString::transcode( errText) << endl;
         
        return;
      }
      catch (...) {
        std::cerr << "\nUnexpected exception while parsing xml file: '" << vpc_measure_file << "'\n";
        return;
      }
      
      //check if parsing failed
      if(errorh->parseFailed()){
        std::cerr << RED("VPCBuilder> Parsing of measure file " << vpc_measure_file << " failed, aborting initialization!") << std::endl;
        return;
      }
      // set treewalker to documentroot
      vpc_measure_TreeWalker = vpc_measure_doc->createTreeWalker( (DOMNode*)vpc_measure_doc->getDocumentElement(), DOMNodeFilter::SHOW_ELEMENT, 0, true);
        
      vpc_measure_TreeWalker->setCurrentNode( (DOMNode*)vpc_measure_doc->getDocumentElement());
      
      DOMNode *n;
      
      // moves the Treewalker to the first Child 
      n = vpc_measure_TreeWalker->firstChild();
      char *name;
      const XMLCh *xname;
      while( n) {
        xname=n->getNodeName();
        name=XMLString::transcode(xname); // for cerr only
        //cerr << RED(name)<< endl;
          
        if(n->getNodeType()==DOMNode::ELEMENT_NODE && 
          0==XMLString::compareNString(xname,constraintStr,sizeof(constraintStr))){
            
          DOMNamedNodeMap * atts=n->getAttributes();
          //cerr << GREEN(XMLString::transcode(atts->getNamedItem(nameAttrStr)->getNodeValue()) ) << NENDL;
          char *sCount,*sDivider,*sName;
          sName=XMLString::transcode(atts->getNamedItem(nameAttrStr)->getNodeValue());
          sCount=XMLString::transcode(atts->getNamedItem(countAttrStr)->getNodeValue());
          sDivider=XMLString::transcode(atts->getNamedItem(dividerAttrStr)->getNodeValue());
            
          Constraint* cons=new Constraint(sName,sCount,sDivider);
          XmlHelper::xmlFillConstraint(cons,n->getFirstChild());
          this->director->addConstraint(cons);
        }
        
        //DOMNode *last = n;
        //vpc_measure_TreeWalker->setCurrentNode( last);
        n = vpc_measure_TreeWalker->nextSibling();
      }
  
      XMLString::release(&name);
      parser->release();
      delete errorh;
      
    }// else !FALLBACK
#ifdef VPC_DEBUG    
    std::cerr << "Initializing VPC finished!" << std::endl;
#endif //VPC_DEBUG
  }

  /**
   * \brief Initialize a component from the configuration file
   * \return pointer to the initialized component
   */
  AbstractComponent* VPCBuilder::initComponent() throw(InvalidArgumentException){
    
    DOMNode* node = this->vpcConfigTreeWalker->getCurrentNode();
    const XMLCh* xmlName = node->getNodeName(); 

      //find all component tags
    if( 0==XMLString::compareNString( xmlName, VPCBuilder::componentStr, sizeof(VPCBuilder::componentStr))){
      
      DOMNamedNodeMap* atts = node->getAttributes();
      char* sName;
      char* sType;
      char* sScheduler;
      AbstractComponent* comp = NULL;
  
      sName = XMLString::transcode(atts->getNamedItem(VPCBuilder::nameAttrStr)->getNodeValue());
      sType = XMLString::transcode(atts->getNamedItem(VPCBuilder::typeAttrStr)->getNodeValue());
      sScheduler = XMLString::transcode(atts->getNamedItem(VPCBuilder::schedulerAttrStr)->getNodeValue());
  
#ifdef VPC_DEBUG
      std::cerr << "VPCBuilder> initComponent: " << sName << std::endl;
#endif //VPC_DEBUG

      // check which kind of component is defined
      if(0==strncmp(sType, STR_VPC_THREADEDCOMPONENTSTRING, sizeof(STR_VPC_THREADEDCOMPONENTSTRING))){

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Found Component name=" << sName << " type=" << sType << endl;
#endif //VPC_DEBUG

        comp = new Component(sName,sScheduler);
        this->knownComps.insert(pair<string, AbstractComponent* >(sName, comp));
        this->initCompAttributes(comp);
        
      }else if(0==strncmp(sType, STR_VPC_RECONFIGURABLECOMPONENTSTRING, sizeof(STR_VPC_RECONFIGURABLECOMPONENTSTRING))){

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Found Component name=" << sName << " type=" << sType << endl;
#endif //VPC_DEBUG
        AbstractController* controller;
        try{
          controller = this->generateController(sScheduler, sName);
        }catch(InvalidArgumentException &e){
          std::cerr << "VPCBuilder> Error: " << e.what() << std::endl;
          std::cerr << "VPCBuilder> setting default FCFS for "<< sName << std::endl;
          controller = this->generateController("FCFS", sName);
        }catch(...){
          std::cerr << "VPCBuilder> unkown exception occured while creating controller instance" << std::endl;
          throw;
        }
        
        comp = new ReconfigurableComponent(sName, controller);
        
        this->knownComps.insert(pair<string, AbstractComponent* >(sName, comp));
        this->initConfigurations((ReconfigurableComponent*)comp);
        this->initCompAttributes(comp);

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Initialized Component name=" << sName << " type=" << sType << endl;
#endif //VPC_DEBUG
      
      }else{
        
        string msg("Unknown Component: name=");
        msg += sName;
        msg += " type=";
        msg += sType;
        throw InvalidArgumentException(msg);
        //cerr << "VPCBuilder> Found unknown Component type: name=" << sName << " type=" << sType << endl;
        
      }

      /*
       * register component for internal use for parsing wihtin VPCBuilder
       */
      this->knownComps.insert(std::pair<std::string, AbstractComponent* >(comp->basename(), comp));
      
      return comp;    
    }

    return NULL;

  }
  
  /**
   * \brief Performs initialization of attribute values for a component
   * \param comp specifies the component to set attributes for
   */
  void VPCBuilder::initCompAttributes(AbstractComponent* comp){

#ifdef VPC_DEBUG
        cerr << "VPC> InitAttribute for Component name=" << comp->basename() << endl;
#endif //VPC_DEBUG

    DOMNode* node = vpcConfigTreeWalker->firstChild();
    if( node != NULL ){
      // find all attributes
      while( node != NULL ){

        const XMLCh* xmlName = node->getNodeName();
        // check if its an attribute to add
        if( 0==XMLString::compareNString( xmlName, attributeStr, sizeof(attributeStr))){
          DOMNamedNodeMap * atts = node->getAttributes();
          char* sType;
          char* sValue;
          sType = XMLString::transcode(atts->getNamedItem(typeAttrStr)->getNodeValue());
          sValue = XMLString::transcode(atts->getNamedItem(valueAttrStr)->getNodeValue());
          comp->processAndForwardParameter(sType,sValue);
        }
        node = vpcConfigTreeWalker->nextSibling();

      }

      // set back walker to upper level
      vpcConfigTreeWalker->parentNode();
    }
    
  }

  /**
   * \brief Initializes the Configurations of an ReconfigurableComponent
   * As long as there are defined Configurations, they will be register and
   * added to the given component.
   * \param comp represents the component for which to initialize the configurations
   */
  void VPCBuilder::initConfigurations(ReconfigurableComponent* comp){

#ifdef VPC_DEBUG
      std::cout << "VPCBuilder> entering initConfiguration"<< endl;
#endif //VPC_DEBUG

    DOMNode* node = vpcConfigTreeWalker->firstChild();
    const XMLCh* xmlName;
    
    while( node != NULL ){
      
      xmlName = node->getNodeName();
      
      // as long as there are exisiting configuration register them to component
      if( 0==XMLString::compareNString( xmlName, VPCBuilder::configurationStr, sizeof(VPCBuilder::configurationStr))){
        
        //create and initialize configuration
        DOMNamedNodeMap* atts=node->getAttributes();
        char* sName;
        char* sLoadTime;
        char* sStoreTime;
        // read values
        sName = XMLString::transcode(atts->getNamedItem(nameAttrStr)->getNodeValue());
        sLoadTime = XMLString::transcode(atts->getNamedItem(loadTimeAttrStr)->getNodeValue());
        sStoreTime = XMLString::transcode(atts->getNamedItem(storeTimeAttrStr)->getNodeValue());
        
#ifdef VPC_DEBUG
        std::cout << "VPCBuilder> Initializing new config: "<< sName << endl;
        std::cout << "VPCBuilder> with load time = "<< sLoadTime << " and store time = " << sStoreTime << endl;
#endif //VPC_DEBUG
  
        Configuration* conf = new Configuration(sName, sLoadTime, sStoreTime);
        // add configuration to node
        comp->addConfiguration(sName, conf);
        
        /*
         * register configuration for inner parsing purposes
         */
        // register as known configuration
        this->knownConfigs.insert(std::pair<std::string, Configuration* >(conf->getName(), conf));
        // register relation between configuration and component
        this->config_to_Comp.insert(std::pair<std::string, std::string>(conf->getName(), comp->basename()));
          
        this->initConfiguration(comp, conf);
        
      }else // if default configuration is defined init
      if( 0==XMLString::compareNString( xmlName, VPCBuilder::defaultConfStr, sizeof(VPCBuilder::defaultConfStr))){
      
        DOMNamedNodeMap* atts=node->getAttributes();
        char* sName;
        sName = XMLString::transcode(atts->getNamedItem(nameAttrStr)->getNodeValue());
      
        comp->setActivConfiguration(sName);
      }
        
      node = vpcConfigTreeWalker->nextSibling();
      
    }
    // set back walker to upper node level
    vpcConfigTreeWalker->parentNode();

  }

  /**
   * \brief Implementation of VPCBuilder::initConfiguration
   */
  void VPCBuilder::initConfiguration(ReconfigurableComponent* comp, Configuration* conf){
  
    // set hierarchie down for inner components
    DOMNode* node = vpcConfigTreeWalker->firstChild();
  
    // points to components defined within current configuration
    AbstractComponent* innerComp;
    
    // as long as there are inner components defined process them
    for(; node != NULL; node = vpcConfigTreeWalker->nextSibling()){
      
      try{
        innerComp = this->initComponent();
      }catch(InvalidArgumentException &e){
        std::cerr << "VPCBuilder> " << e.what() << std::endl;
        std::cerr << "VPCBuilder> ignoring specification of component, going on with initialization" << std::endl;
        continue;
      }
      
#ifdef VPC_DEBUG
      std::cerr << RED("Adding Component=" << innerComp->getName() << " to Configuration=" << conf->getName()) << std::endl;
#endif //VPC_DEBUG
      
      innerComp->setParentController(comp->getController());
      conf->addComponent(innerComp->basename(), innerComp);
      
      // register mapping
      this->virtualComp_to_Config.insert(std::pair<std::string, std::string >(innerComp->basename(), conf->getName()));
      
    }
    
    // set back walker to upper node level
    vpcConfigTreeWalker->parentNode();

  }
    
  /**
   * \brief Initializes the mappings and process structures.
   */
  void VPCBuilder::initMappingAPStruct(){

    DOMNode* node = vpcConfigTreeWalker->getCurrentNode();
    const XMLCh* xmlName=node->getNodeName();

#ifdef VPC_DEBUG
      std::cerr << "VPCBuilder> entering initMappingAPStruct"<< std::endl;
#endif //VPC_DEBUG    
    
    // find mapping tag (not mappings)
    if( 0==XMLString::compareNString( xmlName, mappingStr, sizeof(mappingStr))){
      DOMNamedNodeMap* atts=node->getAttributes();
      const char* sTarget;
      const char* sSource;
      sTarget=XMLString::transcode(atts->getNamedItem(targetAttrStr)->getNodeValue());
      sSource=XMLString::transcode(atts->getNamedItem(sourceAttrStr)->getNodeValue());

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Found mapping attribute: source=" << sSource << " target=" << sTarget << endl; 
#endif //VPC_DEBUG

      // check if component exists
      if(this->knownComps.count(sTarget)==1){
        
#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Target of Mapping: " << sTarget << " exists!" << std::endl; 
#endif //VPC_DEBUG

        std::map<std::string, AbstractComponent* >::iterator iterComp;
        iterComp = this->knownComps.find(sTarget);

        // first of all initialize PCB for task        
        if(iterComp != this->knownComps.end()){

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Configure mapping: " << sSource << "<->" << sTarget << std::endl; 
#endif //VPC_DEBUG

          (iterComp->second)->informAboutMapping(sSource);
          
          //generate new p_struct or get existing one for initialization
          p_struct *p = this->director->generatePCB(sSource);
            
          //walk down hierarchy to attributes
          node = vpcConfigTreeWalker->firstChild();
                
          // find all attributes
          while(node!=0){

            xmlName=node->getNodeName();
            if( 0==XMLString::compareNString( xmlName, attributeStr, sizeof(attributeStr))){
              DOMNamedNodeMap * atts=node->getAttributes();
              char *sType, *sValue;
              sType=XMLString::transcode(atts->getNamedItem(typeAttrStr)->getNodeValue());
              sValue=XMLString::transcode(atts->getNamedItem(valueAttrStr)->getNodeValue());
              if( 0 == strncmp(sType, STR_VPC_PRIORITY, sizeof(STR_VPC_PRIORITY) )){
                sscanf(sValue, "%d", &(p->priority));
              }else if( 0 == strncmp(sType, STR_VPC_DEADLINE, sizeof(STR_VPC_DEADLINE) )){
                sscanf(sValue, "%lf", &(p->deadline));
              }else if( 0 == strncmp(sType, STR_VPC_PERIOD, sizeof(STR_VPC_PERIOD) )){
                sscanf(sValue, "%lf", &(p->period));
              }else if( 0 == strncmp(sType, STR_VPC_DELAY, sizeof(STR_VPC_DELAY) )){
                sscanf(sValue, "%lf", &(p->delay));
              }else{
#ifdef VPC_DEBUG
                  std::cerr << "VPCBuilder> Unknown mapping attribute: type=" << sType << " value=" << sValue << endl; 
                std::cerr << "VPCBuilder> Try to interpret as function specific delay!!" << endl;
#endif //VPC_DEBUG

// <<< here is adding of additional function delay
                double delay;
                if( 1 == sscanf(sValue, "%lf", &delay) ){  
// ---> changed
//                  p->functionDelays.insert(pair<string,double>(sType,delay));
#ifdef VPC_DEBUG
                  std::cerr << YELLOW("VPCBuilder> Try to interpret as function specific delay!!") << endl;
                std::cerr << YELLOW("VPCBuilder> Register delay to: " << sTarget << "; " << sType << ", " << delay) << std::endl;
#endif //VPC_DEBUG
                  std::map<std::string, double> funcDelays = p->compDelays[sTarget];
                  funcDelays.insert(std::pair<std::string, double>(sType, delay));
                  p->compDelays[sTarget]= funcDelays;
                } else {
#ifdef VPC_DEBUG
                  std::cerr <<  "VPCBuilder> Mapping realy unknown!" << endl;
#endif //VPC_DEBUG
                }
              }
            }

            node = vpcConfigTreeWalker->nextSibling();
          }

          //check if component member of a configuration and iterativly adding mapping info
          //while(std::map<std::string, AbstractComponent* >::iterator iter = this->knownComps.find(sTarget);
          std::map<std::string, std::string>::iterator iterVCtC;
          // determine existenz of mapping to configuration
          iterVCtC = this->virtualComp_to_Config.find(sTarget);
          while(iterVCtC != this->virtualComp_to_Config.end()){

#ifdef VPC_DEBUG
            std::cerr << "VPCBuilder> Mapped component " << sTarget << " is wrapped within Configuration: " 
                  << iterVCtC->second << std::endl; 
#endif //VPC_DEBUG
            
            static std::map<std::string, std::string>::iterator iterConftC;
            // determine associated Component of Configuration
            iterConftC = this->config_to_Comp.find(iterVCtC->second);
            
            if(iterConftC != this->config_to_Comp.end()){
              
              // add mapping to controller of surrounding component
              ReconfigurableComponent* comp;
              comp = (ReconfigurableComponent*)this->knownComps[iterConftC->second];
              comp->getController()->registerMapping(sSource, sTarget);
              
#ifdef VPC_DEBUG
              std::cerr << "VPCBuilder> Additional mapping between: " << sSource << "<->" << sTarget << std::endl; 
#endif //VPC_DEBUG
            }
            
            sTarget = (iterConftC->second).c_str();
            iterVCtC = this->virtualComp_to_Config.find(sTarget);
          }
          
          //finally register mapping to Director
          this->director->registerMapping(sSource, sTarget);
          
          node = vpcConfigTreeWalker->parentNode();
        
          // this can be removed as we use generated p_struct from Director
          //p_struct_map_by_name.insert(pair<string,p_struct*>(sSource,p));
        }else{
          std::cerr << "VPCBuilder> No valid component found for mapping: source=" << sSource << " target=" << sTarget<< endl;
        }
      }
    }
  }

  /**
   * \brief Implementation of VPCBuilder::generateController
   */
  AbstractController* VPCBuilder::generateController(const char* controllertype, const char* id)
    throw(InvalidArgumentException){
    // TODO UPDATE IMPLEMENTATION WHEN NEW CONTROLLER IS IMPLEMENTED
    AbstractController* controller;
    
    if(0==strncmp(controllertype, STR_FIRSTCOMEFIRSTSERVE,strlen(STR_FIRSTCOMEFIRSTSERVE))
      || 0==strncmp(controllertype, STR_FCFS,strlen(STR_FCFS))){
      controller = new FCFSController(id);      
    }else 
    if(0==strncmp(controllertype, STR_ROUNDROBIN, strlen(STR_ROUNDROBIN))
      || 0==strncmp(controllertype, STR_RR, strlen(STR_RR))){
      controller = new RoundRobinController(id);
    }else
    if(0==strncmp(controllertype, STR_PRIORITYSCHEDULER, strlen(STR_PRIORITYSCHEDULER))
      || 0==strncmp(controllertype, STR_PS, strlen(STR_PS))){
      controller = new PriorityController(id);
    }else
    if(0==strncmp(controllertype, STR_EARLIESTDEADLINEFIRST, strlen(STR_EARLIESTDEADLINEFIRST))
      || 0==strncmp(controllertype, STR_EDF, strlen(STR_EDF))){
      controller = new EDFController(id);
    }else{
      string msg("Unkown controllertype ");
      msg += controllertype;
      msg += ", cannot create instance";
      throw InvalidArgumentException(msg);
    }
    
    return controller;
  }

}// namespace SystemC_VPC
