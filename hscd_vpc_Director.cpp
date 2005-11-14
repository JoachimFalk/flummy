/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Director.cpp
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
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

#include <iostream>
#include <float.h> 
#include <hscd_vpc_Director.h>
#include <hscd_vpc_Component.h>
#include <hscd_vpc_Term.h>
#include <hscd_vpc_XmlHelper.h>
#include <hscd_vpc_VpcDomErrorHandler.h>

#include <systemc.h>
#include <map>

XERCES_CPP_NAMESPACE_USE 
namespace SystemC_VPC{
  std::auto_ptr<Director> Director::singleton(new Director());
 

  /**
   *
   */
  AbstractComponent& Director::getResource( const char *name ){
    if(!FALLBACKMODE){
      if(1!=mapping_map_by_name.count(name))
	cerr << "Unknown mapping <"<<name<<"> to ??"<<endl; 
      assert(1==mapping_map_by_name.count(name));
      return *(mapping_map_by_name[name]);
    }else{
      return *(mapping_map_by_name["Fallback-Component"]);
    }
  }
  //AbstractComponent& Director::getResource(int process){}

  /**
   *
   */
  Director::Director(){
    
    //init xml
    try {
      XMLPlatformUtils::Initialize();
    }
    catch(const XMLException& e){
      cerr << VPC_ERROR<<"Director> Error initializing Xerces:\n"<<XMLString::transcode(e.getMessage())<<NENDL; // << endl;
    }
    XMLCh *constraintStr  = XMLString::transcode("constraint");
    //    XMLCh *excludeStr     = XMLString::transcode("exclude");
    //    XMLCh *anytermStr     = XMLString::transcode("anyterm");
    XMLCh *measurefileStr = XMLString::transcode("measurefile");
    XMLCh *resultfileStr = XMLString::transcode("resultfile");
    XMLCh *resourcesStr = XMLString::transcode("resources");
    XMLCh *mappingsStr = XMLString::transcode("mappings");
    XMLCh *componentStr = XMLString::transcode("component");
    XMLCh *mappingStr = XMLString::transcode("mapping");
    XMLCh *attributeStr = XMLString::transcode("attribute");
    //XMLCh *Str = XMLString::transcode("");
    
    XMLCh *nameAttrStr    = XMLString::transcode("name");
    XMLCh *countAttrStr   = XMLString::transcode("count");
    XMLCh *typeAttrStr = XMLString::transcode("type");
    XMLCh *dividerAttrStr = XMLString::transcode("divider");
    XMLCh *schedulerAttrStr   = XMLString::transcode("scheduler");
    XMLCh *valueAttrStr   = XMLString::transcode("value");
    XMLCh *targetAttrStr   = XMLString::transcode("target");
    XMLCh *sourceAttrStr   = XMLString::transcode("source");
    //    XMLCh *AttrStr   = XMLString::transcode("");

    FILE *fconffile;
    char *cfile;
    char *vpc_evaluator_prefix = getenv("VPC_EVALUATOR");
    char vpc_conf_file[VPC_MAX_STRING_LENGTH];

    FALLBACKMODE=false;
    if(vpc_evaluator_prefix){
      sprintf(vpc_conf_file,"%s%s",vpc_evaluator_prefix,STR_VPC_CONF_FILE);
#ifdef VPC_DEBUG
      cerr <<"config found"<<vpc_conf_file << endl;
#endif //VPC_DEBUG
      cfile = vpc_conf_file;
      //      fconffile=fopen(vpc_conf_file,"r");
    }else{
      cfile= getenv("VPCCONFIGURATION");
    }
    if(cfile){
      fconffile=fopen(cfile,"r");
      if( NULL == fconffile ){       // test if file exists
	assert( 0 == strncmp(cfile, "", sizeof("")) && "VPCCONFIGURATION is set, but points to nowhere" != NULL );  // for joachim
	FALLBACKMODE=true;
      }else{
	fclose(fconffile);
#ifdef VPC_DEBUG
	cerr << "configuration: "<<cfile << endl;
#endif //VPC_DEBUG
      }
    }else{
	FALLBACKMODE=true;
    }
    

    if(FALLBACKMODE){
      FallbackComponent *fall=new FallbackComponent("Fallback-Component","FCFS");
      component_map_by_name.insert(pair<string,AbstractComponent*>("Fallback-Component",fall));
      mapping_map_by_name.insert(pair<string,AbstractComponent*>("Fallback-Component",fall));
    }else{
      // process xml
      DOMTreeWalker *vpcConfigTreeWalker;
      DOMDocument *vpcConfigDoc;
      DOMBuilder *configParser;
      static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
      DOMImplementation *configImpl = DOMImplementationRegistry::getDOMImplementation(gLS);
      // create an error handler and install it
      VpcDomErrorHandler *configErrorh=new VpcDomErrorHandler();
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
	cerr << "\nError while parsing xml file: '" << cfile << "'\n"
	     << "Exception message is:  \n"
	     << XMLString::transcode( toCatch.getMessage()) << "\n" << endl;
      }
      catch (const DOMException& toCatch) {
	const unsigned int maxChars = 2047;
	XMLCh errText[maxChars + 1];
      
	cerr << "\nDOM Error while parsing xml file: '" << cfile << "'\n"
	     << "DOMException code is:  " << XMLString::transcode( toCatch.msg) << endl;
      
	if (DOMImplementation::loadDOMExceptionMsg(toCatch.code, errText, maxChars))
	  cerr << "Message is: " << XMLString::transcode( errText) << endl;

      }
      catch (...) {
	cerr << "\nUnexpected exception while parsing xml file: '" << cfile << "'\n";
      
      }

      // set treewalker to documentroot
      vpcConfigTreeWalker = vpcConfigDoc->createTreeWalker( (DOMNode*)vpcConfigDoc->getDocumentElement(), DOMNodeFilter::SHOW_ELEMENT, 0, true);
      vpcConfigTreeWalker->setCurrentNode( (DOMNode*)vpcConfigDoc->getDocumentElement());
    
      // moves the Treewalker to the first Child 
      DOMNode *node = vpcConfigTreeWalker->firstChild(); 
      const XMLCh *xmlName;
      while(node!=0){
	xmlName=node->getNodeName();

	// find resources tag
	if( 0==XMLString::compareNString( xmlName, resourcesStr, sizeof(resourcesStr) ) ){

	  // walk down hierachy to components
	  node = vpcConfigTreeWalker->firstChild();

	  while(node!=0){
	    xmlName=node->getNodeName();
	    //find all component tags
	    if( 0==XMLString::compareNString( xmlName, componentStr, sizeof(componentStr))){
	      DOMNamedNodeMap * atts=node->getAttributes();
	      char *sName, *sType, *sScheduler;
	      AbstractComponent *comp=NULL;

	      sName=XMLString::transcode(atts->getNamedItem(nameAttrStr)->getNodeValue());
	      sType=XMLString::transcode(atts->getNamedItem(typeAttrStr)->getNodeValue());
	      sScheduler=XMLString::transcode(atts->getNamedItem(schedulerAttrStr)->getNodeValue());
	      if(0==strncmp(sType, STR_VPC_COMPONENTSTRING, sizeof(STR_VPC_COMPONENTSTRING))){
		comp=new Component(sName, sScheduler);
		component_map_by_name.insert(pair<string, AbstractComponent*>(sName, comp));
	      }else if(0==strncmp(sType, STR_VPC_THREADEDCOMPONENTSTRING, sizeof(STR_VPC_THREADEDCOMPONENTSTRING))){
		comp=new ThreadedComponent(sName,sScheduler);
		component_map_by_name.insert(pair<string, AbstractComponent*>(sName, comp));
		cerr << "VPC> Found Component name=" << sName << "type=" << sType << endl;
	      }else {
		cerr << "VPC> Found unknown Component type: name=" << sName << "type=" << sType << endl;
	      }

	      if(comp!=NULL){
		node = vpcConfigTreeWalker->firstChild();
		if( node == NULL ){
		  node=vpcConfigTreeWalker->getCurrentNode();
		}else{
		  // find all attributes
		  while( node != NULL ){
		    xmlName=node->getNodeName();
		    if( 0==XMLString::compareNString( xmlName, attributeStr, sizeof(attributeStr))){
		      DOMNamedNodeMap * atts=node->getAttributes();
		      char *sType, *sValue;
		      sType=XMLString::transcode(atts->getNamedItem(typeAttrStr)->getNodeValue());
		      sValue=XMLString::transcode(atts->getNamedItem(valueAttrStr)->getNodeValue());
		      comp->processAndForwardParameter(sType,sValue);
		    }
		    node = vpcConfigTreeWalker->nextSibling();
		  }
		  node = vpcConfigTreeWalker->parentNode();
		}
	      }
	    }
	    node = vpcConfigTreeWalker->nextSibling();
	  }
	  node = vpcConfigTreeWalker->parentNode();
	  // find mappings tag (not mapping)
	}else if( 0==XMLString::compareNString( xmlName, mappingsStr, sizeof(mappingsStr) ) ){
	  node = vpcConfigTreeWalker->firstChild();
	  while(node!=0){
	    xmlName=node->getNodeName();

	    // find mapping tag (not mappings)
	    if( 0==XMLString::compareNString( xmlName, mappingStr, sizeof(mappingStr))){
	      DOMNamedNodeMap * atts=node->getAttributes();
	      char *sTarget, *sSource;
	      sTarget=XMLString::transcode(atts->getNamedItem(targetAttrStr)->getNodeValue());
	      sSource=XMLString::transcode(atts->getNamedItem(sourceAttrStr)->getNodeValue());

	      if(component_map_by_name.count(sTarget)==1){
#ifdef VPC_DEBUG
		cerr << "VPC> Found mapping attribute: source=" << sSource << " target=" << sTarget << endl; 
#endif //VPC_DEBUG
		map<string,AbstractComponent*>::iterator iter=component_map_by_name.find(sTarget);
		mapping_map_by_name.insert(pair<string,AbstractComponent*>(sSource,iter->second));
		(iter->second)->informAboutMapping(sSource);
	      
		p_struct *p= new p_struct;
		p->name=sSource;
		p->pid=p_struct_map_by_name.size(); // just enumerate to get unique pid
		p->activation_count=0; //default
		p->state=inaktiv;      //default
		p->priority=0;         //default
		p->deadline = DBL_MAX; //default
		p->period   = DBL_MAX; //default
 	      
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
		      sscanf(sValue, 
"%d", 
&(p->priority));
		    }else if( 0 == strncmp(sType, STR_VPC_DEADLINE, sizeof(STR_VPC_DEADLINE) )){
		      sscanf(sValue, "%lf", &(p->deadline));
		    }else if( 0 == strncmp(sType, STR_VPC_PERIOD, sizeof(STR_VPC_PERIOD) )){
		      sscanf(sValue, "%lf", &(p->period));
		    }else if( 0 == strncmp(sType, STR_VPC_DELAY, sizeof(STR_VPC_DELAY) )){
		      sscanf(sValue, "%lf", &(p->delay));
		    }else{
#ifdef VPC_DEBUG
		      cerr << "VPC> Unknown mapping attribute: type=" << sType << " value=" << sValue << endl; 
		      cerr << "VPC> Try to interpret as function specific delay!!" << endl;
#endif //VPC_DEBUG
		      double delay;
		      if( 1 == sscanf(sValue, "%lf", &delay) ){
			  p->functionDelays.insert(pair<string,double>(sType,delay));
		      } else {
#ifdef VPC_DEBUG
			  cerr <<  "VPC> Mapping realy unknown!" << endl;
#endif //VPC_DEBUG
		      }
		    }
		  }
		  node = vpcConfigTreeWalker->nextSibling();
		}
		node = vpcConfigTreeWalker->parentNode();

		p_struct_map_by_name.insert(pair<string,p_struct*>(sSource,p));
	      }else{
		cerr << "VPC> No valid component found for mapping: source=" << sSource << " target=" << sTarget<< endl;
	      }
	    }
	    node = vpcConfigTreeWalker->nextSibling();
	  }
	  node = vpcConfigTreeWalker->parentNode();
	}else if( 0==XMLString::compareNString( xmlName, measurefileStr, sizeof(measurefileStr) ) ){

	}else if( 0==XMLString::compareNString( xmlName, resultfileStr, sizeof(resultfileStr) ) ){

	}else{

	}

	node = vpcConfigTreeWalker->nextSibling();
       
      }
    }


    if(!vpc_evaluator_prefix){
      /*
	cerr << VPC_ERROR << "No VPC_EVALUATOR Environment\n"
	<< "Hint: try to export/setenv VPC_EVALUATOR"<< NENDL; //<< endl;
      */
    }else{
      char vpc_result_file[VPC_MAX_STRING_LENGTH];
      sprintf(vpc_result_file,"%s%s",vpc_evaluator_prefix,STR_VPC_RESULT_FILE);
      remove(vpc_result_file);
      char vpc_measure_file[VPC_MAX_STRING_LENGTH];
      sprintf(vpc_measure_file,"%s%s",vpc_evaluator_prefix,STR_VPC_MEASURE_FILE);
      cerr << "measure_file: "<< vpc_measure_file << endl;
      if(!vpc_measure_file){
	cerr << VPC_ERROR << "No vpc_measure_file"<< NENDL; //<< endl;
	exit(0); //FIXME
      }  
      DOMTreeWalker *vpc_measure_TreeWalker;
      DOMDocument *vpc_measure_doc;
      DOMBuilder *parser;
      static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
      DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
      // create an error handler and install it
      VpcDomErrorHandler *errorh=new VpcDomErrorHandler();
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
	cerr << "\nError while parsing xml file: '" << vpc_measure_file << "'\n"
	     << "Exception message is:  \n"
	     << XMLString::transcode( toCatch.getMessage()) << "\n" << endl;

      }
      catch (const DOMException& toCatch) {
	const unsigned int maxChars = 2047;
	XMLCh errText[maxChars + 1];
	
	cerr << "\nDOM Error while parsing xml file: '" << vpc_measure_file << "'\n"
	     << "DOMException code is:  " << XMLString::transcode( toCatch.msg) << endl;

	if (DOMImplementation::loadDOMExceptionMsg(toCatch.code, errText, maxChars))
	  cerr << "Message is: " << XMLString::transcode( errText) << endl;

      }
      catch (...) {
	cerr << "\nUnexpected exception while parsing xml file: '" << vpc_measure_file << "'\n";

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
	  
	  Constraint *cons=new Constraint(sName,sCount,sDivider);
	  XmlHelper::xmlFillConstraint(cons,n->getFirstChild());
	  constraints.push_back(cons);
	}

	//DOMNode *last = n;
	//vpc_measure_TreeWalker->setCurrentNode( last);
	n = vpc_measure_TreeWalker->nextSibling();
      }
      XMLString::release(&name);
     
      
    }
  
  }

  /**
   *
   */
  void Director::checkConstraints() {
    vector<Constraint*>::const_iterator iter=constraints.begin();
    for(;iter!=constraints.end();iter++){
      (*iter)->isSatisfied();
    }
  }

  /**
   *
   */
  void Director::getReport(){
    vector<Constraint*>::const_iterator iter=constraints.begin();
    char *cons_name;
    double start=-1;
    double end=-1;
    for(;iter!=constraints.end();iter++){
      cons_name=(*iter)->getName();
      if(0==strncmp("start",cons_name,5))
	start=(*iter)->getSatisfiedTime();
      else if(0==strncmp("end",cons_name,3))
	end=(*iter)->getSatisfiedTime();
    }
    //#  i fdef VPC_DEBUG
    cerr << "start: " << start << " end: " << end << endl;
    //# e ndif //VPC_DEBUG
    if (start!=-1 && end!=-1){
      cout << "latency: " << end - start << endl;
      char* vpc_evaluator_prefix = getenv("VPC_EVALUATOR");
      if(vpc_evaluator_prefix){
	char vpc_result_file[VPC_MAX_STRING_LENGTH];
	sprintf(vpc_result_file,"%s%s",vpc_evaluator_prefix,STR_VPC_RESULT_FILE);
#ifdef VPC_DEBUG
	cerr << "result_file: "<< vpc_result_file << endl;
#endif //VPC_DEBUG
	FILE *resultFile;
	resultFile=fopen(vpc_result_file,"w");
	if(resultFile){
	  fprintf(resultFile,"%lf",end-start);
	}
	fclose(resultFile);
      }
    }
  }

  /**
   *
   */
  Director::~Director(){
    //    cerr << "~Director()"<<endl;
    getReport();
    XMLPlatformUtils::Terminate();

    map<string,AbstractComponent*>::iterator it = component_map_by_name.begin();
    while(it != component_map_by_name.end()){
      delete it->second;
      it++;
    }
    
  }

  p_struct* Director::getProcessControlBlock( const char *name ){
    if(!FALLBACKMODE){
      if(1!=p_struct_map_by_name.count(name)){
	cerr << "VPC> No p_struct found for task \"" << name << "\"" << endl;
	exit(0);
      }
      return p_struct_map_by_name[name];
    }else{
      if(1==p_struct_map_by_name.count(name)){
	return p_struct_map_by_name[name];
      }else{
	//create fallback pcb
	p_struct *p=new p_struct;
	p->name=name;
	p->priority=p_struct_map_by_name.size(); 
	p->deadline=1000;
	p->period=2800.0;
	p->pid=p_struct_map_by_name.size();//hack to create pid!

	p->delay=0;
	p_struct_map_by_name.insert(pair<string,p_struct*>(name,p));//hack to create pid!
	assert(1==p_struct_map_by_name.count(name));
	return p_struct_map_by_name[name]; 
      }
    }
  }
}
