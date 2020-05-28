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
	test-src-snk-multicast \
	test-src-snk-multirate \
	test-src-snk-plain

test_sqr_LDADD					= $(SYSTEMOC_LIBS)
test_sqr_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_sqr_SOURCES				= \
	src/test-sqr/cpp/design-sqr.cpp

test_src_snk_multicast_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multicast_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multicast_SOURCES			= \
	src/test-src-snk-multicast/cpp/design-src-snk-multicast.cpp

test_src_snk_multirate_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multirate_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multirate_SOURCES			= \
	src/test-src-snk-multirate/cpp/design-src-snk-multirate.cpp

test_src_snk_plain_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_plain_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_plain_SOURCES			= \
	src/test-src-snk-plain/cpp/design-src-snk-plain.cpp

%.smx: %
	./'$<' --systemoc-export-smx='$@' || \
	  { rm -f "$@"; exit 1; }

# $(1) is test name
define TEST_template =
$(1)-output/novpc/sim.log: $(1)
	{ mkdir -p '$$(dir $$@)' && \
	  OUTPUTPREFIX='$$(dir $$@)' \
	  $$(srcdir)/scripts/runtest.py \
	    --log '$$@' \
	    --error '$(1)-output/novpc/sim.err' \
	    --clean '$$(dir $$@)' \
	    --filter '^((sqrroot|top)\.[^:]*|src|sink|A|B|Src|Snk):' \
	    -- ./'$$<' 100; }
endef

TEST_LOGS	= $(foreach TEST,$(noinst_PROGRAMS),$(TEST)-output/novpc/sim.log)

$(foreach TEST_LOG,$(TEST_LOGS),\
  $(eval $(call TEST_template,$(patsubst %-output/novpc/sim.log,%,$(TEST_LOG)))))

# $(1) is test name
# $(2) is VPC config name
define TEST_VPC_template =
$(1)-output/vpc/$(2)/sim.log: $(1) src/$(1)/vpc/$(2).vpc.xml $$(SYSTEMOC_PLUGINVPC)
	{ set -- $$^; mkdir -p '$$(dir $$@)' && \
	  VPCTRACEFILEPREFIX='$$(dir $$@)' \
	  VPCCONFIGURATION="$$$$2" \
	  $$(srcdir)/scripts/runtest.py \
	    --log '$$@' \
	    --error '$(1)-output/vpc/$(2)/sim.err' \
	    --clean '$$(dir $$@)' \
	    --filter '^((sqrroot|top)\.[^:]*|src|sink|A|B|Src|Snk):' \
	    -- ./'$$<' 100; }
endef

TEST_VPCS 	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/vpc/%.vpc.xml,$(TEST)-output/vpc/%/sim.log,$(wildcard $(srcdir)/src/$(TEST)/vpc/*.vpc.xml)))

$(foreach TEST_VPC,$(TEST_VPCS),\
  $(eval $(call TEST_VPC_template,$(word 1,$(subst -output/vpc/, ,$(patsubst %/sim.log,%,$(TEST_VPC)))),$(word 2,$(subst -output/vpc/, ,$(patsubst %/sim.log,%,$(TEST_VPC)))))))


clean-local:
	for test in $(noinst_PROGRAMS); do \
	  rm -rf $${test}-output; \
	done

.PHONY: test-logs check-logs
check-am: check-logs
test-logs: $(TEST_LOGS)
check-logs: $(TEST_LOGS)
	@error=0; \
	for test in $(noinst_PROGRAMS); do \
	  if test -f $(srcdir)/src/$${test}/golden.log; then \
	    if ! diff -u $${test}-output/novpc/sim.log $(srcdir)/src/$${test}/golden.log > /dev/null; then \
	      echo "FAILURE for $${test} LOG check."; error=1; \
	      diff -u $${test}-output/novpc/sim.log $(srcdir)/src/$${test}/golden.log | head -10; \
	    else \
	      echo "SUCCESS for $${test} LOG check."; \
	    fi; \
	  else \
	    echo "MISSING for $${test} LOG check."; \
	  fi; \
	done; \
	exit $$error

.PHONY: test-vpcs check-vpcs

diff_vcd_ignore_date=-I '[[:blank:]]*[[:alpha:]]\{3\}[[:blank:]][[:digit:]]\{2\},[[:blank:]][[:digit:]]\{4\}[[:blank:]]*[[:digit:]]\{1,2\}:[[:digit:]]\{2\}:[[:digit:]]\{2\}$$'
diff_vcd_ignore_systemc_version=-I 'SystemC[[:blank:]][[:digit:].]\+\(-Accellera\)\?[[:blank:]]---[[:blank:]]'
diff_vcd_ignore=$(diff_vcd_ignore_date) $(diff_vcd_ignore_systemc_version)

check-am: check-vpcs
test-vpcs: $(TEST_VPCS)
check-vpcs: $(TEST_VPCS)
	@error=0; \
	for simlog in $^; do \
	  test=$${simlog%%-output/vpc/*}; \
	  cmx=$${simlog##*-output/vpc/}; cmx=$${cmx%%/sim.log}; \
	  status=undef; \
	  for golden in $(srcdir)/src/$${test}/vpc/$${cmx}/golden/*; do \
	    output=$${simlog%%/sim.log}/$${golden##*/}; \
	    case "$${golden}" in \
	      */\*|*~|*.bak|.*.sw*) \
	        ;; \
	      *.vcd) \
		if ! diff -u -N $(diff_vcd_ignore) $${output} $${golden} > /dev/null; then \
		  status=failed; \
		else \
		  status=success; \
		fi; \
	      	;; \
	      *) \
		if ! diff -u -N $${output} $${golden} > /dev/null; then \
		  status=failed; \
		else \
		  status=success; \
		fi; \
		;; \
	    esac; \
	  done; \
	  if test x"$$status" = x"failed"; then \
	    echo "FAILURE for $${test} VCD $${cmx} check."; error=1; \
	    for golden in $(srcdir)/src/$${test}/vpc/$${cmx}/golden/*; do \
	      output=$${simlog%%/sim.log}/$${golden##*/}; \
	      case "$${golden}" in \
		*/\*|*~|*.bak|.*.sw*) \
		  ;; \
		*.vcd) \
		  diff -u -N $(diff_vcd_ignore) $${output} $${golden} | head -10; \
		  ;; \
		*) \
		  diff -u -N $${output} $${golden} | head -10; \
		  ;; \
	      esac; \
	    done; \
	  elif test x"$$status" = x"success"; then \
	    echo "SUCCESS for $${test} VCD $${cmx} check."; \
	  else \
	    echo "MISSING for $${test} VCD $${cmx} check."; \
	  fi; \
	done; \
	exit $$error

