
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libngadmin.pc

lib_LTLIBRARIES = libngadmin.la
noinst_HEADERS = lib.h network.h

libngadmin_la_SOURCES = network.c bitrate.c firmware.c libconf.c mirror.c misc.c \
			netconf.c ports.c qos.c session.c vlan.c
libngadmin_la_CPPFLAGS = -I$(top_srcdir)/raw/include/ -I$(top_srcdir)/lib/include/
libngadmin_la_CFLAGS = -fno-strict-aliasing
libngadmin_la_LDFLAGS = -export-symbols-regex '^ngadmin_' -version-info 0:0:0
libngadmin_la_LIBADD = $(top_builddir)/raw/src/librawnsdp.la

