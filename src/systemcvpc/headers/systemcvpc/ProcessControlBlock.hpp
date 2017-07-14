/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef HSCD_VPC_PROCESSCONTROLBLOCK_H_
#define HSCD_VPC_PROCESSCONTROLBLOCK_H_

#include <systemc>
#include <float.h>
#include <map>
#include <vector>
#include <string>

#include <CoSupport/SystemC/systemc_support.hpp>
#include <CoSupport/Tracing/TaskTracer.hpp>

#include "EventPair.hpp"
#include "datatypes.hpp"
#include "FastLink.hpp"
#include "PowerMode.hpp"
#include "config/Timing.hpp"
#include "FunctionTimingPool.hpp"

namespace SystemC_VPC {

namespace Trace{
  class Tracing;
}

  class AbstractComponent;

  typedef size_t ComponentId;

 /**
  * This class represents all necessary data of a simulated process within VPC
  * and provides necessary access methods for its data.
  */
  class ProcessControlBlock {

    public:
    friend class Task;
      /**
       * \brief Default constructor of an PCB instance
       */
      ProcessControlBlock( AbstractComponent * component );


      /**
       * \brief Sets name of instance
       */
      void configure(std::string name, bool tracing);

      /**
       * \brief Used to access name of PCB
       */
      std::string const& getName() const;

      void setPid( ProcessId pid);
      ProcessId getPid( ) const;
      
      /**
       * \brief Sets currently associated function id of process
       */
      void setFunctionId( FunctionId fid);
      FunctionId getFunctionId( ) const;
      
      /**
       * \brief due to pipelining, there may be several instances of a process
       */
      int getInstanceId() const;
      

      /**
       * \brief Gets currently associated function name of PCB instance
       */
      const char* getFuncName() const;

      void setPeriod(sc_core::sc_time period);

      sc_core::sc_time getPeriod() const;
      
      void setPriority(int priority);

      int getPriority() const;

      void setDeadline(sc_core::sc_time deadline);

      sc_core::sc_time getDeadline() const;

      /**
       * \brief Used to increment activation count of this PCB instance
       */
      void incrementActivationCount();

      unsigned int getActivationCount() const;

      void setTraceSignal(Trace::Tracing* signal);

      Trace::Tracing* getTraceSignal();

      void setTiming(const Config::Timing& timing);
      void setBaseDelay(sc_core::sc_time delay);
      void setBaseLatency(sc_core::sc_time latency);
      void addDelay(FunctionId fid, sc_core::sc_time delay);
      void addLatency(FunctionId fid, sc_core::sc_time latency);

      void setActorAsPSM(bool psm);
      bool isPSM();

    private:

      std::string name;
      ProcessId   pid;
      FunctionId  fid;

      int priority;
      sc_core::sc_time period;
      sc_core::sc_time deadline;
      Trace::Tracing * traceSignal;
      AbstractComponent * component;
      CoSupport::Tracing::TaskTracer::Ptr taskTracer;
      bool psm;
      /**
       * \brief Initialize a newly created instance of ProcessControlBlock
       */
      void init();
 
  };
typedef boost::shared_ptr<ProcessControlBlock> ProcessControlBlockPtr;
}
#endif // HSCD_VPC_PROCESSCONTROLBLOCK_H_
