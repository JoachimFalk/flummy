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

SUBDIRS = pkginclude

# This is only needed if there is a configure.in in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

AM_LDFLAGS = @AM_LDFLAGS@

AM_CPPFLAGS= @AM_CPPFLAGS@ \
	-Ipkginclude \
	-I$(srcdir)/pkginclude

pkgconfigdir = $(libdir)/pkgconfig

nodist_pkgconfig_DATA = \
	libsystemcvpc.pc

%.pc: pkgconfig/%.pc
	@sed -e '/^srctreemode_\w\+\s*=/d' \
	     -e 's/\$${srctreemode_libdir}\/lib\([^ 	]*\).la/-l\1/g' \
	     -e 's/\$${srctreemode_\(\w\+\)}/\$${\1}/g' \
	  < $^ > $@

CLEANFILES=$(nodist_pkgconfig_DATA)

lib_LTLIBRARIES = libsystemcvpc.la

libsystemcvpc_la_SOURCES = \
	config/common.cpp \
	config/Component.cpp \
	config/Mappings.cpp \
	config/Route.cpp \
	config/Scheduler.cpp \
	config/Timing.cpp \
	config/VpcTask.cpp \
	config/VpcApi.cpp \
	\
	scheduler/AVBScheduler.cpp \
	scheduler/FCFSScheduler.cpp \
	scheduler/FlexRayScheduler.cpp \
	scheduler/PriorityScheduler.cpp \
	scheduler/PrioritySchedulerNoPreempt.cpp \
	scheduler/RateMonotonicScheduler.cpp \
	scheduler/RoundRobinScheduler.cpp \
	scheduler/TimeTriggeredCCScheduler.cpp \
	scheduler/TDMAScheduler.cpp \
	scheduler/MostScheduler.cpp \
	scheduler/MostSecondaryScheduler.cpp \
	scheduler/StreamShaperScheduler.cpp \
	\
	tracing/DataBaseTracer.cpp \
	tracing/Tracing.cpp \
	\
	debug_config.cpp \
	BlockingTransport.cpp \
	ComponentImpl.cpp \
	ConfigCheck.cpp \
	Director.cpp \
	ScheduledTask.cpp \
	Attribute.cpp \
	Task.cpp \
	VPCBuilder.cpp \
	ProcessControlBlock.cpp \
	FastLink.cpp \
	StaticRoute.cpp \
	RouteImpl.cpp \
	PowerSumming.cpp \
	HysteresisLocalGovernor.cpp \
	SelectFastestPowerModeGlobalGovernor.cpp \
	PluggablePowerGovernor.cpp \
	dynload/dll-elf.cpp \
	ComponentInfo.cpp \
	AbstractComponent.cpp

libsystemcvpc_la_DEPENDENCIES = \
	$(COSUPPORT_SYSTEMC_DEPENDENCIES) \
	$(COSUPPORT_TRACING_DEPENDENCIES) \
	$(COSUPPORT_STREAMS_DEPENDENCIES) \
	$(COSUPPORT_XERCES_DEPENDENCIES)

libsystemcvpc_la_CPPFLAGS = $(AM_CPPFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(SYSTEMC_CPPFLAGS) \
	$(XERCES_CPPFLAGS) \
	$(COSUPPORT_SYSTEMC_CPPFLAGS) \
	$(COSUPPORT_TRACING_CPPFLAGS) \
	$(COSUPPORT_STREAMS_CPPFLAGS) \
	$(COSUPPORT_XERCES_CPPFLAGS)

libsystemcvpc_la_LDFLAGS = $(AM_LDFLAGS) \
	$(filter-out %/libsystemcvpc.la, $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)"; export PKG_CONFIG_PATH; $(PKG_CONFIG) --static --libs libsystemcvpc)) \
	-version-info 0:0:0 \
	-no-undefined \
	-rdynamic
