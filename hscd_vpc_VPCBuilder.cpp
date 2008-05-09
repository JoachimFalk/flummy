#include <iostream>
#include <values.h> 
#include <algorithm>
#include <cctype>
#include <string>

#include "hscd_vpc_VPCBuilder.h"
#include "hscd_vpc_XmlHelper.h"
#include "hscd_vpc_VpcDomErrorHandler.h"
#include "hscd_vpc_datatypes.h"

namespace SystemC_VPC{
#define MAX(x,y) ((x > y) ? x : y)

  /**
   * \brief sets ups VPC Framework
   */
  void VPCBuilder::buildVPC(){

    // open file and check existence
    FILE* fconffile;
    char* cfile;
    char* vpc_evaluator_prefix = getenv("VPC_EVALUATOR");
    char vpc_conf_file[VPC_MAX_STRING_LENGTH];
    char* vpc_measure_file = NULL;
    
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
      std::cerr << "VPCBuilder> VPCCONFIGURATION set to " << cfile
                << std::endl;
#endif //VPC_DEBUG
    }
        
    if(cfile){
      fconffile=fopen(cfile,"r");
      if( NULL == fconffile ){       // test if file exists

        // for joachim
        assert( 0 == strncmp(cfile, "", sizeof("")) 
                && "VPCCONFIGURATION is set, but points to nowhere" != NULL );
        
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
      this->director->FALLBACKMODE = true;
#ifdef VPC_DEBUG
      std::cout << "running fallbackmode" << std::endl;
#endif //VPC_DEBUG
    }else{
      // process xml
      DOMDocument* vpcConfigDoc;
      DOMBuilder* configParser;
      static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
      DOMImplementation* configImpl =
        DOMImplementationRegistry::getDOMImplementation(gLS);
      // create an error handler and install it
      VpcDomErrorHandler* configErrorh=new VpcDomErrorHandler();
      configParser =
        ((DOMImplementationLS*)configImpl)->
          createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
      // turn on validation
      configParser->setFeature(XMLUni::fgDOMValidation, true);
      configParser->setErrorHandler(configErrorh);

      try {                                                             
        // reset document pool - clear all previous allocated data
        configParser->resetDocumentPool();                                    
        vpcConfigDoc = configParser->parseURI(cfile);
      }
      catch (const XMLException& toCatch) {
        std::cerr << "\nVPCBuilder> Error while parsing xml file: '"
                  << cfile << "'\n"
          << "Exception message is:  \n"
          << XMLString::transcode( toCatch.getMessage()) << "\n" << endl;
          return;
      }
      catch (const DOMException& toCatch) {
        const unsigned int maxChars = 2047;
        XMLCh errText[maxChars + 1];
          
        std::cerr << "\nVPCBuilder> DOM Error while parsing xml file: '"
                  << cfile << "'\n"
                  << "DOMException code is:  "
                  << XMLString::transcode( toCatch.msg) << endl;
          
        if (DOMImplementation::loadDOMExceptionMsg(toCatch.code,
                                                   errText,
                                                   maxChars))
          std::cerr << "Message is: "
                    << XMLString::transcode( errText) << endl;
        return;
      }
      catch (...) {
        std::cerr << "\nVPCBuilder> Unexpected exception while parsing"
                  << " xml file: '" << cfile << "'\n";
        return;
      }
      
      //check if parsing failed
      if(configErrorh->parseFailed()){
        std::cerr << VPC_RED("VPCBuilder: Parsing of configuration failed,"
                             " aborting initialization!") << std::endl;
        return;
      }
      
      // set treewalker to documentroot
      vpcConfigTreeWalker =
        vpcConfigDoc->createTreeWalker(
          (DOMNode*)vpcConfigDoc->getDocumentElement(),
          DOMNodeFilter::SHOW_ELEMENT, 0,
          true);

      vpcConfigTreeWalker->setCurrentNode(
        (DOMNode*)vpcConfigDoc->getDocumentElement());
      
      // moves the Treewalker to the first Child 
      DOMNode* node = vpcConfigTreeWalker->firstChild(); 
      // name of xmlTag
      const XMLCh* xmlName;
      
      while(node!=0){
        xmlName = node->getNodeName();
        
        // found template section tag
        if( 0==XMLString::compareNString(xmlName,
                                         templateSectionStr,
                                         sizeof(templateSectionStr))){
          // walk down hierachy to components
          node = vpcConfigTreeWalker->firstChild();
          if(node != NULL){ 
            
            for(; node != NULL; node = vpcConfigTreeWalker->nextSibling()){
              DOMNamedNodeMap* atts = node->getAttributes();
              char* tid =
                XMLString::transcode(
                  atts->getNamedItem(VPCBuilder::nameAttrStr)->getNodeValue());
              //this->initTemplateSpecifications(tid, node->getFirstChild());
              this->initTemplateSpecifications(tid);
              XMLString::release(&tid);
            }

            node = vpcConfigTreeWalker->parentNode();
          }
          // find resources tag
        }else if( 0==XMLString::compareNString( xmlName,
                                                resourcesStr,
                                                sizeof(resourcesStr) ) ){
        
          // walk down hierachy to components
          node = vpcConfigTreeWalker->firstChild();
          if(node != NULL){
            // pointer to currently initiated component
            AbstractComponent* comp;
            // init all components
            for(; node != NULL; node = vpcConfigTreeWalker->nextSibling()){
              try{
                //comp = initComponent(node);
                comp = initComponent();
              }catch(InvalidArgumentException &e){
                std::cerr << "VPCBuilder> " << e.what() << std::endl;
                std::cerr << "VPCBuilder> ignoring specification of component,"
                  " going on with initialization" << std::endl;
                continue;
              }

#ifdef VPC_DEBUG
              std::cout << "VPCBuilder> registering component: "
                        << comp->basename() << " to Director" << endl;
#endif //VPC_DEBUG
              // register "upper-layer" components to Director
              this->director->registerComponent(comp);
              comp->setParentController(this->director);

            }

            node = vpcConfigTreeWalker->parentNode();
          }
        // find mappings tag (not mapping)
        }else if( 0==XMLString::compareNString( xmlName,
                                                mappingsStr,
                                                sizeof(mappingsStr) ) ){

#ifdef VPC_DEBUG
            std::cout << "VPCBuilder> processing mappings " << endl;
#endif //VPC_DEBUG
            
          node = vpcConfigTreeWalker->firstChild();
          if(node != NULL){ 
            //foreach mapping of configuration perfom initialization  
            for(; node!=0; node = this->vpcConfigTreeWalker->nextSibling()){
              //this->initMappingAPStruct(node);
              this->initMappingAPStruct();
            }
        
            node = vpcConfigTreeWalker->parentNode();
          }

        // find measure file declaration and store value for later use
        }else if( 0==XMLString::compareNString( xmlName,
                                                measurefileStr,
                                                sizeof(measurefileStr) ) ){
           
           DOMNamedNodeMap * atts=node->getAttributes();
            vpc_measure_file =
              XMLString::transcode(atts->getNamedItem(nameAttrStr)->getNodeValue());
        
        }else if( 0==XMLString::compareNString( xmlName,
                                                resultfileStr,
                                                sizeof(resultfileStr) ) ){
           
           DOMNamedNodeMap * atts=node->getAttributes();
            std::string vpc_result_file =
              XMLString::transcode(
                atts->getNamedItem(nameAttrStr)->getNodeValue());
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
      std::cerr << "VPCBuilder> finished initialization of components"
                << std::endl;
      std::cerr << "VPCBuilder> starting parsing of measurefile '"
                << vpc_measure_file << "'" << std::endl;
#endif // VPC_DEBUG

      if(!vpc_measure_file){
          cerr << VPC_ERROR << "VPCBuilder> No measurefile"<< NENDL;
          return;
      }else{
        ifstream f;
        f.open(vpc_measure_file);
        if(!f) {
          cerr << "VPCBuilder> Warning: measurefile '" << vpc_measure_file
               << "' does not exist!" << endl; 
         return;
       }
       f.close();
      }
      
      DOMTreeWalker *vpc_measure_TreeWalker;
      DOMDocument *vpc_measure_doc;
      DOMBuilder *parser;
      //static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
      DOMImplementation *impl
        = DOMImplementationRegistry::getDOMImplementation(gLS);
      // create an error handler and install it
      VpcDomErrorHandler* errorh=new VpcDomErrorHandler();
      parser =
        ((DOMImplementationLS*)impl)->createDOMBuilder(
          DOMImplementationLS::MODE_SYNCHRONOUS, 0);

      // turn on validation
      parser->setFeature(XMLUni::fgDOMValidation, true);
      parser->setErrorHandler(errorh);
  
      try {                                                             
        // reset document pool - clear all previous allocated data
        parser->resetDocumentPool();                                    
        vpc_measure_doc = parser->parseURI(vpc_measure_file);
      }
      catch (const XMLException& toCatch) {
        std::cerr << "\nError while parsing xml file: '" << vpc_measure_file
                  << "'\n"
          << "Exception message is:  \n"
          << XMLString::transcode( toCatch.getMessage()) << "\n" << endl;
          return;
      }
      catch (const DOMException& toCatch) {
        const unsigned int maxChars = 2047;
        XMLCh errText[maxChars + 1];
        
        std::cerr << "\nDOM Error while parsing xml file: '"
                  << vpc_measure_file << "'\n"
          << "DOMException code is:  " << XMLString::transcode( toCatch.msg)
                  << endl;
        
        if (DOMImplementation::loadDOMExceptionMsg(toCatch.code,
                                                   errText,
                                                   maxChars))
          std::cerr << "Message is: " << XMLString::transcode( errText)
                    << endl;
         
        return;
      }
      catch (...) {
        std::cerr << "\nUnexpected exception while parsing xml file: '"
                  << vpc_measure_file << "'\n";
        return;
      }
      
      //check if parsing failed
      if(errorh->parseFailed()){
        std::cerr << VPC_RED("VPCBuilder> Parsing of measure file "
                  << vpc_measure_file << " failed, aborting initialization!")
                  << std::endl;
        return;
      }
      // set treewalker to documentroot
      vpc_measure_TreeWalker =
        vpc_measure_doc->createTreeWalker(
          (DOMNode*)vpc_measure_doc->getDocumentElement(),
          DOMNodeFilter::SHOW_ELEMENT,
          0,
          true);
        
      vpc_measure_TreeWalker->setCurrentNode(
        (DOMNode*)vpc_measure_doc->getDocumentElement());
      
      DOMNode *n;
      
      // moves the Treewalker to the first Child 
      n = vpc_measure_TreeWalker->firstChild();
      char *name;
      const XMLCh *xname;
      while( n) {
        xname=n->getNodeName();
        name=XMLString::transcode(xname); // for cerr only
        //cerr << VPC_RED(name)<< endl;
          
        if(n->getNodeType()==DOMNode::ELEMENT_NODE && 
          0==XMLString::compareNString(xname,
                                       constraintStr,
                                       sizeof(constraintStr))){
            
          DOMNamedNodeMap * atts=n->getAttributes();

          char *sCount,*sDivider,*sName;
          sName=XMLString::transcode(
            atts->getNamedItem(nameAttrStr)->getNodeValue());

          sCount=XMLString::transcode(
            atts->getNamedItem(countAttrStr)->getNodeValue());

          sDivider=XMLString::transcode(
            atts->getNamedItem(dividerAttrStr)->getNodeValue());
            
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
   * \throws InvalidArgumentException if requested component within
   * configuration file is unkown
   */
  AbstractComponent* VPCBuilder::initComponent()
    throw(InvalidArgumentException)
  {
 
    DOMNode* node = this->vpcConfigTreeWalker->getCurrentNode();
    const XMLCh* xmlName = node->getNodeName(); 

    // check for component tag
    if( 0==XMLString::compareNString( xmlName,
                                      VPCBuilder::componentStr,
                                      sizeof(VPCBuilder::componentStr))){
      
      DOMNamedNodeMap* atts = node->getAttributes();
      char* sName;
      char* sType;
      char* sScheduler;
      AbstractComponent* comp = NULL;
  
      sName = XMLString::transcode(
        atts->getNamedItem(VPCBuilder::nameAttrStr)->getNodeValue());

      sType = XMLString::transcode(
            atts->getNamedItem(VPCBuilder::typeAttrStr)->getNodeValue());

      sScheduler = XMLString::transcode(
            atts->getNamedItem(VPCBuilder::schedulerAttrStr)->getNodeValue());
  
#ifdef VPC_DEBUG
      std::cerr << "VPCBuilder> initComponent: " << sName << std::endl;
#endif //VPC_DEBUG

      // check which kind of component is defined
      // standard component
      if(0==strncmp(sType,
                    STR_VPC_THREADEDCOMPONENTSTRING,
                    sizeof(STR_VPC_THREADEDCOMPONENTSTRING))){

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Found Component name=" << sName << " type="
                  << sType << endl;
#endif //VPC_DEBUG
        
        // check if component already exists
        //std::map<std::string, AbstractComponent* >::iterator iter
        //  = this->knownComps.find(sName);
        //if(iter == this->knownComps.end()){ 
          comp = new Component(sName,sScheduler);
          this->knownComps.insert(
            pair<string, AbstractComponent* >(sName, comp));
        //}else{
        //  comp = iter->second;
        //}
        //this->initCompAttributes(comp, node->getFirstChild());
        this->initCompAttributes(comp);
        
      }else // reconfigurable component 
      if(0==strncmp(sType,
                    STR_VPC_RECONFIGURABLECOMPONENTSTRING,
                    sizeof(STR_VPC_RECONFIGURABLECOMPONENTSTRING))){

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Found Component name=" << sName << " type="
                  << sType << endl;
#endif //VPC_DEBUG
        AbstractController* controller;
        try{
          controller = this->generateController(sScheduler, sName);
        }catch(InvalidArgumentException &e){
          std::cerr << "VPCBuilder> Error: " << e.what() << std::endl;
          std::cerr << "VPCBuilder> setting default FCFS for "<< sName
                    << std::endl;
          controller = this->generateController("FCFS", sName);
        }catch(...){
          std::cerr << "VPCBuilder> unkown exception occured while creating"
            " controller instance" << std::endl;
          throw;
        }
        
        // check if component already exists
        //std::map<std::string, AbstractComponent* >::iterator iter
        //    = this->knownComps.find(sName);
        //if(iter == this->knownComps.end()){ 
          comp = new ReconfigurableComponent(sName, controller);
          this->knownComps.insert(pair<string, AbstractComponent* >(sName,
                                                                    comp));
        //}else{
        //  comp = iter->second;
        //}
        
        //this->initConfigurations((ReconfigurableComponent*)comp,
        // node->getFirstChild());
        this->initConfigurations((ReconfigurableComponent*)comp);
        //this->initCompAttributes(comp, node->getFirstChild());
        this->initCompAttributes(comp);

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Initialized Component name=" << sName
                  << " type=" << sType << endl;
#endif //VPC_DEBUG
      
      }else // unkown component type
      {
        
        string msg("Unknown Component: name=");
        msg += sName;
        msg += " type=";
        msg += sType;
        throw InvalidArgumentException(msg);
        
      }

      return comp;    
    }

    string msg("Unknown configuration tag: ");
    char *name = XMLString::transcode(xmlName);
    msg.append(name, std::strlen (name));
    XMLString::release(&name);
    throw InvalidArgumentException(msg);

  }
 
  /**
   * \brief Implementation of VPCBuilder::initTemplateSpecification
   */
  //void VPCBuilder::initTemplateSpecifications(char* tid, DOMNode* node){
  void VPCBuilder::initTemplateSpecifications(char* tid){
    DOMNode* node = this->vpcConfigTreeWalker->firstChild();
    
    if(node != NULL){
      std::vector<std::pair<char*, char* > > attributes;
      std::vector<VPCBuilder::Timing > timings;
      // find all attributes
      for(; node != NULL; node = this->vpcConfigTreeWalker->nextSibling()){
        const XMLCh* xmlName = node->getNodeName();

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> init template " << tid << std::endl;
#endif //VPC_DEBUG

        if( 0==XMLString::compareNString( xmlName, timingStr,
                                          sizeof(timingStr))){
	  char *delay=NULL, *dii=NULL, *latency=NULL, *fname=NULL;
	  
	  DOMNamedNodeMap* atts = node->getAttributes();
	  for(unsigned int i=0; i<atts->getLength(); i++){
	    DOMNode* a=atts->item(i);
	    if(0==XMLString::compareNString( a->getNodeName(),
                                             delayAttrStr,
                                             sizeof(delayAttrStr))){
	      delay = XMLString::transcode(a->getNodeValue());
	    }else if(0==XMLString::compareNString( a->getNodeName(),
                                                   latencyAttrStr,
                                                   sizeof(latencyAttrStr))){
	      latency = XMLString::transcode(a->getNodeValue());
	    }else if(0==XMLString::compareNString( a->getNodeName(),
                                                   diiAttrStr,
                                                   sizeof(diiAttrStr))){
	      dii   = XMLString::transcode(a->getNodeValue());
	    }else if(0==XMLString::compareNString( a->getNodeName(),
                                                   fnameAttrStr,
                                                   sizeof(fnameAttrStr))){
	      fname = XMLString::transcode(a->getNodeValue());
	    }
	  }
	  VPCBuilder::Timing t;
	  sc_time sc_latency = SC_ZERO_TIME;
	  sc_time sc_dii     = SC_ZERO_TIME;
	  
	  if(latency != NULL) sc_latency = Director::createSC_Time(latency);
	  if(dii != NULL) sc_dii = Director::createSC_Time(dii);
	  { // latency and delay are synonym -> take maximum if they differ
	    sc_time sc_delay = SC_ZERO_TIME;
	    if(delay != NULL) sc_delay = Director::createSC_Time(delay);
	    sc_latency = MAX(sc_latency,sc_delay);
	  }

	  t.fname   = fname;
	  //per default latency is used as vpc-delay as well as vpc-latency
          // (vpc-delay == dii)
	  t.latency = sc_latency;
	  t.dii   = sc_latency;
	  if( dii != NULL ){
	    t.dii   = sc_latency;
	  }

	  timings.push_back(t);
	  
        // check if its an attribute to add
	}else if( 0==XMLString::compareNString( xmlName,
                                                attributeStr,
                                                sizeof(attributeStr))){

          DOMNamedNodeMap* atts = node->getAttributes();
          char* sType;
          char* sValue;
          sType = XMLString::transcode(
            atts->getNamedItem(typeAttrStr)->getNodeValue());
          sValue = XMLString::transcode(
            atts->getNamedItem(valueAttrStr)->getNodeValue());

          attributes.push_back(std::pair<char*, char* >( sType, sValue));

        }

      }

      // if any attributes associated with template remember it
      if(attributes.size() > 0){
        this->templates.insert(
          std::pair<std::string, std::vector<std::pair<char*, char* > > >(
            std::string(tid, strlen(tid)),
            attributes));

      }
      if(timings.size() > 0){
	timingTemplates[std::string(tid, strlen(tid))] = timings;
      }

      this->vpcConfigTreeWalker->parentNode();
   }
 }

  
  /**
   * \brief Performs initialization of attribute values for a component
   * \param comp specifies the component to set attributes for
   * \param node specifies current position within dom tree
   */
  void VPCBuilder::initCompAttributes(AbstractComponent* comp){
    DOMNode* node = this->vpcConfigTreeWalker->firstChild(); 
#ifdef VPC_DEBUG
    cerr << "VPC> InitAttribute for Component name=" << comp->basename()
         << endl;
#endif //VPC_DEBUG
    if(node != NULL){
      // find all attributes
 
      DOMNamedNodeMap * atts2 = node->getAttributes();
      char* sType2;
      char* sValue2="";
      sType2 = XMLString::transcode(atts2->getNamedItem(typeAttrStr)->getNodeValue());
  
      if(strcmp( sType2 , "FlexRayParams") == 0){
	//neue Realisierung... FlexRay - Parameter ueber class Attribute
	//Anlegen des Attribute-Elements
 	Attribute fr_Attributes( sType2, sValue2);
	//Jetzt Baum entlang nach unten "laufen" und alle Eigenschaften entsprechend einbauen.	       
	//rekursive Methode aufrufen!
	for(unsigned int i=0; i<atts2->getLength(); i++){
	nextAttribute(fr_Attributes, node->getFirstChild());
	}
	//Attribute fertig zusammengebaut -> an den Scheduler weiterreichen!
	comp->processAndForwardAttribute(fr_Attributes);
	
      }else{ // "alte Realisierung"
      //ALT
      for(; node != NULL; node = this->vpcConfigTreeWalker->nextSibling()){
        const XMLCh* xmlName = node->getNodeName();
        DOMNamedNodeMap * atts = node->getAttributes();

        // check if its an attribute to add
        if( 0==XMLString::compareNString( xmlName,
                                          attributeStr,
                                          sizeof(attributeStr))){

          char* sType;
          char* sValue;
          sType = XMLString::transcode(
            atts->getNamedItem(typeAttrStr)->getNodeValue());

          sValue = XMLString::transcode(
            atts->getNamedItem(valueAttrStr)->getNodeValue());

          comp->processAndForwardParameter(sType,sValue);
          XMLString::release(&sType);
          XMLString::release(&sValue);

          // check if template is referred
        }else if( 0==XMLString::compareNString( xmlName,
                                                refTemplateStr,
                                                sizeof(refTemplateStr))){

          char* sKey;
          sKey = XMLString::transcode(
            atts->getNamedItem(nameAttrStr)->getNodeValue());
          this->applyTemplateOnComponent(comp, std::string(sKey));
          XMLString::release(&sKey);

        }
	}
      }
      vpcConfigTreeWalker->parentNode();
    }
  }

  /**
   * \brief Implementation of VPCBuilder::applyTemplateOnComponent
   */
  void VPCBuilder::applyTemplateOnComponent(AbstractComponent* comp,
                                            std::string key){
    std::map<std::string,
      std::vector<std::pair<char*, char* > > >::iterator iter;
    iter = this->templates.find(key);
    
    if(iter != this->templates.end()){
      std::vector<std::pair<char*, char* > >::iterator attiter;
      for(attiter = iter->second.begin();
          attiter != iter->second.end();
          ++attiter){
        comp->processAndForwardParameter(attiter->first, attiter->second);
      }
    }
    
  }
  
  /**
   * \brief Initializes the Configurations of an ReconfigurableComponent
   * As long as there are defined Configurations, they will be register and
   * added to the given component.
   * \param comp represents the component for which to initialize the
   *  configurations
   * \param node specifies current position within dom tree
   */
  void VPCBuilder::initConfigurations(ReconfigurableComponent* comp){

    DOMNode* node = this->vpcConfigTreeWalker->firstChild();
#ifdef VPC_DEBUG
    std::cout << "VPCBuilder> entering initConfigurations"<< endl;
#endif //VPC_DEBUG
    if(node != NULL){
      const XMLCh* xmlName;

      for(; node != NULL; node = this->vpcConfigTreeWalker->nextSibling())
      {

        xmlName = node->getNodeName();

        // as long as there are exisiting configuration register them to
        // component
        if(0==XMLString::compareNString(xmlName,
                                        VPCBuilder::configurationStr,
                                        sizeof(VPCBuilder::configurationStr))){

          //create and initialize configuration
          DOMNamedNodeMap* atts=node->getAttributes();
          char* sName;
          char* sLoadTime;
          char* sStoreTime;
          // read values
          sName = XMLString::transcode(
            atts->getNamedItem(nameAttrStr)->getNodeValue());

          sLoadTime = XMLString::transcode(
            atts->getNamedItem(loadTimeAttrStr)->getNodeValue());

          sStoreTime = XMLString::transcode(
            atts->getNamedItem(storeTimeAttrStr)->getNodeValue());

#ifdef VPC_DEBUG
          std::cout << "VPCBuilder> Initializing new config: "<< sName << endl;
          std::cout << "VPCBuilder> with load time = "<< sLoadTime
                    << " and store time = " << sStoreTime << endl;
#endif //VPC_DEBUG

          Configuration* conf = NULL; 

          // std::map<std::string, Configuration*>::iterator iter;
          //iter = this->knownConfigs.find(sName);
          //if(iter == this->knownConfigs.end()){

          conf = new Configuration(sName, sLoadTime, sStoreTime);
          /*
           * register configuration for inner parsing purposes
           */
          // register as known configuration
          this->knownConfigs.insert(
            std::pair<std::string, Configuration* >(conf->getName(), conf));
          // register relation between configuration and component
          this->config_to_ParentComp.insert(
            std::pair<std::string, std::string>(conf->getName(),
                                                comp->basename()));

          //}else{

          //conf = iter->second;

          //}

          // add configuration to node
          comp->addConfiguration(sName, conf);

          //this->initConfiguration(comp, conf, node->getFirstChild());
          this->initConfiguration(comp, conf);

        }else // if default configuration is defined init
          if(0==XMLString::compareNString(xmlName,
                                          VPCBuilder::defaultConfStr,
                                          sizeof(VPCBuilder::defaultConfStr))){

            DOMNamedNodeMap* atts=node->getAttributes();
            char* sName;
            sName = XMLString::transcode(
              atts->getNamedItem(nameAttrStr)->getNodeValue());

            comp->setActivConfiguration(sName);
          }
      }

      vpcConfigTreeWalker->parentNode();
    }
  }

  /**
   * \brief Implementation of VPCBuilder::initConfiguration
   * \param comp specifies the associated component of the configuration
   * \param conf specifies the  configuration to initialize
   */
  void VPCBuilder::initConfiguration(ReconfigurableComponent* comp,
                                     Configuration* conf){
    
    DOMNode* node = this->vpcConfigTreeWalker->firstChild();

    if(node != NULL){
      // points to components defined within current configuration
      AbstractComponent* innerComp;

      // as long as there are inner components defined process them
      for(; node != NULL; node = this->vpcConfigTreeWalker->nextSibling()){

        try{
          //innerComp = this->initComponent(node);
          innerComp = this->initComponent();
        }catch(InvalidArgumentException &e){
          std::cerr << "VPCBuilder> " << e.what() << std::endl;
          std::cerr << "VPCBuilder> ignoring specification of component,"
            " going on with initialization" << std::endl;
          continue;
        }

#ifdef VPC_DEBUG
        std::cerr << VPC_RED("Adding Component=" << innerComp->basename()
                  << " to Configuration=" << conf->getName()) << std::endl;
#endif //VPC_DEBUG

        innerComp->setParentController(comp->getController());
        conf->addComponent(innerComp->basename(), innerComp);

        // register mapping
        this->subComp_to_Config.insert(
          std::pair<std::string, std::string >(innerComp->basename(),
                                               conf->getName()));

      }
      this->vpcConfigTreeWalker->parentNode();
    }
  }
    
  /**
   * \brief Initializes the mappings and process structures.
   */
  //void VPCBuilder::initMappingAPStruct(DOMNode* node){
  void VPCBuilder::initMappingAPStruct(){

    DOMNode* node = this->vpcConfigTreeWalker->getCurrentNode();
    
    const XMLCh* xmlName=node->getNodeName();

#ifdef VPC_DEBUG
      std::cerr << "VPCBuilder> entering initMappingAPStruct"<< std::endl;
#endif //VPC_DEBUG    
   
    // find mapping tag (not mappings)
    if( 0==XMLString::compareNString( xmlName,
                                      mappingStr,
                                      sizeof(mappingStr))){
      DOMNamedNodeMap* atts=node->getAttributes();
      const char* sTarget;
      const char* sSource;
      sTarget=XMLString::transcode(
        atts->getNamedItem(targetAttrStr)->getNodeValue());

      sSource=XMLString::transcode(
        atts->getNamedItem(sourceAttrStr)->getNodeValue());

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Found mapping attribute: source=" << sSource
                  << " target=" << sTarget << endl; 
#endif //VPC_DEBUG

      // check if component exists
      if(this->knownComps.count(sTarget)==1){
        
#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Target of Mapping: " << sTarget << " exists!"
                  << std::endl; 
#endif //VPC_DEBUG

        std::map<std::string, AbstractComponent* >::iterator iterComp;
        iterComp = this->knownComps.find(sTarget);

        // first of all initialize PCB for task        
        if(iterComp != this->knownComps.end()){

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Configure mapping: " << sSource << "<->"
                  << sTarget << std::endl; 
#endif //VPC_DEBUG

          (iterComp->second)->informAboutMapping(sSource);
          
          //generate new ProcessControlBlock or get existing one for
          // initialization
          ProcessControlBlock& p = this->director->generatePCB(sSource);
            
          //walk down hierarchy to attributes
          DOMNode* attnode = node->getFirstChild();
          
          // find all attributes
          for(; attnode!=NULL; attnode = attnode->getNextSibling()){

            xmlName=attnode->getNodeName();
            DOMNamedNodeMap * atts=attnode->getAttributes();

	    if( 0==XMLString::compareNString( xmlName,
                                              timingStr,
                                              sizeof(timingStr))){
	      char *delay=NULL, *dii=NULL, *latency=NULL, *fname=NULL;
	      for(unsigned int i=0; i<atts->getLength(); i++){
		DOMNode* a=atts->item(i);
		if(0==XMLString::compareNString( a->getNodeName(),
                                                 delayAttrStr,
                                                 sizeof(delayAttrStr))){
		  delay = XMLString::transcode(a->getNodeValue());
		}else if(0==XMLString::compareNString(a->getNodeName(),
                                                      latencyAttrStr,
                                                      sizeof(latencyAttrStr))){
		  latency = XMLString::transcode(a->getNodeValue());
		}else if(0==XMLString::compareNString( a->getNodeName(),
                                                       diiAttrStr,
                                                       sizeof(diiAttrStr))){
		  dii   = XMLString::transcode(a->getNodeValue());
		}else if(0==XMLString::compareNString( a->getNodeName(),
                                                       fnameAttrStr,
                                                       sizeof(fnameAttrStr))){
		  fname = XMLString::transcode(a->getNodeValue());
		}
	      }
	      
	      sc_time sc_latency = SC_ZERO_TIME;
	      sc_time sc_dii     = SC_ZERO_TIME;

	      if(latency != NULL) sc_latency = Director::createSC_Time(latency);
	      if(dii != NULL) sc_dii = Director::createSC_Time(dii);
	      { // latency and delay are synonym -> take maximum if they differ
		sc_time sc_delay = SC_ZERO_TIME;
		if(delay != NULL) sc_delay = Director::createSC_Time(delay);
		sc_latency = MAX(sc_latency,sc_delay);
	      }

	      //per default latency is used as vpc-delay as well as vpc-latency
              // (vpc-delay == dii)
              //	      p.setLatency(sc_latency);
              p.addFuncLatency( this->director, sTarget, fname, sc_latency );
              //	      p.setDelay(sc_latency);
              p.addFuncDelay( this->director, sTarget, fname, sc_latency );

	      // if having a  then dii overides delay
	      if( dii != NULL ){
                //                p.setDelay(sc_dii);
                p.addFuncDelay( this->director, sTarget, fname, sc_dii );
	      }
	    }else if( 0==XMLString::compareNString( xmlName,
                                                    attributeStr,
                                                    sizeof(attributeStr))){
              char *sType, *sValue;
              sType=XMLString::transcode(
                atts->getNamedItem(typeAttrStr)->getNodeValue());
              sValue=XMLString::transcode(
                atts->getNamedItem(valueAttrStr)->getNodeValue());

#ifdef VPC_DEBUG
              std::string msg = "attribute values are: ";
              msg.append(sType, strlen(sType));
              msg += " and ";
              msg.append(sValue, strlen(sValue));
              std::cerr << msg << std::endl;
#endif   
          
              if( 0 == strncmp(sType,
                               STR_VPC_PRIORITY,
                               sizeof(STR_VPC_PRIORITY) )){
                int priority = 0;
                sscanf(sValue, "%d", &priority);
                p.setPriority(priority);
              }else if( 0 == strncmp(sType,
                                     STR_VPC_DEADLINE,
                                     sizeof(STR_VPC_DEADLINE) )){
                p.setDeadline(Director::createSC_Time(sValue));
              }else if( 0 == strncmp(sType,
                                     STR_VPC_PERIOD,
                                     sizeof(STR_VPC_PERIOD) )){
                p.setPeriod(Director::createSC_Time(sValue));
              }else if( 0 == strncmp(sType,
                                     STR_VPC_DELAY,
                                     sizeof(STR_VPC_DELAY) )){
		sc_time delay = Director::createSC_Time(sValue);
                p.setDelay(delay);
                p.addFuncDelay( this->director, sTarget, NULL, delay );
              }else if( 0 == strncmp(sType,
                                     STR_VPC_LATENCY,
                                     sizeof(STR_VPC_LATENCY) )){
		sc_time latency = Director::createSC_Time(sValue);
                p.setLatency(latency);
                p.addFuncLatency( this->director, sTarget, NULL, latency );
              }else{
#ifdef VPC_DEBUG
                std::cerr << "VPCBuilder> Unknown mapping attribute: type="
                          << sType << " value=" << sValue << endl; 
                std::cerr << "VPCBuilder> Try to interpret as function"
                  " specific delay!!" << endl;
#endif //VPC_DEBUG

		try{  
		  sc_time delay = Director::createSC_Time(sValue);
#ifdef VPC_DEBUG
                  std::cerr << VPC_YELLOW("VPCBuilder> Try to interpret as"
                              " function specific delay!!") << endl;
                  std::cerr << VPC_YELLOW("VPCBuilder> Register delay to: "
                            << sTarget << "; " << sType << ", " << delay)
                            << std::endl;
#endif //VPC_DEBUG
                  p.addFuncDelay( this->director, sTarget, sType, delay );
                  // using attribute="functionName" for compatibility:
                  // force to have a latency
                  p.addFuncLatency( this->director, sTarget, sType, delay );

                } catch(const InvalidArgumentException& ex) {
#ifdef VPC_DEBUG
                  std::cerr <<  "VPCBuilder> Mapping realy unknown!" << endl;
#endif //VPC_DEBUG
                }
              }

              XMLString::release(&sType);
              XMLString::release(&sValue);
              
              // check if reference to template
            }else if( 0==XMLString::compareNString( xmlName,
                                                    refTemplateStr,
                                                    sizeof(refTemplateStr))){

              char* sKey;
              sKey = XMLString::transcode(
                atts->getNamedItem(nameAttrStr)->getNodeValue());
              this->applyTemplateOnPStruct(&p,
                                           sTarget,
                                           std::string(sKey, strlen(sKey)));
              XMLString::release(&sKey);
              
            }

          }
          
          // node = vpcConfigTreeWalker->parentNode();
          
          //check if component member of a configuration and iterativly
          // adding mapping info
          this->buildUpBindHierarchy(sSource, sTarget);
          
        }else{
          std::cerr << "VPCBuilder> No valid component found for mapping:"
            " source=" << sSource << " target=" << sTarget<< endl;
        }
      }
    }
  }

  /**
   * \brief Implementation of VPCBiulder::buildUpBindHierarchy
   */
  void VPCBuilder::buildUpBindHierarchy(const char* source, 
                                        const char* target)
  {
    std::map<std::string, std::string>::iterator iterVCtC;
    // determine existence of mapping to configuration
    iterVCtC = this->subComp_to_Config.find(target);
    for(;iterVCtC != this->subComp_to_Config.end(); 
        iterVCtC = this->subComp_to_Config.find(target))
    {

#ifdef VPC_DEBUG
      std::cerr << "VPCBuilder> Mapped component " << target
                << " is wrapped within Configuration: " 
        << iterVCtC->second << std::endl; 
#endif //VPC_DEBUG

      static std::map<std::string, std::string>::iterator iterConftC;
      // determine associated Component of Configuration
      iterConftC = this->config_to_ParentComp.find(iterVCtC->second);

      if(iterConftC != this->config_to_ParentComp.end()){

        // add mapping to controller of surrounding component
        ReconfigurableComponent* comp;
        comp = (ReconfigurableComponent*)this->knownComps[iterConftC->second];
        comp->getController()->registerMapping(source, target);

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> Additional mapping between: " << source
                  << "<->" << target << std::endl; 
#endif //VPC_DEBUG
            
        target = (iterConftC->second).c_str(); 
      }
    }
          
    //finally register mapping to Director
    this->director->registerMapping(source, target);

  }

  /**
   * \brief Implementation of VPCBuilder::applyTemplateOnPStruct
   */
  void VPCBuilder::applyTemplateOnPStruct(ProcessControlBlock* p,
                                          const char* target,
                                          std::string key){
    
    std::map<std::string, std::vector<std::pair<char*, char* > > >::iterator
      iter;
    iter = this->templates.find(key);
    if(iter != this->templates.end()){
      std::vector<std::pair<char*, char* > >::iterator attiter;
      
      for(attiter = iter->second.begin();
          attiter != iter->second.end();
          ++attiter){
        if( 0 == strncmp(attiter->first,
                         STR_VPC_PRIORITY,
                         sizeof(STR_VPC_PRIORITY) )){
          int priority = 0;
          sscanf(attiter->second, "%d", &priority);
          p->setPriority(priority);
        }else if( 0 == strncmp(attiter->first,
                               STR_VPC_DEADLINE,
                               sizeof(STR_VPC_DEADLINE) )){
          p->setDeadline(Director::createSC_Time(attiter->second));
        }else if( 0 == strncmp(attiter->first,
                               STR_VPC_PERIOD,
                               sizeof(STR_VPC_PERIOD) )){
          p->setPeriod(Director::createSC_Time(attiter->second));
        }else if( 0 == strncmp(attiter->first,
                               STR_VPC_DELAY,
                               sizeof(STR_VPC_DELAY) )){
	  sc_time delay = Director::createSC_Time(attiter->second);
          p->setDelay(delay);
          p->addFuncDelay( this->director, target, NULL, delay );
        }else if( 0 == strncmp(attiter->first,
                               STR_VPC_LATENCY,
                               sizeof(STR_VPC_LATENCY) )){
	  sc_time latency = Director::createSC_Time(attiter->second);
	  p->setLatency(latency);
	  p->addFuncLatency( this->director, target, NULL, latency );
	}else{
#ifdef VPC_DEBUG
          std::cerr << "VPCBuilder> Unknown mapping attribute: type="
                    << attiter->first << " value=" << attiter->second << endl; 
          std::cerr << "VPCBuilder> Try to interpret as function specific "
            "delay!!" << endl;
#endif //VPC_DEBUG

          //if( 1 == sscanf(attiter->second, "%lf", &delay) ){  
	  try{
	    sc_time delay = Director::createSC_Time(attiter->second);
#ifdef VPC_DEBUG
            std::cerr << VPC_YELLOW("VPCBuilder> Try to interpret as function"
                                    " specific delay!!") << endl;
            std::cerr << VPC_YELLOW("VPCBuilder> Register delay to: "
                      << target << "; " << attiter->second << ", " << delay)
                      << std::endl;
#endif //VPC_DEBUG
            p->addFuncDelay( this->director, target, attiter->first, delay );

          } catch(const InvalidArgumentException& ex) {

#ifdef VPC_DEBUG
            std::cerr <<  "VPCBuilder> Mapping realy unknown!" << endl;
#endif //VPC_DEBUG
          }
        }
      }
    }
    std::map<std::string, std::vector<VPCBuilder::Timing> >::iterator
      timingIter = timingTemplates.find(key);
    if(timingIter != timingTemplates.end()){
      for(std::vector<VPCBuilder::Timing>::iterator
            timings = timingTemplates[key].begin();
	  timings != timingTemplates[key].end();
          ++timings)
      {
        VPCBuilder::Timing t = *timings;
        p->setDelay( t.dii );
        p->setLatency(t.latency );
        p->addFuncDelay( this->director, target, t.fname, t.dii );
        p->addFuncLatency( this->director, target, t.fname, t.latency );
      }
    }
    
  }
  
  /**
   * \brief Implementation of VPCBuilder::generateController
   * \param controllertype specifies type of controller to instantiate
   * \param id is part of id to be set for the controller
   */
  AbstractController*
  VPCBuilder::generateController(const char* controllertype, const char* id)
    throw(InvalidArgumentException){
    // TODO UPDATE IMPLEMENTATION WHEN NEW CONTROLLER IS IMPLEMENTED
    AbstractController* controller;
  
    // create correct ID for sgEdit compatibility
    std::string cID = string(id);
    cID.append("-");
    cID.append(controllertype);
    
    if(0==strncmp(controllertype,
                  STR_FIRSTCOMEFIRSTSERVE,
                  strlen(STR_FIRSTCOMEFIRSTSERVE))
      || 0==strncmp(controllertype, STR_FCFS,strlen(STR_FCFS))){
      controller = new FCFSController(cID.c_str());      
    }else 
    if(0==strncmp(controllertype,
                  STR_ROUNDROBIN,
                  strlen(STR_ROUNDROBIN))
      || 0==strncmp(controllertype, STR_RR, strlen(STR_RR))){
      controller = new RoundRobinController(cID.c_str());
    }else
    if(0==strncmp(controllertype,
                  STR_PRIORITYSCHEDULER,
                  strlen(STR_PRIORITYSCHEDULER))
      || 0==strncmp(controllertype, STR_PS, strlen(STR_PS))){
      controller = new PriorityController(cID.c_str());
    }else
    if(0==strncmp(controllertype,
                  STR_EARLIESTDEADLINEFIRST,
                  strlen(STR_EARLIESTDEADLINEFIRST))
      || 0==strncmp(controllertype, STR_EDF, strlen(STR_EDF))){
      controller = new EDFController(cID.c_str());
    }else{
      string msg("Unkown controllertype ");
      msg += controllertype;
      msg += ", cannot create instance";
      throw InvalidArgumentException(msg);
    }
    
    return controller;
  }
  
  void VPCBuilder::nextAttribute(Attribute& fr_Attribute, DOMNode* node){
  	//walk down hierarchy to attributes          	
  	for(; node != NULL; node = node->getNextSibling()){
        const XMLCh* xmlName = node->getNodeName();
	DOMNamedNodeMap * atts = node->getAttributes();
		
        // check if its an attribute to add
        if( 0==XMLString::compareNString( xmlName, attributeStr,sizeof(attributeStr))){
          char* sType;
          char* sValue="";
          sType = XMLString::transcode(atts->getNamedItem(typeAttrStr)->getNodeValue());
	  if(atts->getNamedItem(valueAttrStr)!=NULL){
          	sValue = XMLString::transcode(atts->getNamedItem(valueAttrStr)->getNodeValue());
	  }
	  
	  Attribute fr_Attribute2( sType, sValue);

	  //fr_Attribute.addNewAttribute(fr_Attribute2, sValue);
          // XMLString::release(&sValue);
   	  nextAttribute(fr_Attribute2,node->getFirstChild());
	  fr_Attribute.addNewAttribute(fr_Attribute2, sType);
  	}
	// check if its an Parameter to add
        if( 0==XMLString::compareNString( xmlName, parameterStr,sizeof(parameterStr))){
          char* sType;
          char* sValue;
          sType = XMLString::transcode(atts->getNamedItem(typeAttrStr)->getNodeValue());
          sValue = XMLString::transcode(atts->getNamedItem(valueAttrStr)->getNodeValue());
	  fr_Attribute.addNewParameter( sType, sValue);
  	}
	}
  }

}// namespace SystemC_VPC
