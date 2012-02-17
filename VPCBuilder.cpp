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

#include <iostream>
#include <limits.h> 
#include <algorithm>
#include <cctype>
#include <string>
#include <stdlib.h>

#include <CoSupport/XML/xerces_support.hpp>
#include <CoSupport/Tracing/TracingFactory.hpp>


#include <systemcvpc/VPCBuilder.hpp>
#include <systemcvpc/VpcDomErrorHandler.hpp>
#include <systemcvpc/datatypes.hpp>
#include <systemcvpc/Director.hpp>
#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/Route.hpp>
#include <systemcvpc/config/Timing.hpp>
#include <systemcvpc/config/VpcApi.hpp>

#include "ComponentImpl.hpp"
#include "ConfigCheck.hpp"
#include "NonPreemptiveComponent.hpp"
#include "config/Mappings.hpp"

#include <systemcvpc/debug_config.hpp>
// if compiled with DBG_COMPONENT create stream and include debug macros
#ifdef DBG_VPCBUILDER
#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/FilterOStream.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include <systemcvpc/debug_on.hpp>
#else
  #include <systemcvpc/debug_off.hpp>
#endif

//grocki: random
#include <systemcvpc/TimingModifier.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/bernoulli_distribution.hpp>
#include <boost/random/binomial_distribution.hpp>
#include <boost/random/cauchy_distribution.hpp>
#include <boost/random/exponential_distribution.hpp>
#include <boost/random/gamma_distribution.hpp>
#include <boost/random/geometric_distribution.hpp>
#include <boost/random/lognormal_distribution.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/poisson_distribution.hpp>
#include <boost/random/triangle_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <time.h>
typedef boost::minstd_rand base_generator_type;
// grocki: end

namespace SystemC_VPC{

using namespace CoSupport::XML::Xerces;
namespace VC = Config;

#define MAX(x,y) ((x > y) ? x : y)

  const char* VPCBuilder::STR_VPC_THREADEDCOMPONENTSTRING =  "threaded";
  const char* VPCBuilder::STR_VPC_DELAY =                    "delay";
  const char* VPCBuilder::STR_VPC_LATENCY =                  "latency";
  const char* VPCBuilder::STR_VPC_PRIORITY =                 "priority";
  const char* VPCBuilder::STR_VPC_PERIOD =                   "period";
  const char* VPCBuilder::STR_VPC_DEADLINE =                 "deadline";

  void testAndRemoveFile(std::string fileName){
    std::ofstream file(fileName.c_str());
    if (file.good()) {
      file.close();
      std::remove(fileName.c_str());
    }
  }
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
          << NStr( toCatch.getMessage()) << "\n" << std::endl;
          return;
      }
      catch (const DOMException& toCatch) {
        const unsigned int maxChars = 2047;
        XMLCh errText[maxChars + 1];
          
        std::cerr << "\nVPCBuilder> DOM Error while parsing xml file: '"
                  << cfile << "'\n"
                  << "DOMException code is:  "
                  << NStr( toCatch.msg) << std::endl;
          
        if (DOMImplementation::loadDOMExceptionMsg(toCatch.code,
                                                   errText,
                                                   maxChars))
          std::cerr << "Message is: "
                    << NStr( errText) << std::endl;
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

//TODO: grocki: random
      DOMNode* nodetest = (DOMNode*)vpcConfigDoc->getDocumentElement();
      DOMNamedNodeMap* atts=nodetest->getAttributes();
      DOMNode* seed = atts->getNamedItem(seedAttrStr);
      bool hasSeed = (seed != NULL);
      if (hasSeed){
        std::istringstream stm;
        stm.str(NStr(seed->getNodeValue()));
        double value;
        stm >>value;
        this->generator = boost::shared_ptr<base_generator_type>(new base_generator_type(value));
      } else {
        this->generator = boost::shared_ptr<base_generator_type>(new base_generator_type(time(NULL)));
      } 
//TODO: grocki: end
      
      // moves the Treewalker to the first Child 
      DOMNode* node = vpcConfigTreeWalker->firstChild(); 
      // name of xmlTag
      XStr xmlName;

      while(node!=0){
        xmlName = node->getNodeName();
        
        // find resources tag
        if( xmlName == resourcesStr ){
        
          // walk down hierachy to components
          node = vpcConfigTreeWalker->firstChild();
          if(node != NULL){
            // pointer to currently initiated component
            // init all components
            for(; node != NULL; node = vpcConfigTreeWalker->nextSibling()){
              const XStr xmlName = node->getNodeName();
              try{
                if( xmlName == VPCBuilder::componentStr ) {
                  VC::Component::Ptr comp = initComponent();
                  DBG_OUT("VPCBuilder> register component: "
                          << comp->getName() << " to Director" << std::endl);


                }else{
                  if( xmlName == attributeStr ){

                    XStr sType = 
                      node->getAttributes()->getNamedItem(typeAttrStr)->getNodeValue();
                    XStr sValue = "";
                    DOMNode * value = node->getAttributes()->getNamedItem(valueAttrStr);
                    if( value  != NULL){
                      sValue=
                        node->getAttributes()->getNamedItem(valueAttrStr)->getNodeValue();
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
        }else if( xmlName == mappingsStr ){

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

        }else if( xmlName == resultfileStr ){
           
           DOMNamedNodeMap * atts=node->getAttributes();
           XStr vpc_result_file =
             atts->getNamedItem(nameAttrStr)->getNodeValue();
           this->director->setResultFile(vpc_result_file);
        
        }else if( xmlName == topologyStr ){
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
   * configuration file is unknown
   */
  VC::Component::Ptr VPCBuilder::initComponent()
    throw(InvalidArgumentException)
  {
 
    DOMNode* node = this->vpcConfigTreeWalker->getCurrentNode();
    const XStr xmlName = node->getNodeName(); 

    // check for component tag
    if( xmlName == VPCBuilder::componentStr ){
      
      DOMNamedNodeMap* atts = node->getAttributes();
  
      XStr sScheduler =
        atts->getNamedItem(VPCBuilder::schedulerAttrStr)->getNodeValue();
      XStr sName = atts->getNamedItem(VPCBuilder::nameAttrStr)->getNodeValue();
  
      DBG_OUT("VPCBuilder> initComponent: " << sName << std::endl);

      VC::Component::Ptr comp =
          VC::createComponent(sName, VC::Scheduler::parseScheduler(sScheduler));

      this->initCompAttributes(comp);
        
      return comp;    
    }

    std::string msg("Unknown configuration tag: ");
    msg += NStr(xmlName);
    throw InvalidArgumentException(msg);

  }
 
  /**
   * \brief Performs initialization of attribute values for a component
   * \param comp specifies the component to set attributes for
   * \param node specifies current position within dom tree
   */
  void VPCBuilder::initCompAttributes(VC::Component::Ptr comp){
    DOMNode* node = this->vpcConfigTreeWalker->firstChild(); 
    DBG_OUT("VPC> InitAttribute for Component name=" << comp->getName()
         << std::endl);
    if(node != NULL){
      // find all attributes
      for(; node != NULL; node = this->vpcConfigTreeWalker->nextSibling()){
        const XStr xmlName = node->getNodeName();
        DOMNamedNodeMap * atts = node->getAttributes();

        // check if its an attribute to add
        if( xmlName == attributeStr ){

          //NStr sType;
          NStr sValue = "";
          NStr sType = atts->getNamedItem(typeAttrStr)->getNodeValue();

          DOMNode * value = atts->getNamedItem(valueAttrStr);
          if( value  != NULL){
            sValue = atts->getNamedItem(valueAttrStr)->getNodeValue();
          }

          AttributePtr attributes(new Attribute( sType, sValue));

          nextAttribute(attributes, node->getFirstChild());

          // FIXME: replace old parsing?
          comp->addAttribute(attributes);
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
    
    XStr xmlName=node->getNodeName();

    DBG_OUT("VPCBuilder> entering initMappingAPStruct"<< std::endl);    
   
    // find mapping tag (not mappings)
    if( xmlName == mappingStr ){
      DOMNamedNodeMap* atts=node->getAttributes();
      NStr sTarget = atts->getNamedItem(targetAttrStr)->getNodeValue();

      NStr sSource = atts->getNamedItem(sourceAttrStr)->getNodeValue();


      DBG_OUT( "VPCBuilder> Found mapping attribute: source=" << sSource
               << " target=" << sTarget << std::endl); 

      {
        if(VC::hasComponent(sTarget)){
          //AbstractComponent* comp = iterComp->second;
          DBG_OUT( "VPCBuilder> Configure mapping: " << sSource << "<->"
                   << sTarget << std::endl); 

          assert(VC::hasComponent(sTarget));
          VC::Component::Ptr comp = VC::getComponent(sTarget);
          VC::VpcTask::Ptr task = VC::getCachedTask(sSource);
          VC::Mappings::getConfiguredMappings()[task] = comp;
          VC::DefaultTimingsProvider::Ptr provider = comp->getDefaultTimingsProvider();

          //walk down hierarchy to attributes
          DOMNode* attnode = node->getFirstChild();
          
          // find all attributes
          for(; attnode!=NULL; attnode = attnode->getNextSibling()){

            xmlName=attnode->getNodeName();
            DOMNamedNodeMap * atts=attnode->getAttributes();

            if( xmlName == timingStr ){
              try {
                VC::Timing t = this->parseTiming( attnode );
//grocki: random?
                if (t.getFunctionId() == defaultFunctionId) {
                  provider->addDefaultActorTiming(sSource, t);
                } else {
                  provider->add(t);
                }

              } catch(InvalidArgumentException &e) {
                std::string msg("Error with mapping: ");
                msg += sSource + " -> " + sTarget + "\n";
                msg += e.what();
                throw InvalidArgumentException(msg);
              }
              
            }else if( xmlName == attributeStr ){
              XStr sType  = atts->getNamedItem(typeAttrStr)->getNodeValue();
              XStr sValue = atts->getNamedItem(valueAttrStr)->getNodeValue();

              DBG_OUT("attribute values are: " <<sType
                      << " and " << sValue << std::endl);
          
              if( sType == STR_VPC_PRIORITY){
                int priority = 0;
                sscanf(NStr(sValue), "%d", &priority);
                task->setPriority(priority);
              }else if( sType == STR_VPC_DEADLINE){
//                task->setDeadline(Director::createSC_Time(sValue));
              }else if( sType == STR_VPC_PERIOD){
//                task->setPeriod(Director::createSC_Time(sValue));
              }else if( sType == STR_VPC_DELAY){
                sc_time delay = Director::createSC_Time(sValue);
//                task->setBaseDelay( delay );
              }else if( sType == STR_VPC_LATENCY){
                sc_time latency = Director::createSC_Time(sValue);
//                task->setBaseLatency( latency );
              }else{
                DBG_OUT("VPCBuilder> Unknown mapping attribute: type="
                          << sType << " value=" << sValue << endl); 
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
                                 DOMNode* node){
        //walk down hierarchy to attributes             
        for(; node != NULL; node = node->getNextSibling()){
        const XStr xmlName = node->getNodeName();
        DOMNamedNodeMap * atts = node->getAttributes();
                
        // check if its an attribute to add
        if( xmlName == attributeStr ){
          XStr sValue="";
          XStr sType = atts->getNamedItem(typeAttrStr)->getNodeValue();
          if(atts->getNamedItem(valueAttrStr)!=NULL){
                sValue = atts->getNamedItem(valueAttrStr)->getNodeValue();
          }
          
          AttributePtr fr_Attribute2(new Attribute(sType, sValue));

          //fr_Attribute.addNewAttribute(fr_Attribute2, sValue);
          nextAttribute(fr_Attribute2,node->getFirstChild());
          attribute->addAttribute(sType, fr_Attribute2);
        }
        // check if its an Parameter to add
        if( xmlName == parameterStr ){
          XStr sType  = atts->getNamedItem(typeAttrStr)->getNodeValue();
          XStr sValue = atts->getNamedItem(valueAttrStr)->getNodeValue();
          attribute->addParameter( sType, sValue);
        }
     }
  }

  void VPCBuilder::parseTopology( DOMNode* top ){
    // iterate children of <topology>
    try{
      // scan <topology>
      DOMNamedNodeMap * atts = top->getAttributes();
      DOMNode    *tracingAtt = atts->getNamedItem(tracingAttrStr);
      bool topologyTracing = (tracingAtt != NULL) &&
          (std::string("true") == NStr(tracingAtt->getNodeValue()) );

      // check if tracing is enabled for any route -> open trace file
      bool tracingEnabled = false;

      // define the empty default route behavior
      DOMNode    *defaultRouteAttr = atts->getNamedItem(defaultRouteAttrStr);
      director->defaultRoute = (defaultRouteAttr != NULL) &&
          (std::string("ignore") == NStr(defaultRouteAttr->getNodeValue()) );

      for(DOMNode * routeNode = top->getFirstChild();
          routeNode != NULL;
          routeNode = routeNode->getNextSibling()){
        
        const XStr xmlName = routeNode->getNodeName();

        if( xmlName == routeStr ){
          
          // scan <route>
          DOMNamedNodeMap * atts = routeNode->getAttributes();
          std::string src = NStr(
            atts->getNamedItem(sourceAttrStr)->getNodeValue() );
          std::string dest = NStr(
            atts->getNamedItem(destinationAttrStr)->getNodeValue() );

          VC::Route::Type t = VC::Route::StaticRoute;
          if(atts->getNamedItem(typeAttrStr)!=NULL){
            t = VC::Route::parseRouteType(
                NStr(atts->getNamedItem(typeAttrStr)->getNodeValue()));
            //type = NStr( );
          }

          VC::Route::Ptr route = VC::createRoute(src, dest, t);

          //copy default value from <route>
          bool tracing = topologyTracing;
          DOMNode    *tracingAtt = atts->getNamedItem(tracingAttrStr);
          if (tracingAtt != NULL){
            tracing = std::string("true") == NStr(tracingAtt->getNodeValue());
          }


          tracingEnabled |= tracing;
          route->setTracing(tracing);

          // add <hop>s
          for(DOMNode * hopNode = routeNode->getFirstChild();
              hopNode != NULL;
              hopNode = hopNode->getNextSibling()){
            const XStr xmlName = hopNode->getNodeName();
            if( xmlName == hopStr ){
              std::string name =
                NStr( hopNode->getAttributes()->getNamedItem(nameAttrStr)->
                      getNodeValue() );

              assert( VC::hasComponent(name) );
              VC::Component::Ptr comp = VC::getComponent(name);

              VC::Hop &hop = route->addHop( comp );

              // parse <timing>s
              for(DOMNode * timingNode = hopNode->getFirstChild();
                  timingNode != NULL;
                  timingNode = timingNode->getNextSibling()){
                const XStr xmlName = timingNode->getNodeName();
                DOMNamedNodeMap* atts=timingNode->getAttributes();
                if( xmlName == timingStr ){
                  try {
                    VC::Timing t = this->parseTiming( timingNode );
//grocki: random
/*    DOMNode* distribution = atts->getNamedItem(distributionAttrStr);
    bool hasDistribution = (distribution != NULL);
    if (hasDistribution){
      t.setTimingModifier(this->parseTimingModifier(node));
      t.getTimingModifier()->hello();
    }*/
//grocki: end
                    hop.setTransferTiming(t);
                  } catch(InvalidArgumentException &e) {
                    std::string msg("Error with route: ");
                    msg += src + " -> " + dest + " (" + name +")\n";
                    msg += e.what();
                    throw InvalidArgumentException(msg);
                  }
                }else if( xmlName == attributeStr ){
                  XStr sType =
                    atts->getNamedItem(typeAttrStr)->getNodeValue();
                  XStr sValue =
                    atts->getNamedItem(valueAttrStr)->getNodeValue();
    
                  if( sType == STR_VPC_PRIORITY ){
                    int priority = 0;
                    sscanf(NStr(sValue).c_str(), "%d", &priority);
                    
                    hop.setPriority(priority);
                  }
                }
              }
            }
          }
        }
      }

      testAndRemoveFile("tracing.log");
      if (tracingEnabled) {
        CoSupport::Tracing::TracingFactory::getInstance().setTraceFile(
            "tracing.log");
      }

    }catch(InvalidArgumentException &e){
      std::cerr << "VPCBuilder> " << e.what() << std::endl;
      exit(-1);
    }
  }

  //
  VC::Timing VPCBuilder::parseTiming(DOMNode* node) throw(InvalidArgumentException){
    VC::Timing t;
    t.setPowerMode("SLOW");

    DOMNamedNodeMap* atts = node->getAttributes();
    if( NULL != atts->getNamedItem(powerModeStr) ) {
      t.setPowerMode(NStr(atts->getNamedItem(powerModeStr)->getNodeValue()));
    }
    if( NULL != atts->getNamedItem(fnameAttrStr) ) {
      XStr attribute = atts->getNamedItem(fnameAttrStr)->getNodeValue();
      t.setFunction(attribute);
    }

    DOMNode* dii     = atts->getNamedItem(diiAttrStr);
    DOMNode* delay   = atts->getNamedItem(delayAttrStr);
    DOMNode* latency = atts->getNamedItem(latencyAttrStr);
    bool hasDii     = (dii != NULL);
    bool hasDelay   = (delay != NULL);
    bool hasLatency = (latency != NULL);
    if (hasDelay && !hasDii && !hasLatency) {
      sc_time d = Director::createSC_Time(NStr(delay->getNodeValue()));
      t.setDii(d);
      t.setLatency(d);
    } else if (!hasDelay && hasDii && hasLatency) {
      t.setDii(Director::createSC_Time(NStr(dii->getNodeValue())));
      t.setLatency(Director::createSC_Time(NStr(latency->getNodeValue())));
    } else {
      std::string msg("Invalid timing annotation.\n");
      for(unsigned int i=0; i<atts->getLength(); i++){
        DOMNode* a=atts->item(i);
        XStr val  = a->getNodeValue();
        XStr name = a->getNodeName();
        msg += "timing: " + NStr(name) + " = " + NStr(val) + "\n";
      }
      msg += "Please specify values for dii and latency only. Alternatively, specify only the delay value. (E.g. use a delay when having identical values for dii and latency.)";

      throw InvalidArgumentException(msg);
    }

//grocki: random
    DOMNode* distribution = atts->getNamedItem(distributionAttrStr);
    bool hasDistribution = (distribution != NULL);
    if (hasDistribution){
      t.setTimingModifier(this->parseTimingModifier(node));
      t.getTimingModifier()->hello();
    }
//grocki: end

    return t;
  }


//grocki: random
  boost::shared_ptr<TimingModifier> VPCBuilder::parseTimingModifier(DOMNode* node) throw(InvalidArgumentException){
    DOMNamedNodeMap* atts = node->getAttributes();
    DOMNode* min = atts->getNamedItem(minAttrStr);
    DOMNode* max = atts->getNamedItem(maxAttrStr);
    DOMNode* parameter1 = atts->getNamedItem(parameter1AttrStr);
    DOMNode* parameter2 = atts->getNamedItem(parameter2AttrStr);
    DOMNode* parameter3 = atts->getNamedItem(parameter3AttrStr);
    DOMNode* distribution = atts->getNamedItem(distributionAttrStr);
    bool hasMin = (min != NULL);
    bool hasMax = (max != NULL);
    bool hasParameter1 = (parameter1 != NULL);
    bool hasParameter2 = (parameter2 != NULL);
    bool hasParameter3 = (parameter3 != NULL);

      std::string distr = NStr(distribution->getNodeValue());

      double minValue = -1;
      double maxValue = -1;
      if (hasMin){
        std::istringstream stm;
        stm.str(NStr(min->getNodeValue()));
        stm >> minValue;
      }
      if (minValue<-1){
        minValue=0;
      }
      if (hasMax){
        std::istringstream stm;
        stm.str(NStr(max->getNodeValue()));
        stm >> maxValue;
      }
      if (minValue >= maxValue){
        maxValue=-1;
      } 

      if (distr.compare("bernoulli")==0){
	if (hasParameter1){
          std::istringstream stm;
          double param1;
          stm.str(NStr(parameter1->getNodeValue()));
          stm >> param1;
         
          if (param1>=0 && param1<=1){
            return boost::shared_ptr<TimingModifier>(new BernoulliTimingModifier(boost::bernoulli_distribution<>(param1),this->generator,minValue,maxValue));
          }
          throw InvalidArgumentException("invalid parameter for distribution");
	}
        throw InvalidArgumentException("missing parameter for distribution");
      } else if (distr.compare("binomial")==0){
	if (hasParameter1 && hasParameter2){
          std::istringstream stm;
          int param1;
          double param2;
          stm.str(NStr(parameter1->getNodeValue()));
          stm >> param1;
          std::istringstream stm2;
          stm2.str(NStr(parameter2->getNodeValue()));
          stm2 >> param2;
         
          if (param2>=0 && param2<=1 && param1>=0){
            return boost::shared_ptr<TimingModifier>(new BinomialTimingModifier(boost::binomial_distribution<>(param1,param2),this->generator,minValue,maxValue));
          }
          throw InvalidArgumentException("invalid parameter for distribution");
	}
        throw InvalidArgumentException("missing parameter for distribution");
      } else if (distr.compare("cauchy")==0){
	if (hasParameter1 && hasParameter2){
          std::istringstream stm;
          double param1;
          double param2;
          stm.str(NStr(parameter1->getNodeValue()));
          stm >> param1;
          std::istringstream stm2;
          stm2.str(NStr(parameter2->getNodeValue()));
          stm2 >> param2;
         
          return boost::shared_ptr<TimingModifier>(new CauchyTimingModifier(boost::cauchy_distribution<>(param1,param2),this->generator,minValue,maxValue));
	}
        throw InvalidArgumentException("missing parameter for distribution");
      } else if (distr.compare("exponential")==0){
	if (hasParameter1){
          std::istringstream stm;
          double param1;
          stm.str(NStr(parameter1->getNodeValue()));
          stm >> param1;
         
          if (param1>0){
            return boost::shared_ptr<TimingModifier>(new ExponentialTimingModifier(boost::exponential_distribution<>(param1),this->generator,minValue,maxValue));
          }
          throw InvalidArgumentException("invalid parameter for distribution");
	}
        throw InvalidArgumentException("missing parameter for distribution");
      } else if (distr.compare("gamma")==0){
	if (hasParameter1 && hasParameter2){
          std::istringstream stm;
          double param1;
          stm.str(NStr(parameter1->getNodeValue()));
          stm >> param1;
         
          if (param1>0){
            return boost::shared_ptr<TimingModifier>(new GammaTimingModifier(boost::gamma_distribution<>(param1),this->generator,minValue,maxValue));
          }
          throw InvalidArgumentException("invalid parameter for distribution");
	}
        throw InvalidArgumentException("missing parameter for distribution");
      } else if (distr.compare("geometric")==0){
	if (hasParameter1){
          std::istringstream stm;
          double param1;
          stm.str(NStr(parameter1->getNodeValue()));
          stm >> param1;
         
          if (param1>0 && param1<1){
            return boost::shared_ptr<TimingModifier>(new GeometricTimingModifier(boost::geometric_distribution<>(param1),this->generator,minValue,maxValue));
          }
          throw InvalidArgumentException("invalid parameter for distribution");
	}
        throw InvalidArgumentException("missing parameter for distribution");
      } else if (distr.compare("lognormal")==0){
	if (hasParameter1 && hasParameter2){
          double param1;
          double param2;
          std::istringstream stm;
          stm.str(NStr(parameter1->getNodeValue()));
          stm >> param1;
          std::istringstream stm2;
          stm2.str(NStr(parameter2->getNodeValue()));
          stm2 >> param2;
         
          if (param1>0){
            return boost::shared_ptr<TimingModifier>(new LognormalTimingModifier(boost::lognormal_distribution<>(param1,param2),this->generator,minValue,maxValue));
          }
          throw InvalidArgumentException("invalid parameter for distribution");
	}
        throw InvalidArgumentException("missing parameter for distribution");
      } else if (distr.compare("normal")==0){
	if (hasParameter1 && hasParameter2){
          double param1;
          double param2;
          std::istringstream stm;
          stm.str(NStr(parameter1->getNodeValue()));
          stm >> param1;
          std::istringstream stm2;
          stm2.str(NStr(parameter2->getNodeValue()));
          stm2 >> param2;
         
          if (param2>=0){
            return boost::shared_ptr<TimingModifier>(new NormalTimingModifier(boost::normal_distribution<>(param1,param2),this->generator,minValue,maxValue));
          }
          throw InvalidArgumentException("invalid parameter for distribution");
	}
        throw InvalidArgumentException("missing parameter for distribution");
      } else if (distr.compare("poisson")==0){
	if (hasParameter1){
          std::istringstream stm;
          double param1;
          stm.str(NStr(parameter1->getNodeValue()));
          stm >> param1;
         
          if (param1>0){
            return boost::shared_ptr<TimingModifier>(new PoissonTimingModifier(boost::poisson_distribution<>(param1),this->generator,minValue,maxValue));
          }
          throw InvalidArgumentException("invalid parameter for distribution");
	}
        throw InvalidArgumentException("missing parameter for distribution");
      } else if (distr.compare("triangle")==0){
	if (hasParameter1 && hasParameter2 && hasParameter3){
          double param1;
          double param2;
          double param3;
          std::istringstream stm;
          stm.str(NStr(parameter1->getNodeValue()));
          stm >> param1;
          std::istringstream stm2;
          stm2.str(NStr(parameter2->getNodeValue()));
          stm2 >> param2;
          std::istringstream stm3;
          stm3.str(NStr(parameter3->getNodeValue()));
          stm3 >> param3;
         
          if (param1<=param2 && param2<=param3){
            return boost::shared_ptr<TimingModifier>(new TriangleTimingModifier(boost::triangle_distribution<>(param1,param2,param3),this->generator,minValue,maxValue));
          }
          throw InvalidArgumentException("invalid parameter for distribution");
	}
        throw InvalidArgumentException("missing parameter for distribution");
      } else if (distr.compare("uniformReal")==0){
	if (hasParameter1 && hasParameter2){
          double param1;
          double param2;
          std::istringstream stm;
          stm.str(NStr(parameter1->getNodeValue()));
          stm >> param1;
          std::istringstream stm2;
          stm2.str(NStr(parameter2->getNodeValue()));
          stm2 >> param2;
          std::cout << param1 << "," << param2 << std::endl;
         
          if (param1<param2){
            return boost::shared_ptr<TimingModifier>(new UniformRealTimingModifier(boost::uniform_real<>(param1,param2),this->generator,minValue,maxValue));
          }
          throw InvalidArgumentException("invalid parameter for distribution");
	}
        throw InvalidArgumentException("missing parameter for distribution");
      }
      throw InvalidArgumentException("unknown distribution");
  }
//grocki: end

}// namespace SystemC_VPC
