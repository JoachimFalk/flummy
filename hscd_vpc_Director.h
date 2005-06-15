/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Director.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef HSCD_VPC_DIRECTOR_H
#define HSCD_VPC_DIRECTOR_H
#include "hscd_vpc_Component.h"
#include "hscd_vpc_datatypes.h"
#include <string>
namespace SystemC_VPC{
  using std::string;

  class Director{
  private:
    static std::auto_ptr<Director> singleton; 
    Director();
    map<string,AbstractComponent*> component_map_by_name;
    //map<int,AbstractComponent*> component_map_by_pid;
    map<string,AbstractComponent*> mapping_map_by_name;
    //map<int,AbstractComponent*> mapping_map_by_pid;
    map<string,p_struct> p_struct_map_by_name;
    //map<int,p_struct> p_struct_map_by_pid;

  public:
    bool FALLBACKMODE;
    AbstractComponent& getResource( const char *name );
    //  AbstractComponent& getResource(int process);
    //  p_struct& getProcessControlBlock(int process);
    p_struct& getProcessControlBlock( const char *name );
    static Director& getInstance(){
      return *singleton;
    }

    virtual ~Director();
  };
}
#endif
