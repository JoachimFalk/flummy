// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SYSTEMCVPC_POOL_HPP
#define _INCLUDED_SYSTEMCVPC_POOL_HPP

#include <exception>
#include <map>
#include <string>

#include <CoSupport/String/convert.hpp>

#include <iostream>

namespace SystemC_VPC {

  class NotAllocatedException: public std::exception{

    std::string msg;

  public:

    NotAllocatedException() : msg("Not allocated") {
    }

    NotAllocatedException(std::string msg) : msg(msg +" not allocated") {}
    
    ~NotAllocatedException() {}

    const std::string& what(){

      return NotAllocatedException::msg;

    }

  }; 


  template<typename KEY, class OBJECT>
  class AssociativePrototypedPool;

  /**
   * \brief Helper class associated with one type of OBJECT
   * Used to managed one concrete type of OBJECT and to
   * allocated necessary amount of instances.
   */
  template<class OBJECT>
  class PrototypedPool{
  private:
    typedef std::map<size_t, OBJECT* > Objects;
    // references prototype instance of Object 
    OBJECT* prototype;
    // list of currently used Objects
    Objects usedObjects;
    // list of currently available Objects
    Objects freeObjects;
    // reference to "parent" PrototypedPool
    //PrototypedPool *parentPool;

  public:

    /**
     * \brief Default constructor of PrototypedPool
     */
    template<typename P1>
    PrototypedPool( P1 p1 ) {
      prototype = new OBJECT(p1);
    }

    /**
     * \brief constructor of PrototypedPool with parameter support
     */
    template<typename P1, typename P2>
    PrototypedPool( P1 p1, P2 p2 ) {
      prototype = new OBJECT(p1, p2);
    }

    ~PrototypedPool(){
      if(this->usedObjects.size() != 0){
        /*
        std::cerr << "WARNING: PrototypedPool for "
                  << this->prototype->getName()
                  << " still used instances exist!" << std::endl
                  << "Assuming interruption by sc_stop happend."
                  << " Cleaning up all instances of "
                  << this->prototype->getName() << std::endl;
        */
      }

      for(typename Objects::iterator iter
            = this->freeObjects.begin(); 
          iter != this->freeObjects.end();
          ++iter){
        delete iter->second;
      }

      for(typename Objects::iterator iter
            = this->usedObjects.begin(); 
          iter != this->usedObjects.end();
          ++iter){
        delete iter->second;
      }
      
      delete prototype;
    }

    /**
     * \brief Gets prototype instance of managed type
     */ 
    OBJECT& getPrototype() const {
      return *prototype;
    }

    /**
     * \brief retrieves instance of managed Object out of the pool
     * If currently no instance is available a new instance if create
     * to satify request.
     */
    OBJECT* allocate(){

      OBJECT* instance = NULL;

      //      typename Objects::iterator iter
      //        = this->freeObjects.begin();
      //      if( iter != this->freeObjects.end() ){
      if(this->freeObjects.size() > 0){
        typename Objects::iterator iter;
        iter = this->freeObjects.begin();
        instance = iter->second;
        this->usedObjects[iter->first] = instance;
        this->freeObjects.erase(iter);
      }else{
        instance = new OBJECT(*(this->prototype));
        this->usedObjects[instance->getInstanceId()] = instance;
      }

      return instance;

    }

    /**
     * \brief returns instance of managed OBJECT into the pool
     */
    void free(OBJECT* p){

      typename Objects::iterator iter;
      iter = this->usedObjects.find(p->getInstanceId());
      if(iter != this->usedObjects.end()){
        this->usedObjects.erase(iter);
      
        iter = this->freeObjects.find(p->getInstanceId());
        if(iter == this->freeObjects.end()){
          this->freeObjects[p->getInstanceId()] = p;
        }
      }
    }
  };

  /**
   * \brief Class used to manage pools of OBJECTs
   */
  template<typename KEY, class OBJECT>
  class AssociativePrototypedPool{

  private:


    // contains managed pools
    typedef std::map<KEY, PrototypedPool<OBJECT>*>  PrototypedPools;
    PrototypedPools                                 pools;

  public:

    AssociativePrototypedPool() : pools() {}

    ~AssociativePrototypedPool(){

      typename PrototypedPools::iterator iter;
      for(iter = this->pools.begin(); iter != this->pools.end(); iter++){
        delete iter->second;
      }
    }

    OBJECT* allocate( KEY key ) {

      typename PrototypedPools::iterator iter = pools.find(key);
      if( iter != pools.end() ){
        PrototypedPool<OBJECT> *pool = iter->second;
        return pool->allocate();
      }

      throw NotAllocatedException(CoSupport::String::asStr(key));
    }

    void free(KEY key, OBJECT* obj){

      typename PrototypedPools::iterator iter = pools.find(key);
      if( iter != pools.end() ){
        PrototypedPool<OBJECT> *pool = iter->second;
        pool->free(obj);
      }

    }

    /**
     * create a new PrototypedPool and returns Prototype
     * \return Protype of created PrototypedPool
     */
    OBJECT& createObject( KEY key ){
      assert(pools.find(key) == pools.end());
      pools[key] = new PrototypedPool<OBJECT>( this );
      return this->pools[key]->getPrototype();
    }

    /**
     * \return Protype of created PrototypedPool
     */
    OBJECT& getPrototype( KEY key ){
      assert(pools.find(key) != pools.end());
      assert(this->pools[key]);
      return this->pools[key]->getPrototype();
    }

    /**
     *
     */
    bool contains( KEY key ) const {
      return ( pools.find(key)!= pools.end() );
    }

  };
}
#endif /* _INCLUDED_SYSTEMCVPC_POOL_HPP */
