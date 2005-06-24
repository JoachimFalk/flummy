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
     * \brief Read allokation and binding from file.
     */
    Director();
    map<string,AbstractComponent*> component_map_by_name;
    //map<int,AbstractComponent*> component_map_by_pid;
    map<string,AbstractComponent*> mapping_map_by_name;
    //map<int,AbstractComponent*> mapping_map_by_pid;
    map<string,p_struct> p_struct_map_by_name;
    //map<int,p_struct> p_struct_map_by_pid;

  public:
    bool FALLBACKMODE;

    /**
     * \brief A task (identifikation by name) calling this Funktion gets the 
     * AbstractComponent where he is binded to.
     */
    AbstractComponent& getResource( const char *name );
    //  AbstractComponent& getResource(int process);
    //  p_struct& getProcessControlBlock(int process);

    /**
     * \brief Get the process controll block used within SystemC-VPC Modell.
     */
    p_struct& getProcessControlBlock( const char *name );

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
