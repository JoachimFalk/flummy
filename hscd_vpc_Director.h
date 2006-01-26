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
#include "hscd_vpc_AbstractComponent.h"

#include <string>
#include <map.h>
#include <vector.h>

namespace SystemC_VPC{

  struct p_struct;
  class Constraint;

  /**
   * \brief Director knowes all (Abstract-)Components, all mappings (task -> component).
   *
   * Direktor reads allokation and binding from file.
   */
  class Director{
  private:

    /**
     * Singleton design pattern
     */
    static std::auto_ptr<Director> singleton; 

    /**
     * \brief Reads allokation and binding from file.
     */
    Director();
    map<std::string,AbstractComponent*> component_map_by_name;
    //map<int,AbstractComponent*> component_map_by_pid;
    map<std::string,AbstractComponent*> mapping_map_by_name;
    //map<int,AbstractComponent*> mapping_map_by_pid;
    map<std::string,p_struct*> p_struct_map_by_name;
    //map<int,p_struct> p_struct_map_by_pid;

    vector<Constraint*> constraints;

    std::string vpc_result_file;

    bool FALLBACKMODE;
  public:

    /**
     * \brief A task (identifikation by name) calling this Funktion gets the 
     * AbstractComponent where he is binded to.
     */
    AbstractComponent& getResource( const char *name );
    //  AbstractComponent& getResource(int process);

    /**
     * \brief Get the process controll block used within SystemC-VPC Modell.
     */
    p_struct* getProcessControlBlock( const char *name );
    //  p_struct& getProcessControlBlock(int process);

    /**
     *
     */
    map<std::string,p_struct*>& getPcbMap(){
      return p_struct_map_by_name;
    }

    /**
     *
     */
    void checkConstraints();

    /**
     *
     */
    void getReport();

    /**
     * \brief Access to singleton Director. 
     */
    static Director& getInstance(){
      return *singleton;
    }

    virtual ~Director();

  };
}
#endif
