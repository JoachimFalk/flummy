// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2020 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELIMPL_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELIMPL_HPP

#include <systemcvpc/PossibleAction.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/ExecModel.hpp>

#include "TaskInstanceImpl.hpp"

#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>

//#include <vector>
//#include <memory>

namespace SystemC_VPC { namespace Detail {

  class AbstractComponent;

  /**
   * \brief Interface for classes implementing an execution model
   */
  class AbstractExecModel
    : private boost::noncopyable {
    typedef AbstractExecModel this_type;
  public:
    typedef boost::intrusive_ptr<this_type>       Ptr;
    typedef boost::intrusive_ptr<this_type> const ConstPtr;

    class CompState {};
    class ActionInfo {};

    AbstractExecModel(int facadeAdj);

    ExecModel       *getExecModel() {
      // Pointer magic. Shift our this pointer
      // so that it points to the ExecModel
      // base class of our real implementation.
      return reinterpret_cast<ExecModel *>(
          reinterpret_cast<char *>(this) + facadeAdj);
    }
    ExecModel const *getExecModel() const
      { return const_cast<this_type *>(this)->getExecModel(); }

    ///
    /// Interfaces for AbstractComponent
    ///

    /// Allocate opaque CompState object when attaching to an abstract component.
    virtual CompState  *attachToComponent(AbstractComponent *comp) = 0;

    virtual ActionInfo *registerAction(CompState *&execModelComponentState
      , TaskInterface const *actor
      , PossibleAction const *action) = 0;

    /// Change the power mode of a component. This should update the
    /// opaque object pointed to by execModelComponentState.
    virtual void  setPowerMode(CompState *&execModelComponentState
      , std::string const &mode) const = 0;

    /// Initialize ti with action or guard timing and power values.
    virtual void  initTaskInstance(CompState *&execModelComponentState
      , ActionInfo *ai
      , TaskInstanceImpl *ti
      , bool forGuard = false) const = 0;

    virtual ~AbstractExecModel();
  private:
    int                         facadeAdj;
  };

  inline
  void intrusive_ptr_add_ref(AbstractExecModel *p) {
    p->getExecModel()->add_ref();
  }

  inline
  void intrusive_ptr_release(AbstractExecModel *p) {
    if (p->getExecModel()->del_ref())
      // AbstractExecModel has virtual destructor
      delete p;
  }

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_EXECMODELIMPL_HPP */
