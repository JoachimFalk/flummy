# -*- tab-width:8; indent-tabs-mode:true; -*-
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (c)
#   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
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

RESDIR := $(shell cd $(srcdir) && while [ ! -d HscdTeXRes -a x`pwd` != x'/' ]; do cd ..; done; cd HscdTeXRes; pwd)

include $(auxdir)/jfrules.mk

TEX_NO_ALL_TARGET:=yes
TEX_NO_CLEAN_TARGET:=yes
include $(RESDIR)/Rules-TeX-PDF.mk

SUBDIRS =

TEXINPUTS    := figs:$(srcdir)/figs:tex:$(srcdir)/tex:$(RESDIR):$(RESDIR)/beamer_fau_logo/4-3:$(TEXINPUTS)
TEXCLEANDIRS := figs .
TEX_SOURCES  := \
	tex/VPC-Tutorial-slides.tex \
	tex/VPC-Tutorial-report.tex

export SCRTREEMODE_PKG_CONFIG_PATH

SCRTREEMODE_PKG_CONFIG_PATH := $(subst MAGICUNLIKELYPREFIX:,,$(subst :$(top_builddir)/,:$(abs_top_builddir)/,MAGICUNLIKELYPREFIX:$(PKG_CONFIG_PATH)))

src/Makefile: src/Makefile.sample
	@$(MKDIR_P) src && $(SED) \
	    -e 's|[@]prefix[@]|$(prefix)|g' \
	    -e 's|[@]exec_prefix[@]|$(exec_prefix)|g' \
	    -e 's|[@]libdir[@]|$(libdir)|g' \
	  < $^ > $@ || { rm -f $@ && false; }

all-local: src/Makefile tex-all
	@$(MAKE) -C src VPATH=$(abs_srcdir)/src all
clean-local: tex-clean
	@if test -f src/Makefile; then \
	  $(MAKE) -C src VPATH=$(abs_srcdir)/src clean; \
	  rm -f src/Makefile; \
	fi
	@rmdir src 2>/dev/null || :

# CLEANFILES=src/Makefile # cleaned by clean-local

docdir = ${datarootdir}/doc/SystemC-VPC

nobase_doc_DATA = \
	src/CMakeLists.txt \
	src/Makefile \
	src/source-sink-config-api/cpp/vpc-source-sink-config-api.cpp \
	src/source-sink/cpp/vpc-source-sink.cpp \
	src/source-sink/vpc/src-snk.vpc.xml

doc_DATA = \
	tex/VPC-Tutorial-slides.pdf \
	tex/VPC-Tutorial-report.pdf
