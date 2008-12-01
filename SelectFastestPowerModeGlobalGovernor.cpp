#include "SelectFastestPowerModeGlobalGovernor.hpp"
#include "ComponentModel.h"

namespace SystemC_VPC{

  InternalSelectFastestPowerModeGovernor::InternalSelectFastestPowerModeGovernor() :
    m_lastMode(NULL)
  {
    //std::cout << "InternalSelectFastestPowerModeGovernor" << std::endl; 
  }

  void InternalSelectFastestPowerModeGovernor::notify_top(ComponentInfo *ci,
                                                  GenericParameter *param)
  {
    // extract PowerMode from container "GenericParameter"
    PowerModeParameter * p = dynamic_cast<PowerModeParameter*>(param);
    const PowerMode * newMode = p->powerMode;

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

}
