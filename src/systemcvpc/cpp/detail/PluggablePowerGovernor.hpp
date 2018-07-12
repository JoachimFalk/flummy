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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_PLUGGABLEPOWERGOVERNOR_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_PLUGGABLEPOWERGOVERNOR_HPP

#include "PowerGovernor.hpp"
#include "PowerMode.hpp"
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
    const PowerMode      *powerMode;
    PowerModeParameter(PowerMode *mode);
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

    virtual void notify_top(ComponentInfo *ci,
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

    virtual void notify(ComponentInfo *ci) = 0;

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

    virtual void processAttributes(AttributePtr attPtr) = 0;

    virtual PlugIn * createPlugIn() = 0;
  };

  /**
   * factory for plug-in creation
   */
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
    virtual void processAttributes(Attribute att){
      //std::cerr << "Concrete__PlugInFactory::processAttributes" << std::endl;
    }
  };

} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_PLUGGABLEPOWERGOVERNOR_HPP */
