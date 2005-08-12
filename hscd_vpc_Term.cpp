/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_TERM.cpp
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#include "hscd_vpc_Term.h"

namespace SystemC_VPC{
  /**
   *
   */
  char* Constraint::getName(){
    return name;
  }

  /**
   *
   */
  double Constraint::getSatisfiedTime(){
    return activationTime;
  }

  /**
   *
   */
  Constraint::Constraint(const char *sname,  const char *scount,  const char *sdivider){
    assert(sscanf(scount,"%d",&count));
    assert(sscanf(sdivider,"%d",&divider));
    strncpy(name,sname,VPC_MAX_STRING_LENGTH);
    activationTime=-1;
    //cerr << GREEN("created Constraint: name="<<name<<" count="<<count<<" divider="<<divider)<<NENDL;
  }

  /**
   *
   */
  void Constraint::addExclude(const char *sname){
    char excludeTask[VPC_MAX_STRING_LENGTH];
    strncpy(excludeTask,sname,VPC_MAX_STRING_LENGTH);
    excludes.insert(excludeTask);
  }

  /**
   *
   */
  void Constraint::addAnyTerm(const char *state){
    term = new AnyTerm(state);
  }
  
  /**
   *
   */
  bool Constraint::isSatisfied(){
    // cerr << YELLOW("Constraint: "<<name<<" isSatisfied()?  ");
    if(term->isSatisfied(excludes)){
      activationTime=sc_simulation_time();
      //cerr << GREEN("YES "<< sc_simulation_time())<<NENDL;
      return true;
    }else{
      //cerr << RED("NO "<< sc_simulation_time())<<NENDL;
    }
    return false;
  }

  /**
   *
   */
  void Constraint::getReport(){
    cerr << YELLOW("Constraint: "<<name<<" valid activation: " <<activationTime)<<NENDL;
  }
  
  /**
   *
   */
  AnyTerm::AnyTerm(const char* srule){
    satisfiableCounter=1;
    if(0==strncmp(srule,START_STR,strlen(srule))){
      rule=start;
    } else{
      rule=end;
    }
  }

  /**
   *
   */
  bool AnyTerm::isSatisfied(set<string> excludes){
    if(satisfiableCounter!=0){
      map<string,p_struct*> pcb_map =  Director::getInstance().getPcbMap();
      map<string,p_struct*>::const_iterator iter;
      for(iter=pcb_map.begin();iter!=pcb_map.end(); iter++){
	p_struct *pcb=(iter->second);
	if(0==excludes.count(pcb->name)){
	  if((rule==start && pcb->state==starting)){
	    satisfiableCounter=0;
	    return true;
	  }else if((rule==end && pcb->state==ending)){
	    return true;
	  }
	}
      }
    }
    return false;
  }
}
