// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/ComponentObserver.hpp>
#include <systemcvpc/Extending/ComponentObserverIf.hpp>

#include "../common.hpp"

#include <CoSupport/String/quoting.hpp>

#include <systemc>

#include <boost/variant.hpp>
#include <boost/regex.hpp>

#include <string>
#include <fstream>
#include <map>
#include <cassert>
#include <memory> // for std::unique_ptr

namespace SystemC_VPC { namespace Detail { namespace Observers {

  class DeadlineMonitor
    : public Extending::ComponentObserverIf
    , public ComponentObserver
  {
  public:
    DeadlineMonitor(Attributes const &attrs);

    ~DeadlineMonitor();

    bool addAttribute(Attribute const &attr);

    void componentOperation(ComponentOperation co
      , Component const &c
      , OComponent      &oc);

    void taskOperation(TaskOperation to
      , Component const &c
      , OComponent      &oc
      , Task      const &t
      , OTask           &ot);

    void taskInstanceOperation(TaskInstanceOperation tio
      , Component    const &c
      , OComponent         &oc
      , OTask              &ot
      , TaskInstance const &ti
      , OTaskInstance      &oti);

  private:
    class RegisterMe;

    static RegisterMe registerMe;

    std::unique_ptr<std::ostream> resultFile;

    class DMTask;

    typedef std::map<std::string, DMTask *> Tasks;
    Tasks                                   tasks;

    class EndActor {
    public:
      EndActor(std::string const &name, sc_core::sc_time const &d)
        : name(name), actor(nullptr), deadline(d) {}

      std::string const      &getName() const
        { return name; }
      DMTask                 *getActor() const
        { return actor; }
      sc_core::sc_time const &getDeadline() const
        { return deadline; }

      DMTask **resolveActor(Tasks const &tasks) {
        Tasks::const_iterator iter = tasks.find(name);
        if (iter != tasks.end()) {
          actor = iter->second;
          return nullptr;
        } else
          return &actor;
      }
    protected:
      std::string       name;
      DMTask           *actor;
      sc_core::sc_time  deadline;
    };

    typedef std::list<EndActor>                 EndActors;
    typedef std::map<std::string, DMTask **>   PendingEndActors;

    // List of EndActors missing their actor attribute
    PendingEndActors pendingEndActors;

    class DMTask: public OTask {
    public:
      DMTask() {}

      /// If this is a start actor, this will point to
      /// all the end actors.
      EndActors endActors;
      /// If this is an end actor this will store all
      /// active (absolute) deadlines.
      std::list<sc_core::sc_time> activeDeadlines;
    };

    class DeadlineInfo {
    public:

      DeadlineInfo(std::string const &startActorName, EndActors &&endActors)
        : startActor(startActorName), endActors(endActors) {}
      DeadlineInfo(boost::regex const &startActorRegex, EndActors &&endActors)
        : startActor(startActorRegex), endActors(endActors) {}

      bool matchStartActor(std::string const &actorName, EndActors &endActors) const {
        endActors.clear();

        switch (startActor.which()) {
          case 0: {
            boost::smatch m;
            if (!regex_match(actorName, m, boost::get<boost::regex const &>(startActor)))
              return false;
            std::map<std::string, std::string> vars;
            int groupNr = 0;
            for (boost::smatch::value_type const &group : m)
              vars.insert(std::make_pair(std::to_string(groupNr++), group.str()));
            for (EndActor const &ea : this->endActors) {
              std::string endActor = CoSupport::String::dequote(
                  CoSupport::String::QuoteMode::DOUBLE_NO_QUOTES
                , ea.getName().c_str()
                , vars);
              endActors.push_back(EndActor(endActor, ea.getDeadline()));
            }
            return true;
          }
          case 1: {
            if (actorName != boost::get<std::string const &>(startActor))
              return false;
            endActors = this->endActors;
            return true;
          }
        }

        switch (startActor.which()) {
          case 0:
            return regex_match(actorName,
                boost::get<boost::regex const &>(startActor));
          case 1:
            return actorName == boost::get<std::string const &>(startActor);
        }
        return false;
      }
    protected:
      boost::variant<boost::regex, std::string> startActor;
      EndActors                                 endActors;
    };

    typedef std::list<DeadlineInfo> DeadlineInfos;
    DeadlineInfos                   deadlineInfos;

  };

  class DeadlineMonitor::RegisterMe {
  public:
    RegisterMe() {
      DeadlineMonitor::registerObserver("DeadlineMonitor",
        [](Attributes const &attrs) { return new DeadlineMonitor(attrs); });
    }
  } DeadlineMonitor::registerMe;

  DeadlineMonitor::DeadlineMonitor(Attributes const &attrs)
    : Extending::ComponentObserverIf(
          reinterpret_cast<char *>(static_cast<ComponentObserver              *>(this)) -
          reinterpret_cast<char *>(static_cast<Extending::ComponentObserverIf *>(this))
        , 0, sizeof(DMTask), 0)
    , ComponentObserver(
          reinterpret_cast<char *>(static_cast<Extending::ComponentObserverIf *>(this)) -
          reinterpret_cast<char *>(static_cast<ComponentObserver              *>(this))
        , "DeadlineMonitor")
  {
    for (Attribute const &attr : attrs) {
      if (attr.getType() == "resultFileName") {
        if (resultFile) {
          throw ConfigException(
              "Duplicate attribute resultFileName in DeadlineMonitor component observer!");
        }
        std::string const &resultFileName = attr.getValue();
        resultFile.reset(new std::ofstream(resultFileName));
        if (!resultFile->good()) {
          std::stringstream msg;

          msg << "DeadlineMonitor component observer failed to open " << resultFileName << " for output: " << strerror(errno);
          throw ConfigException(msg.str());
        }
      } else if (attr.getType() == "startActor" || attr.getType() == "startActorRegex") {
        EndActors endActors;
        for (Attribute const &attr : attr.getAttributes()) {
          if (attr.getType() == "endActor") {
            bool             deadlineSpecified = false;
            sc_core::sc_time deadline;
            for (Attribute const &attr : attr.getAttributes()) {
              if (!deadlineSpecified && attr.getType() == "deadline") {
                deadlineSpecified = true;
                deadline = Detail::createSC_Time(attr.getValue().c_str());
              } else {
                std::stringstream msg;

                msg << "DeadlineMonitor component observers do not support the " << attr.getType();
                msg << " attribute inside an endActor attribute!";
                msg << " Only a single deadline attribute is supported.";
                throw ConfigException(msg.str());
              }
            }
            if (!deadlineSpecified)
              throw ConfigException("DeadlineMonitor component observers require exactly one "
                  "deadline attribute inside the endActor attribute!");
            endActors.push_back(EndActor(attr.getValue(), deadline));
          } else {
            std::stringstream msg;

            msg << "DeadlineMonitor component observers do not support the " << attr.getType();
            msg << " attribute inside a startActor or startActorRegex attribute!";
            msg << " Only the endActor attribute is supported.";
            throw ConfigException(msg.str());
          }
        }
        if (attr.getType() == "startActor")
          deadlineInfos.push_back(DeadlineInfo(attr.getValue(), std::move(endActors)));
        else
          deadlineInfos.push_back(DeadlineInfo(boost::regex(attr.getValue()), std::move(endActors)));
      } else {
        std::stringstream msg;

        msg << "DeadlineMonitor component observers do not support the "+attr.getType()+" attribute! ";
        msg << "Only the resultFileName, startActor, or startActorRegex attributes are supported.";
        throw ConfigException(msg.str());
      }
    }
    if (!resultFile) {
      throw ConfigException(
          "DeadlineMonitor component observer is missing required resultFileName attribute!");
    }
  }

  DeadlineMonitor::~DeadlineMonitor()
  {
//  *resultFile << "Sum: " << energySum << std::endl;
  }
  
  bool DeadlineMonitor::addAttribute(Attribute const &attr) {
    throw ConfigException("The DeadlineMonitor observer does not support attributes!");
  }

  void DeadlineMonitor::componentOperation(ComponentOperation co
    , Component const &c
    , OComponent      &oc)
  {
  }

  void DeadlineMonitor::taskOperation(TaskOperation to
    , Component const &c
    , OComponent      &oc
    , Task      const &t
    , OTask           &ot)
  {
    DMTask &dmTask = static_cast<DMTask &>(ot);

    switch (TaskOperation((int) to & (int) TaskOperation::MEMOP_MASK)) {
      case TaskOperation::ALLOCATE: {
        new (&dmTask) DMTask();
        sassert(tasks.insert(Tasks::value_type(t.getName(), &dmTask)).second);
        // Resolve pending end actors
        {
          PendingEndActors::iterator iter = pendingEndActors.find(t.getName());
          if (iter != pendingEndActors.end()) {
            *iter->second = &dmTask;
            pendingEndActors.erase(iter);
          }
        }
        for (DeadlineInfo di : deadlineInfos) {
          if (di.matchStartActor(t.getName(), dmTask.endActors)) {
            *resultFile << "start actor " << t.getName() << std::endl;
            for (EndActor &ea : dmTask.endActors) {
              *resultFile << "end actor " << ea.getName() << ": " << ea.getDeadline() << std::endl;
              if (DMTask **pending = ea.resolveActor(tasks))
                sassert(pendingEndActors.insert(std::make_pair(ea.getName(), pending)).second);
            }
          }
        }
        break;
      }
      case TaskOperation::DEALLOCATE: {
        dmTask.~DMTask();
        break;
      }
    }
  }

  void DeadlineMonitor::taskInstanceOperation(TaskInstanceOperation tio
    , Component    const &c
    , OComponent         &oc
    , OTask              &ot
    , TaskInstance const &ti
    , OTaskInstance      &oti)
  {
    DMTask &dmTask = static_cast<DMTask &>(ot);

    switch (TaskInstanceOperation((int) tio & ~ (int) TaskInstanceOperation::MEMOP_MASK)) {
      case TaskInstanceOperation::RELEASE:
        for (EndActor const &ea : dmTask.endActors) {
          ea.getActor()->activeDeadlines.push_back(
              sc_core::sc_time_stamp() + ea.getDeadline());
        }
        break;
      case TaskInstanceOperation::FINISHLAT:
        if (!dmTask.activeDeadlines.empty()) {
          sc_core::sc_time const &now = sc_core::sc_time_stamp();
          sc_core::sc_time const &absDeadline = dmTask.activeDeadlines.front();
          *resultFile << ti.getName() << "@" << now << " deadline: " << absDeadline << std::endl;
          if (absDeadline < now) {
            *resultFile << ti.getName() << " had " << (now-absDeadline) << " deadline violation!" << std::endl;
          } else {
            *resultFile << ti.getName() << " had " << (absDeadline-now) << " slack!" << std::endl;
          }
          dmTask.activeDeadlines.pop_front();
        }
        break;
      default:
        break;
    }
  }

} } } // namespace SystemC_VPC::Detail::Observers
