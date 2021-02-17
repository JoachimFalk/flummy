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

    class DMTask: public OTask {
    public:
      DMTask() {}

    };

    typedef std::map<std::string, DMTask *> Tasks;
    Tasks                                   tasks;

    class DeadlineInfo {
    public:
      DeadlineInfo(std::string const &startActorName)
        : startActor(startActorName) {}
      DeadlineInfo(boost::regex const &startActorRegex)
        : startActor(startActorRegex) {}


      bool matchStartActor(std::string const &actorName) const {
        switch (startActor.which()) {
          case 0:
            return regex_match(actorName,
                boost::get<boost::regex const &>(startActor));
          case 1:
            return actorName == boost::get<std::string const &>(startActor);
        }
        return false;
      }

      struct EndActor {
        EndActor(std::string const &endActor, sc_core::sc_time const &d)
          : endActor(endActor), deadline(d) {}

        std::string      endActor;
        sc_core::sc_time deadline;
      };

      std::list<EndActor> endActors;

    protected:
      boost::variant<boost::regex, std::string> startActor;
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
        if (attr.getType() == "startActor")
          deadlineInfos.push_back(DeadlineInfo(attr.getValue()));
        else
          deadlineInfos.push_back(DeadlineInfo(boost::regex(attr.getValue())));
        DeadlineInfo &di = deadlineInfos.back();

        for (Attribute const &attr : attr.getAttributes()) {
          if (attr.getType() == "endActor") {
            di.endActors.push_back(DeadlineInfo::EndActor(attr.getValue(), sc_core::SC_ZERO_TIME));

          } else {
            std::stringstream msg;

            msg << "DeadlineMonitor component observers do not support the " << attr.getType();
            msg << " attribute inside a startActor or startActorRegex attribute!";
            msg << "Only the endActor attribute is supported.";
            throw ConfigException(msg.str());
          }
        }
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
      case TaskOperation::ALLOCATE:
        new (&dmTask) DMTask();
        sassert(tasks.insert(Tasks::value_type(t.getName(), &dmTask)).second);

        for (DeadlineInfo di : deadlineInfos) {
          if (di.matchStartActor(t.getName())) {
            *resultFile << "start actor " << t.getName() << std::endl;
            for (DeadlineInfo::EndActor ea : di.endActors) {
              *resultFile << "end actor " << ea.endActor << ": " << ea.deadline << std::endl;
            }
          }
        }
        break;
      case TaskOperation::DEALLOCATE:
        dmTask.~DMTask();
        break;
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
      case TaskInstanceOperation::FINISHLAT:
        *resultFile << ti.getName() << std::endl;
        break;
      default:
        break;
    }
  }

} } } // namespace SystemC_VPC::Detail::Observers
