srutils-common-cflags := -Wall \
                         -Wextra \
                         -Wformat=2 \
                         -Wundef \
                         -Wshadow \
                         -Wcast-align \
                         -Wmissing-declarations \
                         -D_GNU_SOURCE \
                         -I $(TOPDIR)/include \
                         -fvisibility=hidden \
                         $(EXTRA_CFLAGS)

srutils-common-ldflags := $(common-cflags) \
                          -Wl,--as-needed \
                          -Wl,-z,start-stop-visibility=hidden \
                          $(EXTRA_LDFLAGS)
