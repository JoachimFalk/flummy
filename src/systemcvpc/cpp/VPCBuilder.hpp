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

#ifndef HSCD_VPC_VPCBUILDER_H_
#define HSCD_VPC_VPCBUILDER_H_

/*
#include <xercesc/dom/DOMTreeWalker.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
 */

#include <CoSupport/XML/Xerces/Handler.hpp>
#include <systemcvpc/config/common.hpp>

#include <map>
#include <string>
#include <vector>

#include "AbstractComponent.hpp"

#include <systemcvpc/InvalidArgumentException.hpp>

#include <systemcvpc/config/Component.hpp>
#include <systemcvpc/config/Timing.hpp>
#include <systemcvpc/Attribute.hpp>

#include <systemcvpc/TimingModifier.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
typedef boost::minstd_rand base_generator_type;

//XERCES_CPP_NAMESPACE_USE

namespace SystemC_VPC{
  namespace CX = CoSupport::XML::Xerces;

  class Director;

  /**
   * VPCBuilder sets up VPC framework through a given specification file before
   * simulation start.
   */
  class VPCBuilder {
  public:

    VPCBuilder(Director *director);
    ~VPCBuilder();

    void setDirector(Director *director)
      { this->director = director; }

    bool FALLBACKMODE;

    /**
     * \brief Initializes VPC Framework using a configuration file
     */
    void buildVPC();

  private:
    static const char *B_TRANSPORT;
    static const char *STATIC_ROUTE;
    static const char *STR_VPC_THREADEDCOMPONENTSTRING;
    static const char *STR_VPC_DELAY;
    static const char *STR_VPC_LATENCY;
    static const char *STR_VPC_PRIORITY;
    static const char *STR_VPC_PERIOD;
    static const char *STR_VPC_DEADLINE;

    /*
     * SECTION: init tag values for comparison while initializing
     */
    CX::XStr measurefileStr;
    CX::XStr resultfileStr;
    CX::XStr resourcesStr;
    CX::XStr mappingsStr;
    CX::XStr componentStr;
    CX::XStr mappingStr;
    CX::XStr attributeStr;
    CX::XStr timingStr;
    CX::XStr parameterStr;
    CX::XStr topologyStr;
    CX::XStr hopStr;
    CX::XStr routeStr;
    CX::XStr powerModeStr;
    
    CX::XStr nameAttrStr;
    CX::XStr countAttrStr;
    CX::XStr typeAttrStr;
    CX::XStr dividerAttrStr;
    CX::XStr schedulerAttrStr;
    CX::XStr valueAttrStr;
    CX::XStr targetAttrStr;
    CX::XStr sourceAttrStr;
    CX::XStr delayAttrStr;
    CX::XStr diiAttrStr;
    CX::XStr latencyAttrStr;
    CX::XStr minAttrStr;
    CX::XStr maxAttrStr;
    CX::XStr parameter1AttrStr;
    CX::XStr parameter2AttrStr;
    CX::XStr parameter3AttrStr;
    CX::XStr baseAttrStr;
    CX::XStr fixedAttrStr;
    CX::XStr distributionAttrStr;
    CX::XStr seedAttrStr;
    CX::XStr dataAttrStr;
    CX::XStr scaleAttrStr;
    CX::XStr distributionsStr;
    CX::XStr distributionStr;
    CX::XStr fnameAttrStr;
    CX::XStr destinationAttrStr;
    CX::XStr tracingAttrStr;
    CX::XStr defaultRouteAttrStr;
    
    // The handler for loading a xerces xml document
    CX::Handler handler;

    // walker over parsed configure file
    // used as instance variable to enable code modularization
    CX::XN::DOMTreeWalker* vpcConfigTreeWalker;
    
    /*
     * HELPER STRUCTURES FOR INITIALIZATION
     */
    // map of all created components
    std::map<std::string, AbstractComponent *> knownComps;

    // pointer to Director to be initialized
    Director *director;
    
    /**
     * \brief Initialize a distribution from the configuration file
     */
    void initDistribution();
    
    /**
     * \brief Initialize a component from the configuration file
     * \return pointer to the initialized component
     */
    Config::Component::Ptr initComponent() throw(InvalidArgumentException);
    
    /**
     * \brief Performs initialization of attribute values for a component
     * \param comp specifies the component to set attributes for
     */
    void initCompAttributes(Config::Component::Ptr comp);
    
    /**
     * \brief Initializes mapping between tasks and components
     */
    //void initMappingAPStruct(DOMNode* node);
    void initMappingAPStruct();

    /**
    * \brief Used to create the Attribute-Object recursively
    */
    void nextAttribute(AttributePtr attributePtr, CX::XN::DOMNode *node);
     
    /**
    * \brief Topology parsing related code
    */
    void parseTopology(CX::XN::DOMNode* node);

    /**
    * \brief Parsing helper for <timing>
    */
    Config::Timing parseTiming(CX::XN::DOMNode* node) throw(InvalidArgumentException);

    //variables for the generation of random times
    boost::uniform_real<> distribution;
    boost::shared_ptr<base_generator_type> generator;
    boost::shared_ptr<boost::mt19937> gen;
    boost::shared_ptr<DistributionTimingModifier> parseTimingModifier(CX::XN::DOMNode* node) throw(InvalidArgumentException);
  };
    
}

#endif /*HSCD_VPC_VPCBUILDER_H_*/
