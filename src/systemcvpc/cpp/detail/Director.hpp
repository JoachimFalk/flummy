// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_DIRECTOR_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_DIRECTOR_HPP

#include <systemcvpc/vpc_config.h>
#include <systemcvpc/EventPair.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/InvalidArgumentException.hpp>
#include <systemcvpc/Attribute.hpp>

#include "FastLink.hpp"

#include <boost/function.hpp>

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <memory>

#include <stdio.h>

namespace SystemC_VPC { namespace Detail {

  template <class T> class DLLFactory;

  typedef std::vector<std::string> FunctionNames;

  class PowerSumming;
  class Delayer;
  class AbstractRoute;
  class PluggableGlobalPowerGovernor;

  template <class T> class PlugInFactory;

  template<typename KEY, class OBJECT>
  class AssociativePrototypedPool;

  /**
   * \brief Director knows all (Abstract-)Components, all mappings (task -> component).
   *
   * Director reads allocation and binding from file.
   */
  class Director {
  public:
    bool FALLBACKMODE;
    bool defaultRoute;
    bool checkVpcConfig;

    /**
     * \brief Access to singleton Director. 
     */
    static Director &getInstance() {
      if (!singleton.get())
        singleton.reset(new Director());
      return *singleton;
    }

    /**
     * end_of_elaboration call back
     * called from SysteMoC in order to cleanup/delete VPC objects
     */
    static void endOfSystemcSimulation(){
      delete singleton.release();
    }

    ~Director();

    /**
     * \brief resolve mapping
     */
    const Delayer *getComponent(FastLink const *vpcLink) const ;
    
    void setResultFile(std::string vpc_result_file){
      this->vpc_result_file = vpc_result_file;
      remove(vpc_result_file.c_str());
    }
    
    std::string getResultFile(){
      return this->vpc_result_file;
    }

    static ProcessId getProcessId(std::string process_or_source,
        std::string destination = "");

    static bool hasFunctionId(const std::string& function);
    static FunctionId getFunctionId(const std::string& function);
    static FunctionId createFunctionId(const std::string& function);

    // FIXME !!!
    PluggableGlobalPowerGovernor   *topPowerGov;
    DLLFactory<PlugInFactory<PluggableGlobalPowerGovernor> >
                                   *topPowerGovFactory;
    void loadGlobalGovernorPlugin(std::string plugin, AttributePtr attPtr);

    std::string getTaskName(ProcessId id);
    
    static sc_core::sc_time getEnd() {
      return end;
    }

    void beforeVpcFinalize();
    void endOfVpcFinalize();
    bool hasValidConfig() const;

    // time of latest acknowledge simulated task
    static sc_core::sc_time end;

    std::map<ProcessId, std::set<std::string> > debugFunctionNames;
  private:

//  void debugUnknownNames( ) const;

    /**
     * Singleton design pattern
     */
    static std::unique_ptr<Director> singleton;

    /**
     * \brief Reads allocation and binding from file.
     */
    Director();

    // output file to write result to
    std::string vpc_result_file;
    

#ifndef NO_POWER_SUM
    std::ofstream  powerConsStream;
    PowerSumming  *powerSumming;
#endif // NO_POWER_SUM
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_DIRECTOR_HPP */
