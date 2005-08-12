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
 
#include "hscd_vpc_Director.h"
#include "hscd_vpc_Component.h"
#include "hscd_vpc_Term.h"
#include "hscd_vpc_XmlHelper.h"
#include "hscd_vpc_VpcDomErrorHandler.h"

#include "systemc.h"
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
    XMLCh *excludeStr     = XMLString::transcode("exclude");
    XMLCh *anytermStr     = XMLString::transcode("anyterm");
    //XMLCh *Str = XMLString::transcode("");
    
    XMLCh *nameAttrStr    = XMLString::transcode("name");
    XMLCh *countAttrStr   = XMLString::transcode("count");
    XMLCh *processAttrStr = XMLString::transcode("process");
    XMLCh *dividerAttrStr = XMLString::transcode("divider");
    XMLCh *stateAttrStr   = XMLString::transcode("state");
    

    char *vpc_evaluator_prefix = getenv("VPC_EVALUATOR");
    FILE *fconffile;

    FALLBACKMODE=false;
    if(vpc_evaluator_prefix){
      char vpc_conf_file[VPC_MAX_STRING_LENGTH];
      sprintf(vpc_conf_file,"%s%s",vpc_evaluator_prefix,STR_VPC_CONF_FILE);
      cerr <<"config found"<<vpc_conf_file << endl;
      fconffile=fopen(vpc_conf_file,"r");
    }else{
      cerr << "-"<< getenv("VPCCONFIGURATION")<< "-"<< endl;
      char *cfile= getenv("VPCCONFIGURATION");
      if(!cfile)FALLBACKMODE=true;
      fconffile=fopen(cfile,"r");
    }
    if(!fconffile)FALLBACKMODE=true;
    char module[VPC_MAX_STRING_LENGTH],component[VPC_MAX_STRING_LENGTH],scheduler[VPC_MAX_STRING_LENGTH];
    double delay;
    int priority;
    if(!FALLBACKMODE){
      while(!feof(fconffile)){
	fscanf(fconffile,"%s",module);
	if(0==strcmp(module,"component:")){
	  //eine Komponente
	  fscanf(fconffile,"%s %s",component,scheduler);
	  component_map_by_name.insert(pair<string,AbstractComponent*>(component,new Component(component,scheduler)));
	  //cerr << "comp " << module << component << scheduler<<endl;
	}else{
	  //eine Abbildung: process -> Komponente
	  fscanf(fconffile,"%s %lf %i",component,&delay,&priority);
	  assert(component_map_by_name.count(component)==1);//Component not in conf-file!
	  map<string,AbstractComponent*>::iterator iter;
	  iter=component_map_by_name.find(component);
	  mapping_map_by_name.insert(pair<string,AbstractComponent*>(module,iter->second));
	  //cerr << "mapping " << module << component << delay<<endl;
	  ((Component*)iter->second)->informAboutMapping(module);
	  p_struct *p= new p_struct;
	  p->name=module;
	  p->priority=priority;
	  p->deadline=1000;//FIXME
	  p->period=2800.0;//FIXME
	  p->pid=p_struct_map_by_name.size();//HACK
	  p->delay=delay;
	  p->activation_count=0;
	  p->state=inaktiv;
	  p_struct_map_by_name.insert(pair<string,p_struct*>(module,p));
	}
      }
    }else{
      FallbackComponent *fall=new FallbackComponent("Fallback-Component","FCFS");
      component_map_by_name.insert(pair<string,AbstractComponent*>("Fallback-Component",fall));
      mapping_map_by_name.insert(pair<string,AbstractComponent*>("Fallback-Component",fall));
    }
    
    if(!vpc_evaluator_prefix){
      cerr << VPC_ERROR << "No VPC_EVALUATOR Environment\n"
	   << "Hint: try to export/setenv VPC_EVALUATOR"<< NENDL; //<< endl;
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
	  char *scount,*sdivider,*sname;
	  sname=XMLString::transcode(atts->getNamedItem(nameAttrStr)->getNodeValue());
	  scount=XMLString::transcode(atts->getNamedItem(countAttrStr)->getNodeValue());
	  sdivider=XMLString::transcode(atts->getNamedItem(dividerAttrStr)->getNodeValue());
	  
	  Constraint *cons=new Constraint(sname,scount,sdivider);
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
    cerr << "start: " << start << " end: " << end << endl;
    if (start!=-1 && end!=-1){
      cout << "latency: " << end - start << endl;
      char* vpc_evaluator_prefix = getenv("VPC_EVALUATOR");
      if(vpc_evaluator_prefix){
	char vpc_result_file[VPC_MAX_STRING_LENGTH];
	sprintf(vpc_result_file,"%s%s",vpc_evaluator_prefix,STR_VPC_RESULT_FILE);
	cerr << "result_file: "<< vpc_result_file << endl;
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
      assert(1==p_struct_map_by_name.count(name));
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
