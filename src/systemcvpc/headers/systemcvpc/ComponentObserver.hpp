// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2020 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_SYSTEMCVPC_COMPONENTOBSERVER_HPP
#define _INCLUDED_SYSTEMCVPC_COMPONENTOBSERVER_HPP

#include "Component.hpp"
#include "Extending/Task.hpp"
#include "Extending/TaskInstance.hpp"

#include <CoSupport/SmartPtr/RefCountObject.hpp>

namespace SystemC_VPC { namespace Detail {

  class ObservableComponent;

} } // namespace SystemC_VPC::Detail

namespace SystemC_VPC {

  class ComponentObserver
    : public CoSupport::SmartPtr::RefCountObject
  {
    friend class Detail::ObservableComponent;

    typedef ComponentObserver this_type;
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type const> ConstPtr;

  protected:
    class OTask         {};
    class OTaskInstance {};

    enum class ComponentOperation {
      PWRCHANGE = 1,
    };
    enum class TaskOperation {
      MEMOP_MASK = 3,
      /// Operation done once per actor to construct OTask.
      ALLOCATE   = 1,
      /// Operation done once per actor to destroy OTask.
      DEALLOCATE = 2
    };
    enum class TaskInstanceOperation {
      MEMOP_MASK = 3,
      /// Operation done once per actor firing to construct OTaskInstance.
      ALLOCATE   = 1,
      /// Operation done once per actor firing to destroy OTaskInstance.
      DEALLOCATE = 2,
      /// First operation of a task instance to indicate an enabled actor firing.
      RELEASE    = 4,
      /// Operation done possibly multiple times to indicate that the task
      /// instance runs on the component.
      ASSIGN     = 8,
      /// Operation done possibly multiple times to indicate that the task
      /// instance suspends execution on the component.
      RESIGN     = 12,
      /// Operation done possibly multiple times to indicate that the task
      /// instance is blocked waiting for something.
      BLOCK      = 16,
      /// Operation done at most once per actor firing to indicate that
      /// the DII of the task instance is over.
      FINISHDII  = 20,
      /// Final operation of a task instance to indicate that the latency
      /// of the task instance is over.
      FINISHLAT  = 24
    };

  protected:
    ComponentObserver(size_t rt, size_t rti)
      : reservePerTask(rt)
      , reservePerTaskInstance(rti) {}

    virtual void componentOperation(ComponentOperation co
      , Component       const &c) = 0;

    virtual void taskOperation(TaskOperation to
      , Component       const &c
      , Extending::Task const &t
      , OTask                 &ot) = 0;

    virtual void taskInstanceOperation(TaskInstanceOperation tio
      , Component               const &c
      , Extending::TaskInstance const &ti
      , OTask                         &ot
      , OTaskInstance                 &oti) = 0;

    size_t getReservePerTask() const
      { return reservePerTask; }
    size_t getReservePerTaskInstance() const
      { return reservePerTaskInstance; }
  private:
    size_t reservePerTask;
    size_t reservePerTaskInstance;
  };

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_COMPONENTOBSERVER_HPP */
