#ifndef POWERGOVERNOR_IMPL_H_
#define POWERGOVERNOR_IMPL_H_

#include <map>
#include <deque>
#include <systemc.h>

#include "PowerMode.h"
#include "PowerGovernor.h"

#include "hscd_vpc_Component.h"

namespace SystemC_VPC{

class SelectFastestPowerModeGovernor : public GlobalPowerGovernor<PowerMode>
{
  public:
    SelectFastestPowerModeGovernor()
    {}

    void notify_top(ComponentInfo *ci, PowerMode newMode)
    {
      if(m_components.find(ci) == m_components.end())
        ci->setPowerMode(m_lastMode);

      m_components[ci] = newMode;

      if(newMode < m_lastMode) {
        for(std::map<ComponentInfo*, PowerMode>::iterator
          iter  = m_components.begin();
          iter != m_components.end();
          iter++)
        {
          if(iter->second > newMode)
            newMode = iter->second;
        }
      }

      if(newMode != m_lastMode) {
        for(std::map<ComponentInfo*, PowerMode>::iterator
          iter  = m_components.begin();
          iter != m_components.end();
          iter++)
        {
          iter->first->setPowerMode(newMode);
        }
        m_lastMode = newMode;
      }
    }

  private:
    PowerMode                           m_lastMode;
    std::map<ComponentInfo*, PowerMode> m_components;
};

class LoadHysteresisGovernor : public LocalPowerGovernor<PowerMode>,
                               public sc_module
{
  public:
    LoadHysteresisGovernor(GlobalPowerGovernor<PowerMode> *tpg,
                           const sc_time windowTime,
                           const sc_time fastTime,
                           const sc_time slowTime) :
      LocalPowerGovernor<PowerMode>(tpg),
      sc_module(NULL),
      m_windowTime(windowTime),
      m_fastTime(fastTime),
      m_slowTime(slowTime),
      m_ci(NULL),
      m_lastState(ComponentState::IDLE)
    {
      SC_THREAD(process);
      sensitive << m_wakeup_ev;
      dont_initialize();
    }

    ~LoadHysteresisGovernor()
    {}

    SC_HAS_PROCESS(LoadHysteresisGovernor);

    void notify(ComponentInfo *ci)
    {
      ComponentState newState = ci->getComponentState();
      if(newState == m_lastState)
        return;
      m_stateHistory.push_back(std::pair<ComponentState, sc_time>(m_lastState, sc_time_stamp()));
      m_lastState = newState;
      m_ci = ci;
      m_wakeup_ev.notify(SC_ZERO_TIME);
    }

  private:
    const sc_time                                   m_windowTime;
    const sc_time                                   m_fastTime;
    const sc_time                                   m_slowTime;
    ComponentInfo                                  *m_ci;
    PowerMode                                       m_lastMode;
    ComponentState                                  m_lastState;
    std::deque<std::pair<ComponentState, sc_time> > m_stateHistory;
    sc_event                                        m_wakeup_ev;

    void process()
    {
      const PowerMode FAST =  m_ci->translatePowerMode("FAST");
      const PowerMode SLOW =  m_ci->translatePowerMode("SLOW");

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
        if(iter->first != ComponentState::IDLE)
          execTime += iter->second - startTime;
        startTime = iter->second;
      }
      if(m_lastState != ComponentState::IDLE)
        execTime += sc_time_stamp() - startTime;

      // notify power mode suggestions
      if((m_lastMode == SLOW) && (execTime > m_fastTime)) {
        m_tpg->notify_top(m_ci, FAST);
        m_lastMode = FAST;
      } else if((m_lastMode == FAST) && (execTime < m_slowTime)) {
        m_tpg->notify_top(m_ci, SLOW);
        m_lastMode = SLOW;
      }

      // wake up process when reaching load boundary
      if((m_lastMode == SLOW) && (m_lastState != ComponentState::IDLE)) {
        m_wakeup_ev.notify(m_fastTime - execTime);
      } else if((m_lastMode == FAST) && (m_lastState == ComponentState::IDLE)) {
        m_wakeup_ev.notify(execTime - m_slowTime);
      }
    }
};

}

#endif // POWERGOVERNOR_IMPL_H_
