#ifndef PSTRUCTPOOL_H_
#define PSTRUCTPOOL_H_

#include <exception>
#include <map>
#include <string>

#include "hscd_vpc_ProcessControlBlock.h"

namespace SystemC_VPC {

  class AlreadyLockedException: public std::exception{

    std::string msg;
    
    public:

      AlreadyLockedException() : msg("Already locked"){
      }

      ~AlreadyLockedException() throw(){}

      const std::string& what(){

        return msg;

      }

  };
   
  class NotLockedException: public std::exception{

    std::string msg;
    
    public:

      NotLockedException() : msg("Not locked"){
      }

      ~NotLockedException() throw(){}

      const std::string& what(){

        return NotLockedException::msg;

      }

  }; 
 
  class NotAllocatedException: public std::exception{
      
    std::string msg;
    
    public:

      NotAllocatedException() : msg("Not allocated") {
      }

      ~NotAllocatedException() throw(){}

      const std::string& what(){

        return NotAllocatedException::msg;

      }

  }; 
   
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

        private:

          int lockCount;
          // references base instance of PCB 
          ProcessControlBlock* base;
          // list of currently used PCBs
          std::map<int, ProcessControlBlock* > usedPCB;
          // list of currently available PCBs
          std::map<int, ProcessControlBlock* > freePCB;
          // list of currently locked PCBs
          std::map<int, ProcessControlBlock* > lockedPCB;
          
        public:

          /**
           * \brief Default constructor of TypePool
           * \param pcb specifies the associated PCB to be managed and replicated
           */
          TypePool() :  lockCount(0) {
            base = new ProcessControlBlock();
          }

          ~TypePool();
          
          ProcessControlBlock& getBase(){
            return *base;
          }
          
          /**
           * \brief retrieves instance of managed PCB out of the pool
           * If currently no instance is available a new instance if created
           * to satify request.
           */
          ProcessControlBlock* allocate();

          int lock(ProcessControlBlock* p) throw(AlreadyLockedException, NotAllocatedException);
          
          void unlock(int lockid) throw(NotLockedException);
          
          /**
           * \brief returns instance of managed PCB into the pool
           */
          void free(ProcessControlBlock* p);

      };

      // used to create unique ids
      int pid_count;
      // contains managed typepools
      std::map<std::string, TypePool* > typepools;

    public:

      PCBPool() : pid_count(0) {}

      ProcessControlBlock* allocate(std::string type) throw (NotAllocatedException);

      int lock(ProcessControlBlock* p) throw(AlreadyLockedException, NotAllocatedException);

      void unlock(std::string type, int lockid) throw(NotLockedException);
      
      void free(ProcessControlBlock* p);

      ProcessControlBlock& registerPCB(std::string type);
      
      bool hasPCBType(std::string type);

  };

}
#endif //PSTRUCTPOOL_H_
