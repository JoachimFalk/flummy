// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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
 */

#include "ObservableComponent.hpp"

#include "AbstractComponent.hpp"

#include <CoSupport/sassert.h>

namespace SystemC_VPC { namespace Detail {

  ObservableComponent::ObservableComponent()
    : nextFreeCompOffset(0)
    , nextFreeTaskOffset(sizeof(TaskImpl))
    , nextFreeTaskInstanceOffset(sizeof(TaskInstanceImpl))
    , oComponent(nullptr) {}

  void ObservableComponent::addObserver(Extending::ComponentObserverIf::Ptr const &obs) {
    assert(isObserverRegistrationAllowed());
    ObserverInfo oi;
    // Get next free 16 byte aligned component offset.
    nextFreeCompOffset = (nextFreeCompOffset + 15UL) & ~15UL;
    oi.compOffset = nextFreeCompOffset;
    nextFreeCompOffset += obs->getReservePerComponent();
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
    assert(oComponent);
    for (Observers::value_type const &e : observers)
      e.first->componentOperation(co
        , c, *reinterpret_cast<OComponent *>(oComponent + e.second.compOffset));
  }

  void ObservableComponent::taskOperation(TaskOperation to
    , AbstractComponent const &c
    , TaskImpl                &t)
  {
    assert(oComponent);
    for (Observers::value_type const &e : observers) {
      e.first->taskOperation(to
        , c, *reinterpret_cast<OComponent *>(oComponent + e.second.compOffset)
        , t, *reinterpret_cast<OTask *>(
            reinterpret_cast<char *>(&t) + e.second.taskOffset));
    }
  }

  void ObservableComponent::taskInstanceOperation(TaskInstanceOperation tio
    , AbstractComponent const &c
    , TaskInstanceImpl        &ti)
  {
    assert(oComponent);

    if (tio == TaskInstanceOperation::FINISHLAT)
      tio = TaskInstanceOperation((int) tio | (int) TaskInstanceOperation::DEALLOCATE);

    for (Observers::value_type const &e : observers) {
      e.first->taskInstanceOperation(tio
        , c, *reinterpret_cast<OComponent *>(oComponent + e.second.compOffset)
        , *reinterpret_cast<OTask *>(
            reinterpret_cast<char *>(ti.getTask()) + e.second.taskOffset)
        , ti, *reinterpret_cast<OTaskInstance *>(
            reinterpret_cast<char *>(&ti) + e.second.taskInstanceOffset));
    }
    // FIXME: Free storage in case of TaskInstanceOperation::DEALLOCATE; This causes SIGSEGV
    //if ((int) tio & (int) TaskInstanceOperation::DEALLOCATE) {
    //  ti.~TaskInstanceImpl();
    //  free(&ti);
    //}
  }

  void ObservableComponent::finalize() {
    // This function will disable observer registration.
    assert(isObserverRegistrationAllowed());

    Observers::const_iterator iter = observers.begin();

    char *storage = (char *) malloc(nextFreeCompOffset);
    if (!storage)
      throw std::bad_alloc();
    try {
      /// Iterate over all observers and let them initialize
      /// their component private data.
      while (iter != observers.end()) {
        Observers::value_type const &e = *iter++; // this line must not throw
        e.first->componentOperation(ComponentOperation::ALLOCATE
          , *static_cast<AbstractComponent *>(this)
          , *reinterpret_cast<OComponent *>(storage + e.second.compOffset));
      }
    } catch (...) {
      // Cleanup
      assert(iter != observers.begin());
      // This really must not throw!
      try {
        do {
          Observers::value_type const &e = *--iter;
          e.first->componentOperation(ComponentOperation::DEALLOCATE
            , *static_cast<AbstractComponent *>(this)
            , *reinterpret_cast<OComponent *>(storage + e.second.compOffset));
        } while (iter != observers.begin());
      } catch (...) {
        assert(!"Oops, OComponent destruction throws!");
      }
      free(storage);
      throw;
    }
    oComponent = storage; // This disables observer registration.
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
    assert(oComponent);

    Observers::const_iterator iter = observers.begin();

    char *storage = (char *) malloc(nextFreeTaskOffset);
    if (!storage)
      throw std::bad_alloc();
    try {
      factory(storage);
    } catch (...) {
      free(storage);
      throw;
    }

    TaskImpl          *taskImpl = reinterpret_cast<TaskImpl *>(storage);
    AbstractComponent &c        = *static_cast<AbstractComponent *>(this);

    try {
      /// Iterate over all observers and let them initialize
      /// their task private data.
      while (iter != observers.end()) {
        Observers::value_type const &e = *iter++; // this line must not throw
        e.first->taskOperation(TaskOperation::ALLOCATE
          , c, *reinterpret_cast<OComponent *>(oComponent + e.second.compOffset)
          , *taskImpl
          , *reinterpret_cast<OTask *>(storage + e.second.taskOffset));
      }
      tasks.push_back(taskImpl);
      return taskImpl;
    } catch (...) {
      // Cleanup
      assert(iter != observers.begin());
      // This really must not throw!
      try {
        do {
          Observers::value_type const &e = *--iter;
          e.first->taskOperation(TaskOperation::DEALLOCATE
            , c, *reinterpret_cast<OComponent *>(oComponent + e.second.compOffset)
            , *taskImpl
            , *reinterpret_cast<OTask *>(storage + e.second.taskOffset));
        } while (iter != observers.begin());
      } catch (...) {
        assert(!"Oops, OTask destruction throws!");
      }
      taskImpl->~TaskImpl();
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
    assert(oComponent);

    Observers::const_iterator iter = observers.begin();

    char *storage = (char *) malloc(nextFreeTaskInstanceOffset);
    if (!storage)
      throw std::bad_alloc();

    TaskInstanceImpl *taskInstanceImpl = nullptr;

    try {
      taskInstanceImpl =
          new (storage) TaskInstanceImpl(taskImpl, type
              , firingRuleInterface, diiCallback, latCallback);
    } catch (...) {
      free(storage);
      throw;
    }
    assert(taskInstanceImpl == reinterpret_cast<TaskInstanceImpl *>(storage));

    AbstractComponent &c = *static_cast<AbstractComponent *>(this);

    try {
      /// Iterate over all observers and let them initialize
      /// their task instance private data.
      while (iter != observers.end()) {
        Observers::value_type const &e = *iter++; // this line must not throw
        e.first->taskInstanceOperation(TaskInstanceOperation::ALLOCATE
          , c, *reinterpret_cast<OComponent *>(oComponent + e.second.compOffset)
          , *reinterpret_cast<OTask *>(reinterpret_cast<char *>(taskImpl) + e.second.taskOffset)
          , *taskInstanceImpl
          , *reinterpret_cast<OTaskInstance *>(storage + e.second.taskInstanceOffset));
      }
      return taskInstanceImpl;
    } catch (...) {
      // Cleanup
      assert(iter != observers.begin());
      // This really must not throw!
      try {
        do {
          Observers::value_type const &e = *--iter;
          e.first->taskInstanceOperation(TaskInstanceOperation::DEALLOCATE
            , c, *reinterpret_cast<OComponent *>(oComponent + e.second.compOffset)
            , *reinterpret_cast<OTask *>(reinterpret_cast<char *>(taskImpl) + e.second.taskOffset)
            , *taskInstanceImpl
            , *reinterpret_cast<OTaskInstance *>(storage + e.second.taskInstanceOffset));
        } while (iter != observers.begin());
      } catch (...) {
        assert(!"Oops, OTaskInstance destruction throws!");
      }
      taskInstanceImpl->~TaskInstanceImpl();
      free(storage);
      throw;
    }
  }

  ObservableComponent::~ObservableComponent() {
    AbstractComponent &c        = *static_cast<AbstractComponent *>(this);

    // FIXME: Free storage remaining task instances!

    for (TaskImpl *taskImpl : tasks) {
      char *storage = reinterpret_cast<char *>(taskImpl);

      try {
        for (Observers::reverse_iterator iter = observers.rbegin();
             iter != observers.rend();
             ++iter) {
          Observers::value_type const &e = *iter;
          e.first->taskOperation(TaskOperation::DEALLOCATE
            , c, *reinterpret_cast<OComponent *>(oComponent + e.second.compOffset)
            , *taskImpl
            , *reinterpret_cast<OTask *>(storage + e.second.taskOffset));
        }
      } catch (...) {
        assert(!"Oops, OTask destruction throws!");
      }

      taskImpl->~TaskImpl();
      free(storage);
    }
    if (oComponent) {
    // This really must not throw!
      try {
        for (Observers::reverse_iterator iter = observers.rbegin();
             iter != observers.rend();
             ++iter) {
          Observers::value_type const &e = *iter;
          e.first->componentOperation(ComponentOperation::DEALLOCATE
            , *static_cast<AbstractComponent *>(this)
            , *reinterpret_cast<OComponent *>(oComponent + e.second.compOffset));
        }
      } catch (...) {
        assert(!"Oops, OComponent destruction throws!");
      }
      free(oComponent);
    }
  }

} } // namespace SystemC_VPC::Detail
