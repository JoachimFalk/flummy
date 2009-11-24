#include <iostream>
#include <limits.h> 
#include <algorithm>
#include <cctype>
#include <string>

#include <systemcvpc/hscd_vpc_VPCBuilder.h>
#include <systemcvpc/hscd_vpc_VpcDomErrorHandler.h>
#include <systemcvpc/hscd_vpc_datatypes.h>
#include <systemcvpc/hscd_vpc_Component.h>
#include <systemcvpc/FCFSComponent.h>
#include <systemcvpc/BlockingTransport.h>
#include <systemcvpc/hscd_vpc_Director.h>
#include <systemcvpc/StaticRoute.h>
#include <systemcvpc/RoutePool.h>

#include <systemcvpc/debug_config.h>
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_VPCBUILDER
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include <systemcvpc/debug_on.h>
#else
  #include <systemcvpc/debug_off.h>
#endif

namespace SystemC_VPC{
#define MAX(x,y) ((x > y) ? x : y)

  const char* VPCBuilder::B_TRANSPORT =                      "blocking";
  const char* VPCBuilder::STATIC_ROUTE =                     "static_route";
  const char* VPCBuilder::STR_VPC_THREADEDCOMPONENTSTRING =  "threaded";
  const char* VPCBuilder::STR_VPC_DELAY =                    "delay";
  const char* VPCBuilder::STR_VPC_LATENCY =                  "latency";
  const char* VPCBuilder::STR_VPC_PRIORITY =                 "priority";
  const char* VPCBuilder::STR_VPC_PERIOD =                   "period";
  const char* VPCBuilder::STR_VPC_DEADLINE =                 "deadline";

  /**
   * \brief sets ups VPC Framework
   */
  void VPCBuilder::buildVPC(){

    FALLBACKMODE=false;
    
    // open file and check existence
    FILE* fconffile;
    char* cfile = getenv("VPCCONFIGURATION");
    DBG_OUT("VPCBuilder> VPCCONFIGURATION set to " << cfile
            << std::endl);
        
    if(cfile){
      fconffile=fopen(cfile,"r");
      if( NULL == fconffile ){       // test if file exists

        // for joachim
        std::cerr << "VPCBuilder> Warning: could not open '" << cfile << "'"
                  << std::endl;

        FALLBACKMODE=true;
      }else{
        fclose(fconffile);
        DBG_OUT("configuration: "<<cfile << std::endl);
      }
    }else{
      FALLBACKMODE=true;
    }
    
    // init vars for parsing
    if(FALLBACKMODE){
      this->director->FALLBACKMODE = true;
      DBG_OUT("running fallbackmode" << std::endl);
    }else{
      // process xml
      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* vpcConfigDoc;
      XERCES_CPP_NAMESPACE_QUALIFIER DOMBuilder* configParser;
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
          << XMLString::transcode( toCatch.getMessage()) << "\n" << std::endl;
          return;
      }
      catch (const DOMException& toCatch) {
        const unsigned int maxChars = 2047;
        XMLCh errText[maxChars + 1];
          
        std::cerr << "\nVPCBuilder> DOM Error while parsing xml file: '"
                  << cfile << "'\n"
                  << "DOMException code is:  "
                  << XMLString::transcode( toCatch.msg) << std::endl;
          
        if (DOMImplementation::loadDOMExceptionMsg(toCatch.code,
                                                   errText,
                                                   maxChars))
          std::cerr << "Message is: "
                    << XMLString::transcode( errText) << std::endl;
        return;
      }
      catch (...) {
        std::cerr << "\nVPCBuilder> Unexpected exception while parsing"
                  << " xml file: '" << cfile << "'\n";
        return;
      }
      
      //check if parsing failed
      if(configErrorh->parseFailed()){
        DBG_OUT("VPCBuilder: Parsing of configuration failed,"
                " aborting initialization!" << std::endl);
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
        
        // find resources tag
        if( 0==XMLString::compareNString( xmlName,
                                          resourcesStr,
                                          sizeof(resourcesStr) ) ){
        
          // walk down hierachy to components
          node = vpcConfigTreeWalker->firstChild();
          if(node != NULL){
            // pointer to currently initiated component
            AbstractComponent* comp;
            // init all components
            for(; node != NULL; node = vpcConfigTreeWalker->nextSibling()){
              const XMLCh* xmlName = node->getNodeName();
              try{
                if( 0==XMLString::compareNString(
                         xmlName,
                         VPCBuilder::componentStr,
                         sizeof(VPCBuilder::componentStr))) {
                  comp = initComponent();
                  DBG_OUT("VPCBuilder> registering component: "
                          << comp->getName() << " to Director" << std::endl);
                  // register "upper-layer" components to Director
                  this->director->registerComponent(comp);
                  comp->setParentController(this->director);


                }else{
                  if( 0==XMLString::compareNString( xmlName,
                                                    attributeStr,
                                                    sizeof(attributeStr))){

                    char* sType = XMLString::transcode(
                      node->getAttributes()->getNamedItem(typeAttrStr)->getNodeValue());
                    char* sValue = "";
                    DOMNode * value = node->getAttributes()->getNamedItem(valueAttrStr);
                    if( value  != NULL){
                      sValue= XMLString::transcode(
                        node->getAttributes()->getNamedItem(valueAttrStr)->getNodeValue());
                    }

                    if( 0==XMLString::compareNString( sType,
                                                       "global_governor",
                                                       sizeof("global_governor"))){
                       AttributePtr gov(new Attribute("global_governor", sValue));
                       nextAttribute(gov, node->getFirstChild());
                       director->loadGlobalGovernorPlugin(sValue, gov);
                     }

                    
                    
                    XMLString::release(&sType);
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
        }else if( 0==XMLString::compareNString( xmlName,
                                                mappingsStr,
                                                sizeof(mappingsStr) ) ){

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
    DBG_OUT("Initializing VPC finished!" << std::endl);
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
      const char* sType = STR_VPC_THREADEDCOMPONENTSTRING;
      char* sScheduler;
      AbstractComponent* comp = NULL;
  
      sName = XMLString::transcode(
        atts->getNamedItem(VPCBuilder::nameAttrStr)->getNodeValue());

      DOMNode * value = atts->getNamedItem(VPCBuilder::typeAttrStr);
      if( value  != NULL){
        sType = XMLString::transcode(
            atts->getNamedItem(VPCBuilder::typeAttrStr)->getNodeValue());
      }

      sScheduler = XMLString::transcode(
            atts->getNamedItem(VPCBuilder::schedulerAttrStr)->getNodeValue());
  
      DBG_OUT("VPCBuilder> initComponent: " << sName << std::endl);

      // check which kind of component is defined
      // standard component
      if(0==strncmp(sType,
                    STR_VPC_THREADEDCOMPONENTSTRING,
                    sizeof(STR_VPC_THREADEDCOMPONENTSTRING))){

        DBG_OUT("VPCBuilder> Found Component name=" << sName << " type="
                  << sType << std::endl);
        
        // check if component already exists
        //std::map<std::string, AbstractComponent* >::iterator iter
        //  = this->knownComps.find(sName);
        //if(iter == this->knownComps.end()){ 
        if( 0==strncmp(sScheduler, STR_FIRSTCOMEFIRSTSERVE,
                       strlen(STR_FIRSTCOMEFIRSTSERVE))
            || 0==strncmp(sScheduler,STR_FCFS,strlen(STR_FCFS))){

          // ** FIXME: here we add the new FCFSComponent
          //comp = new FCFSComponent(sName, director);
          comp = new Component(sName, sScheduler, director);
        }else{
          comp = new Component(sName,sScheduler,director);
        }
          this->knownComps.insert(
            std::pair<std::string, AbstractComponent* >(sName, comp));
        //}else{
        //  comp = iter->second;
        //}
        //this->initCompAttributes(comp, node->getFirstChild());
        this->initCompAttributes(comp);
        
      }else // unkown component type
      {
        
        std::string msg("Unknown Component: name=");
        msg += sName;
        msg += " type=";
        msg += sType;
        throw InvalidArgumentException(msg);
        
      }

      return comp;    
    }

    std::string msg("Unknown configuration tag: ");
    char *name = XMLString::transcode(xmlName);
    msg.append(name, std::strlen (name));
    XMLString::release(&name);
    throw InvalidArgumentException(msg);

  }
 
  /**
   * \brief Performs initialization of attribute values for a component
   * \param comp specifies the component to set attributes for
   * \param node specifies current position within dom tree
   */
  void VPCBuilder::initCompAttributes(AbstractComponent* comp){
    DOMNode* node = this->vpcConfigTreeWalker->firstChild(); 
    DBG_OUT("VPC> InitAttribute for Component name=" << comp->getName()
         << std::endl);
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

          AttributePtr attributes(new Attribute( sType, sValue));

          XMLString::release(&sType);
          if( value  != NULL){
            XMLString::release(&sValue);
          }

          nextAttribute(attributes, node->getFirstChild());

          comp->setAttribute(attributes);
        }
      }
      vpcConfigTreeWalker->parentNode();
    }
  }

  /**
   * \brief Initializes the mappings and process structures.
   */
  //void VPCBuilder::initMappingAPStruct(DOMNode* node){
  void VPCBuilder::initMappingAPStruct(){

    DOMNode* node = this->vpcConfigTreeWalker->getCurrentNode();
    
    const XMLCh* xmlName=node->getNodeName();

    DBG_OUT("VPCBuilder> entering initMappingAPStruct"<< std::endl);    
   
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


      DBG_OUT( "VPCBuilder> Found mapping attribute: source=" << sSource
               << " target=" << sTarget << std::endl); 

      // check if component exists
      if(this->knownComps.count(sTarget)==1){
        
        DBG_OUT( "VPCBuilder> Target of Mapping: " << sTarget << " exists!"
                 << std::endl); 

        std::map<std::string, AbstractComponent* >::iterator iterComp;
        iterComp = this->knownComps.find(sTarget);

        // first of all initialize PCB for task        
        if(iterComp != this->knownComps.end()){
          AbstractComponent* comp = iterComp->second;
          DBG_OUT( "VPCBuilder> Configure mapping: " << sSource << "<->"
                   << sTarget << std::endl); 

          // adding mapping info
          this->director->registerMapping(sSource, sTarget);
          comp->informAboutMapping(sSource);
          
          //generate new ProcessControlBlock or get existing one for
          // initialization
          ProcessControlBlock& p =
            comp->createPCB(this->director->getProcessId(sSource));
          p.setName(sSource);
            
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

              DBG_OUT( "PCB: " << sSource
                       << " " << t.fid
                       << " " << t.latency
                       << " " << t.dii
                       << std::endl);
              //p.addLatency( t.fid, t.latency );
              //p.addDelay( t.fid, t.dii );
              p.setTiming(t);
              
            }else if( 0==XMLString::compareNString( xmlName,
                                                    attributeStr,
                                                    sizeof(attributeStr))){
              char *sType, *sValue;
              sType=XMLString::transcode(
                atts->getNamedItem(typeAttrStr)->getNodeValue());
              sValue=XMLString::transcode(
                atts->getNamedItem(valueAttrStr)->getNodeValue());

              DBG_OUT("attribute values are: " <<sType
                      << " and " << sValue << std::endl);
          
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
                p.setBaseDelay( delay );
              }else if( 0 == strncmp(sType,
                                     STR_VPC_LATENCY,
                                     sizeof(STR_VPC_LATENCY) )){
                sc_time latency = Director::createSC_Time(sValue);
                p.setBaseLatency( latency );
              }else{
                DBG_OUT("VPCBuilder> Unknown mapping attribute: type="
                          << sType << " value=" << sValue << endl); 
                DBG_OUT("VPCBuilder> Try to interpret as function"
                  " specific delay!!" << std::endl);

                try{  
                  sc_time delay = Director::createSC_Time(sValue);
                  DBG_OUT("VPCBuilder> Try to interpret as"
                          " function specific delay!!" << std::endl);
                  DBG_OUT("VPCBuilder> Register delay to: "
                            << sTarget << "; " << sType << ", " << delay
                            << std::endl);
                  p.addDelay( this->director->createFunctionId(sType),
                              delay );
                  // using attribute="functionName" for compatibility:
                  // force to have a latency
                  p.addLatency( this->director->createFunctionId(sType),
                                delay );

                } catch(const InvalidArgumentException& ex) {
                  DBG_OUT("VPCBuilder> Mapping realy unknown!" << std::endl);
                }
              }

              XMLString::release(&sType);
              XMLString::release(&sValue);
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
                                 DOMNode* node){
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
          
          AttributePtr fr_Attribute2(new Attribute(sType, sValue));

          //fr_Attribute.addNewAttribute(fr_Attribute2, sValue);
          // XMLString::release(&sValue);
          nextAttribute(fr_Attribute2,node->getFirstChild());
          attribute->addAttribute(sType, fr_Attribute2);
        }
        // check if its an Parameter to add
        if( 0==XMLString::compareNString( xmlName, parameterStr,sizeof(parameterStr))){
          char* sType;
          char* sValue;
          sType = XMLString::transcode(atts->getNamedItem(typeAttrStr)->getNodeValue());
          sValue = XMLString::transcode(atts->getNamedItem(valueAttrStr)->getNodeValue());
          attribute->addParameter( sType, sValue);
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

          std::string type = STATIC_ROUTE;
          if(atts->getNamedItem(typeAttrStr)!=NULL){
            type = XMLString::transcode(
                      atts->getNamedItem(typeAttrStr)->getNodeValue() );
          }

          Route * route = NULL;
          if(type == B_TRANSPORT){
            route = new RoutePool<BlockingTransport>(src, dest);
          } else if(type == STATIC_ROUTE) {
            route = new RoutePool<StaticRoute>(src, dest);
          } else{
            std::string msg("Unknown Routing type: type=");
            msg += type;
            std::cerr << msg << endl;
            throw InvalidArgumentException(msg);
          }

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
              AbstractComponent* hop = iterComp->second; 
              ProcessControlBlock& pcb =
                hop->createPCB(this->director->getProcessId(route->getName()));
              pcb.setName(route->getName());

              
              route->addHop( name, hop );
              (iterComp->second)->informAboutMapping(route->getName());

              // parse <timing>s
              for(DOMNode * timingNode = hopNode->getFirstChild();
                  timingNode != NULL;
                  timingNode = timingNode->getNextSibling()){
                const XMLCh* xmlName = timingNode->getNodeName();
                DOMNamedNodeMap* atts=timingNode->getAttributes();
                if( 0==XMLString::compareNString( xmlName,
                                                  timingStr,
                                                  sizeof(timingStr) ) ){
                  Timing t = this->parseTiming( timingNode );
                  //pcb.addDelay( t.fid, t.dii );
                  //pcb.addLatency( t.fid, t.latency );
                 // pcb.setPriority(5);
                  pcb.setTiming(t);
                }else if( 0==XMLString::compareNString( xmlName,
                                                    attributeStr,
                                                    sizeof(attributeStr))){
                  char *sType, *sValue;
                  sType=XMLString::transcode(
                    atts->getNamedItem(typeAttrStr)->getNodeValue());
                  sValue=XMLString::transcode(
                    atts->getNamedItem(valueAttrStr)->getNodeValue());
    
                  if( 0 == strncmp(sType,
                                  STR_VPC_PRIORITY,
                                  sizeof(STR_VPC_PRIORITY) )){
                    int priority = 0;
                    sscanf(sValue, "%d", &priority);
                    
                    pcb.setPriority(priority);
                  }
                }
              }
            }
          }
          //this->director->registerComponent(route);
          //this->director->registerMapping(route->getName(), route->getName());
          this->director->registerRoute(route);

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
    char *powerMode=NULL;
          
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
      }else if(0==XMLString::compareNString( a->getNodeName(),
                                             powerModeStr,
                                             sizeof(powerModeStr))){
        powerMode = XMLString::transcode(a->getNodeValue());
      }
    }
    Timing t;
    sc_time sc_latency = SC_ZERO_TIME;
    sc_time sc_dii     = SC_ZERO_TIME;
    t.powerMode        = "SLOW";
  
    if(latency != NULL) sc_latency = Director::createSC_Time(latency);
    if(dii != NULL) sc_dii = Director::createSC_Time(dii);
    { // latency and delay are synonym -> take maximum if they differ
      sc_time sc_delay = SC_ZERO_TIME;
      if(delay != NULL) sc_delay = Director::createSC_Time(delay);
      sc_latency = MAX(sc_latency,sc_delay);
    }

    t.fid = defaultFunctionId;
    if(fname != NULL){
      t.fid = this->director->createFunctionId(fname);
    }

    if(powerMode != NULL){
      t.powerMode = powerMode;
    }
    
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
