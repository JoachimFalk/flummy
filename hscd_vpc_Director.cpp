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
#include <iostream>
#include <float.h> 

#include <hscd_vpc_Director.h>
#include <hscd_vpc_AbstractComponent.h>
#include <hscd_vpc_Term.h>
#include <hscd_vpc_VPCBuilder.h>

#include <systemc.h>
#include <map>

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
    
    VPCBuilder* builder = new VPCBuilder((Director*)this);
   
    builder->buildVPC();
    
    delete builder;

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
    //#  ifdef VPC_DEBUG
    cerr << "start: " << start << " end: " << end << endl;
    //# endif //VPC_DEBUG
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
    cerr << "~Director()" <<endl;
    
    getReport();
    

    map<string,AbstractComponent*>::iterator it = component_map_by_name.begin();
    
    while(it != component_map_by_name.end()){
		delete it->second;
		it++;
    }
    
    component_map_by_name.clear();
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

	/************************/
	/*  EXTENSION SECTION   */
	/************************/

	void Director::compute(const char* name, const char* funcname, CoSupport::SystemC::Event* end){
		
#ifdef VPC_DEBUG
		std::cerr << RED("Director> compute(") << WHITE(name) << RED(",") << WHITE(funcname) << RED(") at: ") << sc_simulation_time() << std::endl;
#endif //VPC_DEBUG

		if(!FALLBACKMODE){
			
			if(1!=mapping_map_by_name.count(name))
				cerr << "Unknown mapping <"<<name<<"> to ??"<<endl; 
			
			assert(1==mapping_map_by_name.count(name));
			// replace upper code with this when enablung multple dynamical binding
			// assert(0 < this->mapping_map_by_name.count(name));
			// and dont forget to add apropiate code fragment after this
			
			// get Component
			AbstractComponent* comp = mapping_map_by_name.find(name)->second;

#ifdef VPC_DEBUG
			std::cerr << RED("Director> delegating to ") << WHITE(comp->getName()) << std::endl;
#endif //VPC_DEBUG			
			
			// compute task on found component
			comp->compute(name, funcname, end);
		
		}else{
			AbstractComponent* comp = (component_map_by_name["Fallback-Component"]);
			if(comp == NULL){
				std::cerr << "no Fallback defined!" << std::endl;
			}
			
			comp->compute(name, funcname, end);
			
		}
	
	}

	void Director::compute(const char* name, CoSupport::SystemC::Event* end){
	
		this->compute(name, "", end);
	
	}
	  
	/**
	 * \brief Implementation of Director::addConstraint
	 */
	void Director::addConstraint(Constraint* cons){
		
		this->constraints.push_back(cons);
		
	}

	/**
	 * \brief Implementation of Director::registerComponent
	 */
	void Director::registerComponent(AbstractComponent* comp){
		
		this->component_map_by_name.insert(std::pair<std::string, AbstractComponent* >(comp->getName(), comp));
		
	}
    
    /**
     * \brief Implementation of Director::registerMapping
     */
    void Director::registerMapping(const char* taskName, const char* compName){
    	
    	std::map<std::string,AbstractComponent*>::iterator iter;
    	iter = this->component_map_by_name.find(compName);
		//check if component is known
    	if(iter != this->component_map_by_name.end()){

#ifdef VPC_DEBUG
			std::cout << "Director: registering mapping: "<< taskName << " <-> " << compName << endl;
#endif //VPC_DEBUG

			this->mapping_map_by_name.insert(std::pair<std::string, AbstractComponent* >(taskName,iter->second));
    	}
    	
    }
    
    /**
     * \brief Implementation of  Director::generatePCB
     */
    p_struct* Director::generatePCB(const char* name){
    	
    	std::map<std::string, p_struct* >::iterator iter;
    	iter = this->p_struct_map_by_name.find(name);
    	if(iter != this->p_struct_map_by_name.end()){
    		return iter->second;	
    	}
    	
    	p_struct* newPCB = new p_struct();
    	newPCB->name = name;
		newPCB->pid = this->p_struct_map_by_name.size(); // just enumerate to get unique pid
		newPCB->activation_count = 0; //default
		newPCB->state = inaktiv;      //default
		newPCB->priority = 0;         //default
		newPCB->deadline = DBL_MAX; //default
		newPCB->period = DBL_MAX;   //default
    
    	this->p_struct_map_by_name.insert(std::pair<std::string, p_struct* >(name, newPCB));
    	
    	return newPCB;
    	
    }
    
	/**************************/
	/*  END OF EXTENSION      */
	/**************************/

}

