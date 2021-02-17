// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
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

#include <systemcvpc/ConfigException.hpp>
#include <systemcvpc/ComponentObserver.hpp>
#include <systemcvpc/Extending/ComponentObserverIf.hpp>

#include <systemc>

#include <fstream>
#include <map>
#include <cassert>
#include <memory> // for std::unique_ptr

namespace SystemC_VPC { namespace Detail { namespace Observers {

  class PowerSumming
    : public Extending::ComponentObserverIf
    , public ComponentObserver
  {
  public:
    PowerSumming(Attributes const &attrs);

    ~PowerSumming();

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
    class PSComponent;
    class RegisterMe;

    static RegisterMe registerMe;

    double energySum;

    std::unique_ptr<std::ostream> resultFile;

    void calculateNewEnergySum(
        Component const &component
      , PSComponent     &psComponent);
  };

  class PowerSumming::PSComponent: public OComponent {
  public:
    PSComponent()
      : powerValue(0, Power::W)
      , powerValueOld(-1, Power::W) {}

    sc_core::sc_time powerTime;
    Power            powerValue;
    Power            powerValueOld;
  };

  class PowerSumming::RegisterMe {
  public:
    RegisterMe() {
      PowerSumming::registerObserver("PowerSumming",
        [](Attributes const &attrs) { return new PowerSumming(attrs); });
    }
  } PowerSumming::registerMe;

  PowerSumming::PowerSumming(Attributes const &attrs)
    : Extending::ComponentObserverIf(
          reinterpret_cast<char *>(static_cast<ComponentObserver              *>(this)) -
          reinterpret_cast<char *>(static_cast<Extending::ComponentObserverIf *>(this))
        , sizeof(PSComponent), 0, 0)
    , ComponentObserver(
          reinterpret_cast<char *>(static_cast<Extending::ComponentObserverIf *>(this)) -
          reinterpret_cast<char *>(static_cast<ComponentObserver              *>(this))
        , "PowerSumming")
    , energySum(0)
  {
    for (Attribute const &attr : attrs) {
      if (attr.getType() == "resultFileName") {
        if (resultFile) {
          throw ConfigException(
              "Duplicate attribute resultFileName in PowerSumming component observer!");
        }
        std::string const &resultFileName = attr.getValue();
        resultFile.reset(new std::ofstream(resultFileName));
        if (!resultFile->good()) {
          std::stringstream msg;

          msg << "PowerSumming component observer failed to open " << resultFileName << " for output: " << strerror(errno);
          throw ConfigException(msg.str());
        }
      } else {
        std::stringstream msg;

        msg << "PowerSumming component observers do not support the "+attr.getType()+" attribute! ";
        msg << "Only the resultFileName attribute is supported.";
        throw ConfigException(msg.str());
      }
    }
    if (!resultFile) {
      throw ConfigException(
          "PowerSumming component observer is missing required resultFileName attribute!");
    }
  }

  PowerSumming::~PowerSumming()
  {
    *resultFile << "Sum: " << energySum << std::endl;
  }
  
  bool PowerSumming::addAttribute(Attribute const &attr) {
    throw ConfigException("The PowerSumming observer does not support attributes!");
  }

  void PowerSumming::componentOperation(ComponentOperation co
    , Component const &c
    , OComponent      &oc)
  {
    PSComponent &psComponent = static_cast<PSComponent &>(oc);

    if (ComponentOperation((int) co & (int) ComponentOperation::MEMOP_MASK) ==
        ComponentOperation::ALLOCATE) {
      new (&psComponent) PSComponent();
    }
    switch (ComponentOperation((int) co & ~ (int) ComponentOperation::MEMOP_MASK)) {
      case ComponentOperation::PWRCHANGE:
        calculateNewEnergySum(c, psComponent);
        break;
      default:
        break;
    }
    if (ComponentOperation((int) co & (int) ComponentOperation::MEMOP_MASK) ==
        ComponentOperation::DEALLOCATE) {
      psComponent.~PSComponent();
    }
  }

  void PowerSumming::taskOperation(TaskOperation to
    , Component const &c
    , OComponent      &oc
    , Task      const &t
    , OTask           &ot)
  {}

  void PowerSumming::taskInstanceOperation(TaskInstanceOperation tio
    , Component    const &c
    , OComponent         &oc
    , OTask              &ot
    , TaskInstance const &ti
    , OTaskInstance      &oti)
  {
    PSComponent &psComponent = static_cast<PSComponent &>(oc);
    calculateNewEnergySum(c, psComponent);
  }

  /*
   * This method calculates the energy by integrating the power throughout
   * the time period between the last power mode change and the current time
   * stamp and increments the energySum by the calculated value.
   */
  void PowerSumming::calculateNewEnergySum(
      Component const &component
    , PSComponent     &psComponent)
  {
    sc_core::sc_time const &now = sc_core::sc_time_stamp();

    if (now > psComponent.powerTime) {
      if (psComponent.powerValue != psComponent.powerValueOld)
        *resultFile << component.getName() << "@" << psComponent.powerTime << " consumes " << psComponent.powerValue << std::endl;
      // Energy calculation
      energySum += psComponent.powerValue.value() * (now - psComponent.powerTime).to_seconds();
      psComponent.powerValueOld = psComponent.powerValue;
    }
    psComponent.powerTime  = now;
    psComponent.powerValue = component.getPowerConsumption();
  }

} } } // namespace SystemC_VPC::Detail::Observers
