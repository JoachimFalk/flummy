#ifndef __INCLUDED__POOL__H_
#define __INCLUDED__POOL__H_

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
    
    ~NotAllocatedException() throw(){}

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

    // references prototype instance of PCB 
    OBJECT* prototype;
    // list of currently used PCBs
    std::map<int, OBJECT* > usedPCB;
    // list of currently available PCBs
    std::map<int, OBJECT* > freePCB;
    // reference to "parent" PrototypedPool
    //PrototypedPool *parentPool;

  public:

    /**
     * \brief Default constructor of PrototypedPool
     * \param pcb specifies the associated PCB to be managed and replicated
     */
    template<typename KEY>
    PrototypedPool( AssociativePrototypedPool<KEY, OBJECT> *parent ) {
      prototype = new OBJECT(parent);
    }

    ~PrototypedPool(){
      if(this->usedPCB.size() != 0){
        std::cerr << "WARNING: PrototypedPool for "
                  << this->prototype->getName()
                  << " still used instances exist!" << std::endl
                  << "Assuming interruption by sc_stop happend."
                  << " Cleaning up all instances of "
                  << this->prototype->getName() << std::endl;
      }

      for(typename std::map<int, OBJECT* >::iterator iter
            = this->freePCB.begin(); 
          iter != this->freePCB.end();
          ++iter){
        delete iter->second;
      }
    }

    /**
     * \brief Gets prototype instance of managed type
     */ 
    OBJECT& getPrototype(){
      return *prototype;
    }

    /**
     * \brief retrieves instance of managed PCB out of the pool
     * If currently no instance is available a new instance if create
     * to satify request.
     */
    OBJECT* allocate(){

      OBJECT* instance = NULL;

      if(this->freePCB.size() > 0){
        typename std::map<int, OBJECT* >::iterator iter;
        iter = this->freePCB.begin();
        instance = iter->second;
        this->usedPCB[iter->first] = instance;
        this->freePCB.erase(iter);
      }else{
        instance = new OBJECT(*(this->prototype));
        this->usedPCB[instance->getInstanceId()] = instance;
      }

      return instance;

    }

    /**
     * \brief returns instance of managed OBJECT into the pool
     */
    void free(OBJECT* p){

      typename std::map<int, OBJECT* >::iterator iter;
      iter = this->usedPCB.find(p->getInstanceId());
      if(iter != this->usedPCB.end()){
        this->usedPCB.erase(iter);
      
        iter = this->freePCB.find(p->getInstanceId());
        if(iter == this->freePCB.end()){
          this->freePCB[p->getInstanceId()] = p;
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
        delete *iter;
      }
    }

    OBJECT* allocate( KEY key )
      throw (NotAllocatedException){

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

  };
}
#endif //__INCLUDED__POOL__H_
