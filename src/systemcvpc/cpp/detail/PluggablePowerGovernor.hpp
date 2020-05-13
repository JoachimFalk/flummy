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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PLUGGABLEPOWERGOVERNOR_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PLUGGABLEPOWERGOVERNOR_HPP

#include <systemcvpc/PowerMode.hpp>

#include "PowerGovernor.hpp"
#include "dynload/dll.hpp"

namespace SystemC_VPC { namespace Detail {

  /**
   * base class for custom parameters
   */
  class GenericParameter{
  public:
    virtual ~GenericParameter();
  };

  /**
   * simple parameter containing only a PowerMode
   */
  class PowerModeParameter : public GenericParameter {
  public:
    PowerModeParameter(PowerMode const &mode);

    PowerMode powerMode;
    virtual ~PowerModeParameter();
  };

  /**
   * plug-in interface for GlobalPowerGovernor
   */
  class PluggableGlobalPowerGovernor :
    public GlobalPowerGovernor<GenericParameter *> {
  public:
    PluggableGlobalPowerGovernor()
    {}

    virtual ~PluggableGlobalPowerGovernor()
    {}

    virtual void notify_top(Component *ci,
                            GenericParameter *param) = 0;
  };

  /**
   * plug-in interface for LocalPowerGovernor
   */
  class PluggableLocalPowerGovernor :
    public LocalPowerGovernor<GenericParameter *> 
  {
  public:
    PluggableLocalPowerGovernor() :
      LocalPowerGovernor<GenericParameter *> ()
    {}

    virtual ~PluggableLocalPowerGovernor()
    {}

    virtual void notify(Component *ci) = 0;

  };

  /**
   * factory for plug-in creation
   */
  template<class PlugIn>
  class PlugInFactory {
  public:
    PlugInFactory() {
      //std::cout << "PlugInFactory Created" << std::endl;
    }

    virtual ~PlugInFactory() {
      //std::cout << "PlugInFactory Destroy" << std::endl;
    }

    virtual void processAttributes(Attribute const &attr) = 0;

    virtual PlugIn * createPlugIn() = 0;
  };

  /**
   * factory for plug-in creation
  template<class ConcretePlugIn, class PlugIn>
  class ConcretePlugInFactory : public PlugInFactory<PlugIn> {
  public:
    ConcretePlugInFactory() {
      //std::cout << "Concrete__PlugInFactory Created" << std::endl;
    }

    virtual ~ConcretePlugInFactory() {
      //std::cout << "Concrete__PlugInFactory Destroy" << std::endl;
    }

    virtual PlugIn * createPlugIn(){
      return new ConcretePlugIn();
    }
    virtual void processAttributes(Attribute const &attr){
      //std::cerr << "Concrete__PlugInFactory::processAttributes" << std::endl;
    }
  };
   */

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PLUGGABLEPOWERGOVERNOR_HPP */
