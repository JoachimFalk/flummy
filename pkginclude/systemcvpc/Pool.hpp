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

    // references prototype instance of Object 
    OBJECT* prototype;
    // list of currently used Objects
    std::map<int, OBJECT* > usedObjects;
    // list of currently available Objects
    std::map<int, OBJECT* > freeObjects;
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

      for(typename std::map<int, OBJECT* >::iterator iter
            = this->freeObjects.begin(); 
          iter != this->freeObjects.end();
          ++iter){
        delete iter->second;
      }

      for(typename std::map<int, OBJECT* >::iterator iter
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

      //      typename std::map<int, OBJECT* >::iterator iter
      //        = this->freeObjects.begin();
      //      if( iter != this->freeObjects.end() ){
      if(this->freeObjects.size() > 0){
        typename std::map<int, OBJECT* >::iterator iter;
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

      typename std::map<int, OBJECT* >::iterator iter;
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

    /**
     *
     */
    bool contains( KEY key ){
      return ( pools.find(key)!= pools.end() );
    }

  };
}
#endif //__INCLUDED__POOL__H_
