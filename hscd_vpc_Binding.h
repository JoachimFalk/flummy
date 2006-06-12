#ifndef HSCD_VPC_BINDING_H_
#define HSCD_VPC_BINDING_H_

#include <set>
#include <string>

#include "hscd_vpc_MappingInformation.h"

namespace SystemC_VPC {

  class Binding;

  class ChildIterator {
  
    private:

      std::set<Binding* >* childs;
      std::set<Binding* >::iterator iter;

    public:

      ChildIterator(std::set<Binding* >* childs);

      bool hasNext();

      Binding* getNext();

      void reset();
      
  };
  
	/**
	 * \brief Enables access to MappingInformations associated with a target component
	 */
	class MappingInformationIterator {

		private:

			std::set<MappingInformation* >* mInfos;
			std::set<MappingInformation* >::iterator iter;

		public:

			MappingInformationIterator(std::set<MappingInformation* >* mInfos);

			~MappingInformationIterator();

			bool hasNext();

			MappingInformation* getNext();

			void reset();
			
	};
		
	
  class Binding {

    private:

      std::string id;
      std::set<Binding* > childs;

			std::set<MappingInformation* > mInfos;

    public:

      Binding(std::string id);

      virtual ~Binding();

      /**
       * \brief Returns the associated id of the Binding instance
			 * Id refers to component id the binding represents.
       */
      std::string& getID() const;

      /**
       * \brief registers additional binding possibility to a Binding
       * If already a binding for given target exist only the MappingInformation
       * is added.
       * \param target refers to the additional target
       */
      virtual void addBinding(Binding* target);

      void addMappingInformation(MappingInformation* mI);

			/**
			 * \brief returns iterator over successing binding possibilities
			 */
			ChildIterator* getChildIterator();
			
			/**
			 * \brief returns iterator over associated mapping informations
			 */
			MappingInformationIterator* getMappingInformationIterator();
			
  };

}

#endif //HSCD_VPC_BINDING_H_
