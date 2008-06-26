#ifndef PSTRUCTPOOL_H_
#define PSTRUCTPOOL_H_

#include <exception>
#include <map>
#include <string>

#include <CoSupport/String/convert.hpp>

#include "hscd_vpc_ProcessControlBlock.h"
#include "FastLink.h"

namespace SystemC_VPC {

  class NotAllocatedException: public std::exception{

    std::string msg;

    public:

    NotAllocatedException() : msg("Not allocated") {
    }

    NotAllocatedException(std::string msg) : msg(msg +" not allocated") {}
    
    NotAllocatedException(ProcessId id) : msg(CoSupport::String::asStr(id) +
                                           " not allocated") {}
    
    ~NotAllocatedException() throw(){}

    const std::string& what(){

      return NotAllocatedException::msg;

    }

  }; 

  class PCBIterator;

  /**
   * \brief Class used to managed pool of ProcessControlBlocks
   */
  class PCBPool{



    private:

      /**
       * \brief Helper class associated with one type of ProcessControlBlock
       * Used to managed one concrete type of ProcessControlBlock and to
       * allocated necessary amount of instances.
       */
      class TypePool{

        friend class InstanceIterator;
        
        public: 
        
        /**
         * \brief Iterator over all PCB instance within TypePool
         */
        class InstanceIterator {

          enum pcbIteratorState {
            pos_free, 
            pos_used, 
            pos_end
          };

          private:  

          pcbIteratorState state;
          TypePool* pool;
          std::map<int, ProcessControlBlock* >::iterator iter;

          public:

          InstanceIterator(TypePool* pool);

          bool hasNext();

          ProcessControlBlock const & getNext();

        };
        
        private:

        // references base instance of PCB 
        ProcessControlBlock* base;
        // list of currently used PCBs
        std::map<int, ProcessControlBlock* > usedPCB;
        // list of currently available PCBs
        std::map<int, ProcessControlBlock* > freePCB;
        // reference to "parent" PCBPool
        PCBPool *parentPool;

        public:

        /**
         * \brief Default constructor of TypePool
         * \param pcb specifies the associated PCB to be managed and replicated
         */
        TypePool( PCBPool *parent ) {
          base = new ProcessControlBlock(parent);
        }

        ~TypePool();

        /**
         * \brief Gets base instance of managed type
         */ 
        ProcessControlBlock& getBase(){
          return *base;
        }

        /**
         * \brief retrieves instance of managed PCB out of the pool
         * If currently no instance is available a new instance if create
         * to satify request.
         */
        ProcessControlBlock* allocate();

        /**
         * \brief returns instance of managed PCB into the pool
         */
        void free(ProcessControlBlock* p);

        /**
         * \brief returns iterator over currently managed instances of TypePool
         */
        InstanceIterator* getInstanceIterator();
      };

      friend class PCBIterator;

      // contains managed typepools
      typedef std::vector<PCBPool::TypePool*>    TypePools;
      TypePools                                  typepools;

    public:

      PCBPool();

      ~PCBPool();

      ProcessControlBlock* allocate( ProcessId pid )
        throw (NotAllocatedException);

      void free(ProcessControlBlock* p);

      ProcessControlBlock& registerPCB( ProcessId pid );

      PCBIterator getPCBIterator();

      ProcessControlBlock& getBase( ProcessId pid );

  };

  /**
   * \brief Iterator over all PCB instance within PCBPool
   */
  class PCBIterator {

    PCBPool::TypePools*                  pool;
    PCBPool::TypePools::iterator         typeIter;
    PCBPool::TypePool::InstanceIterator* instanceIter;

    public:

    PCBIterator( PCBPool::TypePools * pool );

    bool hasNext();

    ProcessControlBlock const & getNext();

  };


}
#endif //PSTRUCTPOOL_H_
