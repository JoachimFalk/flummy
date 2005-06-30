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

#include "hscd_vpc_Director.h"
#include "hscd_vpc_Component.h"
#include "systemc.h"
#include <map>
namespace SystemC_VPC{
  std::auto_ptr<Director> Director::singleton(new Director());
 
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
  Director::Director(){

    srand(time(NULL));

    FALLBACKMODE=false;
    //  cerr << "-"<< getenv("VPCCONFIG")<< "-"<< endl;
    char* cfile= getenv("VPCCONFIGURATION");
    if(!cfile)FALLBACKMODE=true;
    FILE* fconffile=fopen(cfile,"r");
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
	  p_struct p;
	  p.name=module;
	  p.priority=priority;
	  p.deadline=1000;//FIXME
	  p.period=2800.0;//FIXME
	  p.pid=p_struct_map_by_name.size();//HACK
	  p.delay=delay;
	  p_struct_map_by_name.insert(pair<string,p_struct>(module,p));
	}
      }
    }else{
      FallbackComponent *fall=new FallbackComponent("Fallback-Component","FCFS");
      component_map_by_name.insert(pair<string,AbstractComponent*>("Fallback-Component",fall));
      mapping_map_by_name.insert(pair<string,AbstractComponent*>("Fallback-Component",fall));
    }
  }
  Director::~Director(){
    //    cerr << "~Director()"<<endl;
    
    map<string,AbstractComponent*>::iterator it = component_map_by_name.begin();
    while(it != component_map_by_name.end()){
      delete it->second;
      it++;
    }
    
  }

  p_struct& Director::getProcessControlBlock( const char *name ){
    if(!FALLBACKMODE){
      assert(1==p_struct_map_by_name.count(name));
      return p_struct_map_by_name[name];
    }else{
      p_struct p;
      p.name=name;
      p.priority=p_struct_map_by_name.size(); //hack to create pid!
      p.deadline=1000;
      p.period=2800.0;
      p.pid=p_struct_map_by_name.size();
      p.delay=10;
      p_struct_map_by_name.insert(pair<string,p_struct>(name,p));//hack to create pid!
      assert(1==p_struct_map_by_name.count(name));
      return p_struct_map_by_name[name]; 
    }
  }
}
