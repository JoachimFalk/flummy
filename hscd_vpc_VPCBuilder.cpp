#include <iostream>
#include <values.h> 
#include <algorithm>
#include <cctype>
#include <string>

#include "hscd_vpc_VPCBuilder.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_VpcDomErrorHandler.h"
#include "hscd_vpc_datatypes.h"
#include "StaticRoute.h"

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
        std::cerr << "VPCBuilder> Warning: could not open '" << cfile << "'"
                  << std::endl;

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
                        << comp->getName() << " to Director" << endl;
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

        }else if( 0==XMLString::compareNString( xmlName,
                                                resultfileStr,
                                                sizeof(resultfileStr) ) ){
           
           DOMNamedNodeMap * atts=node->getAttributes();
            std::string vpc_result_file =
              XMLString::transcode(
                atts->getNamedItem(nameAttrStr)->getNodeValue());
            this->director->setResultFile(vpc_result_file);
        
        }else if( 0==XMLString::compareNString( xmlName,
                                                topologyStr,
                                                sizeof(resourcesStr) ) ){
          node = vpcConfigTreeWalker->getCurrentNode();
          parseTopology( node );
        }else{
        
        }

        node = vpcConfigTreeWalker->nextSibling();
      }

      // clean up pareser
      configParser->release();
      delete configErrorh;
      
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
      std::vector<Timing > timings;
      // find all attributes
      for(; node != NULL; node = this->vpcConfigTreeWalker->nextSibling()){
        const XMLCh* xmlName = node->getNodeName();

#ifdef VPC_DEBUG
        std::cerr << "VPCBuilder> init template " << tid << std::endl;
#endif //VPC_DEBUG

        if( 0==XMLString::compareNString( xmlName, timingStr,
                                          sizeof(timingStr))){
          Timing t = this->parseTiming( node );
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
    cerr << "VPC> InitAttribute for Component name=" << comp->getName()
         << endl;
#endif //VPC_DEBUG
    if(node != NULL){
      // find all attributes
      for(; node != NULL; node = this->vpcConfigTreeWalker->nextSibling()){
        const XMLCh* xmlName = node->getNodeName();
        DOMNamedNodeMap * atts = node->getAttributes();

        // check if its an attribute to add
        if( 0==XMLString::compareNString( xmlName,
                                          attributeStr,
                                          sizeof(attributeStr))){

          char* sType;
          char* sValue = "";
          sType = XMLString::transcode(
            atts->getNamedItem(typeAttrStr)->getNodeValue());

          DOMNode * value = atts->getNamedItem(valueAttrStr);
          if( value  != NULL){
            sValue= XMLString::transcode(
              atts->getNamedItem(valueAttrStr)->getNodeValue());
          }

          Attribute attributes( sType, sValue);
          cerr << "create Attribute t=" << sType;
          cerr << " v=" << attributes.getValue();
          cerr << std::endl;

          XMLString::release(&sType);
          if( value  != NULL){
            XMLString::release(&sValue);
          }

          nextAttribute(attributes, node->getFirstChild());

          //comp->processAndForwardParameter(sType,sValue);
          comp->setAttribute(attributes);

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
              Timing t = this->parseTiming( attnode );

              p.addFuncLatency( this->director, sTarget, t.fname, t.latency );
              p.addFuncDelay( this->director, sTarget, t.fname, t.dii );
              
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
          this->director->registerMapping(sSource, sTarget);
          
        }else{
          std::cerr << "VPCBuilder> No valid component found for mapping:"
            " source=" << sSource << " target=" << sTarget<< endl;
        }
      }
    }
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
    std::map<std::string, std::vector<Timing> >::iterator
      timingIter = timingTemplates.find(key);
    if(timingIter != timingTemplates.end()){
      for(std::vector<Timing>::iterator
            timings = timingTemplates[key].begin();
	  timings != timingTemplates[key].end();
          ++timings)
      {
        Timing t = *timings;
        p->setDelay( t.dii );
        p->setLatency(t.latency );
        p->addFuncDelay( this->director, target, t.fname, t.dii );
        p->addFuncLatency( this->director, target, t.fname, t.latency );
      }
    }
    
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
          cerr << "create Attribute t=" << sType << endl;

	  //fr_Attribute.addNewAttribute(fr_Attribute2, sValue);
          // XMLString::release(&sValue);
   	  nextAttribute(fr_Attribute2,node->getFirstChild());
	  fr_Attribute.addAttribute(sType, fr_Attribute2);
  	}
	// check if its an Parameter to add
        if( 0==XMLString::compareNString( xmlName, parameterStr,sizeof(parameterStr))){
          char* sType;
          char* sValue;
          sType = XMLString::transcode(atts->getNamedItem(typeAttrStr)->getNodeValue());
          sValue = XMLString::transcode(atts->getNamedItem(valueAttrStr)->getNodeValue());
	  fr_Attribute.addParameter( sType, sValue);
          cerr << "add Parameter t=" << sType << " v=" << sValue << endl;
  	}
	}
  }

  void VPCBuilder::parseTopology( DOMNode* top ){
    // iterate children of <topology>
    try{
      for(DOMNode * routeNode = top->getFirstChild();
          routeNode != NULL;
          routeNode = routeNode->getNextSibling()){
        
        const XMLCh* xmlName = routeNode->getNodeName();

        if( 0==XMLString::compareNString( xmlName,
                                          routeStr,
                                          sizeof(routeStr))){
          
          // scan <route>
          DOMNamedNodeMap * atts = routeNode->getAttributes();
          std::string src = XMLString::transcode(
            atts->getNamedItem(sourceAttrStr)->getNodeValue() );
          std::string dest = XMLString::transcode(
            atts->getNamedItem(destinationAttrStr)->getNodeValue() );

          StaticRoute * route = new StaticRoute(src, dest); 
          this->director->registerComponent(route);
          this->director->registerMapping(route->getName(), route->getName());

          ProcessControlBlock& pcb =
            this->director->generatePCB( route->getName() );

          // add <hop>s
          for(DOMNode * hopNode = routeNode->getFirstChild();
              hopNode != NULL;
              hopNode = hopNode->getNextSibling()){
            const XMLCh* xmlName = hopNode->getNodeName();
            if( 0==XMLString::compareNString( xmlName,
                                              hopStr,
                                              sizeof(hopStr) ) ){
              std::string name =
                XMLString::transcode(hopNode->getAttributes()->
                                     getNamedItem(nameAttrStr)->
                                     getNodeValue() );

              std::map<std::string, AbstractComponent* >::iterator iterComp =
                this->knownComps.find(name);
              assert( iterComp != this->knownComps.end() );
              
              route->addHop( name, iterComp->second );
              (iterComp->second)->informAboutMapping(route->getName());

              // parse <timing>s
              for(DOMNode * timingNode = hopNode->getFirstChild();
                  timingNode != NULL;
                  timingNode = timingNode->getNextSibling()){
                const XMLCh* xmlName = timingNode->getNodeName();
              
                if( 0==XMLString::compareNString( xmlName,
                                                  timingStr,
                                                  sizeof(timingStr) ) ){
                  Timing t = this->parseTiming( timingNode );
                  pcb.addFuncDelay( this->director, name, t.fname, t.dii );
                  pcb.addFuncLatency( this->director, name , t.fname, t.latency );
                }
              }
            }
          }
        }
      }
    }catch(InvalidArgumentException &e){
      std::cerr << "VPCBuilder> " << e.what() << std::endl;
      std::cerr << "VPCBuilder> ignoring topology section,"
        " continue initialization" << std::endl;
    }
  }

  //
  Timing VPCBuilder::parseTiming(DOMNode* node){
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
    Timing t;
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
      t.dii   = sc_dii;
    }
    
    return t;
  }

}// namespace SystemC_VPC
