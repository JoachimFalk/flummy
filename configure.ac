#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(
 [SystemC-VPC], [1.0],
 [joachim.falk@fau.de],
 [SystemC-VPC--devel-])
AC_PREREQ(2.57)
ACJF_INIT

AC_CONFIG_SRCDIR([src/systemcvpc/cpp/VpcApi.cpp])
AC_CONFIG_AUX_DIR([../BuildSystem])
AC_CONFIG_MACRO_DIR([../BuildSystem/m4])

AM_INIT_AUTOMAKE([subdir-objects])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl Checks for programs.
ACJF_CHECK_CROSSTOOL(CC, gcc)
ACJF_CHECK_CROSSTOOL(CXX, g++)
ACJF_CHECK_CROSSTOOL(AR, ar)
ACJF_CHECK_CROSSTOOL(RANLIB, ranlib)
ACJF_CHECK_CROSSTOOL(NM, nm)
AC_PROG_CXXCPP
AC_PROG_CPP
# Checks for programs.
#AC_PROG_LIBTOOL
#AC_PROG_INSTALL
#AC_PROG_LN_S
#AC_PROG_MAKE_SET

AC_ENABLE_SHARED
AC_PROG_LIBTOOL

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

### VPC power tracing  ###
# --enable-power-tracing option
AC_ARG_ENABLE(
 [power-tracing],
 [[  --enable-power-tracing  enable power tracing for VPC [default=no]]],
 [case "$enableval" in
    yes)  enable_power_tracing=yes  ;;
    no)   enable_power_tracing=no   ;;
    *)    AC_MSG_ERROR(bad value ${enableval} for --enable-power-tracing) ;;
  esac],
 [enable_power_tracing=no #default]
)

AM_CONDITIONAL([NO_POWER_SUM], [test x"$enable_power_tracing" = x"no"])
# This is a define for pkginclude/systemcvpc/vpc_config.h to guarantee
# consistency for Examples and Testcases
if test x"$enable_power_tracing" = x"no"; then
  AC_DEFINE([NO_POWER_SUM], [yes], [Define to suppress power tracing for VPC])
fi

AC_CONFIG_FILES(
 [Makefile]dnl
 [testcases/Makefile]dnl
 [tutorial/Makefile]dnl
 [src/systemcvpc/headers/Makefile]dnl
 [pkgconfig/libsystemcvpc.pc]dnl
)
AC_CONFIG_HEADER(
 [src/systemcvpc/cpp/config.h]dnl
 [src/systemcvpc/headers/systemcvpc/vpc_config.h]dnl
)

ACJF_DONE

AC_OUTPUT
