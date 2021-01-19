# $(1) is test name
define TEST_template =
$(1)-output/novpc/sim.log: $(1)
	{ mkdir -p '$$(dir $$@)' && \
	  OUTPUTPREFIX='$$(dir $$@)' \
	  $$(srcdir)/scripts/runtest.py \
	    --log '$$@' \
	    --error '$(1)-output/novpc/sim.err' \
	    --clean '$$(dir $$@)' \
	    --filter '^((sqrroot|top|pgTop)\.[^:]*|src|sink|A|B|Src|Snk):' \
	    -- ./'$$<' 100; }
endef

TEST_LOGS := $(foreach TEST,$(noinst_PROGRAMS),$(TEST)-output/novpc/sim.log)

$(foreach TEST_LOG,$(TEST_LOGS),\
  $(eval $(call TEST_template,$(patsubst %-output/novpc/sim.log,%,$(TEST_LOG)))))

# $(1) is test name
# $(2) is VPC config name
define TEST_VPC_template =
$(1)_$(2)_SYSTEMOC_SMX :=
$(1)_$(2)_SYSTEMOC_IMPORT_SMX :=
ifneq (,$(wildcard $(srcdir)/src/$(1)/vpc/$(2).smx))
$(1)_$(2)_SYSTEMOC_SMX := src/$(1)/vpc/$(2).smx
$(1)_$(2)_SYSTEMOC_IMPORT_SMX := --systemoc-import-smx $(srcdir)/src/$(1)/vpc/$(2).smx
endif
$(1)-output/vpc/$(2)/sim.log: $(1) src/$(1)/vpc/$(2).vpc.xml $$($(1)_$(2)_SYSTEMOC_SMX) $$(SYSTEMOC_PLUGINVPC)
	{ set -- $$^; mkdir -p '$$(dir $$@)' && \
	  VPCTRACEFILEPREFIX='$$(dir $$@)' \
	  VPCCONFIGURATION="$$$$2" \
	  $$(srcdir)/scripts/runtest.py \
	    --log '$$@' \
	    --error '$(1)-output/vpc/$(2)/sim.err' \
	    --clean '$$(dir $$@)' \
	    --filter '^((sqrroot|top|pgTop)\.[^:]*|src|sink|A|B|Src|Snk):' \
	    -- ./'$$<' $$($(1)_$(2)_SYSTEMOC_IMPORT_SMX) 100; }
endef

ifeq (no,$(SYSTEMOC_ENABLE_SGX))
TEST_VPCS_FILTEROUT := $(foreach TEST,$(noinst_PROGRAMS),\
    $(patsubst $(srcdir)/src/$(TEST)/vpc/%.smx,$(TEST)-output/vpc/%/sim.log,$(wildcard $(srcdir)/src/$(TEST)/vpc/*.smx)))
else
TEST_VPCS_FILTEROUT :=
endif

TEST_VPCS := $(filter-out $(TEST_VPCS_FILTEROUT),\
  $(foreach TEST,$(noinst_PROGRAMS),\
    $(patsubst $(srcdir)/src/$(TEST)/vpc/%.vpc.xml,$(TEST)-output/vpc/%/sim.log,$(wildcard $(srcdir)/src/$(TEST)/vpc/*.vpc.xml))))

$(foreach TEST_VPC,$(TEST_VPCS),\
  $(eval $(call TEST_VPC_template,$(word 1,$(subst -output/vpc/, ,$(patsubst %/sim.log,%,$(TEST_VPC)))),$(word 2,$(subst -output/vpc/, ,$(patsubst %/sim.log,%,$(TEST_VPC)))))))



