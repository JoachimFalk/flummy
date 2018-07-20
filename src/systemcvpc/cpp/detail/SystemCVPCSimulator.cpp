// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of
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

#include "config.h"

#include <smoc/SimulatorAPI/SchedulerInterface.hpp>
#include <smoc/SimulatorAPI/TaskInterface.hpp>
#include <smoc/SimulatorAPI/PortInterfaces.hpp>
#include <smoc/SimulatorAPI/SimulatorInterface.hpp>

#include <systemcvpc/VpcApi.hpp>
#include <systemcvpc/Routing/Ignore.hpp>

#include "Director.hpp"
#include "Configuration.hpp"
#include "VPCBuilder.hpp"
#include "AbstractComponent.hpp"
#include "AbstractRoute.hpp"
#include "Routing/IgnoreImpl.hpp"

#include "DebugOStream.hpp"

#include <systemc>

namespace SystemC_VPC { namespace Detail {

using namespace smoc::SimulatorAPI;

namespace po = boost::program_options;
namespace VC = SystemC_VPC;

class SystemCVPCSimulator
  : public SimulatorInterface
  , public sc_core::sc_module
{
public:
  SystemCVPCSimulator(sc_core::sc_module_name name);

  ///
  /// Handle SimulatorInterface
  ///

  void populateOptionsDescription(
      int &argc, char ** &argv,
      boost::program_options::options_description &pub,
      boost::program_options::options_description &priv);

  EnablementStatus evaluateOptionsMap(
      boost::program_options::variables_map &vm);

  void registerTask(
      TaskInterface                          *actor,
      std::list<FiringRuleInterface *> const &firingRules);

  void registerPort(PortInInterface *port);
  void registerPort(PortOutInterface *port);

  ///
  /// Use sc_core::sc_module to trigger consistency checks
  ///
  void end_of_elaboration();

  void start_of_simulation();

  void end_of_simulation();
};

SystemCVPCSimulator::SystemCVPCSimulator(sc_core::sc_module_name name)
  : sc_core::sc_module(name) {}

void SystemCVPCSimulator::populateOptionsDescription(
    int &argc, char ** &argv,
    boost::program_options::options_description &pub,
    boost::program_options::options_description &priv)
{
  // Provide --systemoc-vpc-debug option
  {
    std::stringstream sstr;
    sstr << "set debug level; level 0 is off; level " << Debug::None.level << " is most verbose";
#ifdef SYSTEMCVPC_ENABLE_DEBUG
    pub.add_options()
#else //!defined(SYSTEMCVPC_ENABLE_DEBUG)
    priv.add_options()
#endif //!defined(SYSTEMCVPC_ENABLE_DEBUG)
      ("systemoc-vpc-debug",
       po::value<size_t>()->default_value(0),
       sstr.str().c_str())
      ;
  }
  // Provide --systemoc-vpc-config option and its backward compatibility version
  {
    pub.add_options()
      ("systemoc-vpc-config",
       getenv("VPCCONFIGURATION")
         ? po::value<std::string>()->default_value(getenv("VPCCONFIGURATION"))
         : po::value<std::string>(),
       "use specified SystemC-VPC configuration file")
      ;
    // Backward compatibility cruft
    priv.add_options()
      ("vpc-config",
       po::value<std::string>())
      ;
  }
}

SystemCVPCSimulator::EnablementStatus SystemCVPCSimulator::evaluateOptionsMap(
    boost::program_options::variables_map &vm)
{
  EnablementStatus retval;

  std::string vpcConfigFile;
  if (vm.count("systemoc-vpc-config")) {
    vpcConfigFile = vm["systemoc-vpc-config"].as<std::string>();
    retval = MUSTBE_ACTIVE;
  } else if (vm.count("vpc-config")) {
    vpcConfigFile = vm["systemoc-vpc-config"].as<std::string>();
    retval = MUSTBE_ACTIVE;
  } else {
    retval = IS_DISABLED;
  }

#ifdef SYSTEMCVPC_ENABLE_DEBUG
  int debugLevel = Debug::None.level - vm["systemoc-vpc-debug"].as<size_t>();
  getDbgOut().setLevel(debugLevel < 0 ? 0 : debugLevel);
  getDbgOut() << Debug::High;
#else  //!defined(SYSTEMCVPC_ENABLE_DEBUG)
  if (vm["systemoc-vpc-debug"].as<size_t>() != 0)
    std::cerr << "libsystemc-vpc: Warning debug support not compiled in and, thus, --systemoc-vpc-debug option ignored!" << std::endl;
#endif //!defined(SYSTEMCVPC_ENABLE_DEBUG)

  if (retval != IS_DISABLED) {
#ifdef _MSC_VER
    std::string envVar("VPCCONFIGURATION=" + vpcConfigFile);
    putenv((char *) envVar.c_str());
#else
    setenv("VPCCONFIGURATION", vpcConfigFile.c_str(), 1);
#endif // _MSC_VER

    try {
      VPCBuilder builder(&Director::getInstance());
      builder.buildVPC();
    } catch (InvalidArgumentException &e) {
      std::cerr << "VPCBuilder> Got exception while setting up VPC:\n"
                << e.what() << std::endl;
      exit(-1);
    } catch (const std::exception &e) {
      std::cerr << "VPCBuilder> Got exception while setting up VPC:\n"
                << e.what() << std::endl;
      exit(-1);
    }

    Director::getInstance().beforeVpcFinalize();
    if (Director::getInstance().FALLBACKMODE) {
      if (getDbgOut().isVisible(Debug::High))
        getDbgOut() << "SystemC_VPC has invalid configuration " << getenv("VPCCONFIGURATION") << " => VPC still off" << std::endl;
      retval = IS_DISABLED;
    } else {
      if (getDbgOut().isVisible(Debug::High))
        getDbgOut() << "SystemC_VPC has valid configuration " << getenv("VPCCONFIGURATION") << " => turning VPC on" << std::endl;
    }
    unsetenv("VPCCONFIGURATION");
  }

  return retval;
}

void SystemCVPCSimulator::registerTask(
    TaskInterface                          *actor,
    std::list<FiringRuleInterface *> const &firingRules) {
  Configuration::getInstance().registerTask(actor, firingRules);
}

void SystemCVPCSimulator::registerPort(PortInInterface *port) {
  Configuration::getInstance().registerRoute(port);
}
void SystemCVPCSimulator::registerPort(PortOutInterface *port) {
  Configuration::getInstance().registerRoute(port);
}

void SystemCVPCSimulator::end_of_elaboration() {
  Configuration::getInstance().finalize();
}

void SystemCVPCSimulator::start_of_simulation() {

}

void SystemCVPCSimulator::end_of_simulation() {
  Director::endOfSystemcSimulation();
}

} } // namespace SystemC_VPC::Detail

// This must be outside of the namespace for SysteMoC plugin load pickup!
// Prefix all SysteMoC internal modules with __smoc_ to enable filtering out the module on smx dump!
SystemC_VPC::Detail::SystemCVPCSimulator systemCVPCSimulator("__smoc_systemcvpcsimulator");
