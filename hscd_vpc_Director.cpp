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

  const char* Director::IN_ID106005 = "0";
  const char* Director::C_IN_RF_ID106026 = "1";
  const char* Director::C_IN_BM_ID106017 = "2";
  const char* Director::C_IN_DIFF_ID106018 = "3";
  const char* Director::RF_ID106010 = "4";
  const char* Director::C_RF_BM_ID106025 = "5";
  const char* Director::BM_ID106001 = "6";
  const char* Director::C_BM_LF_ID106013 = "7";
  const char* Director::LF_ID106007 = "8";
  const char* Director::C_LF_DIFF_ID106020 = "9";
  const char* Director::C_LF_REC_ID106021 = "10";
  const char* Director::DIFF_ID106003 = "11";
  const char* Director::C_DIFF_DCT_ID106015 = "12";
  const char* Director::DCT_ID106002 = "13";
  const char* Director::C_DCT_Q_ID106014 = "14";
  const char* Director::Q_ID106008 = "15";
  const char* Director::C_Q_RLC_ID106023 = "16";
  const char* Director::C_Q_IQ_ID106022 = "17";
  const char* Director::IQ_ID106006 = "18";
  const char* Director::C_IQ_IDCT_ID106019 = "19";
  const char* Director::IDCT_ID106004 = "20";
  const char* Director::C_IDCT_REC_ID106016 = "21";
  const char* Director::REC_ID106009 = "22";
  const char* Director::C_REC_SF_ID106024 = "23";
  const char* Director::SF_ID106012 = "24";
  const char* Director::C_SF_RLC_ID106027 = "25";
  const char* Director::RLC_ID106011 = "26";
 
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
	  p.priority=priority;//p_struct_map_by_name.size();
	  p.deadline=1000;
	  p.period=2800.0;
	  p.pid=p_struct_map_by_name.size();
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
  }
  /*
    p_struct& Director::getProcessControlBlock(int pid){
    p_struct *pcb;
    pcb= new p_struct;
    pcb->pid=pid;
    pcb->period=2800.0;
    return *pcb;
    }
  */
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
