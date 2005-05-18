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
    if(1!=mapping_map_by_name.count(name))
	cerr << "Unknown mapping <"<<name<<"> to ??"<<endl; 
    assert(1==mapping_map_by_name.count(name));
    return *(mapping_map_by_name[name]);
}
AbstractComponent& Director::getResource(int process){

}
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
    if(!FALLBACKMODE){
	while(!feof(fconffile)){
	    fscanf(fconffile,"%s",module);
	    if(0==strcmp(module,"component:")){
		//eine Komponente
		fscanf(fconffile,"%s %s",component,scheduler);
		component_map_by_name.insert(pair<string,Component*>(component,new Component(component,scheduler)));
		//cerr << "comp " << module << component << scheduler<<endl;
	    }else{
//eine Abbildung: process -> Komponente
		fscanf(fconffile,"%s %lf",component,&delay);
		assert(component_map_by_name.count(component)==1);//Component not in conf-file!
		map<string,Component*>::iterator iter;
		iter=component_map_by_name.find(component);
		mapping_map_by_name.insert(pair<string,Component*>(module,iter->second));
		//cerr << "mapping " << module << component << delay<<endl;
		iter->second->informAboutMapping(module);
		p_struct p;
		p.name=module;
		p.priority=p_struct_map_by_name.size();
		p.deadline=1000;
		p.period=2800.0;
		p.pid=p_struct_map_by_name.size();
		p.delay=delay;
		p_struct_map_by_name.insert(pair<string,p_struct>(module,p));
	    }
	  
	  
	}
	string test=module;
    }else{
	component_map_by_name.insert(pair<string,Component*>("Comp1",new Component("Comp1","RM")));//["Comp1"]=new Component("Comp1",1);
	component_map_by_name.insert(pair<string,Component*>("Comp2",new Component("Comp2","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp3",new Component("Comp3","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp4",new Component("Comp4","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp5",new Component("Comp5","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp6",new Component("Comp6","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp7",new Component("Comp7","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp8",new Component("Comp8","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp9",new Component("Comp9","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp10",new Component("Comp10","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp11",new Component("Comp11","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp12",new Component("Comp12","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp13",new Component("Comp13","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp14",new Component("Comp14","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp15",new Component("Comp15","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp16",new Component("Comp16","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp17",new Component("Comp17","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp18",new Component("Comp18","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp19",new Component("Comp19","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp20",new Component("Comp20","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp21",new Component("Comp21","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp22",new Component("Comp22","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp23",new Component("Comp23","RM")));
	component_map_by_name.insert(pair<string,Component*>("Comp24",new Component("Comp24","RM")));
      
      
	map<string,Component*>::iterator iter;
	p_struct pcb;
	pcb.priority=3;
	pcb.deadline=1000;
	pcb.period=2800.0;

	pcb.name="sim_mod.h_hca.h_recv_port";
	pcb.pid=0;
	pcb.delay=20;
	iter=component_map_by_name.find("Comp1");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_recv_port",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_recv_port",pcb));
	pcb.name="sim_mod.h_hca.h_mfetch";
	pcb.pid=1;
	pcb.delay=30;
	iter=component_map_by_name.find("Comp2");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_mfetch",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_mfetch",pcb));
	pcb.name="sim_mod.h_hca.h_sched";
	pcb.pid=2;
	pcb.delay=40;
	iter=component_map_by_name.find("Comp3");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_sched",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_sched",pcb));
	pcb.name="sim_mod.h_hca2fifo";
	pcb.pid=3;
	pcb.delay=20;
	iter=component_map_by_name.find("Comp4");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca2fifo",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca2fifo",pcb));
	pcb.name="sim_mod.h_libhca";
	pcb.delay=12;
	pcb.pid=4;
	iter=component_map_by_name.find("Comp5");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_libhca",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_libhca",pcb));
	pcb.name="sim_mod.h_hca.h_wqm";
	pcb.pid=5;
	pcb.delay=15;
	iter=component_map_by_name.find("Comp6");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_wqm",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_wqm",pcb));
	pcb.name="sim_mod.h_hca.h_transmit_queue";
	pcb.pid=6;
	pcb.delay=19;
	iter=component_map_by_name.find("Comp7");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_transmit_queue",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_transmit_queue",pcb));
	pcb.name="sim_mod.h_fifo2hca";
	pcb.pid=7;
	pcb.delay=22;
	iter=component_map_by_name.find("Comp8");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_fifo2hca",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_fifo2hca",pcb));
	pcb.name="sim_mod.h_hca.h_cqm";
	pcb.pid=8;
	pcb.delay=100;
	iter=component_map_by_name.find("Comp9");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_cqm",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_cqm",pcb));
	pcb.name="sim_mod.h_libhca_writer";
	pcb.pid=9;
	pcb.delay=120;
	iter=component_map_by_name.find("Comp10");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_libhca_writer",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_libhca_writer",pcb));
	pcb.name="sim_mod.h_hca.h_receive_queue";
	pcb.pid=10;
	pcb.delay=104;
	iter=component_map_by_name.find("Comp11");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_receive_queue",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_receive_queue",pcb));
	pcb.name="sim_mod.h_hca.qpc_manager";
	pcb.pid=11;
	pcb.delay=170;
	iter=component_map_by_name.find("Comp12");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.qpc_manager",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.qpc_manager",pcb));
	pcb.name="sim_mod.h_hca.pi_context";
	pcb.pid=12;
	pcb.delay=5;
	iter=component_map_by_name.find("Comp13");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.pi_context",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.pi_context",pcb));
	pcb.name="sim_mod.h_hca.vl_manager";
	pcb.pid=13;
	pcb.delay=19;
	iter=component_map_by_name.find("Comp14");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.vl_manager",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.vl_manager",pcb));
	pcb.name="sim_mod.h_hca.h_ack_queue";
	pcb.pid=14;
	pcb.delay=10;
	iter=component_map_by_name.find("Comp15");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_ack_queue",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_ack_queue",pcb));
	pcb.name="sim_mod.h_hca.h_rethgen";
	pcb.pid=15;
	pcb.delay=30;
	iter=component_map_by_name.find("Comp16");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_rethgen",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_rethgen",pcb));
	pcb.name="sim_mod.h_hca.h_rethchk";
	pcb.pid=16;
	pcb.delay=80;
	iter=component_map_by_name.find("Comp17");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_rethchk",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_rethchk",pcb));
	pcb.name="sim_mod.h_hca.h_aethgen";
	pcb.pid=17;
	pcb.delay=50;
	iter=component_map_by_name.find("Comp18");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_aethgen",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_aethgen",pcb));
	pcb.name="sim_mod.h_hca.h_aethchk";
	pcb.pid=18;
	pcb.delay=40;
	iter=component_map_by_name.find("Comp19");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_aethchk",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_aethchk",pcb));
	pcb.name="sim_mod.h_hca.h_bth_grh_gen";
	pcb.pid=19;
	pcb.delay=30;
	iter=component_map_by_name.find("Comp20");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_bth_grh_gen",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_bth_grh_gen",pcb));
	pcb.name="sim_mod.h_hca.h_bth_grh_chk";
	pcb.pid=20;
	pcb.delay=50;
	iter=component_map_by_name.find("Comp21");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_bth_grh_chk",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_bth_grh_chk",pcb));
	pcb.name="sim_mod.h_hca.h_atu";
	pcb.pid=21;
	pcb.delay=20;
	iter=component_map_by_name.find("Comp22");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_atu",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_atu",pcb));
	pcb.name="sim_mod.h_hca.h_send_port";
	pcb.pid=22;
	pcb.delay=40;
	iter=component_map_by_name.find("Comp23");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_send_port",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_send_port",pcb));
	pcb.name="sim_mod.h_hca.h_mstore";
	pcb.pid=23;
	pcb.delay=30;
	iter=component_map_by_name.find("Comp24");
	component_map_by_name.insert(pair<string,Component*>("sim_mod.h_hca.h_mstore",iter->second));
	p_struct_map_by_name.insert(pair<string,p_struct>("sim_mod.h_hca.h_mstore",pcb));
	
      
    }
}
Director::~Director(){
}
p_struct& Director::getProcessControlBlock(int pid){
    p_struct *pcb;
    pcb= new p_struct;
  
//  pcb->name=PROCESS[pid];
    // pcb->delay=DELAY[pid];
    pcb->pid=pid;
    // pcb->priority=SUCCESSOR[pid];
    //  pcb->deadline=DEADLINE[pid];
    pcb->period=2800.0;
    return *pcb;
}
p_struct& Director::getProcessControlBlock( const char *name ){
    assert(1==p_struct_map_by_name.count(name));
    return p_struct_map_by_name[name];
}
