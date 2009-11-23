#include <systemcvpc/HysteresisLocalGovernor.hpp>

namespace SystemC_VPC{


  InternalLoadHysteresisGovernor::InternalLoadHysteresisGovernor(const sc_time& windowTime,
                                                 const sc_time& fastTime,
                                                 const sc_time& slowTime) :
    sc_module(sc_module_name("InternalLoadHysteresisGovernor")),
    m_windowTime(windowTime),
    m_fastTime(fastTime),
    m_slowTime(slowTime),
    m_mode(NULL),
    m_ci(NULL),
    m_lastState(ComponentState::IDLE)
  {
    //std::cout << "InternalLoadHysteresisGovernor" << std::endl;
    if(m_windowTime > SC_ZERO_TIME){
      SC_METHOD(process);
      sensitive << m_wakeup_ev;
      dont_initialize();
    }
  }


  InternalLoadHysteresisGovernor::~InternalLoadHysteresisGovernor()
  {}

  void InternalLoadHysteresisGovernor::notify(ComponentInfo *ci)
  {
    //    std::cerr << "InternalLoadHysteresisGovernor::notify() @ " << sc_time_stamp() << std::endl;

    const ComponentState newState = ci->getComponentState();

    if(m_mode.powerMode == NULL) {
      assert(m_ci == NULL);
      m_ci = ci;
      m_mode.powerMode = m_ci->getPowerMode();
      m_tpg->notify_top(m_ci, &m_mode);
      m_wakeup_ev.notify(SC_ZERO_TIME);
    }

    assert(m_ci == ci);

    if(newState != m_lastState) {
      m_stateHistory.push_back(std::pair<ComponentState, sc_time>(m_lastState, sc_time_stamp()));
      m_lastState = newState;
      m_wakeup_ev.notify(SC_ZERO_TIME);
    }
  }

  void InternalLoadHysteresisGovernor::process()
  {
    //    std::cerr << "InternalLoadHysteresisGovernor::process() @ " << sc_time_stamp() << std::endl;

    const PowerMode *SLOW = m_ci->translatePowerMode("SLOW");
    const PowerMode *FAST = m_ci->translatePowerMode("FAST");

    sc_time execTime(SC_ZERO_TIME);
    sc_time startTime(SC_ZERO_TIME);

    // calculate start time for sliding window
    if(sc_time_stamp() > m_windowTime)
      startTime = sc_time_stamp() - m_windowTime;

    // drop old state history
    while(!m_stateHistory.empty() &&
          (m_stateHistory.front().second) < startTime)
      {
        m_stateHistory.pop_front();
      }

    // sum up execution times
    for(std::deque<std::pair<ComponentState, sc_time> >::const_iterator
          iter  = m_stateHistory.begin();
        iter != m_stateHistory.end();
        iter++)
      {
        assert(iter->second >= startTime);
        if(iter->first != ComponentState::IDLE)
          execTime += iter->second - startTime;
        startTime = iter->second;
      }
    if(m_lastState != ComponentState::IDLE)
      execTime += sc_time_stamp() - startTime;

    //    std::cerr << "execTime = " << execTime << std::endl;

    // notify power mode suggestions
    if((m_mode.powerMode == SLOW) && (execTime >= m_fastTime)) {
      m_mode.powerMode = FAST;
      m_tpg->notify_top(m_ci, &m_mode);
    } else if((m_mode.powerMode == FAST) && (execTime <= m_slowTime)) {
      m_mode.powerMode = SLOW;
      m_tpg->notify_top(m_ci, &m_mode);
    }

    // wake up process when reaching load boundary
    if((m_mode.powerMode == SLOW) && (m_lastState != ComponentState::IDLE)) {
      m_wakeup_ev.notify(m_fastTime - execTime);
    } else if((m_mode.powerMode == FAST) && (m_lastState == ComponentState::IDLE)) {
      m_wakeup_ev.notify(execTime - m_slowTime);
    }
  }

}
