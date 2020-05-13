# -*- tab-width:8; indent-tabs-mode:true; -*-
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (c)
#   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
#   2010 FAU -- Sebastian Graf <sebastian.graf@fau.de>
#   2011 FAU -- Jens Gladigau <jens.gladigau@cs.fau.de>
#   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
#   2011 Thomas Russ <tr.thomas.russ@googlemail.com>
#   2012 FAU -- Sebastian Graf <sebastian.graf@fau.de>
#   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2017 FAU -- Simone Müller <simone.mueller@fau.de>
#   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
# 
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
# 
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

## Process this file with automake to produce Makefile.in

include $(auxdir)/jfrules.mk
include ../BuildSystem/doxygen.am

SUBDIRS = . src/systemcvpc/headers testcases tutorial

AUTOMAKE_OPTIONS = nostdinc
# This is only needed if there is a configure.ac in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

AM_LDFLAGS = @AM_LDFLAGS@
AM_CPPFLAGS= @AM_CPPFLAGS@

pkgconfigdir = $(libdir)/pkgconfig

nodist_pkgconfig_DATA = \
	libsystemcvpc.pc

%.pc: pkgconfig/%.pc
	@sed -e '/^srctreemode_\w\+\s*=/d' \
	     -e 's/\$${srctreemode_libdir}\/lib\([^ 	]*\).la/-l\1/g' \
	     -e 's/\$${srctreemode_\(\w\+\)}/\$${\1}/g' \
	  < $^ > $@

src/systemcvpc/cpp/%-xsd.c: %.xsd
	{ set -e; echo "static char const $(notdir $*)XSD[] ="; \
	  sed -e 's/\(["\\]\)/\\\1/g' -e 's/^\(.*\)$$/  "\1\\n"/' $<; \
	  echo ";"; \
	} > $@
src/systemcvpc/cpp/%-dtd.c: %.dtd
	{ set -e; echo "static char const $(notdir $*)DTD[] ="; \
	  sed -e 's/\(["\\]\)/\\\1/g' -e 's/^\(.*\)$$/  "\1\\n"/' $<; \
	  echo ";"; \
	} > $@

BUILT_SOURCES=src/systemcvpc/cpp/vpc-dtd.c

CLEANFILES=$(nodist_pkgconfig_DATA)

lib_LTLIBRARIES = libsystemcvpc.la

libsystemcvpc_la_SOURCES = \
	src/systemcvpc/cpp/Attribute.cpp \
	src/systemcvpc/cpp/Component.cpp \
	src/systemcvpc/cpp/ComponentObserver.cpp \
	src/systemcvpc/cpp/ExecModel.cpp \
	src/systemcvpc/cpp/Power.cpp \
	src/systemcvpc/cpp/PowerMode.cpp \
	src/systemcvpc/cpp/Route.cpp \
	src/systemcvpc/cpp/Scheduler.cpp \
	src/systemcvpc/cpp/Time.cpp \
	src/systemcvpc/cpp/Timing.cpp \
	src/systemcvpc/cpp/VpcApi.cpp \
	src/systemcvpc/cpp/VpcTask.cpp \
	\
	src/systemcvpc/cpp/ExecModelling/LookupPowerTimeModel.cpp \
	\
	src/systemcvpc/cpp/Extending/ComponentObserverIf.cpp \
	src/systemcvpc/cpp/Extending/ComponentTracerIf.cpp \
	src/systemcvpc/cpp/Extending/TaskInstance.cpp \
	\
	src/systemcvpc/cpp/Routing/Ignore.cpp \
	src/systemcvpc/cpp/Routing/Static.cpp \
	\
	src/systemcvpc/cpp/detail/dynload/dll-elf.cpp \
	src/systemcvpc/cpp/detail/dynload/dll-win32.cpp \
	\
	src/systemcvpc/cpp/detail/ExecModelling/LookupPowerTimeModelImpl.cpp \
	\
	src/systemcvpc/cpp/detail/NonPreemptiveScheduler/DynamicPriorityComponent.cpp \
	src/systemcvpc/cpp/detail/NonPreemptiveScheduler/FcfsComponent.cpp \
	src/systemcvpc/cpp/detail/NonPreemptiveScheduler/NonPreemptiveComponent.cpp \
	src/systemcvpc/cpp/detail/NonPreemptiveScheduler/PriorityComponent.cpp \
	src/systemcvpc/cpp/detail/NonPreemptiveScheduler/RoundRobinComponent.cpp \
	\
	src/systemcvpc/cpp/detail/PreemptiveScheduler/PreemptiveComponent.cpp \
	src/systemcvpc/cpp/detail/PreemptiveScheduler/AVBScheduler.cpp \
	src/systemcvpc/cpp/detail/PreemptiveScheduler/FlexRayScheduler.cpp \
	src/systemcvpc/cpp/detail/PreemptiveScheduler/PriorityScheduler.cpp \
	src/systemcvpc/cpp/detail/PreemptiveScheduler/RateMonotonicScheduler.cpp \
	src/systemcvpc/cpp/detail/PreemptiveScheduler/RoundRobinScheduler.cpp \
	src/systemcvpc/cpp/detail/PreemptiveScheduler/TimeTriggeredCCScheduler.cpp \
	src/systemcvpc/cpp/detail/PreemptiveScheduler/TDMAScheduler.cpp \
	src/systemcvpc/cpp/detail/PreemptiveScheduler/MostScheduler.cpp \
	src/systemcvpc/cpp/detail/PreemptiveScheduler/MostSecondaryScheduler.cpp \
	src/systemcvpc/cpp/detail/PreemptiveScheduler/StreamShaperScheduler.cpp \
	\
	src/systemcvpc/cpp/detail/Routing/BlockingTransport.cpp \
	src/systemcvpc/cpp/detail/Routing/IgnoreImpl.cpp \
	src/systemcvpc/cpp/detail/Routing/StaticImpl.cpp \
	\
	src/systemcvpc/cpp/detail/tracing/ComponentDBTracer.cpp \
	src/systemcvpc/cpp/detail/tracing/ComponentPajeTracer.cpp \
	src/systemcvpc/cpp/detail/tracing/ComponentVCDTracer.cpp \
	\
	src/systemcvpc/cpp/detail/AbstractComponent.cpp \
	src/systemcvpc/cpp/detail/AbstractExecModel.cpp \
	src/systemcvpc/cpp/detail/AbstractRoute.cpp \
	src/systemcvpc/cpp/detail/common.cpp \
	src/systemcvpc/cpp/detail/ConfigCheck.cpp \
	src/systemcvpc/cpp/detail/Configuration.cpp \
	src/systemcvpc/cpp/detail/DebugOStream.cpp \
	src/systemcvpc/cpp/detail/Director.cpp \
	src/systemcvpc/cpp/detail/HysteresisLocalGovernor.cpp \
	src/systemcvpc/cpp/detail/LegacyComponentObserver.cpp \
	src/systemcvpc/cpp/detail/ObservableComponent.cpp \
	src/systemcvpc/cpp/detail/PluggablePowerGovernor.cpp \
	src/systemcvpc/cpp/detail/PowerSumming.cpp \
	src/systemcvpc/cpp/detail/SelectFastestPowerModeGlobalGovernor.cpp \
	src/systemcvpc/cpp/detail/SystemCVPCSimulator.cpp \
	src/systemcvpc/cpp/detail/TaskImpl.cpp \
	src/systemcvpc/cpp/detail/TaskInstanceImpl.cpp \
	src/systemcvpc/cpp/detail/TimeQueue.cpp \
	src/systemcvpc/cpp/detail/VPCBuilder.cpp

libsystemcvpc_la_DEPENDENCIES = \
	$(COSUPPORT_STRING_DEPENDENCIES) \
	$(COSUPPORT_SMARTPTR_DEPENDENCIES) \
	$(COSUPPORT_SYSTEMC_DEPENDENCIES) \
	$(COSUPPORT_TRACING_DEPENDENCIES) \
	$(COSUPPORT_STREAMS_DEPENDENCIES) \
	$(COSUPPORT_XERCES_DEPENDENCIES) \
	$(SYSTEMOC_DEPENDENCIES)

libsystemcvpc_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-I$(builddir)/src/systemcvpc/cpp \
	-I$(builddir)/src/systemcvpc/headers \
	-I$(srcdir)/src/systemcvpc/cpp \
	-I$(srcdir)/src/systemcvpc/headers \
	$(BOOST_CPPFLAGS) \
	$(SYSTEMC_CPPFLAGS) \
	$(COSUPPORT_STRING_CPPFLAGS) \
	$(COSUPPORT_SMARTPTR_CPPFLAGS) \
	$(COSUPPORT_SYSTEMC_CPPFLAGS) \
	$(COSUPPORT_TRACING_CPPFLAGS) \
	$(COSUPPORT_STREAMS_CPPFLAGS) \
	$(COSUPPORT_XERCES_CPPFLAGS) \
	$(SYSTEMOC_CPPFLAGS)

libsystemcvpc_la_LDFLAGS = $(AM_LDFLAGS) \
	$(filter-out %/libsystemcvpc.la, $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)"; export PKG_CONFIG_PATH; $(PKG_CONFIG) --static --libs libsystemcvpc)) \
	-Wl,--no-undefined \
	-version-info 0:0:0 \
	-module \
	-rdynamic
