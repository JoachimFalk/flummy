#ifndef IPREEMPTABLE_
#define IPREEMPTABLE_

#include <systemc.h>

namespace SystemC_VPC{
	
	class IPreemptable{
	
		public:
			
			/**
			 * \brief Preempts execution of component
			 * Used to preempt the current execution of a component.
			 * Actual executed tasks are "stored" for later execution
			 * or discarded depending on the parameter flag!
			 * \param kill indicates if "hard" preemption should be initiated
			 * and currently registered task are killed without restoring
			 * \return pointer to sc_time specifying time need for preemption
			 */
			virtual void preempt(bool kill)=0;
				
			/**
			 * \brief Resumes preempted execution
			 * Used to resume execution of preempted component.
			 * \return pointer to sc_time specifying time need for resuming
			 */
			virtual void resume()=0;
			
			/**
			 * \brief Gets time needed to preempt a preemptable object
			 * Used to determine time needed to store current state of an
			 * preemptable instance, if restoring after preemption is desired.
			 * \return time needed to store actual state of an instance to preempt
			 */
			virtual sc_time* timeToPreempt()=0;
			
			/**
			 * \brief Gets time needed to restore a preempted object
			 * Used to determine time needed to restore the state of an
			 * preemptable instance before its preemption.
			 * \return time needed to restore actual state of an instance to resume
			 */
			virtual sc_time* timeToResume()=0;
			
	};

}
#endif /*IPREEMPTABLE_*/
