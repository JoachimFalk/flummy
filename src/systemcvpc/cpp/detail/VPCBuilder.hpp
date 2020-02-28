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
    * \brief Used to create the Attribute-Object recursively
    */
    void nextAttribute(Attribute::Ptr attributePtr, CX::XN::DOMNode *node);
     
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
