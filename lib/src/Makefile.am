
lib_LTLIBRARIES = libngadmin.la

libngadmin_la_SOURCES = network.c bitrate.c firmware.c libconf.c mirror.c misc.c \
			netconf.c ports.c qos.c session.c vlan.c
libngadmin_la_CPPFLAGS = -DBUILD_LIB -I$(top_srcdir)/raw/include/ -I$(top_srcdir)/lib/include/
libngadmin_la_CFLAGS = -fno-strict-aliasing -fvisibility=hidden
libngadmin_la_LIBADD = $(top_builddir)/raw/src/librawnsdp.la

