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

#include <systemcvpc/TimingModifier.hpp>
#include <systemcvpc/vpc_config.h>

#include "AbstractComponent.hpp"
#include "ProcessControlBlock.hpp"
#include <systemcvpc/Director.hpp>
#include <CoSupport/Tracing/TracingFactory.hpp>

#include <ctime>
#include <cfloat>

#include "DebugOStream.hpp"

namespace SystemC_VPC{

  ProcessControlBlock::ProcessControlBlock( AbstractComponent * component )
    : name("NN"), component(component), psm(false)
  {
    this->deadline = sc_core::sc_time(DBL_MAX, sc_core::SC_SEC);
    this->period = sc_core::sc_time(DBL_MAX, sc_core::SC_SEC);
    this->priority = 0;
    this->traceSignal = NULL;
  }

  void ProcessControlBlock::configure(std::string name, bool tracing){
    this->name = name;
    taskTracer =
        CoSupport::Tracing::TracingFactory::getInstance().createTaskTracer(name,
            component->getName());
  }

  std::string const& ProcessControlBlock::getName() const{
    return this->name;
  }

  void ProcessControlBlock::setPid( ProcessId pid){
    this->pid=pid;
  }

  ProcessId ProcessControlBlock::getPid( ) const{
    return this->pid;
  }
      
  void ProcessControlBlock::setFunctionId( FunctionId fid){
    this->fid=fid;
  }

  FunctionId ProcessControlBlock::getFunctionId( ) const{
    return this->fid;
  }

  const char* ProcessControlBlock::getFuncName() const{
    assert(0);
    return "";
  }

  void ProcessControlBlock::setPeriod(sc_core::sc_time period){
    this->period = period;
  }

  sc_core::sc_time ProcessControlBlock::getPeriod() const{
    return this->period;
  }

  void ProcessControlBlock::setPriority(int priority){
    this->priority = priority;
  }

  int ProcessControlBlock::getPriority() const{
    return this->priority;
  }

  void ProcessControlBlock::setDeadline(sc_core::sc_time deadline){
    if(deadline > sc_core::SC_ZERO_TIME){
      this->deadline = deadline;
    }else{
      this->deadline = sc_core::SC_ZERO_TIME;
    }
  }

  sc_core::sc_time ProcessControlBlock::getDeadline() const{
    return this->deadline;
  }

  void ProcessControlBlock::setTraceSignal(Trace::Tracing* signal){
    this->traceSignal = signal;
  }

  Trace::Tracing* ProcessControlBlock::getTraceSignal() const {
    return this->traceSignal;
  }

  void ProcessControlBlock::setTiming(const Config::Timing& timing){
    const PowerMode *mode = this->component->translatePowerMode(timing.getPowerMode());
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->setTiming(timing);
  }

  void ProcessControlBlock::setBaseDelay(sc_core::sc_time delay){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->setBaseDelay(delay);
  }

  void ProcessControlBlock::setBaseLatency(sc_core::sc_time latency){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->setBaseLatency(latency);
  }

  void ProcessControlBlock::addDelay(FunctionId fid, sc_core::sc_time delay){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->addDelay(fid, delay);
  }

  void ProcessControlBlock::addLatency(FunctionId fid, sc_core::sc_time latency){
    const PowerMode *mode = this->component->translatePowerMode("SLOW");
    FunctionTimingPtr ft =this->component->getTiming(mode, this->getPid());
    ft->addLatency(fid, latency);
  }

  void ProcessControlBlock::setActorAsPSM(bool psm)
  {
    this->psm = psm;
  }

  bool ProcessControlBlock::isPSM()
  {
    return this->psm;
  }

} // namespace SystemC_VPC
