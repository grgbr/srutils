################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of srutils.
# Copyright (C) 2026 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

include ../common.mk

common-cflags      := $(srutils-common-cflags)
common-ldflags     := $(srutils-common-ldflags)

ifneq ($(filter y,$(CONFIG_SREPO_ASSERT)),)
common-cflags      := $(filter-out -DNDEBUG,$(common-cflags))
common-ldflags     := $(filter-out -DNDEBUG,$(common-ldflags))
endif # ($(filter y,$(CONFIG_SREPO_ASSERT)),)

libsrepo-objs       := schema.o
libsrepo-objs       += data.o
libsrepo-objs       += xpath.o
libsrepo-objs       += $(call kconf_enabled,SREPO_LOG,log.o)
libsrepo-objs       += common.o

arlibs              := libsrepo.a
libsrepo.a-objs     := $(addprefix static/,$(libsrepo-objs))
libsrepo.a-cflags   := $(common-cflags)
libsrepo.a-pkgconf  := sysrepo libyang libstroll

solibs              := libsrepo.so
libsrepo.so-objs    := $(addprefix shared/,$(libsrepo-objs))
libsrepo.so-cflags  := $(filter-out -fpie -fPIE,$(common-cflags)) -fpic
libsrepo.so-ldflags := $(filter-out -pie -fpie -fPIE,$(common-ldflags)) \
                       -shared -Bsymbolic -fpic -Wl,-soname,libsrepo.so
libsrepo.so-pkgconf := sysrepo libyang libstroll

# ex: filetype=make :
