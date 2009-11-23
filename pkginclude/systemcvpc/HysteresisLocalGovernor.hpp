#ifndef __INCLUDED__HYSTERESISLOCALGOVERNOR_IMPL_H_
#define __INCLUDED__HYSTERESISLOCALGOVERNOR_IMPL_H_

#include <map>
#include <deque>
#include <systemc.h>

#include <CoSupport/SystemC/systemc_time.hpp>

#include <systemcvpc/PowerMode.h>
#include <systemcvpc/hscd_vpc_Director.h>
#include <systemcvpc/PluggablePowerGovernor.hpp>

namespace SystemC_VPC{

  class InternalLoadHysteresisGovernor : public PluggableLocalPowerGovernor,
                                 public sc_module {
  public:
    InternalLoadHysteresisGovernor(const sc_time& windowTime,
                           const sc_time& fastTime,
                           const sc_time& slowTime);

    ~InternalLoadHysteresisGovernor();

    SC_HAS_PROCESS(InternalLoadHysteresisGovernor);

    virtual void notify(ComponentInfo *ci);

  private:
    sc_time                                         m_windowTime;
    sc_time                                         m_fastTime;
    sc_time                                         m_slowTime;
    PowerModeParameter                              m_mode;
    ComponentInfo                                  *m_ci;
    ComponentState                                  m_lastState;
    std::deque<std::pair<ComponentState, sc_time> > m_stateHistory;
    sc_event                                        m_wakeup_ev;

    void process();
  };

  /**
   * creates an InternalLoadHysteresisGovernor
   */
  class InternalLoadHysteresisGovernorFactory
    : public PlugInFactory<PluggableLocalPowerGovernor>{

  public:
    InternalLoadHysteresisGovernorFactory()
      : windowTime( SC_ZERO_TIME ),
        fastTime(   sc_time(12.1, SC_MS) ),
        slowTime(   sc_time( 4.0, SC_MS) )
    {
    }

    PluggableLocalPowerGovernor * createPlugIn(){
      return new InternalLoadHysteresisGovernor(windowTime, fastTime, slowTime);
    }

    virtual void processAttributes(Attribute powerAtt){
      //std::cerr << "InternalLoadHysteresisGovernorFactory::processAttributes" << std::endl;
      if(powerAtt.isType("governor")){
        if(powerAtt.hasParameter("sliding_window")){
          std::string v = powerAtt.getParameter("sliding_window");
          windowTime = CoSupport::SystemC::createSCTime(v.c_str());
        }
        if(powerAtt.hasParameter("upper_threshold")){
          std::string v = powerAtt.getParameter("upper_threshold");
          fastTime = windowTime * atof(v.c_str());
        }
        if(powerAtt.hasParameter("lower_threshold")){
          std::string v = powerAtt.getParameter("lower_threshold");
          slowTime = windowTime * atof(v.c_str());
        }
      }
    }
  private:
    sc_time windowTime;
    sc_time fastTime;
    sc_time slowTime;
  };
}

#endif // __INCLUDED__HYSTERESISLOCALGOVERNOR_IMPL_H_
