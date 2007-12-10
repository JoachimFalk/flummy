/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 * hscd_vpc_TERM.cpp
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/
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
  Constraint::Constraint(const char *sname,
                         const char *scount,
                         const char *sdivider){
    assert(sscanf(scount,"%d",&count));
    assert(sscanf(sdivider,"%d",&divider));
    strncpy(name,sname,VPC_MAX_STRING_LENGTH);
    activationTime=-1;
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
    // cerr << VPC_YELLOW("Constraint: "<<name<<" isSatisfied()?  ");
    if(term->isSatisfied(excludes)){
      activationTime=sc_time_stamp().to_default_time_units();
      //cerr << VPC_GREEN("YES "<< sc_simulation_time())<<NENDL;
      return true;
    }else{
      //cerr << VPC_RED("NO "<< sc_simulation_time())<<NENDL;
    }
    return false;
  }

  /**
   *
   */
  void Constraint::getReport(){
    cerr << VPC_YELLOW("Constraint: "<< name <<" valid activation: "
         <<activationTime)<<NENDL;
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
      PCBIterator iter = Director::getInstance().getPCBPool().getPCBIterator();

      while(iter.hasNext()){
        const ProcessControlBlock& pcb= iter.getNext();
        if( excludes.end() == excludes.find(pcb.getName()) ){
          if((rule==start && pcb.getState()==starting)){
            satisfiableCounter=0;
            return true;
          }else if((rule==end && pcb.getState()==ending)){
            return true;
          }
        }
      }
    }
    return false;
  }
}
