#ifndef POWERGOVERNOR_IMPL_H_
#define POWERGOVERNOR_IMPL_H_

#include <map>
#include <deque>
#include <systemc.h>

#include "PowerMode.h"
#include "PowerGovernor.h"

namespace SystemC_VPC{

class SelectFastestPowerModeGovernor : public GlobalPowerGovernor<const PowerMode*>
{
  public:
    SelectFastestPowerModeGovernor() :
      m_lastMode(NULL)
    {}

    void notify_top(ComponentInfo *ci, const PowerMode *newMode)
    {
      if(m_lastMode == NULL)
        m_lastMode = newMode;

      if(m_components.find(ci) == m_components.end()) {
        std::cerr << "@" << sc_time_stamp() << ": setPowerMode(" << newMode->getName() << ");" << std::endl;
        ci->getModel()->setPowerMode(m_lastMode);
      }

      m_components[ci] = newMode;

      if(*newMode < *m_lastMode) {
        for(std::map<ComponentInfo*, const PowerMode*>::iterator
          iter  = m_components.begin();
          iter != m_components.end();
          iter++)
        {
          if(*iter->second > *newMode)
            newMode = iter->second;
        }
      }

      if(*newMode != *m_lastMode) {
        std::cerr << "@" << sc_time_stamp() << ": for all components setPowerMode(" << newMode->getName() << ");" << std::endl;

        for(std::map<ComponentInfo*, const PowerMode*>::iterator
          iter  = m_components.begin();
          iter != m_components.end();
          iter++)
        {
          iter->first->getModel()->setPowerMode(newMode);
        }
        m_lastMode = newMode;
      }
    }

  private:
    const PowerMode                           *m_lastMode;
    std::map<ComponentInfo*, const PowerMode*> m_components;
};

class LoadHysteresisGovernor : public LocalPowerGovernor<const PowerMode*>,
                               public sc_module
{
  public:
    LoadHysteresisGovernor(GlobalPowerGovernor<const PowerMode*> *tpg,
                           const sc_time windowTime,
                           const sc_time fastTime,
                           const sc_time slowTime) :
      LocalPowerGovernor<const PowerMode*>(tpg),
      sc_module(sc_module_name("LoadHysteresisGovernor")),
      m_windowTime(windowTime),
      m_fastTime(fastTime),
      m_slowTime(slowTime),
      m_mode(NULL),
      m_ci(NULL),
      m_lastState(ComponentState::IDLE)
    {
      SC_METHOD(process);
      sensitive << m_wakeup_ev;
      dont_initialize();
    }

    ~LoadHysteresisGovernor()
    {}

    SC_HAS_PROCESS(LoadHysteresisGovernor);

    void notify(ComponentInfo *ci)
    {
//    std::cerr << "LoadHysteresisGovernor::notify() @ " << sc_time_stamp() << std::endl;

      const ComponentState newState = ci->getComponentState();

      if(m_mode == NULL) {
        assert(m_ci == NULL);
        m_ci = ci;
        m_mode = m_ci->getPowerMode();
        m_tpg->notify_top(m_ci, m_mode);
        m_wakeup_ev.notify(SC_ZERO_TIME);
      }

      assert(m_ci == ci);

      if(newState != m_lastState) {
        m_stateHistory.push_back(std::pair<ComponentState, sc_time>(m_lastState, sc_time_stamp()));
        m_lastState = newState;
        m_wakeup_ev.notify(SC_ZERO_TIME);
      }
    }

  private:
    const sc_time                                   m_windowTime;
    const sc_time                                   m_fastTime;
    const sc_time                                   m_slowTime;
    const PowerMode                                *m_mode;
    ComponentInfo                                  *m_ci;
    ComponentState                                  m_lastState;
    std::deque<std::pair<ComponentState, sc_time> > m_stateHistory;
    sc_event                                        m_wakeup_ev;

    void process()
    {
//    std::cerr << "LoadHysteresisGovernor::process() @ " << sc_time_stamp() << std::endl;

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
      if((m_mode == SLOW) && (execTime >= m_fastTime)) {
        m_tpg->notify_top(m_ci, FAST);
        m_mode = FAST;
      } else if((m_mode == FAST) && (execTime <= m_slowTime)) {
        m_tpg->notify_top(m_ci, SLOW);
        m_mode = SLOW;
      }

      // wake up process when reaching load boundary
      if((m_mode == SLOW) && (m_lastState != ComponentState::IDLE)) {
        m_wakeup_ev.notify(m_fastTime - execTime);
      } else if((m_mode == FAST) && (m_lastState == ComponentState::IDLE)) {
        m_wakeup_ev.notify(execTime - m_slowTime);
      }
    }
};

}

#endif // POWERGOVERNOR_IMPL_H_
