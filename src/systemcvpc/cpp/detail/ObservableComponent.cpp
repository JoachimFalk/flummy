// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of
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

#include "ObservableComponent.hpp"

#include "AbstractComponent.hpp"

#include <CoSupport/sassert.h>

namespace SystemC_VPC { namespace Detail {

  ObservableComponent::ObservableComponent()
    : observerRegistrationAllowed(true)
    , nextFreeTaskOffset(sizeof(TaskImpl))
    , nextFreeTaskInstanceOffset(sizeof(TaskInstanceImpl)) {}

  void ObservableComponent::addObserver(Extending::ComponentObserverIf::Ptr const &obs) {
    assert(observerRegistrationAllowed);
    ObserverInfo oi;
    // Get next free 16 byte aligned task offset.
    nextFreeTaskOffset = (nextFreeTaskOffset + 15UL) & ~15UL;
    oi.taskOffset = nextFreeTaskOffset;
    nextFreeTaskOffset += obs->getReservePerTask();
    // Get next free 16 byte aligned task offset.
    nextFreeTaskInstanceOffset = (nextFreeTaskInstanceOffset + 15UL) & ~15UL;
    oi.taskInstanceOffset = nextFreeTaskInstanceOffset;
    nextFreeTaskInstanceOffset += obs->getReservePerTaskInstance();
    observers.insert(std::make_pair(obs, oi));
  }

  void ObservableComponent::componentOperation(ComponentOperation co
    , AbstractComponent const &c)
  {
    for (Observers::value_type const &e : observers)
      e.first->componentOperation(co, c);
  }

  void ObservableComponent::taskOperation(TaskOperation to
    , AbstractComponent const &c
    , TaskImpl                &t)
  {
    assert(!observerRegistrationAllowed);
    for (Observers::value_type const &e : observers) {
      e.first->taskOperation(to, c, t,
          *reinterpret_cast<OTask *>(
              reinterpret_cast<char *>(&t) + e.second.taskOffset));
    }
  }

  void ObservableComponent::taskInstanceOperation(TaskInstanceOperation tio
    , AbstractComponent const &c
    , TaskInstanceImpl        &ti)
  {
    assert(!observerRegistrationAllowed);

    if (tio == TaskInstanceOperation::FINISHLAT)
      tio = TaskInstanceOperation((int) tio | (int) TaskInstanceOperation::DEALLOCATE);

    for (Observers::value_type const &e : observers) {
      e.first->taskInstanceOperation(tio, c, ti
        , *reinterpret_cast<OTask *>(
              reinterpret_cast<char *>(ti.getTask()) + e.second.taskOffset)
        , *reinterpret_cast<OTaskInstance *>(
            reinterpret_cast<char *>(&ti) + e.second.taskInstanceOffset));
    }
  }

  TaskImpl *ObservableComponent::createTask(TaskInterface *taskInterface)
  {
    return createTask([taskInterface] (char *s) {
      sassert(new (s) TaskImpl(taskInterface) == reinterpret_cast<TaskImpl *>(s));
    });
  }
  TaskImpl *ObservableComponent::createTask(std::string const &taskName)
  {
    return createTask([&taskName] (char *s) {
      sassert(new (s) TaskImpl(taskName) == reinterpret_cast<TaskImpl *>(s));
    });
  }
  TaskImpl *ObservableComponent::createTask(std::function<void (char *)> factory)
  {
    observerRegistrationAllowed = false;

    char *storage = (char *) malloc(nextFreeTaskOffset);
    if (!storage)
      throw std::bad_alloc();
    try {
      factory(storage);
      TaskImpl *taskImpl = reinterpret_cast<TaskImpl *>(storage);
      // FIXME: Exceptions
      for (Observers::value_type const &e : observers) {
        e.first->taskOperation(TaskOperation::ALLOCATE
          , *static_cast<AbstractComponent *>(this)
          , *taskImpl
          , *reinterpret_cast<OTask *>(storage + e.second.taskOffset));
      }
      try {
        tasks.push_back(taskImpl);
        return taskImpl;
      } catch (...) {
        taskImpl->~TaskImpl();
        throw;
      }
    } catch (...) {
      free(storage);
      throw;
    }
  }

  TaskInstanceImpl *ObservableComponent::createTaskInstance(
      TaskImpl                                       *taskImpl
    , TaskInstanceImpl::Type                          type
    , PossibleAction                                 *firingRuleInterface
    , std::function<void (TaskInstanceImpl *)> const &diiCallback
    , std::function<void (TaskInstanceImpl *)> const &latCallback)
  {
    observerRegistrationAllowed = false;

    char *storage = (char *) malloc(nextFreeTaskInstanceOffset);
    if (!storage)
      throw std::bad_alloc();
    try {
      TaskInstanceImpl *taskInstanceImpl =
          new (storage) TaskInstanceImpl(taskImpl, type
              , firingRuleInterface, diiCallback, latCallback);
      // FIXME: Exceptions
      for (Observers::value_type const &e : observers) {
        e.first->taskInstanceOperation(TaskInstanceOperation::ALLOCATE
          , *static_cast<AbstractComponent *>(this)
          , *taskInstanceImpl
          , *reinterpret_cast<OTask *>(reinterpret_cast<char *>(taskImpl) + e.second.taskOffset)
          , *reinterpret_cast<OTaskInstance *>(storage + e.second.taskInstanceOffset));
      }
      return taskInstanceImpl;
    } catch (...) {
      free(storage);
      throw;
    }
  }


  ObservableComponent::~ObservableComponent()
    {}

} } // namespace SystemC_VPC::Detail
