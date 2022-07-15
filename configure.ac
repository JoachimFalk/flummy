# -*- tab-width:8; indent-tabs-mode:true; -*-
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (c)
#   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
#   2011 FAU -- Jens Gladigau <jens.gladigau@cs.fau.de>
#   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
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

##                                               -*- Autoconf -*-
## Process this file with autoconf to produce a configure script.

AC_INIT(
 [SystemC-VPC], [1.0],
 [joachim.falk@fau.de],
 [SystemC-VPC--devel-])
AC_PREREQ(2.57)
ACJF_INIT

AC_CONFIG_SRCDIR([src/systemcvpc/cpp/VpcApi.cpp])
AC_CONFIG_AUX_DIR([../BuildSystem])
AC_CONFIG_MACRO_DIR([../BuildSystem/m4])

ACJF_ENABLE_DIALECTS_C99_CXX11

AM_INIT_AUTOMAKE([subdir-objects])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl Checks for programs.
ACJF_CHECK_CROSSTOOL(CC, gcc)
ACJF_CHECK_CROSSTOOL(CXX, g++)
ACJF_CHECK_CROSSTOOL(AR, ar)
ACJF_CHECK_CROSSTOOL(RANLIB, ranlib)
AC_PROG_CXXCPP
AC_PROG_CPP
# Checks for programs.
#AC_PROG_INSTALL
#AC_PROG_LN_S
#AC_PROG_MAKE_SET

AC_ENABLE_SHARED
LT_INIT

ACJF_ARG_DEBUG([yes], [], [-DNDEBUG])
if test x"$enable_debug" = x"yes"; then
  AC_DEFINE([SYSTEMCVPC_ENABLE_DEBUG], [yes], [Defined if debug support is enabled])
fi

AC_DEFINE([SYSTEMOC_SIMULATOR_COUPLING_COMPILATION], [yes],
 [This is defined to enable the coupling interface between SystemC-VPC and SysteMoC])

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(ON)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(ON)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN("AC_PACKAGE_NAME", [doxygen.cfg], [doxygen-doc])
DX_ENV_APPEND(COSUPPORT_INCPATH, [\$(COSUPPORT_SYSTEMC_INCPATH) \$(COSUPPORT_TRACING_INCPATH) \$(COSUPPORT_STREAMS_INCPATH) \$(COSUPPORT_XERCES_INCPATH)])
DX_ENV_APPEND(BOOST_INCPATH, [\$(BOOST_INCPATH)])

dnl check for exe extension
AC_EXEEXT

# Checks for header files.
# AC_FUNC_ALLOCA
# AC_HEADER_STDC
# AC_CHECK_HEADERS([OS.h fcntl.h limits.h mach/mach.h malloc.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

dnl ACJF_CHECK_LIB_SYSTEMOC must be first in order for the tutorial to build.
dnl The library link order used by the tutorial seems to depend on the order of
dnl directories in PKG_CONFIG_PATH and, thus, the directory containg the
dnl SysteMoC library must be the first one.
ACJF_CHECK_LIB_SYSTEMOC
ACJF_CHECK_LIB_BOOST
ACJF_CHECK_LIB_SYSTEMC
dnl ACJF_CHECK_LIB_XERCES
ACJF_CHECK_LIB_COSUPPORT_XERCES
ACJF_CHECK_LIB_COSUPPORT_STRING
ACJF_CHECK_LIB_COSUPPORT_SMARTPTR
ACJF_CHECK_LIB_COSUPPORT_SYSTEMC
ACJF_CHECK_LIB_COSUPPORT_TRACING
ACJF_CHECK_LIB_COSUPPORT_STREAMS

SYSTEMOC_PLUGINPATH="$acjf_abs_top_builddir/.libs"
AC_SUBST([SYSTEMOC_PLUGINPATH])
AC_SUBST([SYSTEMOC_ENABLE_SGX])

AC_CONFIG_FILES(
 [Makefile]dnl
 [testcases/Makefile]dnl
 [tutorial/Makefile]dnl
 [src/systemcvpc/headers/Makefile]dnl
 [pkgconfig/libsystemcvpc.pc]dnl
)
AC_CONFIG_HEADERS(
 [src/systemcvpc/cpp/config.h]dnl
 [src/systemcvpc/headers/systemcvpc/vpc_config.h]dnl
)

ACJF_DONE

AC_OUTPUT
