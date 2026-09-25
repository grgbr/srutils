################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of srutils.
# Copyright (C) 2026 Grégor Boirie <gregor.boirie@free.fr>
################################################################################

include ../common.mk

common-cflags  := $(srutils-common-cflags)
common-ldflags := $(srutils-common-ldflags)

ifneq ($(filter y,$(CONFIG_SRPLUG_ASSERT)),)
common-cflags       := $(filter-out -DNDEBUG,$(common-cflags))
common-ldflags      := $(filter-out -DNDEBUG,$(common-ldflags))
endif # ($(filter y,$(CONFIG_SRPLUG_ASSERT)),)

arlibs               := libsrplug.a
libsrplug.a-objs     := static/data.o static/common.o
libsrplug.a-objs     += $(call kconf_enabled,SRPLUG_DAEMON,static/daemon.o)
libsrplug.a-objs     += $(call kconf_enabled,SRPLUG_THREAD,static/thread.o)
libsrplug.a-cflags   := $(common-cflags)
libsrplug.a-cflags   += $(call kconf_enabled,SRPLUG_THREAD,-pthread)
libsrplug.a-pkgconf  := $(call kconf_enabled,SRPLUG_DAEMON,libelog) \
                       libetux_timer_list libstroll

solibs               := libsrplug.so
libsrplug.so-objs    := shared/data.o shared/common.o
libsrplug.so-objs    += $(call kconf_enabled,SRPLUG_DAEMON,shared/daemon.o)
libsrplug.so-objs    += $(call kconf_enabled,SRPLUG_THREAD,shared/thread.o)
libsrplug.so-cflags  := $(filter-out -fpie -fPIE,$(common-cflags)) -fpic
libsrplug.so-cflags  += $(call kconf_enabled,SRPLUG_THREAD,-pthread)
libsrplug.so-ldflags := $(filter-out -pie -fpie -fPIE,$(common-ldflags)) \
                        -shared -Bsymbolic -fpic -Wl,-soname,libsrplug.so
libsrplug.so-pkgconf := $(call kconf_enabled,SRPLUG_DAEMON,libelog) \
                        libetux_timer_list libstroll

# ex: filetype=make :
