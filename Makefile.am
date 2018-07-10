## Process this file with automake to produce Makefile.in
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (C) Joachim Falk <joachim.falk@gmx.de> $Date: 2003/01/15 16:43:20 $
#
# Makefile.am is part of the jflib distribution of Joachim Falk;
# you can redistribute it and/or modify it under the terms of the
# GNU General Public License as published by the Free Software Foundation;
# either version 2 of the License, or (at your option) any later version.
#
# The jflib distributio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# $Log$

include $(auxdir)/jfrules.mk
include ../BuildSystem/doxygen.am

SUBDIRS = src/systemcvpc/headers/

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
	src/systemcvpc/cpp/config/common.cpp \
	src/systemcvpc/cpp/config/Component.cpp \
	src/systemcvpc/cpp/config/Mappings.cpp \
	src/systemcvpc/cpp/config/Route.cpp \
	src/systemcvpc/cpp/config/Scheduler.cpp \
	src/systemcvpc/cpp/config/Timing.cpp \
	src/systemcvpc/cpp/config/VpcTask.cpp \
	src/systemcvpc/cpp/config/VpcApi.cpp \
	\
	src/systemcvpc/cpp/dynload/dll-elf.cpp \
	src/systemcvpc/cpp/dynload/dll-win32.cpp \
	\
	src/systemcvpc/cpp/NonPreemptiveScheduler/DynamicPriorityComponent.cpp \
	src/systemcvpc/cpp/NonPreemptiveScheduler/FcfsComponent.cpp \
	src/systemcvpc/cpp/NonPreemptiveScheduler/NonPreemptiveComponent.cpp \
	src/systemcvpc/cpp/NonPreemptiveScheduler/PriorityComponent.cpp \
	src/systemcvpc/cpp/NonPreemptiveScheduler/RoundRobinComponent.cpp \
	\
	src/systemcvpc/cpp/PreemptiveScheduler/PreemptiveComponent.cpp \
	src/systemcvpc/cpp/PreemptiveScheduler/AVBScheduler.cpp \
	src/systemcvpc/cpp/PreemptiveScheduler/FlexRayScheduler.cpp \
	src/systemcvpc/cpp/PreemptiveScheduler/PriorityScheduler.cpp \
	src/systemcvpc/cpp/PreemptiveScheduler/RateMonotonicScheduler.cpp \
	src/systemcvpc/cpp/PreemptiveScheduler/RoundRobinScheduler.cpp \
	src/systemcvpc/cpp/PreemptiveScheduler/TimeTriggeredCCScheduler.cpp \
	src/systemcvpc/cpp/PreemptiveScheduler/TDMAScheduler.cpp \
	src/systemcvpc/cpp/PreemptiveScheduler/MostScheduler.cpp \
	src/systemcvpc/cpp/PreemptiveScheduler/MostSecondaryScheduler.cpp \
	src/systemcvpc/cpp/PreemptiveScheduler/StreamShaperScheduler.cpp \
	\
	src/systemcvpc/cpp/tracing/DataBaseTracer.cpp \
	src/systemcvpc/cpp/tracing/NullTracer.cpp \
	src/systemcvpc/cpp/tracing/PajeTracer.cpp \
	src/systemcvpc/cpp/tracing/TraceableComponent.cpp \
	src/systemcvpc/cpp/tracing/TracerIf.cpp \
	src/systemcvpc/cpp/tracing/VcdTracer.cpp \
	\
	src/systemcvpc/cpp/AbstractComponent.cpp \
	src/systemcvpc/cpp/Attribute.cpp \
	src/systemcvpc/cpp/BlockingTransport.cpp \
	src/systemcvpc/cpp/ComponentInfo.cpp \
	src/systemcvpc/cpp/ConfigCheck.cpp \
	src/systemcvpc/cpp/DebugOStream.cpp \
	src/systemcvpc/cpp/Delayer.cpp \
	src/systemcvpc/cpp/Director.cpp \
	src/systemcvpc/cpp/FastLink.cpp \
	src/systemcvpc/cpp/FunctionTiming.cpp \
	src/systemcvpc/cpp/HysteresisLocalGovernor.cpp \
	src/systemcvpc/cpp/PluggablePowerGovernor.cpp \
	src/systemcvpc/cpp/PowerSumming.cpp \
	src/systemcvpc/cpp/ProcessControlBlock.cpp \
	src/systemcvpc/cpp/RouteImpl.cpp \
	src/systemcvpc/cpp/SelectFastestPowerModeGlobalGovernor.cpp \
	src/systemcvpc/cpp/StaticRoute.cpp \
	src/systemcvpc/cpp/SystemCVPCSimulator.cpp \
	src/systemcvpc/cpp/TaskInstance.cpp \
	src/systemcvpc/cpp/VPCBuilder.cpp

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
	-version-info 0:0:0 \
	-no-undefined \
	-rdynamic
