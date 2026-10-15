srutils-common-cflags := -Wall \
                         -Wextra \
                         -Wformat=2 \
                         -Wundef \
                         -Wshadow \
                         -Wmissing-declarations \
                         -D_GNU_SOURCE \
                         -I $(TOPDIR)/include \
                         -fvisibility=hidden \
                         $(EXTRA_CFLAGS)

# This generates spurious warnings with GCC 8.x. Keep this disabled for now...
#srutils-common-cflags += -Wcast-align

srutils-common-ldflags := $(common-cflags) \
                          -Wl,--as-needed \
                          -Wl,-z,start-stop-visibility=hidden \
                          $(EXTRA_LDFLAGS)
