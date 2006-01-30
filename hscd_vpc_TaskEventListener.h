#ifndef HSCD_VPC_TASKEVENTLISTENER_
#define HSCD_VPC_TASKEVENTLISTENER_

#include <hscd_vpc_datatypes.h>

namespace SystemC_VPC{

class TaskEventListener{

public:
	
	virtual void signalTaskEvent(p_struct* pcb)=0;
	
	virtual void signalStateChanged(){};
	
};

}

#endif /*HSCD_VPC_TASKEVENTLISTENER_*/
