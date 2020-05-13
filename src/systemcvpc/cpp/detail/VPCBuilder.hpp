// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 Tim Grocki <grocki@codesign.informatik.uni-erlangen.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMCVPC_DETAIL_VPCBUILDER_HPP
#define _INCLUDED_SYSTEMCVPC_DETAIL_VPCBUILDER_HPP

#include <systemcvpc/InvalidArgumentException.hpp>
#include <systemcvpc/Component.hpp>
#include <systemcvpc/Timing.hpp>
#include <systemcvpc/Attribute.hpp>
#include <systemcvpc/TimingModifier.hpp>
#include <systemcvpc/Route.hpp>

#include <CoSupport/XML/Xerces/Handler.hpp>

#include <boost/random/mersenne_twister.hpp>    // for boost::mt19937

#include <boost/smart_ptr/shared_ptr.hpp>

#include <map>
#include <string>
#include <vector>

namespace SystemC_VPC { namespace Detail {

  namespace CX = CoSupport::XML::Xerces;

  /**
   * VPCBuilder sets up VPC framework through a given specification file before
   * simulation start.
   */
  class VPCBuilder {
  public:

    VPCBuilder();
    ~VPCBuilder();

    /**
     * \brief Initializes VPC Framework using a configuration file
     */
    bool buildVPC(std::string const &vpcConfigFile);

  private:
    static const char *B_TRANSPORT;
    static const char *STATIC_ROUTE;
    static const char *STR_VPC_THREADEDCOMPONENTSTRING;
    static const char *STR_VPC_DELAY;
    static const char *STR_VPC_LATENCY;
    static const char *STR_VPC_PRIORITY;
    static const char *STR_VPC_PERIOD;
    static const char *STR_VPC_DEADLINE;

    // The handler for loading a xerces xml document
    CX::Handler handler;

    // walker over parsed configure file
    // used as instance variable to enable code modularization
    CX::XN::DOMTreeWalker* vpcConfigTreeWalker;
    
    /**
     * \brief Parse the resources tag.
     */
    void parseResources();

    /**
     * \brief Parse the observer tag configuring a resource observer.
     */
    void parseResourceObserve();

    /**
     * \brief Parse the tracer tag configuring a resource tracer.
     */
    void parseResourceTracer();

    /**
     * \brief Initialize a distribution from the configuration file
     */
    void initDistribution();
    
    /**
     * \brief Initialize a component from the configuration file
     * \return pointer to the initialized component
     */
    SystemC_VPC::Component::Ptr initComponent();
    
    /**
     * \brief Performs initialization of attribute values for a component
     * \param comp specifies the component to set attributes for
     */
    void initCompAttributes(SystemC_VPC::Component::Ptr comp);
    
    /**
     * \brief Initializes mapping between tasks and components
     */
    //void initMappingAPStruct(DOMNode* node);
    void initMappingAPStruct();

    /**
     * \brief Parse attribute tag
     */
    Attribute parseAttribute(CX::XN::DOMNode *node);

    /**
     * \brief Parse nested attribute tags inside a parent tag.
     */
    Attributes parseAttributes(CX::XN::DOMNode *parent);
     
    /**
     * \brief Topology parsing related code
     */
    void parseTopology(CX::XN::DOMNode *node);

    /**
     * \brief Parse a static route in the topology.
     */
    Routing::Static::Ptr parseStaticRoute(CX::XN::DOMNode *routeNode);

    /**
     * \brief Parse a hop in a static route.
     */
    void parseStaticHop(
        Routing::Static      *route,
        Routing::Static::Hop *parentHop,
        CX::XN::DOMNode      *hopNode);

    /**
    * \brief Parsing helper for <timing>
    */
    SystemC_VPC::Timing parseTiming(CX::XN::DOMNode* node);

    //variables for the generation of random times
    boost::shared_ptr<boost::mt19937> gen;
    boost::shared_ptr<DistributionTimingModifier> parseTimingModifier(CX::XN::DOMNode* node);
  };
    
} } // namespace SystemC_VPC::Detail

#endif /* _INCLUDED_SYSTEMCVPC_DETAIL_VPCBUILDER_HPP */
