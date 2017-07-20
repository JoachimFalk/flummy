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

#ifndef _INCLUDED_SYSTEMCVPC_POWERGOVERNOR_HPP
#define _INCLUDED_SYSTEMCVPC_POWERGOVERNOR_HPP

#include "ComponentObserver.hpp"
#include <systemcvpc/Attribute.hpp>

namespace SystemC_VPC{

template <typename T>
class GlobalPowerGovernor
{
  public:
    GlobalPowerGovernor()
    {}

    virtual ~GlobalPowerGovernor()
    {}

    virtual void notify_top(ComponentInfo *ci, T val) = 0;
};

template <class T>
class LocalPowerGovernor : public ComponentObserver
{
  public:
    LocalPowerGovernor() :
      m_tpg(NULL)
    {}

    virtual ~LocalPowerGovernor()
    {}

    virtual void notify(ComponentInfo *ci) = 0;

    void setGlobalGovernor(GlobalPowerGovernor<T> *tpg)
    {
      //std::cerr << "LocalPowerGovernor::setGlobalGovernor" << std::endl;
      this->m_tpg = tpg;
    }

  protected:
    GlobalPowerGovernor<T> *m_tpg;
};


}

#endif /* _INCLUDED_SYSTEMCVPC_POWERGOVERNOR_HPP */
