#include "hscd_vpc_OfflineFile.h"

namespace SystemC_VPC{
  
  /**
   * OfflineFile realizes file access to the schedulerfile for settings of OfflineBinder and OfflineAllocator
   */
    //Contructor
    OfflineFile::OfflineFile(const char *name){
    this->filename = name;
   
    }
    //Destructor
    OfflineFile::~OfflineFile(){
      file.close();
    }
    bool OfflineFile::open(){
      file.open(this->filename, std::ios_base::binary);
      return file.good();
          
    }
    void OfflineFile::close(){
      file.close();
    }
    
    int OfflineFile::getlength(){
      if(!file.good()) return 0;
      file.seekg (0, std::ios_base::beg);
      file.seekg (0, std::ios_base::end);
      int length = file.tellg();
      file.seekg (0, std::ios_base::beg);
      return length;
    }
    
    std::string OfflineFile::getbuffer(){
      if(!file.good()) return NULL;
      // get pointer to associated buffer object
      std::filebuf *filebuffer = file.rdbuf();
    
      // get file size using buffer's members
      int size=filebuffer->pubseekoff (0,std::ios::end,std::ios::in);
      filebuffer->pubseekpos (0,std::ios::in);
    
      // allocate memory to contain file data
      char* buffer=new char[size];
    
      // get file data  
      filebuffer->sgetn (buffer,size);
      
      std::string rtr(buffer);
      return rtr;
    }
    
    void OfflineFile::print(){
      if(!file.good()) return;
      char ch;
      std::cerr << "OfflineFile>" << std::endl;
      while (file.get(ch)) std::cout << ch;
      std::cerr << std::endl;
      file.seekg (0, std::ios_base::beg);
    }
    
    void OfflineFile::cleanstring(std::string *output){
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
    
  sc_time OfflineFile::generate_sctime(std::string starttime){
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
