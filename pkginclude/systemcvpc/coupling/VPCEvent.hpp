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

#ifndef LOSABLEEVENT_HPP_
#define LOSABLEEVENT_HPP_

//#include <CoSupport/SystemC>

namespace SystemC_VPC
{

namespace Coupling
{

  /*
   * Event used for messages in order to invalidate them in the Component
   */
  class VPCEvent
  : public CoSupport::SystemC::RefCountEvent {
  public:
    typedef boost::intrusive_ptr<VPCEvent> Ptr;
    typedef VPCEvent this_type;
    //if packet is dropped (e.g. packetloss) dropped is set true
    bool dropped;
  public:
    VPCEvent(bool startNotified = false)
      : CoSupport::SystemC::RefCountEvent(startNotified), dropped(false) {}

    VPCEvent(const Event& e)
        : CoSupport::SystemC::RefCountEvent(e), dropped(false) {}

    CoSupport::SystemC::EventWaiter *reset(CoSupport::SystemC::EventListener *el = NULL) {
      dropped = false;
      return CoSupport::SystemC::RefCountEvent::reset(el);
    }

    void setDropped(bool drop){
      dropped = drop;
    }

    bool getDropped(void){
      return dropped;
    }

    bool isDropped(void){
      return getDropped();
    }
  };

}
}
  #endif /* LOSABLEEVENT_HPP_ */
