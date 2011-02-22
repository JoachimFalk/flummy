/*
 * Route.cpp
 *
 *  Created on: Feb 18, 2011
 *      Author: streubuehr
 */

#include <systemcvpc/config/ConfigException.hpp>
#include <systemcvpc/config/Route.hpp>

#include <iostream>

namespace SystemC_VPC
{

namespace Config
{

//
Hop::Hop(Component::Ptr component) :
  component_(component), transferTiming_(component->getTransferTiming()),
      priority_()
{
}

//
void Hop::setPriority(size_t priority_)
{
  this->priority_ = priority_;
}

//
void Hop::setTransferTiming(Timing transferTiming_)
{
  this->transferTiming_ = transferTiming_;
}

//
Route::Type Route::parseRouteType(std::string name)
{
  static const std::string B_TRANSPORT = "blocking";
  static const std::string STATIC_ROUTE = "static_route";

  if (name == STATIC_ROUTE) {
    return StaticRoute;
  } else if (name == B_TRANSPORT) {
    return BlockingTransport;
  }

  throw Config::ConfigException("Unknown scheduler \"" + name
      + "\" for component: " + name);
  return StaticRoute;
}

Route::Route()
{

}

bool Route::getTracing() const
{
  return tracing_;
}

void Route::setTracing(bool tracing_)
{
  this->tracing_ = tracing_;
}
//
ComponentId Route::getComponentId() const
{
  std::cerr << " Route::getComponentId() " << this->getSequentialId()
      << std::endl;
  return this->getSequentialId();
}

//
Hop Route::addHop(Component::Ptr component)
{
  Hop hop(component);
  hops_.push_back(hop);
  return hop;
}

} // namespace Config
} // namespace SystemC_VPC
