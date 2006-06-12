#include "hscd_vpc_Binding.h"

namespace SystemC_VPC {

	/*****************************
	 *  SECTION ChildIterator
	 *****************************/

	ChildIterator::ChildIterator(std::set<Binding* >* childs) : childs(childs) {
		if(this->childs != NULL){
			this->iter = this->childs->begin();
		}
	}

	bool ChildIterator::hasNext(){
		return (this->iter != this->childs->end());
	}

	Binding*  ChildIterator::getNext(){
		Binding* b = *(this->iter);
		this->iter++;
		return b;
	}

	void  ChildIterator::reset(){
		if(this->childs != NULL){
			this->iter = this->childs->begin();
		}
	}

	/*****************************
	 * SECTION MappingInformationIterator
	 *****************************/

	MappingInformationIterator::MappingInformationIterator(std::set<MappingInformation* >* mInfos) : mInfos(mInfos) {
		if(this->mInfos != NULL){
			this->iter = this->mInfos->begin();
		}
	}

	MappingInformationIterator::~MappingInformationIterator() {}

	bool MappingInformationIterator::hasNext(){
		return (this->iter != this->mInfos->end());
	}

	MappingInformation* MappingInformationIterator::getNext(){
		MappingInformation* mI = *(this->iter);
		this->iter++;
		return mI;
	}

	void MappingInformationIterator::reset() {
		this->iter = this->mInfos->begin();
	}

	/*****************************
	 * SECTION Binding
	 *****************************/
 
	Binding::Binding(std::string id) : id(id) {}

  Binding::~Binding(){
		// only leaves have to clean up mapping infos
		if(this->childs.size() == 0){
			std::set<MappingInformation* >:: iterator iter;
			
			for(iter = this->mInfos.begin(); iter != this->mInfos.end(); iter++){
				delete *iter;
			}
		}
	}
  
	/**
	 * \brief Returns the associated id of the Binding instance
	 */
	std::string& Binding::getID() const{
		return const_cast<std::string&>(this->id);
	}

	/**
	 * \brief registers additional binding possibility to a Binding
	 * If already a binding for given target exist only the MappingInformation
	 * is added.
	 * \param target refers to the additional target
	 */
	void Binding::addBinding(Binding* target){

		// never allow insertion of self as child!
		assert(this != target);
		
		// check if binding exists else add new one
		std::set<Binding* >::iterator iter;
		iter = this->childs.find(target);
		if(iter == this->childs.end()){
			this->childs.insert(target); 
		}

		// add all MappingInformations of successor to parent
		MappingInformationIterator* mIter = target->getMappingInformationIterator();
		while(mIter->hasNext()){
			this->addMappingInformation(mIter->getNext());
		}
	}

	void Binding::addMappingInformation(MappingInformation* mInfo){

		// check if binding is leaf and has already one mapping info associated, in this case stop!
		assert(this->childs.size() > 0 || this->mInfos.size() == 0);
		
		// add mInfo to list of possibilities
		this->mInfos.insert(mInfo);

	}
	
	ChildIterator* Binding::getChildIterator(){

		return new ChildIterator(&(this->childs));
	
	}

	MappingInformationIterator* Binding::getMappingInformationIterator(){

		return new MappingInformationIterator(&(this->mInfos));

	}

}

