#include "hscd_vpc_StringParser.h"

namespace SystemC_VPC{
    
  StringParser::StringParser(){
  }
  
  StringParser::~StringParser(){
  }
    
  void StringParser::cleanstring(std::string *output){
    std::string::iterator iter = output->begin();
        while(*iter == ' ' || *iter == '\t' ) {
          iter = output->erase(iter);
        }
        iter = output->end()-1;
        while(*iter == ' ' || *iter == '\t') {
          output->erase(iter);
          iter = output->end()-1;
        }
    return;
  }
    
  sc_time StringParser::generate_sctime(std::string starttime){
    //trenne Zahl und einheit
    std::string numbers = "0123456789.";
    std::string::iterator iter = starttime.begin();
    while( numbers.find(*iter) != std::string::npos) iter++;
    std::string time = starttime.substr(0, iter - starttime.begin());
    std::string unit = starttime.substr(iter - starttime.begin(), starttime.end() - iter);
    
    //std::cerr << "time:"<<time<<"Unit:"<<unit<<std::endl;
      
    std::istringstream timex;
    double timeindouble;
    timex.str( time );
    timex >> timeindouble;
    
    this->cleanstring(&unit);
#ifdef VPC_DEBUG    
    std::cerr << "time:"<<timeindouble<<"Unit:"<<unit<<std::endl;
#endif //VPC_DEBUG     
    //generiere sc_time(zahl,einheit)
    sc_time_unit scUnit = SC_NS;
    if(      0==unit.compare(0, 2, "fs") ) scUnit = SC_FS;
    else if( 0==unit.compare(0, 2, "ps") ) scUnit = SC_PS;
    else if( 0==unit.compare(0, 2, "ns") ) scUnit = SC_NS;
    else if( 0==unit.compare(0, 2, "us") ) scUnit = SC_US;
    else if( 0==unit.compare(0, 2, "ms") ) scUnit = SC_MS;
    else if( 0==unit.compare(0, 1, "s" ) ) scUnit = SC_SEC;
    
    return sc_time(timeindouble,scUnit);
  }
}
