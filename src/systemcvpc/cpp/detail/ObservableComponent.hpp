// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_OBSERVABLECOMPONENT_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_OBSERVABLECOMPONENT_HPP

#include <systemcvpc/Extending/ComponentObserverIf.hpp>
#include <systemcvpc/Component.hpp>

#include "TaskImpl.hpp"
#include "TaskInstanceImpl.hpp"

#include <map>
#include <vector>

namespace SystemC_VPC { namespace Detail {

  class AbstractComponent;

  /**
   * \brief Interface for classes implementing delay simulation.
   */
  class ObservableComponent {
  public:
    void addObserver(Extending::ComponentObserverIf::Ptr const &obs);

    TaskImpl *createTask(TaskInterface     *taskInterface);
    TaskImpl *createTask(std::string const &taskName);

    TaskInstanceImpl *createTaskInstance(
        TaskImpl                                       *taskImpl
      , TaskInstanceImpl::Type                          type
      , PossibleAction                                 *firingRuleInterface
      , std::function<void (TaskInstanceImpl *)> const &diiCallback
      , std::function<void (TaskInstanceImpl *)> const &latCallback);
  protected:
    typedef std::vector<TaskImpl *> Tasks;

    ObservableComponent();

    typedef Extending::ComponentObserverIf::ComponentOperation
        ComponentOperation;
    typedef Extending::ComponentObserverIf::TaskOperation
        TaskOperation;
    typedef Extending::ComponentObserverIf::TaskInstanceOperation
        TaskInstanceOperation;

    void componentOperation(ComponentOperation co
      , AbstractComponent const &c);

    void taskOperation(TaskOperation to
      , AbstractComponent const &c
      , TaskImpl                &t);

    void taskInstanceOperation(TaskInstanceOperation tio
      , AbstractComponent const &c
      , TaskInstanceImpl        &ti);

    Tasks const &getTasks()
      { return tasks; }

    void finalize();

    ~ObservableComponent();
  private:
    TaskImpl *createTask(std::function<void (char *)> factory);

    bool isObserverRegistrationAllowed() const
      { return !oComponent; }

    typedef Extending::ComponentObserverIf::OComponent
        OComponent;
    typedef Extending::ComponentObserverIf::OTask
        OTask;
    typedef Extending::ComponentObserverIf::OTaskInstance
        OTaskInstance;

    struct ObserverInfo {
      int compOffset;
      int taskOffset;
      int taskInstanceOffset;
    };
    typedef std::map<Extending::ComponentObserverIf::Ptr, ObserverInfo> Observers;
    
    size_t nextFreeCompOffset;
    size_t nextFreeTaskOffset;
    size_t nextFreeTaskInstanceOffset;

    Observers observers;
    Tasks     tasks;
    /// Component private data for the observers
    char     *oComponent;
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_OBSERVABLECOMPONENT_HPP */
