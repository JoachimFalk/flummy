// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2020 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#include "LegacyComponentObserver.hpp"

namespace SystemC_VPC { namespace Detail {

  LegacyComponentObserver::LegacyComponentObserver()
    : ComponentObserver(0,0) {}

  void LegacyComponentObserver::componentOperation(ComponentOperation co
    , Component       const &c)
    { this->notify(const_cast<Component *>(&c)); }

  void LegacyComponentObserver::taskOperation(TaskOperation to
    , Component       const &c
    , Extending::Task const &t
    , OTask                 &ot)
    { this->notify(const_cast<Component *>(&c)); }

  void LegacyComponentObserver::taskInstanceOperation(TaskInstanceOperation tio
    , Component               const &c
    , Extending::TaskInstance const &ti
    , OTask                         &ot
    , OTaskInstance                 &oti)
    { this->notify(const_cast<Component *>(&c)); }

} } // namespace SystemC_VPC::Detail
