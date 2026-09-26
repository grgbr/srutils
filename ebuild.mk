################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Stroll.
# Copyright (C) 2026 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

config-in   := Config.in
config-h    := srutils/priv/config.h
config-obj  := config.o

subdirs     := srepo
subdirs     += srplug
srplug-deps := srepo

HEADERDIR   := $(CURDIR)/include
headers     := srutils/srepo/common.h
headers     += srutils/srepo/data.h
headers     += srutils/srepo/schema.h
headers     += srutils/srplug/common.h
headers     += srutils/srplug/data.h
headers     += $(call kconf_enabled,SRPLUG_DAEMON,srutils/srplug/daemon.h)
headers     += $(call kconf_enabled,SRPLUG_THREAD,srutils/srplug/thread.h)


override libsrepo_pkgconf_libs := Libs: -L$${libdir} \
                                        -Wl,--push-state,--as-needed \
                                        -lsrepo \
                                        -Wl,--pop-state

define libsrepo_pkgconf_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib
includedir=$${prefix}/include

Name: libsrepo
Description: srepo library
Version: $(VERSION)
Cflags: -I$${includedir}
Requires: sysrepo libyang
$(libsrepo_pkgconf_libs)
endef

pkgconfigs       := libsrepo.pc
libsrepo.pc-tmpl := libsrepo_pkgconf_tmpl

override libsrplug_pkgconf_libs := Libs: -L$${libdir} \
                                         -Wl,--push-state,--as-needed \
                                         -lsrplug \
                                         -Wl,--pop-state

define libsrplug_pkgconf_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib
includedir=$${prefix}/include

Name: libsrplug
Description: srplug library
Version: $(VERSION)
Cflags: -I$${includedir}
Requires: libsrepo
$(libsrplug_pkgconf_libs)
endef

pkgconfigs        += libsrplug.pc
libsrplug.pc-tmpl := libsrplug_pkgconf_tmpl

################################################################################
# Source code tags generation
################################################################################

tagfiles  := $(shell find $(HEADERDIR) $(addprefix $(CURDIR)/,$(subdirs)) \
                          -type f)

################################################################################
# Documentation generation
################################################################################

doxyconf  := $(CURDIR)/sphinx/Doxyfile
doxyenv   := SRCDIR="$(SRCDIR)"

#sphinxsrc := $(CURDIR)/sphinx
#sphinxenv := \
#	VERSION="$(VERSION)" \
#	$(if $(strip $(EBUILDDOC_TARGET_PATH)), \
#	     EBUILDDOC_TARGET_PATH="$(strip $(EBUILDDOC_TARGET_PATH))") \
#	$(if $(strip $(EBUILDDOC_INVENTORY_PATH)), \
#	     EBUILDDOC_INVENTORY_PATH="$(strip $(EBUILDDOC_INVENTORY_PATH))") \
#	$(if $(strip $(STROLLDOC_TARGET_PATH)), \
#	     STROLLDOC_TARGET_PATH="$(strip $(STROLLDOC_TARGET_PATH))") \
#	$(if $(strip $(STROLLDOC_INVENTORY_PATH)), \
#	     STROLLDOC_INVENTORY_PATH="$(strip $(STROLLDOC_INVENTORY_PATH))") \
#	$(if $(strip $(UTILSDOC_TARGET_PATH)), \
#	     UTILSDOC_TARGET_PATH="$(strip $(UTILSDOC_TARGET_PATH))") \
#	$(if $(strip $(UTILSDOC_INVENTORY_PATH)), \
#	     UTILSDOC_INVENTORY_PATH="$(strip $(UTILSDOC_INVENTORY_PATH))") \
#	$(if $(strip $(ELOGDOC_TARGET_PATH)), \
#	     ELOGDOC_TARGET_PATH="$(strip $(ELOGDOC_TARGET_PATH))") \
#	$(if $(strip $(ELOGDOC_INVENTORY_PATH)), \
#	     ELOGDOC_INVENTORY_PATH="$(strip $(ELOGDOC_INVENTORY_PATH))")

################################################################################
# Source distribution generation
################################################################################

# Declare the list of files under revision control to include into final source
# distribution tarball.
override distfiles = $(list_versioned_recipe)

# Override InterSphinx eBuild base documentation URI and make it point to online
# GitHub pages when building final source distribution tarball
dist: export EBUILDDOC_TARGET_PATH := http://grgbr.github.io/ebuild/
dist: export STROLLDOC_TARGET_PATH := http://grgbr.github.io/stroll/
dist: export UTILSDOC_TARGET_PATH  := http://grgbr.github.io/utils/
dist: export ELOGDOC_TARGET_PATH   := http://grgbr.github.io/elog/

# ex: filetype=make :
