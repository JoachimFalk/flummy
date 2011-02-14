/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 *
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#include <systemcvpc/config/Mappings.hpp>

namespace SystemC_VPC
{

namespace Config
{

Mappings::Mappings()
{
}

//
std::map<VpcTask::Ptr, Component::Ptr>& Mappings::getConfiguredMappings()
{
  static std::map<VpcTask::Ptr, Component::Ptr> configuredMappings;
  return configuredMappings;
}

//
std::map<Component::Ptr, AbstractComponent *> & Mappings::getComponents()
{
  static std::map<Component::Ptr, AbstractComponent *> components;
  return components;
}

//
bool Mappings::isMapped(VpcTask::Ptr task, Component::Ptr component)
{
  std::map<VpcTask::Ptr, Component::Ptr>& mappings =
      Mappings::getConfiguredMappings();
  return (mappings.find(task) != mappings.end()) && mappings[task] == component;
}

}

}
