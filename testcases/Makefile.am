# -*- tab-width:8; indent-tabs-mode:true; -*-
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (c)
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

SUBDIRS=

AUTOMAKE_OPTIONS = nostdinc
# This is only needed if there is a configure.ac in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

export SYSTEMOC_PLUGINPATH = @SYSTEMOC_PLUGINPATH@
SYSTEMOC_PLUGINVPC = libsystemcvpc.so

vpath $(SYSTEMOC_PLUGINVPC) $(SYSTEMOC_PLUGINPATH)

AM_CPPFLAGS= @AM_CPPFLAGS@ $(SYSTEMOC_INCLUDE) $(BOOST_INCLUDE)
AM_LDFLAGS = @AM_LDFLAGS@  $(SYSTEMOC_LDFLAGS) $(BOOST_LDFLAGS)

noinst_PROGRAMS= \
	test-sqr \
	test-sqr-int \
	test-src-snk-multicast \
	test-src-snk-multirate \
	test-src-snk-plain

%.smx: %
	./'$<' --systemoc-export-smx='$@' || \
	  { rm -f "$@"; exit 1; }

%.log: %
	$(srcdir)/scripts/runtest.py --log '$@' --filter '^((sqrroot|top)\.[^:]*|src|sink|A|B|Src|Snk):' -- ./'$<' 100
	
TEST_LOGS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/golden%.log,$(TEST)%.log,$(wildcard $(srcdir)/src/$(TEST)/golden*.log)))
TEST_VPC_LOGS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/vpc/%.vpc.xml,$(TEST)-%.log,$(wildcard $(srcdir)/src/$(TEST)/vpc/*.vpc.xml)))
TEST_VPC_VCDS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/vpc/%-golden.vcd,$(TEST)-%.vcd,$(wildcard $(srcdir)/src/$(TEST)/vpc/*-golden.vcd)))

test_sqr_LDADD					= $(SYSTEMOC_LIBS)
test_sqr_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_sqr_SOURCES				= \
	src/test-sqr/cpp/design-sqr.cpp

test_sqr_int_LDADD				= $(SYSTEMOC_LIBS)
test_sqr_int_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_sqr_int_SOURCES				= \
	src/test-sqr-int/cpp/design-sqr.cpp

test_src_snk_multicast_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multicast_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multicast_SOURCES			= \
	src/test-src-snk-multicast/cpp/design-src-snk-multicast.cpp

test-src-snk-multicast-%.log test-src-snk-multicast-%-CPU.vcd test-src-snk-multicast-%-CPU1.vcd test-src-snk-multicast-%-CPU2.vcd test-src-snk-multicast-%-BUS.vcd test-src-snk-multicast-%-MEM.vcd test-src-snk-multicast-%-MEM1.vcd test-src-snk-multicast-%-MEM2.vcd: test-src-snk-multicast src/test-src-snk-multicast/vpc/%.vpc.xml $(SYSTEMOC_PLUGINVPC)
	set -- $^; \
	VPCTRACEFILEPREFIX=test-src-snk-multicast-$*- \
	$(srcdir)/scripts/runtest.py \
		--log 'test-src-snk-multicast-$*.log' \
		--clean 'test-src-snk-multicast-$*-*' \
		-- ./'$<' --systemoc-vpc-config "$$2" 100

test_src_snk_multirate_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multirate_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multirate_SOURCES			= \
	src/test-src-snk-multirate/cpp/design-src-snk-multirate.cpp

test_src_snk_plain_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_plain_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_plain_SOURCES			= \
	src/test-src-snk-plain/cpp/design-src-snk-plain.cpp

test-src-snk-plain-%.log test-src-snk-plain-%.vcd test-src-snk-plain-%-CPU.vcd test-src-snk-plain-%-BUS.vcd test-src-snk-plain-%-MEM.vcd: test-src-snk-plain src/test-src-snk-plain/vpc/%.vpc.xml $(SYSTEMOC_PLUGINVPC)
	set -- $^; \
	VPCTRACEFILEPREFIX=test-src-snk-plain-$*- \
	VPCCONFIGURATION="$$2" \
	$(srcdir)/scripts/runtest.py \
		--log 'test-src-snk-plain-$*.log' \
		--clean 'test-src-snk-plain-$*-*' \
		-- ./'$<' 100

clean-local:
	rm -f $(TEST_LOGS) $(TEST_VPC_VCDS) $(TEST_VPC_LOGS)

.PHONY: test-logs check-logs
check-am: check-logs
test-logs: $(TEST_LOGS)
check-logs: $(TEST_LOGS)
	@error=0; \
	for test in $(noinst_PROGRAMS); do \
	  case "$$test" in \
	    test-*) \
	      if test -f $(srcdir)/src/$${test}/golden.log; then \
		if ! diff -u $${test}.log $(srcdir)/src/$${test}/golden.log > /dev/null; then \
		  echo "FAILURE for $${test} LOG check."; error=1; \
		  diff -u $${test}.log $(srcdir)/src/$${test}/golden.log; \
		else \
		  echo "SUCCESS for $${test} LOG check."; \		
		fi; \
	      else \
		status=success; \
		shopt -s nullglob; \
		for log in $(srcdir)/src/$${test}/golden-*.log; do \
		  if ! diff -u $${test}-$${log##*/golden-} $${log} > /dev/null; then \
		    status=failed; \
		  fi; \
		done; \
		if test x"$$status" = x"failed"; then \
		  echo "FAILURE for $${test} LOG check."; error=1; \
		  for log in $(srcdir)/src/$${test}/golden-*.log; do \
		    diff -u $${test}-$${log##*/golden-} $${log}; \
		  done; \
		else \
		  echo "SUCCESS for $${test} LOG check."; \		
		fi; \
	      fi; \
	      ;; \
 	  esac; \
	done; \
	exit $$error



.PHONY: test-vcds check-vcds

diff_vcd_ignore_date=-I '[[:blank:]]*[[:alpha:]]\{3\}[[:blank:]][[:digit:]]\{2\},[[:blank:]][[:digit:]]\{4\}[[:blank:]]*[[:digit:]]\{1,2\}:[[:digit:]]\{2\}:[[:digit:]]\{2\}$$'
diff_vcd_ignore_systemc_version=-I 'SystemC[[:blank:]][[:digit:].]\+\(-Accellera\)\?[[:blank:]]---[[:blank:]]'
diff_vcd_ignore=$(diff_vcd_ignore_date) $(diff_vcd_ignore_systemc_version)

check-am: check-vcds
test-vcds: $(TEST_VPC_VCDS)
check-vcds: $(TEST_VPC_VCDS)
	@error=0; \
	for test in $(noinst_PROGRAMS); do \
	  case "$$test" in \
	    test-*) \
	      for cmx in $(srcdir)/src/$${test}/vpc/*.vpc.xml; do \
		cmx=$${cmx##*/}; cmx=$${cmx%%.vpc.xml}; \
		status=success; \
		for golden_vcd in $(srcdir)/src/$${test}/vpc/$${cmx}-*golden.vcd; do \
		  test_vcd=$${golden_vcd##*/}; test_vcd=$${test}-$${test_vcd%%-golden.vcd}.vcd; \
		  if test -f $${golden_vcd} && ! diff -u $(diff_vcd_ignore) $${test_vcd} $${golden_vcd} > /dev/null; then \
		    status=failed; \
		  fi; \
		done; \
		if test x"$$status" = x"failed"; then \
		  echo "FAILURE for $${test}-$${cmx} VCD check."; error=1; \
		  for golden_vcd in $(srcdir)/src/$${test}/vpc/$${cmx}-*golden.vcd; do \
		    test_vcd=$${golden_vcd##*/}; test_vcd=$${test}-$${test_vcd%%-golden.vcd}.vcd; \
		    diff -u $(diff_vcd_ignore) $${test_vcd} $${golden_vcd}; \
		  done; \
		else \
		  echo "SUCCESS for $${test}-$${cmx} VCD check."; \		
		fi; \
	      done; \
	      ;; \
 	  esac; \
	done; \
	exit $$error
