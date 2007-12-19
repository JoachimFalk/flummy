#include "hscd_vpc_OfflineFile.h"
#include <string>
#include <iostream>
#include <fstream>

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
    
       
}
