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

arlibs              := libsrepo.a
libsrepo.a-objs     := static/schema.o static/data.o static/common.o
libsrepo.a-cflags   := $(common-cflags)
libsrepo.a-pkgconf  := libstroll

solibs              := libsrepo.so
libsrepo.so-objs    := shared/schema.o shared/data.o shared/common.o
libsrepo.so-cflags  := $(filter-out -fpie -fPIE,$(common-cflags)) -fpic
libsrepo.so-ldflags := $(filter-out -pie -fpie -fPIE,$(common-ldflags)) \
                       -shared -Bsymbolic -fpic -Wl,-soname,libsrepo.so
libsrepo.so-pkgconf := libstroll

# ex: filetype=make :
