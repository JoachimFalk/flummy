// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2011 Thomas Russ <tr.thomas.russ@googlemail.com>
 *   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 Tim Grocki <grocki@codesign.informatik.uni-erlangen.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_VPCAPI_HPP
#define _INCLUDED_SYSTEMCVPC_VPCAPI_HPP

#include "Timing.hpp"
#include "VpcTask.hpp"
#include "Component.hpp"
#include "Route.hpp"
#include "ConfigException.hpp"
#include "Routing/Ignore.hpp"
#include "Routing/Static.hpp"
#include "ExecModel.hpp"
#include "ExecModelling/LookupPowerTimeModel.hpp"
#include "ComponentObserver.hpp"
#include "ComponentTracer.hpp"

namespace SystemC_VPC {

bool hasDistribution(std::string const &name);

TimingModifier::Ptr getDistribution(std::string const &name);

void createDistribution(std::string const &name, boost::shared_ptr<DistributionTimingModifier> modifier);

std::string   getResultFileName();
std::ostream &getResultFile();

void setResultFile(std::string const &resultFileName);
void setResultFile(
    std::ostream      &resultFile
  , std::string const &resultFileName = "???");

} // namespace SystemC_VPC

#endif /* _INCLUDED_SYSTEMCVPC_VPCAPI_HPP */
