#include "hscd_vpc_Director.h"
#include "hscd_vpc_Component.h"
#include "systemc.h"
#include "hscd_vpc_RateMonotonicScheduler.h"

std::auto_ptr<Director> Director::singleton(new Director());
 

static AbstractComponent *risc1=0;
static AbstractComponent *risc2=0;
static AbstractComponent *risc3=0;


const double Director::STARTTIME[27]={141,0,76,131,10,0,46,126,141,40,106,66,61,76,91,41,0,30,0,15,76,86,111,40,25,0,40};


const double Director::DELAY[27]={
10,//IN
20,//C_IN_RF
110,//C_IN_BM
100,//C_IN_DIFF
100,//RF
50,//C_RF_BM
100,//BM
50,//C_BM_LF
100,//LF
10,//C_LF_DIFF
600,//C_LF_REC
10,//DIFF
50,//C_DIFF_DCT
150,//DCT
50,//C_DCT_Q
100,//Q
500,//C_Q_RLC
10,//C_Q_IQ
100,//IQ
50,//C_IQ_IDCT
150,//IDCT
50,//C_IDCT_REC
150,//REC
50,//C_REC_SF
100,//SF
10,//C_SF_RLC
150 //RLC
};

const double Director::DEADLINE[27]={
2000,//IN
2000,//C_IN_RF
1999,//C_IN_BM
1998,//C_IN_DIFF
2000,//RF
2000,//C_RF_BM
2000,//BM
2000,//C_BM_LF
2000,//LF
2000,//C_LF_DIFF
2020,//C_LF_REC
2000,//DIFF
2000,//C_DIFF_DCT
2000,//DCT
2000,//C_DCT_Q
2000,//Q
2000,//C_Q_RLC
2000,//C_Q_IQ
2000,//IQ
2000,//C_IQ_IDCT
2000,//IDCT
2000,//C_IDCT_REC
2000,//REC
2000,//C_REC_SF
2000,//SF
2000,//C_SF_RLC
2000 //RLC
};


char* Director::PROCESS[27]={"IN","C_IN_RF","C_IN_BM","C_IN_DIFF","RF","C_RF_BM","BM","C_BM_LF","LF",
			     "C_LF_DIFF","C_LF_REC","DIFF","C_DIFF_DCT","DCT","C_DCT_Q","Q","C_Q_RLC",
			     "C_Q_IQ","IQ","C_IQ_IDCT","IDCT","C_IDCT_REC","REC","C_REC_SF","SF","C_SF_RLC",
			     "RLC"};

const double Director::SUCCESSOR[27]={
 26 ,// IN
  23,//C_IN_RF
  21,//C_IN_BM
  16,//C_IN_DIFF
  22,//RF
  21,//C_RF_BM
  20,//BM
  19,//C_BM_LF
  18,//LF
  16,//c_LF_DIFF
  5,//C_LF_REC
  15,//DIFF
  14,//C_DIFF_DCT
  13,//DCT
  12,//C_DCT_Q
  11,//Q
  1,//C_Q_RLC
  9,//C_Q_IQ
  8,//IQ
  7,//C_IQ_IDCT
  6,//IDCT
  5,//C_IDCT_REC
  4,//REC
  3,//C_REC_SF
  2,//SF
  1,//C_SF_RLC
  0,//RLC

};

AbstractComponent& Director::getResource( const char *name ){
    return (*risc1);
}
AbstractComponent& Director::getResource(int process){
  switch(process){
  case(IN_ID106005):
  case(C_IN_DIFF_ID106018):
  case(C_IN_BM_ID106017):
  case(C_RF_BM_ID106025):
  case(C_BM_LF_ID106013):
  case(BM_ID106001):
  case(LF_ID106007):
  case(RF_ID106010):

    return(* risc1);
  case(C_IN_RF_ID106026): ///
  case(C_LF_REC_ID106021):
  case(DIFF_ID106003):
  case(C_DIFF_DCT_ID106015):
  case(DCT_ID106002):
  case(C_DCT_Q_ID106014):
  case(Q_ID106008):
    return (*risc2);



  case(C_LF_DIFF_ID106020):///
  case(RLC_ID106011):
  case(C_Q_IQ_ID106022):
  case(SF_ID106012):
  case(C_REC_SF_ID106024):
  case(C_SF_RLC_ID106027):
  case(C_IDCT_REC_ID106016):
  case(C_Q_RLC_ID106023):
  case(IDCT_ID106004):
  case(IQ_ID106006):
  case(C_IQ_IDCT_ID106019):
  case(REC_ID106009):
  default:
    return (*risc3);
  }
}
Director::Director(){
  risc1=new Component(Component::RISC1);
  risc2=new Component(Component::RISC2);
  risc3=new Component(Component::RISC3);
}
Director::~Director(){
  delete risc1;
  delete risc2;
  delete risc3;
}
// void Director::init(){
// cout << "<init name=\"Director\">" << endl;
// //manag=new Director;

// cout << "</init>"<<endl;
// }
// void Director::stop(){
//   delete risc1_id200010;
//   delete risc2;
// }
p_struct& Director::getProcessControlBlock(int pid){
  p_struct *pcb;
  pcb= new p_struct;
  
  pcb->name=PROCESS[pid];
  pcb->delay=DELAY[pid];
  pcb->pid=pid;
  pcb->priority=SUCCESSOR[pid];
  pcb->deadline=DEADLINE[pid];
  pcb->period=2800.0;
  return *pcb;
}
