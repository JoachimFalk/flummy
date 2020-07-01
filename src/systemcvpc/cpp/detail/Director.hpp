// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_DIRECTOR_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_DIRECTOR_HPP

#include <systemcvpc/vpc_config.h>
#include <systemcvpc/EventPair.hpp>
#include <systemcvpc/ScheduledTask.hpp>
#include <systemcvpc/InvalidArgumentException.hpp>
#include <systemcvpc/Attribute.hpp>
#include <systemcvpc/datatypes.hpp>

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

    static ProcessId getProcessId(std::string process_or_source,
        std::string destination = "");

    // FIXME !!!
    PluggableGlobalPowerGovernor   *topPowerGov;
    DLLFactory<PlugInFactory<PluggableGlobalPowerGovernor> >
                                   *topPowerGovFactory;
    void loadGlobalGovernorPlugin(std::string plugin, Attribute const &attr);

    std::string getTaskName(ProcessId id);
    
    void beforeVpcFinalize();
    void endOfVpcFinalize();

    std::map<ProcessId, std::set<std::string> > debugFunctionNames;
  private:
    /**
     * Singleton design pattern
     */
    static std::unique_ptr<Director> singleton;

    /**
     * \brief Reads allocation and binding from file.
     */
    Director();
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_DIRECTOR_HPP */
