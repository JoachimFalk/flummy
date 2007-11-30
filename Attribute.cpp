#include <Attribute.h>
#include <iostream>

namespace SystemC_VPC{

Attribute::Attribute(){
	type=NULL;
	value=NULL;
}

Attribute::Attribute( char* newType, char* newValue){
	type=newType;
	value=newValue;
}

std::pair<std::string, std::string> Attribute::getNextParameter(int pos)throw(InvalidArgumentException){
	if(pos<=parameters.size()) return parameters[pos];
	throw new InvalidArgumentException("getNextParameter");
}

void Attribute::addNewParameter(char* newType,char* newValue){
	std::pair<std::string, std::string> toadd(newType,newValue);
  	this->parameters.push_back(toadd);
}

std::pair<std::string, Attribute > Attribute::getNextAttribute(int pos)throw(InvalidArgumentException){
	if(pos<=attributes.size()) return attributes[pos];
	throw new InvalidArgumentException("getNextAttribute");
	
}

void Attribute::addNewAttribute( char* newType, char* newValue){
	Attribute toadd1(newType, newValue);
	std::pair<std::string, SystemC_VPC::Attribute > toadd(newValue,toadd1);		
  	attributes.push_back(toadd);
}

void Attribute::addNewAttribute( Attribute& toadd1, char* newType){
	std::pair<std::string, SystemC_VPC::Attribute > toadd(newType,toadd1);		
	attributes.push_back(toadd);
}

int Attribute::getParameterSize(){
	return parameters.size();
}

int Attribute::getAttributeSize(){
	return attributes.size();
}

char* Attribute::getValue(){
	return value;
}

char* Attribute::getType(){
	return type;
}

void Attribute::setValue(char* newValue){
	value=newValue;
}

void Attribute::setType(char* newType){
	type=newType;
}
		
	

}
