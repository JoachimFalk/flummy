#include "hscd_vpc_Component.h"
#include "hscd_vpc_datatypes.h"
#ifndef HSCD_VPC_DIRECTOR_H
#define HSCD_VPC_DIRECTOR_H

class Director{
private:
  static std::auto_ptr<Director> singleton; 
  Director();

public:
  AbstractComponent& getResource( const char *name );
  AbstractComponent& getResource(int process);
  p_struct& getProcessControlBlock(int process);
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
  static const double Director::SUCCESSOR[27];


  static const int IN_ID106005 = 0;
  static const int C_IN_RF_ID106026 = 1;
  static const int C_IN_BM_ID106017 = 2;
  static const int C_IN_DIFF_ID106018 = 3;
  static const int RF_ID106010 = 4;
  static const int C_RF_BM_ID106025 = 5;
  static const int BM_ID106001 = 6;
  static const int C_BM_LF_ID106013 = 7;
  static const int LF_ID106007 = 8;
  static const int C_LF_DIFF_ID106020 = 9;
  static const int C_LF_REC_ID106021 = 10;
  static const int DIFF_ID106003 = 11;
  static const int C_DIFF_DCT_ID106015 = 12;
  static const int DCT_ID106002 = 13;
  static const int C_DCT_Q_ID106014 = 14;
  static const int Q_ID106008 = 15;
  static const int C_Q_RLC_ID106023 = 16;
  static const int C_Q_IQ_ID106022 = 17;
  static const int IQ_ID106006 = 18;
  static const int C_IQ_IDCT_ID106019 = 19;
  static const int IDCT_ID106004 = 20;
  static const int C_IDCT_REC_ID106016 = 21;
  static const int REC_ID106009 = 22;
  static const int C_REC_SF_ID106024 = 23;
  static const int SF_ID106012 = 24;
  static const int C_SF_RLC_ID106027 = 25;
   static const int RLC_ID106011 = 26;


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
#endif
