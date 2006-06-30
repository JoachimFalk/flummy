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

root_srcdir=$(top_srcdir)/..
root_builddir=$(top_builddir)/..

SUBDIRS=
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

ac_aux_dir = @ac_aux_dir@

include $(ac_aux_dir)/jfrules.mk

#pkgincludeprefix=systemoc/


compileheader-libsystemcvpc.stamp: $(libsystemcvpc_CH_LTOBJS)
	@date > $@

# FIXME: Decouple SystemC-VPC and SysteMoC by moving SysteMoC
#        dependant code from SystemC-VPC into SysteMoC
#        restore thread based compute interface to SystemC-VPC
INCLUDES = @INCLUDES@					\
	   @SYSTEMC_INCLUDE@				\
	   @XERCES_INCLUDE@				\
	   -I./include					\
	   -I$(srcdir)

#AM_LDFLAGS=

#noinst_SCRIPTS  = test_pathstuff.sh

EXTRA_DIST	=

#noinst_PROGRAMS = test_pathstuff

PROGLDFLAGS=

pkgincludeprefix=systemcvpc/

lib_LTLIBRARIES = libsystemcvpc.la
pkginclude_HEADERS =				\
	hscd_vpc_Tracing.h			\
	hscd_vpc_AbstractComponent.h            \
	hscd_vpc_AbstractController.h           \
	hscd_vpc_AbstractDirector.h             \
	hscd_vpc_AbstractConfigurationPool.h	\
	hscd_vpc_AbstractConfigurationMapper.h	\
	hscd_vpc_datatypes.h                    \
	hscd_vpc_Director.h                     \
	hscd_vpc_TaskEventListener.h		\
	hscd_vpc_Director.h			\
	hscd_vpc_ProcessControlBlock.h		\
	hscd_vpc_PCBPool.h			\
	hscd_vpc_AbstractBinder.h		\
	hscd_vpc_Binding.h			\
	hscd_vpc_MappingInformation.h		\
	hscd_vpc_DelayMapper.h			\
	hscd_vpc_IPreemptable.h			\
	hscd_vpc_EventPair.h	

#noinst_HEADERS=	oneof.hpp

AM_CPPFLAGS=-Wall -O1 -Wuninitialized

libsystemcvpc_CH_LTOBJS=			\
	$(hscd_op_hpp_CH_LTOBJS)		\
	$(hscd_pggen_hpp_CH_LTOBJS)

## DISABLE -lefence because is causes crashes on systemc where libefence is linked with -lpthread
## because of quick-thread (SystemC) pthread incompatibilities
#libsystemcvpc_la_LDFLAGS = -version-info 0:0:0 -no-undefined @SYSTEMC_LDFLAGS@ @XERCES_LDFLAGS@ -lsystemc -lxerces-c -lstdc++ -lefence
libsystemcvpc_la_LDFLAGS = -version-info 0:0:0 -no-undefined @SYSTEMC_LDFLAGS@ @XERCES_LDFLAGS@ -lsystemc -lxerces-c -lstdc++

libsystemcvpc_la_LIBADD = $(libsystemcvpc_CH_LTOBJS)
libsystemcvpc_la_DEPENDENCIES = compileheader-libsystemcvpc.stamp include




libsystemcvpc_la_SOURCES =			\
	hscd_vpc_Component.cpp			\
	hscd_vpc_Director.cpp			\
	hscd_vpc_PriorityScheduler.cpp		\
	hscd_vpc_RoundRobinScheduler.cpp	\
	hscd_vpc_FCFSScheduler.cpp		\
	hscd_vpc_RateMonotonicScheduler.cpp	\
	hscd_vpc_XmlHelper.cpp			\
	hscd_vpc_Configuration.cpp		\
	hscd_vpc_Controller.cpp			\
	hscd_vpc_EDFAllocator.cpp		\
	hscd_vpc_FCFSAllocator.cpp		\
	hscd_vpc_PriorityAllocator.cpp		\
	hscd_vpc_RoundRobinAllocator.cpp	\
	hscd_vpc_ReconfigurableComponent.cpp	\
	hscd_vpc_VPCBuilder.cpp			\
	hscd_vpc_ProcessControlBlock.cpp	\
	hscd_vpc_PCBPool.cpp			\
	hscd_vpc_Binding.cpp			\
	hscd_vpc_SimpleBinder.cpp		\
	hscd_vpc_RRBinder.cpp			\
	hscd_vpc_PriorityBinder.cpp		\
	hscd_vpc_LeastCurrentlyBoundPE.cpp	\
	hscd_vpc_LeastFrequentlyUsedPE.cpp	\
	hscd_vpc_ConfigurationPool.cpp		\
	hscd_vpc_Allocator.cpp			\
	hscd_vpc_RREAllocator.cpp		\
	hscd_vpc_DelayMapper.cpp		\
	hscd_vpc_MappingInformation.cpp		\
	hscd_vpc_ARBinder.cpp			\
	hscd_vpc_BindingGraph.cpp		\
	hscd_vpc_AbstractBinder.cpp		\
	hscd_vpc_Term.cpp			
#	hscd_vpc_SchedulerProxy.cpp		


# Need pkinclude files in include/$(pkgincludeprefix)
$(libsystemcvpc_la_OBJECTS) $(libsystemcvpc_CH_LTOBJS): include

#test_pathstuff_LDADD = libsystemcvpc.la
#test_pathstuff_SOURCES = test_pathstuff.C
