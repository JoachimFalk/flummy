/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Director.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef HSCD_VPC_DIRECTOR_H
#define HSCD_VPC_DIRECTOR_H
#include "hscd_vpc_Component.h"
#include "hscd_vpc_datatypes.h"
#include <string>
namespace SystemC_VPC{
  using std::string;

  class Director{
  private:
    static std::auto_ptr<Director> singleton; 
    Director();
    map<string,Component*> component_map_by_name;
    map<int,Component*> component_map_by_pid;
    map<string,Component*> mapping_map_by_name;
    map<int,Component*> mapping_map_by_pid;
    map<string,p_struct> p_struct_map_by_name;
    map<int,p_struct> p_struct_map_by_pid;

  
    // int create_fallback_id;

  public:
    bool FALLBACKMODE;
    AbstractComponent& getResource( const char *name );
    //  AbstractComponent& getResource(int process);
    //  p_struct& getProcessControlBlock(int process);
    p_struct& getProcessControlBlock( const char *name );
    static Director& getInstance(){
      return *singleton;
    }
    static void init();
    static void stop();  
    virtual ~Director();

    static const double STARTTIME[27];
    static const double DELAY[27];
    static const double DEADLINE[27];
    static char* PROCESS[27];
    static const double SUCCESSOR[27];

 
    static const char* IN_ID106005;
    static const char* C_IN_RF_ID106026;
    static const char* C_IN_BM_ID106017;
    static const char* C_IN_DIFF_ID106018;
    static const char* RF_ID106010;
    static const char* C_RF_BM_ID106025;
    static const char* BM_ID106001;
    static const char* C_BM_LF_ID106013;
    static const char* LF_ID106007;
    static const char* C_LF_DIFF_ID106020;
    static const char* C_LF_REC_ID106021;
    static const char* DIFF_ID106003;
    static const char* C_DIFF_DCT_ID106015;
    static const char* DCT_ID106002;
    static const char* C_DCT_Q_ID106014;
    static const char* Q_ID106008;
    static const char* C_Q_RLC_ID106023;
    static const char* C_Q_IQ_ID106022;
    static const char* IQ_ID106006;
    static const char* C_IQ_IDCT_ID106019;
    static const char* IDCT_ID106004;
    static const char* C_IDCT_REC_ID106016;
    static const char* REC_ID106009;
    static const char* C_REC_SF_ID106024;
    static const char* SF_ID106012;
    static const char* C_SF_RLC_ID106027;
    static const char* RLC_ID106011;


    /*  static const int RLC_ID106011 = 0;
	static const int C_IN_DIFF_ID106018 = 1;
	static const int C_Q_IQ_ID106022 = 2;
	static const int SF_ID106012 = 3;
	static const int C_RF_BM_ID106025 = 4;
	static const int IN_ID106005 = 5;
	static const int DCT_ID106002 = 6;
	static const int C_REC_SF_ID106024 = 7;
	static const int C_SF_RLC_ID106027 = 8;
	static const int DIFF_ID106003 = 9;
	static const int C_IDCT_REC_ID106016 = 10;
	static const int Q_ID106008 = 11;
	static const int C_DCT_Q_ID106014 = 12;
	static const int C_Q_RLC_ID106023 = 13;
	static const int IDCT_ID106004 = 14;
	static const int C_DIFF_DCT_ID106015 = 15;
	static const int C_IN_BM_ID106017 = 16;
	static const int LF_ID106007 = 17;
	static const int C_IN_RF_ID106026 = 18;
	static const int BM_ID106001 = 19;
	static const int IQ_ID106006 = 20;
	static const int C_IQ_IDCT_ID106019 = 21;
	static const int REC_ID106009 = 22;
	static const int C_LF_REC_ID106021 = 23;
	static const int C_BM_LF_ID106013 = 24;
	static const int RF_ID106010 = 25;
	static const int C_LF_DIFF_ID106020 = 26;*/
  
  };
}
#endif
